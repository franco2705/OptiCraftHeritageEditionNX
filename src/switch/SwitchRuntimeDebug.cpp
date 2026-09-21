#include "switch/SwitchRuntimeDebug.h"

#include "platform/Diagnostics.h"

#include <cstdio>

namespace
{
const char *g_stage = "boot";
std::uint64_t g_frame = 0;
std::uint64_t g_ticks = 0;
std::uint64_t g_lastWorldRenderMicros = 0;
bool g_hasWorld = false;
bool g_hasPlayer = false;
std::uint64_t g_terrainListsRequested = 0;
std::uint64_t g_displayListsFound = 0;
std::uint64_t g_displayListsMissing = 0;
std::uint64_t g_drawCalls = 0;
std::uint64_t g_vertices = 0;
}

void switchDebugFrameBegin(bool hasWorld, bool hasPlayer)
{
    ++g_frame;
    g_hasWorld = hasWorld;
    g_hasPlayer = hasPlayer;
    g_terrainListsRequested = 0;
    g_displayListsFound = 0;
    g_displayListsMissing = 0;
    g_drawCalls = 0;
    g_vertices = 0;
    g_stage = "frame-begin";
}

void switchDebugTerrainListsRequested(int count)
{
    if (count > 0)
        g_terrainListsRequested += static_cast<std::uint64_t>(count);
}

void switchDebugDisplayListResult(bool found, bool drawn, int vertices)
{
    if (!found)
    {
        ++g_displayListsMissing;
        return;
    }
    ++g_displayListsFound;
    if (drawn)
    {
        ++g_drawCalls;
        if (vertices > 0)
            g_vertices += static_cast<std::uint64_t>(vertices);
    }
}

void switchDebugCheckpoint(const char *stage)
{
    g_stage = stage ? stage : "(null)";
}

void switchDebugWorldRenderComplete(std::uint64_t elapsedMicros)
{
    g_lastWorldRenderMicros = elapsedMicros;
    g_stage = "world-done";
}

void switchDebugTickComplete()
{
    ++g_ticks;
    g_stage = "tick-done";
}

std::string switchDebugLine(int line)
{
    char text[160]{};
    char heapText[32]{};
    const long heapKb = platformHeapFreeKb();
    if (heapKb < 0)
        std::snprintf(heapText, sizeof(heapText), "n/a");
    else
        std::snprintf(heapText, sizeof(heapText), "%ld KB", heapKb);
    switch (line)
    {
        case 0:
            std::snprintf(text, sizeof(text), "SWDBG f=%llu t=%llu stage=%s",
                static_cast<unsigned long long>(g_frame),
                static_cast<unsigned long long>(g_ticks), g_stage);
            break;
        case 1:
            std::snprintf(text, sizeof(text), "world=%d player=%d render=%llu us heap=%s",
                g_hasWorld ? 1 : 0, g_hasPlayer ? 1 : 0,
                static_cast<unsigned long long>(g_lastWorldRenderMicros), heapText);
            break;
        case 2:
            std::snprintf(text, sizeof(text), "terrain=%llu lists=%llu missing=%llu draws=%llu verts=%llu",
                static_cast<unsigned long long>(g_terrainListsRequested),
                static_cast<unsigned long long>(g_displayListsFound),
                static_cast<unsigned long long>(g_displayListsMissing),
                static_cast<unsigned long long>(g_drawCalls),
                static_cast<unsigned long long>(g_vertices));
            break;
        default:
            return {};
    }
    return text;
}
