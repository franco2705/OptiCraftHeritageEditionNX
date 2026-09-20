#include "switch/render/SwitchLegacyRenderer.h"

#include <glad/glad.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace SwitchLegacyRenderer
{
namespace
{
using Matrix = std::array<float, 16>;

Matrix identity()
{
    return {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

Matrix multiply(const Matrix &a, const Matrix &b)
{
    Matrix result{};
    for (int column = 0; column < 4; ++column)
        for (int row = 0; row < 4; ++row)
            for (int k = 0; k < 4; ++k)
                result[column * 4 + row] += a[k * 4 + row] * b[column * 4 + k];
    return result;
}

std::vector<Matrix> g_modelView{identity()};
std::vector<Matrix> g_projection{identity()};
std::vector<Matrix> g_texture{identity()};
std::vector<Matrix> *g_current = &g_modelView;
std::array<float, 4> g_color{1, 1, 1, 1};
bool g_alphaTest = false;
float g_alphaReference = 0.1f;
RenderCompare g_alphaFunction = RenderCompare::Greater;
GLuint g_program = 0;
GLuint g_vao = 0;
GLuint g_vbo = 0;

constexpr const char *kVertexShader = R"glsl(
#version 330 core
layout(location=0) in vec3 position;
layout(location=1) in vec2 texCoord;
layout(location=2) in vec4 color;
uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 textureMatrix;
uniform vec4 constantColor;
uniform bool hasColor;
out vec2 fragmentTexCoord;
out vec4 fragmentColor;
void main() {
    gl_Position = projection * modelView * vec4(position, 1.0);
    fragmentTexCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
    fragmentColor = hasColor ? color : constantColor;
}
)glsl";

constexpr const char *kFragmentShader = R"glsl(
#version 330 core
in vec2 fragmentTexCoord;
in vec4 fragmentColor;
uniform sampler2D texture0;
uniform bool hasTexture;
uniform bool alphaTest;
uniform float alphaReference;
uniform int alphaFunction;
out vec4 outputColor;
void main() {
    outputColor = fragmentColor * (hasTexture ? texture(texture0, fragmentTexCoord) : vec4(1.0));
    bool pass = alphaFunction == 0 ? false :
                alphaFunction == 1 ? outputColor.a <  alphaReference :
                alphaFunction == 2 ? outputColor.a == alphaReference :
                alphaFunction == 3 ? outputColor.a <= alphaReference :
                alphaFunction == 4 ? outputColor.a >  alphaReference :
                alphaFunction == 5 ? outputColor.a != alphaReference :
                alphaFunction == 6 ? outputColor.a >= alphaReference : true;
    if (alphaTest && !pass) discard;
}
)glsl";

GLuint compileShader(GLenum type, const char *source)
{
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_TRUE) return shader;
    char message[1024]{};
    glGetShaderInfoLog(shader, sizeof(message), nullptr, message);
    std::fprintf(stderr, "Switch GL shader: %s\n", message);
    glDeleteShader(shader);
    return 0;
}

bool initialize()
{
    if (g_program != 0) return true;
    const GLuint vertex = compileShader(GL_VERTEX_SHADER, kVertexShader);
    const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (vertex == 0 || fragment == 0)
    {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return false;
    }
    g_program = glCreateProgram();
    glAttachShader(g_program, vertex);
    glAttachShader(g_program, fragment);
    glLinkProgram(g_program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint ok = GL_FALSE;
    glGetProgramiv(g_program, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE)
    {
        char message[1024]{};
        glGetProgramInfoLog(g_program, sizeof(message), nullptr, message);
        std::fprintf(stderr, "Switch GL program: %s\n", message);
        reset();
        return false;
    }
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    return true;
}

GLenum primitive(RenderPrimitive value)
{
    switch (value)
    {
        case RenderPrimitive::Points: return GL_POINTS;
        case RenderPrimitive::Lines: return GL_LINES;
        case RenderPrimitive::LineLoop: return GL_LINE_LOOP;
        case RenderPrimitive::LineStrip: return GL_LINE_STRIP;
        case RenderPrimitive::Triangles: return GL_TRIANGLES;
        case RenderPrimitive::TriangleStrip: return GL_TRIANGLE_STRIP;
        case RenderPrimitive::TriangleFan: return GL_TRIANGLE_FAN;
        default: return 0;
    }
}

void apply(const Matrix &matrix)
{
    g_current->back() = multiply(g_current->back(), matrix);
}
}

bool draw(const RenderInterleavedMesh &mesh)
{
    const GLenum mode = primitive(mesh.primitive);
    if (!mesh.data || mesh.stride <= 0 || mesh.count <= 0 || mode == 0 || !initialize()) return false;
    const auto *bytes = static_cast<const unsigned char *>(mesh.data) +
                        static_cast<std::size_t>(mesh.first) * mesh.stride;
    glUseProgram(g_program);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelView"), 1, GL_FALSE, g_modelView.back().data());
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projection"), 1, GL_FALSE, g_projection.back().data());
    glUniformMatrix4fv(glGetUniformLocation(g_program, "textureMatrix"), 1, GL_FALSE, g_texture.back().data());
    glUniform4fv(glGetUniformLocation(g_program, "constantColor"), 1, g_color.data());
    glUniform1i(glGetUniformLocation(g_program, "hasColor"), mesh.hasColor);
    glUniform1i(glGetUniformLocation(g_program, "hasTexture"), mesh.hasTexture);
    glUniform1i(glGetUniformLocation(g_program, "alphaTest"), g_alphaTest);
    glUniform1f(glGetUniformLocation(g_program, "alphaReference"), g_alphaReference);
    glUniform1i(glGetUniformLocation(g_program, "alphaFunction"), static_cast<int>(g_alphaFunction));
    glUniform1i(glGetUniformLocation(g_program, "texture0"), 0);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.count) * mesh.stride, bytes, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, mesh.positionShort ? GL_SHORT : GL_FLOAT, GL_FALSE, mesh.stride, nullptr);
    if (mesh.hasTexture)
    {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, mesh.stride,
                              reinterpret_cast<const void *>(static_cast<uintptr_t>(mesh.texCoordOffset)));
    }
    else glDisableVertexAttribArray(1);
    if (mesh.hasColor)
    {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, mesh.stride,
                              reinterpret_cast<const void *>(static_cast<uintptr_t>(mesh.colorOffset)));
    }
    else glDisableVertexAttribArray(2);
    glDrawArrays(mode, 0, mesh.count);
    glBindVertexArray(0);
    return glGetError() == GL_NO_ERROR;
}

void color(float r, float g, float b, float a) { g_color = {r, g, b, a}; }
void alphaTest(bool enabled, RenderCompare function, float reference) { g_alphaTest = enabled; g_alphaFunction = function; g_alphaReference = reference; }
void matrixMode(RenderMatrixMode mode) { g_current = mode == RenderMatrixMode::Projection ? &g_projection : mode == RenderMatrixMode::Texture ? &g_texture : &g_modelView; }
void loadIdentity() { g_current->back() = identity(); }
void pushMatrix() { g_current->push_back(g_current->back()); }
void popMatrix() { if (g_current->size() > 1) g_current->pop_back(); }

void translate(float x, float y, float z)
{
    Matrix matrix = identity(); matrix[12] = x; matrix[13] = y; matrix[14] = z; apply(matrix);
}
void scale(float x, float y, float z)
{
    Matrix matrix = identity(); matrix[0] = x; matrix[5] = y; matrix[10] = z; apply(matrix);
}
void rotate(float degrees, float x, float y, float z)
{
    const float length = std::sqrt(x*x + y*y + z*z); if (length == 0) return;
    x /= length; y /= length; z /= length;
    const float radians = degrees * 0.01745329251994329577f;
    const float c = std::cos(radians), s = std::sin(radians), t = 1-c;
    Matrix m = {t*x*x+c,t*x*y+s*z,t*x*z-s*y,0, t*x*y-s*z,t*y*y+c,t*y*z+s*x,0,
                t*x*z+s*y,t*y*z-s*x,t*z*z+c,0, 0,0,0,1}; apply(m);
}
void frustum(double l,double r,double b,double t,double n,double f)
{
    Matrix m{}; m[0]=2*n/(r-l);m[5]=2*n/(t-b);m[8]=(r+l)/(r-l);m[9]=(t+b)/(t-b);
    m[10]=-(f+n)/(f-n);m[11]=-1;m[14]=-(2*f*n)/(f-n);apply(m);
}
void ortho(double l,double r,double b,double t,double n,double f)
{
    Matrix m=identity();m[0]=2/(r-l);m[5]=2/(t-b);m[10]=-2/(f-n);
    m[12]=-(r+l)/(r-l);m[13]=-(t+b)/(t-b);m[14]=-(f+n)/(f-n);apply(m);
}
void getMatrix(RenderMatrixQuery query, float *values)
{
    const Matrix &m=query==RenderMatrixQuery::Projection?g_projection.back():query==RenderMatrixQuery::Texture?g_texture.back():g_modelView.back();
    std::memcpy(values,m.data(),sizeof(Matrix));
}
void reset()
{
    if (g_vbo) glDeleteBuffers(1,&g_vbo); if (g_vao) glDeleteVertexArrays(1,&g_vao); if (g_program) glDeleteProgram(g_program);
    g_vbo=0;g_vao=0;g_program=0;
}
}
