#include "switch/SwitchRuntimeDebug.h"

#include "platform/Diagnostics.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>

namespace
{
std::atomic<const char *> g_stage{"boot"};
std::atomic<std::uint64_t> g_frame{0};
std::atomic<std::uint64_t> g_ticks{0};
std::once_flag g_watchdogOnce;
std::uint64_t g_lastWorldRenderMicros = 0;
bool g_hasWorld = false;
bool g_hasPlayer = false;
std::uint64_t g_terrainListsRequested = 0;
std::uint64_t g_displayListsFound = 0;
std::uint64_t g_displayListsMissing = 0;
std::uint64_t g_drawCalls = 0;
std::uint64_t g_vertices = 0;

void reportStall(std::uint64_t frame, std::uint64_t ticks, const char *stage)
{
    char line[192]{};
    std::snprintf(line, sizeof(line),
        "[SWDBG][STALL] frame=%llu ticks=%llu stage=%s (no new frame for 2 seconds)\n",
        static_cast<unsigned long long>(frame),
        static_cast<unsigned long long>(ticks), stage ? stage : "(null)");
    std::fputs(line, stderr);
    std::fflush(stderr);

    // This file survives a forced close or fatal error and can be inspected
    // directly from the SD card when nxlink was not attached.
    if (FILE *file = std::fopen("sdmc:/switch/OptiCraft/.minecraft/switch-watchdog.log", "a"))
    {
        std::fputs(line, file);
        std::fclose(file);
    }
}

void startWatchdog()
{
    std::call_once(g_watchdogOnce, []
    {
        std::thread([]
        {
            std::uint64_t previousFrame = g_frame.load(std::memory_order_relaxed);
            int unchangedSamples = 0;
            bool reported = false;
            for (;;)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                const std::uint64_t frame = g_frame.load(std::memory_order_relaxed);
                if (frame != previousFrame)
                {
                    previousFrame = frame;
                    unchangedSamples = 0;
                    reported = false;
                    continue;
                }
                if (++unchangedSamples >= 4 && !reported)
                {
                    reportStall(frame, g_ticks.load(std::memory_order_relaxed),
                        g_stage.load(std::memory_order_relaxed));
                    reported = true;
                }
            }
        }).detach();
    });
}
}

void switchDebugFrameBegin(bool hasWorld, bool hasPlayer)
{
    startWatchdog();
    g_frame.fetch_add(1, std::memory_order_relaxed);
    g_hasWorld = hasWorld;
    g_hasPlayer = hasPlayer;
    g_terrainListsRequested = 0;
    g_displayListsFound = 0;
    g_displayListsMissing = 0;
    g_drawCalls = 0;
    g_vertices = 0;
    g_stage.store("frame-begin", std::memory_order_relaxed);
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
    g_stage.store(stage ? stage : "(null)", std::memory_order_relaxed);
}

void switchDebugWorldRenderComplete(std::uint64_t elapsedMicros)
{
    g_lastWorldRenderMicros = elapsedMicros;
    g_stage.store("world-done", std::memory_order_relaxed);
}

void switchDebugTickComplete()
{
    g_ticks.fetch_add(1, std::memory_order_relaxed);
    g_stage.store("tick-done", std::memory_order_relaxed);
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
                static_cast<unsigned long long>(g_frame.load(std::memory_order_relaxed)),
                static_cast<unsigned long long>(g_ticks.load(std::memory_order_relaxed)),
                g_stage.load(std::memory_order_relaxed));
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
