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

// Switch must not inherit the desktop startup policy. Desktop synchronously
// preloads a 17x17 area and may rebuild essentially the entire dirty renderer
// queue in one frame. Wii and PS2 both use bounded streaming, which is why they
// keep presenting and polling input while a new world fills in.
# undef PLATFORM_PRELOAD_RADIUS_BLOCKS
# define PLATFORM_PRELOAD_RADIUS_BLOCKS 32
# undef PLATFORM_CHUNK_CACHE_RADIUS
# define PLATFORM_CHUNK_CACHE_RADIUS 6
# undef PLATFORM_CHUNK_UNLOAD_RADIUS
# define PLATFORM_CHUNK_UNLOAD_RADIUS 10
# undef PLATFORM_MAX_CHUNK_UNLOADS_PER_TICK
# define PLATFORM_MAX_CHUNK_UNLOADS_PER_TICK 4
# undef PLATFORM_MESH_BUDGET
# define PLATFORM_MESH_BUDGET 1
# undef PLATFORM_MIN_RENDERER_UPDATES_PER_FRAME
# define PLATFORM_MIN_RENDERER_UPDATES_PER_FRAME 1
# undef PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME
# define PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME 2
# undef PLATFORM_RENDERER_UPDATE_CANDIDATES_PER_FRAME
# define PLATFORM_RENDERER_UPDATE_CANDIDATES_PER_FRAME 8
# undef PLATFORM_CHUNK_BUILD_BUDGET_MS
# define PLATFORM_CHUNK_BUILD_BUDGET_MS 4

// Missing non-adjacent columns are generated as resumable jobs instead of in
// the render/tick call that first asks for them. Keep the player's immediate
// column synchronous so collision never observes the blank fallback.
# undef PLATFORM_GENERATE_SYNC_RADIUS
# define PLATFORM_GENERATE_SYNC_RADIUS 1
# undef PLATFORM_GENERATE_CHUNKS_PER_TICK
# define PLATFORM_GENERATE_CHUNKS_PER_TICK 1
# undef PLATFORM_INCREMENTAL_CHUNK_GENERATION
# define PLATFORM_INCREMENTAL_CHUNK_GENERATION 1
# undef PLATFORM_GENERATION_STEPS_PER_TICK
# define PLATFORM_GENERATION_STEPS_PER_TICK 8
# undef PLATFORM_GENERATION_BUDGET_US
# define PLATFORM_GENERATION_BUDGET_US 2500
# undef PLATFORM_GENERATION_STEPS_PER_FRAME
# define PLATFORM_GENERATION_STEPS_PER_FRAME 8
# undef PLATFORM_GENERATION_FRAME_BUDGET_US
# define PLATFORM_GENERATION_FRAME_BUDGET_US 2500

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
