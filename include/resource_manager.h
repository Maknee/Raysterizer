#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	class ResourceManager
	{
	public:
		explicit ResourceManager();
		explicit ResourceManager(Context* c);
		~ResourceManager();

		Error EnqueueDestroy(Buffer buffer);
		Error EnqueueDestroy(Image image);
		Error EnqueueDestroy(ImageView image_view);
		Error EnqueueDestroy(Texture texture);
		Error EnqueueDestroy(Sampler sampler);
		Error EnqueueDestroy(DescriptorPool descriptor_pool);
		Error EnqueueDestroy(DescriptorSetLayout descriptor_set_layout);
		Error EnqueueDestroy(DescriptorSet descriptor_set);
		Error EnqueueDestroy(ShaderModule shader_module);
		Error EnqueueDestroy(RenderPass render_pass);
		Error EnqueueDestroy(PipelineLayoutInfo pipeline_layout_info);
		Error EnqueueDestroy(PipelineLayout pipeline_layout);
		Error EnqueueDestroy(GraphicsPipeline graphics_pipeline);
		Error EnqueueDestroy(ComputePipeline compute_pipeline);
		Error EnqueueDestroy(RaytracingPipeline raytracing_pipeline);
		Error EnqueueDestroy(Semaphore semaphore);
		Error EnqueueDestroy(Fence fence);
		Error EnqueueDestroy(CommandPool command_pool);
		Error EnqueueDestroy(CommandBuffer command_buffer);
		Error EnqueueDestroy(BottomLevelAccelerationStructure bottom_level_acceleration_structure);
		Error EnqueueDestroy(TopLevelAccelerationStructure top_level_acceleration_structure);
		Error EnqueueDestroy(QueryPool query_pool);
		Error EnqueueDestroy(FrameBuffer frame_buffer);

		Error Flush();
		Error FlushEntirely();

	private:
		Context* c{};

		std::vector<CacheFrameCounterEntryWithCompleted<Buffer>> buffers;
		std::vector<CacheFrameCounterEntryWithCompleted<Image>> images;
		std::vector<CacheFrameCounterEntryWithCompleted<ImageView>> image_views;
		std::vector<CacheFrameCounterEntryWithCompleted<Texture>> textures;
		std::vector<CacheFrameCounterEntryWithCompleted<Sampler>> samplers;
		std::vector<CacheFrameCounterEntryWithCompleted<DescriptorPool>> descriptor_pools;
		std::vector<CacheFrameCounterEntryWithCompleted<DescriptorSetLayout>> descriptor_set_layouts;
		std::vector<CacheFrameCounterEntryWithCompleted<DescriptorSet>> descriptor_sets;
		std::vector<CacheFrameCounterEntryWithCompleted<ShaderModule>> shader_modules;
		std::vector<CacheFrameCounterEntryWithCompleted<RenderPass>> render_passes;
		std::vector<CacheFrameCounterEntryWithCompleted<PipelineLayoutInfo>> pipeline_layout_infos;
		std::vector<CacheFrameCounterEntryWithCompleted<PipelineLayout>> pipeline_layouts;
		std::vector<CacheFrameCounterEntryWithCompleted<GraphicsPipeline>> graphics_pipelines;
		std::vector<CacheFrameCounterEntryWithCompleted<ComputePipeline>> compute_pipelines;
		std::vector<CacheFrameCounterEntryWithCompleted<RaytracingPipeline>> raytracing_pipelines;
		std::vector<CacheFrameCounterEntryWithCompleted<Semaphore>> semaphores;
		std::vector<CacheFrameCounterEntryWithCompleted<Fence>> fences;
		std::vector<CacheFrameCounterEntryWithCompleted<CommandPool>> command_pools;
		std::vector<CacheFrameCounterEntryWithCompleted<CommandBuffer>> command_buffers;
		std::vector<CacheFrameCounterEntryWithCompleted<BottomLevelAccelerationStructure>> bottom_level_acceleration_structures;
		std::vector<CacheFrameCounterEntryWithCompleted<TopLevelAccelerationStructure>> top_level_acceleration_structures;
		std::vector<CacheFrameCounterEntryWithCompleted<QueryPool>> query_pools;
		std::vector<CacheFrameCounterEntryWithCompleted<FrameBuffer>> frame_buffers;
	};
}