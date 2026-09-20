#pragma once
// Conservative first-pass limits: leave headroom for Horizon services and the
// native framebuffer while the GPU command backend is brought online.
#if PLATFORM_SWITCH
# undef PLATFORM_DEFAULT_RENDER_DISTANCE
# define PLATFORM_DEFAULT_RENDER_DISTANCE 2
# undef PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME
# define PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME 2
# undef PLATFORM_MESH_STAGING_SLOTS
# define PLATFORM_MESH_STAGING_SLOTS 2
#endif
