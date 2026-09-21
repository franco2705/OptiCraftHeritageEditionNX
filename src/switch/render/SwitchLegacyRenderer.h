#pragma once

#include "platform/RenderAPI.h"

namespace SwitchLegacyRenderer
{
bool draw(const RenderInterleavedMesh &mesh);
void color(float red, float green, float blue, float alpha);
void alphaTest(bool enabled, RenderCompare function = RenderCompare::Greater, float reference = 0.1f);
void matrixMode(RenderMatrixMode mode);
void loadIdentity();
void pushMatrix();
void popMatrix();
void translate(float x, float y, float z);
void rotate(float angleDegrees, float x, float y, float z);
void scale(float x, float y, float z);
void frustum(double left, double right, double bottom, double top, double nearValue, double farValue);
void ortho(double left, double right, double bottom, double top, double nearValue, double farValue);
void getMatrix(RenderMatrixQuery query, float *values);
void reset();
}
