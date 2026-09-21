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
}

void switchDebugFrameBegin(bool hasWorld, bool hasPlayer)
{
    ++g_frame;
    g_hasWorld = hasWorld;
    g_hasPlayer = hasPlayer;
    g_stage = "frame-begin";
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
    switch (line)
    {
        case 0:
            std::snprintf(text, sizeof(text), "SWDBG f=%llu t=%llu stage=%s",
                static_cast<unsigned long long>(g_frame),
                static_cast<unsigned long long>(g_ticks), g_stage);
            break;
        case 1:
            std::snprintf(text, sizeof(text), "world=%d player=%d render=%llu us heap=%ld KB",
                g_hasWorld ? 1 : 0, g_hasPlayer ? 1 : 0,
                static_cast<unsigned long long>(g_lastWorldRenderMicros), platformHeapFreeKb());
            break;
        default:
            return {};
    }
    return text;
}
