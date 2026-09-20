# OptiCraft Heritage

OptiCraft Heritage is a heavily modified, clean-room C++ implementation of classic Minecraft-era gameplay designed around portability, low-end hardware, and console-specific optimization.

This repository is not intended to be a line-for-line source translation. The runtime, platform layers, rendering paths, input backends, storage systems, user interface, asset loading, memory policies, and console support have been extensively reworked for the needs of this project.

## Project goals

- Keep the implementation portable across desktop PC, PlayStation 2, Nintendo Wii, and native Nintendo Switch Homebrew.
- Preserve the intended classic gameplay and visual behavior where practical while allowing platform-specific adaptations.
- Run on constrained hardware through aggressive memory, rendering, chunk, and asset-loading optimizations.
- Keep platform code isolated behind explicit backends instead of scattering host-specific logic through the game code.
- Maintain a debuggable and production-oriented C++17 codebase.

## Clean-room implementation

OptiCraft Heritage is developed as a clean-room implementation. The project code is independently implemented in C/C++ and is heavily modified around its own runtime and platform architecture.

The project does not rely on original proprietary game source code as part of its implementation. Compatibility-oriented behavior may be reproduced from observable behavior, documented formats, protocol behavior, and independently developed interfaces.

This project is not affiliated with, endorsed by, or sponsored by Mojang Studios or Microsoft.

## Supported targets

### PC

The desktop build uses SDL2, OpenGL, and the shared platform abstraction layer. A dedicated 32-bit legacy profile is available for older SSE2-class CPUs and legacy OpenGL hardware.

### PlayStation 2

The PS2 build uses a native platform backend with PS2SDK support, GS-specific rendering, console-aware memory policies, asynchronous asset loading, platform storage, controller input, and optional VU-assisted terrain paths.

The expected USB application directory is:

```text
mass:/OptiCraftHeritage/
```

### Nintendo Wii

The Wii build uses devkitPPC/libogc and a native GX rendering path. The Homebrew Channel layout remains:

```text
apps/OptiCraft/
```

### Nintendo Switch Homebrew

The Switch port has two deliberately separate paths: a small `devkitA64`/libnx
hardware diagnostic and the full game. The game creates a native EGL/OpenGL
context through the Switch Mesa/Nouveau portlibs and uses Switch-specific graphics,
Joy-Con/Pro Controller input, SD storage, and applet lifecycle backends; it does
not reuse either legacy console renderer. Both paths produce:

```text
bin/switch/OptiCraft.nro
```

The full-game renderer translates the shared legacy matrix, texture, alpha-test,
interleaved-mesh, and retained chunk-list operations into a shader-backed OpenGL
3.3 pipeline running inside that native context.

## Source layout

```text
src/
  client/       Client-side shared code
  java/         Java compatibility/runtime helpers
  net/          Game implementation
  platform/     Shared platform interfaces and backend selection
  pc/           Desktop-specific implementation
  ps2/          PlayStation 2 implementation
  switch/       Nintendo Switch Homebrew implementation
  wii/          Nintendo Wii implementation
  util/         Shared utility code

cmake/          Toolchains, source selection, and platform build logic
external/       Third-party dependencies
```

Platform targets deliberately select one implementation for each public backend. This keeps PC, PS2, and Wii implementations from accidentally entering the same link target.

## Building

CMake 3.21 or newer is required. Presets are defined in `CMakePresets.json`.

### Desktop

```text
cmake --preset gcc-debug
cmake --build --preset gcc-debug
```

For a normal optimized build:

```text
cmake --preset gcc-release
cmake --build --preset gcc-release
```

### 32-bit / legacy PC

The CMake presets do not hardcode an MSYS2 installation path. On Windows, use:

```text
build_gcc32.bat legacy
```

The batch file owns the local MSYS2 installation path instead of exposing it through CMake. To use another installation without editing the project:

```bat
set OPTICRAFT_MSYS2_ROOT=D:\Tools\msys64
build_gcc32.bat legacy
```

The accepted modes are `debug`, `release`, and `legacy`.

### PlayStation 2

```text
cmake --preset ps2-release
cmake --build --preset ps2-release
```

Use `ps2-debug` for a debug build. Asset staging remains a separate step so large runtime data is not recopied after every link.

### Nintendo Wii

```text
cmake --preset wii-release
cmake --build --preset wii-release
```

Use `wii-debug` for a debug build and `wii-bringup` for the minimal hardware/toolchain bring-up target.

### Nintendo Switch Homebrew

Install the devkitPro `switch-dev` package group. Two presets intentionally keep
hardware diagnosis separate from the game:

```text
# Minimal libnx/controller/framebuffer diagnostic
cmake --preset switch-bringup
cmake --build --preset switch-bringup

# Full-game development target (Release)
cmake --preset switch-release
cmake --build --preset switch-release
cmake --build build/switch-release --target switch-data

# Full-game development target (Debug)
cmake --preset switch-debug
cmake --build --preset switch-debug
cmake --build build/switch-debug --target switch-data
```

Alternatively, the repository helper performs configure, build, and data
staging in the correct order, including in a fresh checkout with no CMake
cache:

```text
./build_switch.sh debug --data-root /path/to/runtime-data
```

Use `release` or `bringup` instead of `debug` as needed. Pass `--no-data` when
only the NRO should be built.

Each build directory is created by its matching configure command. If CMake
reports `could not load cache`, run `cmake --preset switch-debug` (or
`switch-release`) before the corresponding build command.

Both modes run `nacptool` and `elf2nro` and produce
`bin/switch/OptiCraft.nro`. Copy it to
`sdmc:/switch/OptiCraft/OptiCraft.nro`. The playable target reads runtime data
from `sdmc:/switch/OptiCraft/data`. The `switch-data` target stages matching
host-side `assets/` and `resources/` trees from `data/`. If those runtime files
live elsewhere, configure with `-DSWITCH_DATA_ROOT=/path/to/runtime-data`.
Saves and options are stored under
`sdmc:/switch/OptiCraft/.minecraft`.

The initial controller mapping uses the left stick for movement and the right
stick for camera/cursor motion. `A` jumps or confirms, `B` cancels, `X` opens
the inventory, `Y` drops the selected item, `ZR` attacks, `ZL` uses an item,
`+` opens the pause menu, and the D-pad supplies menu navigation. The Home
button and applet lifecycle remain managed by libnx.

`switch-bringup` remains the recommended first boot on new hardware; it does not
include the game and is only a diagnostic. Use a Homebrew-enabled Switch only;
this project does not provide instructions for modifying a console.

## Development notes

OptiCraft Heritage contains substantial platform-specific changes compared with the behavior it reproduces. Examples include custom render backends, legacy UI work, low-memory chunk policies, console input layers, asset streaming, platform storage, audio backends, profiling, and console-specific performance tuning.

When changing shared systems, keep the platform abstraction boundary intact and avoid introducing PC-only assumptions into common code. Likewise, console-specific optimizations should remain behind platform policies or dedicated backends whenever possible.

## Third-party software

Third-party libraries are kept under `external/` and retain their respective licenses and notices. Review those licenses independently before redistributing binaries.
