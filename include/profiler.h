#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	class Context;
	class CommandBuffer;

	class GPUSample
	{
	public:
		GPUSample(vk::CommandBuffer command_buffer_, std::string_view name);
		~GPUSample();

	private:
		vk::CommandBuffer command_buffer;
	};

	class Profiler
	{
	public:
		explicit Profiler();
		explicit Profiler(Context* c_);
		void Setup();
#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
		void SetupNvidiaNSightAfterMath();
#endif
#ifdef ENABLE_PROFILER
		~Profiler();
#endif
		void Update(vk::CommandBuffer cb, std::string_view name);
		void EndFrame();

		void SetFrame(std::size_t frame_counter) { frame = frame_counter; }

		tracy::VkCtx* Get(CommandBuffer* command_buffer);

		GPUSample CreateGPUSample(vk::CommandBuffer command_buffer, std::string_view name) { return GPUSample(command_buffer, name); }

#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
		std::shared_ptr<GpuCrashTracker> GetGpuCrashTracker() { return gpu_crash_tracker; }
		GpuCrashTracker::MarkerMap& GetGpuMarkerMap() { return marker_map; }
#endif
	private:
		Context* c{};
#ifdef ENABLE_PROFILER
		tracy::VkCtx* present_context{};
		tracy::VkCtx* graphics_context{};
		tracy::VkCtx* compute_context{};
		tracy::VkCtx* transfer_context{};
#endif
#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
		std::shared_ptr<GpuCrashTracker> gpu_crash_tracker;
		GpuCrashTracker::MarkerMap marker_map;
#endif
		std::size_t frame;
	};

#ifdef ENABLE_PROFILER
#define ScopedGPUProfile(profiler, cmdbuf, name) if(profiler) { TracyVkZone(profiler->Get(&cmdbuf), *cmdbuf, name); } RaysterizerEngine::GPUSample UNIQUIFY(CAT(gpu_sample_, sample))(*cmdbuf, name); if(profiler) { profiler->Update(*cmdbuf, name); }
#define CollectGPUProfile(profiler, cmdbuf) if(profiler) { TracyVkCollect(profiler->Get(&cmdbuf), *cmdbuf); }
#define ScopedCPUProfile(name) ZoneScopedN(name)
#define ScopedCPUProfileDepth(name, depth) ZoneScopedNS(name, depth)
#define ScopedCPUProfileColor(name, color) ZoneScopedNC(name, color)
#define ScopedCPUProfileColorDepth(name, color, depth) ZoneScopedNCS(name, color, depth)
#else
#define ScopedGPUProfile(profiler, cmdbuf, name) 
#define CollectGPUProfile(profiler, cmdbuf)
#define ScopedCPUProfile(name)
#define ScopedCPUProfileDepth(name, depth)
#define ScopedCPUProfileColor(name, color)
#define ScopedCPUProfileColorDepth(name, color, depth)
#endif

}