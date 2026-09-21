#pragma once

#include <cstdint>
#include <string>

// Lightweight, always-on diagnostics for early Switch hardware testing. All
// calls are made by the main thread; the API deliberately avoids allocations
// at checkpoint sites so it can also identify allocation-related stalls.
void switchDebugFrameBegin(bool hasWorld, bool hasPlayer);
void switchDebugCheckpoint(const char *stage);
void switchDebugWorldRenderComplete(std::uint64_t elapsedMicros);
void switchDebugTickComplete();
void switchDebugTerrainListsRequested(int count);
void switchDebugDisplayListResult(bool found, bool drawn, int vertices);
std::string switchDebugLine(int line);
