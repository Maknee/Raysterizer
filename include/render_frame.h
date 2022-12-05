#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	// Does not deallocate any memory (always keeps old ones and reuse)
	template<typename T>
	class GrowingPool
	{
	public:
		void SetCreateFunction(std::function<CMShared<T>()> create_function_)
		{
			if (!create_function)
			{
				create_function = create_function_;
			}
		}

		CMShared<T> Get()
		{
			if (!create_function)
			{
				PANIC("Create function not made...");
			}

			if (unused.empty())
			{
				auto item = create_function();
				used.emplace(item);

				return item;
			}
			else
			{
				auto iter = std::begin(unused);
				auto item = *iter;

				used.emplace(item);
				unused.erase(iter);
				return item;
			}
		}

		void Remove(CMShared<T> item)
		{
			if (item)
			{
				used.erase(item);
				unused.emplace(item);
			}
		}
		
		void Clear()
		{
			unused.insert(std::begin(used), std::end(used));
			used.clear();
		}

		auto& GetUsed() { return used; }
		const auto& GetUsed() const { return used; }

		auto& GetUnused() { return unused; }
		const auto& GetUnused() const { return unused; }

	private:
		std::function<CMShared<T>()> create_function;
		phmap::flat_hash_set<CMShared<T>> used;
		phmap::flat_hash_set<CMShared<T>> unused;
	};

	struct TransferJob
	{
		FrameCounter frame_counter{};
		CMShared<CommandBuffer> command_buffer{};

		CMShared<Semaphore> semaphore{};
		CMShared<Fence> fence{};

		CMShared<Buffer> upload_buffer{};

		// Buffer related
		CMShared<Buffer> gpu_buffer{};

		// Texture related
		CMShared<Texture> gpu_texture{};
		CMShared<CommandBuffer> graphics_sync_command_buffer{};
		CMShared<Semaphore> graphics_sync_semaphore{};
		CMShared<Fence> graphics_sync_fence{};

		// Acceleration structure related
		CMShared<BottomLevelAccelerationStructure> blas{};
		CMShared<TopLevelAccelerationStructure> tlas{};

		CMShared<CommandBuffer> blas_compact_command_buffer{};
		CMShared<Semaphore> blas_compact_semaphore{};
		CMShared<Fence> blas_compact_fence{};
	};

	class RenderFrame
	{
	public:
		explicit RenderFrame() {}
		explicit RenderFrame(Context* c_);

		vk::Device GetDevice() const;

		Expected<DescriptorSetLayouts> Get(const DescriptorSetLayoutCreateInfos& descriptor_set_layout_create_info);
		Expected<CMShared<PipelineLayoutInfo>> Get(const PipelineLayoutCreateInfo& pipeline_layout_create_info);
		Expected<CMShared<GraphicsPipeline>> Get(const GraphicsPipelineCreateInfo& graphics_pipeline_create_info);
		Expected<CMShared<RenderPass>> Get(const RenderPassCreateInfo& render_pass_create_info);
		Expected<CMShared<ShaderModule>> Get(const ShaderModuleCreateInfo& shader_module_create_info);
		Expected<CMShared<DescriptorPool>> Get(const DescriptorPoolCreateInfo& descriptor_pool_create_info);

		Expected<CMShared<DescriptorSet>> Get(const DescriptorSetCreateInfo& descriptor_set_create_info);
		Expected<CMShared<CommandPool>> Get(const CommandPoolCreateInfo& command_pool_create_info);
		Expected<CMShared<CommandBuffer>> Get(const CommandBufferCreateInfo& command_buffer_create_info);

		Expected<CMShared<Texture>> Get(const TextureCreateInfo& texture_create_info);
		Expected<CMShared<Buffer>> Get(const BufferCreateInfo& buffer_create_info);

		Expected<CMShared<Fence>> GetFence();
		Error WaitForFences(uint64_t timeout);
		Error ResetFences();
		Expected<CMShared<Semaphore>> GetSemaphore();
		Error ResetSemaphores();
		Expected<CMShared<Semaphore>> GetBinarySemaphore();

		Expected<CMShared<Texture>> CreateTexture(CommandBuffer command_buffer, vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t layer_count = 1);
		Error UploadData(CMShared<Image> image, PointerView data, uint32_t array_index, uint32_t mip_level, vk::ImageLayout src_image_layout = vk::ImageLayout::eUndefined, vk::ImageLayout dst_image_layout = vk::ImageLayout::eShaderReadOnlyOptimal);
		Error GenerateMipMaps(CommandBuffer& command_buffer, CMShared<Image> image, vk::ImageLayout src_image_layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout dst_image_layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eColor, vk::Filter filter = vk::Filter::eLinear);
		Error GenerateMipMaps(CMShared<CommandBuffer> command_buffer, CMShared<Image> image, vk::ImageLayout src_image_layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout dst_image_layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eColor, vk::Filter filter = vk::Filter::eLinear);
		Error GenerateMipMaps(CMShared<Image> image, vk::ImageLayout src_image_layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout dst_image_layout = vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eColor, vk::Filter filter = vk::Filter::eLinear);
		Expected<CMShared<Buffer>> CreateBuffer(MemoryUsage memory_usage, vk::BufferUsageFlags buffer_usage_flags, std::size_t size, bool map = false);
		Expected<CMShared<Buffer>> ResizeBuffer(CMShared<Buffer> buffer, std::size_t size);
		Expected<CMShared<Buffer>> CopyBuffer(CMShared<Buffer> buffer);
		Expected<CMShared<Buffer>> CreateStagingBuffer(std::size_t size);

		Expected<TransferJob> UploadDataToGPUBuffer(CMShared<Buffer> upload_buffer, vk::BufferUsageFlags buffer_usage_flags);
		Expected<TransferJob> UploadDataToGPUBuffer(PointerView data, vk::BufferUsageFlags buffer_usage_flags);
		Expected<TransferJob> UploadDataToGPUTexture(vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t depth = 1);
		Expected<TransferJob> UploadDataToGPUTexture2DArray(vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t depth = 1);
		Expected<TransferJob> UploadDataToGPUTexture3D(vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t depth = 1);
		Expected<TransferJob> UploadDataToGPUTextureCubeMap(vk::Format format, PointerView data, uint32_t width, uint32_t height);

		Expected<CMShared<CommandBuffer>> GetCommandBuffer(QueueType queue_type, vk::CommandPoolCreateFlags command_pool_create_flags = Constants::DEFAULT_COMMAND_POOL_CREATE_FLAGS);
		Expected<CMShared<CommandBuffer>> GetCommandBufferFromResetPool(QueueType queue_type);
		Error ResetCommandBufferPools();

		Expected<CMShared<CommandBuffer>> GetSecondaryCommandBuffer(QueueType queue_type, vk::CommandPoolCreateFlags command_pool_create_flags = Constants::DEFAULT_COMMAND_POOL_CREATE_FLAGS);


		Error AddDependencyBetween(CMShared<Fence> fence, CMShared<CommandBuffer> command_buffer);

		Error AddBeginningOfFrameUpdate(std::function<void()> callback);
		Error PerformBeginningOfFrameUpdates();
		Error AddEndOfFrameUpdate(std::function<void()> callback);
		Error PerformEndOfFrameUpdates();
		Error EndFrame();
		Error ReleaseUnusedResources();
		Error FlushCommandBuffers();

		Error FlushPendingWrites(CMShared<DescriptorSet> descriptor_sets);
		Expected<CMShared<DescriptorSet>> Copy(CMShared<DescriptorSet> descriptor_sets);

		Error BindPushConstant(CMShared<CommandBuffer> command_buffer, CMShared<PipelineLayout> pipeline_layout, PointerView data);
		Error BindPushConstant(CommandBuffer& command_buffer, CMShared<PipelineLayout> pipeline_layout, PointerView data);

		const auto& GetCommandBufferCache() const { return command_buffer_cache; }
		auto& GetCommandBufferCache() { return command_buffer_cache; }

	private:
		Context* c{};
		
		CacheMappingWithFrameCounter<PipelineLayoutCreateInfo, GrowingPool<PipelineLayoutInfo>> pipeline_layout_info_cache;
		CacheMappingWithFrameCounter<DescriptorSetCreateInfo, GrowingPool<DescriptorSet>> descriptor_set_cache;
		CacheMappingWithFrameCounter<CommandPoolCreateInfo, CMShared<CommandPool>> command_pool_cache;
		//std::array<CMShared<CommandPool>, QueueType::End - 1> command_pools;
		CacheMappingWithFrameCounter<CommandBufferCreateInfo, GrowingPool<CommandBuffer>> command_buffer_cache;
		CacheMappingWithFrameCounter<CommandBufferCreateInfo, GrowingPool<CommandBuffer>> secondary_command_buffer_cache;

		flat_hash_map<CMShared<CommandPool>, std::vector<CMShared<CommandBuffer>>> reset_command_pool_to_buffers;

		GrowingPool<Fence> fence_pool;
		GrowingPool<Semaphore> semaphore_pool;
		GrowingPool<Semaphore> binary_semaphore_pool;

		std::vector<std::function<void()>> beginning_of_frame_updates;
		std::vector<std::function<void()>> end_of_frame_updates;
	};
}