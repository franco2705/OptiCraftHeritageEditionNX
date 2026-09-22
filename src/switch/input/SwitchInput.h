#pragma once

// Poll libnx once per frame and publish both the platform snapshots and the
// LWJGL-compatible events consumed by the shared menus/gameplay code.
void switchInputPoll();

