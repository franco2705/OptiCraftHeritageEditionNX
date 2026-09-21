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

// The generic desktop profile drains as many as 500 flood-fill lighting jobs
// in one rendered frame with no wall-clock limit. Entering a newly generated
// world can enqueue thousands of those jobs, making the Switch appear
// permanently frozen before it can present or poll input again.
# undef PLATFORM_LIGHTING_UPDATES_PER_FRAME
# define PLATFORM_LIGHTING_UPDATES_PER_FRAME 4
# undef PLATFORM_LIGHTING_BUDGET_US
# define PLATFORM_LIGHTING_BUDGET_US 2000
# undef PLATFORM_LIGHTING_INTERACTIVE_QUEUE_MAX
# define PLATFORM_LIGHTING_INTERACTIVE_QUEUE_MAX 8
# undef PLATFORM_LIGHTING_INTERACTIVE_BURST
# define PLATFORM_LIGHTING_INTERACTIVE_BURST 8
# undef PLATFORM_STREAMING_FRAME_BUDGET_US
# define PLATFORM_STREAMING_FRAME_BUDGET_US 4000
#endif
