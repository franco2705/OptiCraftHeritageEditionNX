# Nintendo Switch (devkitA64/libnx) build. SWITCH_BRINGUP keeps the small
# diagnostic executable; OFF builds the game with Switch-native backends.
cmake_minimum_required(VERSION 3.21)
include(${CMAKE_SOURCE_DIR}/cmake/SourceSelection.cmake)
option(SWITCH_BRINGUP "Build only the native Switch hardware diagnostic" OFF)
option(SWITCH_ENABLE_SOUND "Enable Switch audio" OFF)
set(MC_LOG_LEVEL "0" CACHE STRING "Unified diagnostic verbosity")
set(SWITCH_DATA_ROOT "${CMAKE_SOURCE_DIR}/data" CACHE PATH
    "Directory containing the assets/ and resources/ runtime data trees")

if(SWITCH_BRINGUP)
    set(SWITCH_SOURCES "${CMAKE_SOURCE_DIR}/src/switch/tools/SwitchBringup.cpp")
    message(STATUS "Switch build: BRINGUP diagnostics")
else()
    mcbeta_collect_platform_sources(SWITCH_SOURCES switch)
    set(SWITCH_MINIZIP_SOURCES
        "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip/ioapi.c"
        "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip/unzip.c")
    list(APPEND SWITCH_SOURCES ${SWITCH_MINIZIP_SOURCES})
    set_source_files_properties(${SWITCH_MINIZIP_SOURCES} PROPERTIES COMPILE_DEFINITIONS USE_FILE32API)
    mcbeta_exclude_sources(SWITCH_SOURCES "[/\\]switch[/\\]tools[/\\]SwitchBringup\\.cpp$")
    mcbeta_exclude_remote_stats_sources(SWITCH_SOURCES)
    mcbeta_select_platform_backends(SWITCH_SOURCES SWITCH SWITCH SWITCH)
    message(STATUS "Switch build: FULL game")
endif()

add_executable(OptiCraft ${SWITCH_SOURCES})
set_target_properties(OptiCraft PROPERTIES SUFFIX ".elf" CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/switch")
target_compile_definitions(OptiCraft PRIVATE SWITCH_PLATFORM=1 NO_NETWORK
    MC_LOG_LEVEL=${MC_LOG_LEVEL} $<$<NOT:$<BOOL:${SWITCH_ENABLE_SOUND}>>:NO_SOUND>)
target_compile_options(OptiCraft PRIVATE -march=armv8-a+crc -mtp=soft -fPIE
    -ffunction-sections -fdata-sections -fno-math-errno -fno-trapping-math)
target_include_directories(OptiCraft PRIVATE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/src/pc"
    "${CMAKE_SOURCE_DIR}/external/stb" "${LIBNX}/include")
target_link_directories(OptiCraft PRIVATE "${LIBNX}/lib")

# The diagnostic deliberately depends only on libnx. zlib is a Switch portlib
# used by the game's region/minizip code; requiring it for bring-up prevents the
# hardware test from linking on an otherwise complete switch-dev install.
set(_SWITCH_LIBS nx m)
if(NOT SWITCH_BRINGUP)
    set(SWITCH_PORTLIBS "${DEVKITPRO}/portlibs/switch")
    find_library(SWITCH_ZLIB NAMES z HINTS "${SWITCH_PORTLIBS}/lib"
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    if(NOT SWITCH_ZLIB)
        message(FATAL_ERROR
            "Switch full game requires switch-zlib. Install it with: dkp-pacman -S switch-zlib")
    endif()
    target_include_directories(OptiCraft PRIVATE "${SWITCH_PORTLIBS}/include")
    target_link_directories(OptiCraft PRIVATE "${SWITCH_PORTLIBS}/lib")
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/src/switch/render/SwitchGraphicsContext.cpp"
        PROPERTIES COMPILE_DEFINITIONS __SWITCH__)
    list(PREPEND _SWITCH_LIBS glad EGL glapi drm_nouveau)
    list(APPEND _SWITCH_LIBS z)
endif()
target_link_libraries(OptiCraft PRIVATE ${_SWITCH_LIBS})
target_link_options(OptiCraft PRIVATE "-specs=${LIBNX}/switch.specs" -march=armv8-a+crc -mtp=soft -fPIE
    "-Wl,-Map,${CMAKE_BINARY_DIR}/OptiCraft.map" -Wl,--gc-sections)

find_program(SWITCH_NACPTOOL NAMES nacptool HINTS "${DEVKITPRO}/tools/bin" REQUIRED)
find_program(SWITCH_ELF2NRO NAMES elf2nro HINTS "${DEVKITPRO}/tools/bin" REQUIRED)
set(SWITCH_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/switch")
set(SWITCH_NACP "${CMAKE_CURRENT_BINARY_DIR}/OptiCraft.nacp")
add_custom_command(OUTPUT "${SWITCH_NACP}" COMMAND "${SWITCH_NACPTOOL}" --create
    "OptiCraft Heritage Edition" "OptiCraft contributors" "1.0.0" "${SWITCH_NACP}" VERBATIM)
add_custom_target(switch-nacp DEPENDS "${SWITCH_NACP}")
add_dependencies(OptiCraft switch-nacp)
add_custom_command(TARGET OptiCraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SWITCH_OUTPUT_DIR}"
    COMMAND "${SWITCH_ELF2NRO}" "$<TARGET_FILE:OptiCraft>" "${SWITCH_OUTPUT_DIR}/OptiCraft.nro" "--nacp=${SWITCH_NACP}"
    COMMENT "elf2nro: ${SWITCH_OUTPUT_DIR}/OptiCraft.nro" VERBATIM)
add_custom_target(switch-data
    COMMAND ${CMAKE_COMMAND}
            "-DSOURCE_ROOT=${SWITCH_DATA_ROOT}"
            "-DOUTPUT_ROOT=${SWITCH_OUTPUT_DIR}/data"
            -P "${CMAKE_SOURCE_DIR}/cmake/StageSwitchData.cmake"
    COMMENT "Staging Switch runtime data" VERBATIM)
