#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	class SwapchainOutOfDateError : public llvm::ErrorInfo<SwapchainOutOfDateError>
	{
	public:
		SwapchainOutOfDateError() {}

		void log(std::ostream& OS) const override {
			OS << "PrepareFrame Error";
		}

		std::error_code convertToErrorCode() const override {
			return llvm::inconvertibleErrorCode();
		}

	public:
		inline static const char ID = 0;
	};

	class Context
	{
	public:
		explicit Context();
		~Context();
		Error Setup(std::shared_ptr<VulkanWindow> window_);

		vk::Instance GetInstance() const;
		vk::PhysicalDevice GetPhysicalDevice() const;
		vk::Device GetDevice() const;
		uint32_t GetGraphicsQueueFamily() const;
		uint32_t GetComputeQueueFamily() const;
		uint32_t GetTransferQueueFamily() const;
		uint32_t GetPresentQueueFamily() const;
		bool IsUnifiedGraphicsAndTransferQueue() const;
		vk::Queue GetGraphicsQueue() const;
		vk::Queue GetComputeQueue() const;
		vk::Queue GetTransferQueue() const;
		vk::Queue GetPresentQueue() const;
		vk::Queue GetQueue(QueueType queue_type) const;
		vk::PresentModeKHR GetPresentMode() const;
		vk::SwapchainKHR GetSwapchain() const;
		const std::vector<vk::Image>& GetSwapchainImages() const;
		const std::vector<vk::ImageView>& GetSwapchainImageViews() const;
		uint32_t GetNumFrames() const;
		vk::Format GetSwapchainFormat() const;
		vk::Extent2D GetWindowExtent() const;
		vk::Extent3D GetWindowExtent3D() const;
		vk::PhysicalDeviceFeatures GetPhysicalDeviceFeatures() const;
		vk::PhysicalDeviceProperties GetPhysicalDeviceProperties() const;

		template<typename T, typename... Args>
		CMShared<T> MakeContextManaged(Args&&... args);
		Expected<CMShared<Image>> CreateImage(const ImageCreateInfo& image_create_info);
		Expected<CMShared<ImageView>> CreateImageView(const ImageViewCreateInfo& image_view_create_info);
		Expected<CMShared<Texture>> CreateTexture(TextureCreateInfo& texture_create_info);
		Expected<CMShared<Sampler>> CreateSampler(const SamplerCreateInfo& sampler_create_info);
		Expected<CMShared<Buffer>> CreateBuffer(const BufferCreateInfo& buffer_create_info);

		Expected<CMShared<DescriptorPool>> CreateDescriptorPool(const DescriptorPoolCreateInfo& descriptor_pool_create_info);
		Expected<CMShared<DescriptorPool>> CreateDescriptorPool(const vk::DescriptorPoolCreateInfo& descriptor_pool_create_info);
		Expected<CMShared<DescriptorSetLayout>> CreateDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& descriptor_set_layout_create_info);
		Expected<CMShared<DescriptorSet>> CreateDescriptorSet(const DescriptorSetCreateInfo& descriptor_set_create_info);

		Expected<CMShared<ShaderModule>> CreateShaderModule(const ShaderModuleCreateInfo& shader_module_create_info);
		Expected<CMShared<RenderPass>> CreateRenderPass(const RenderPassCreateInfo& render_pass_create_info);
		Expected<CMShared<PipelineLayoutInfo>> CreatePipelineLayoutInfo(const PipelineLayoutCreateInfo& pipeline_layout_create_info);
		Expected<CMShared<PipelineLayout>> CreatePipelineLayout(CMShared<PipelineLayoutInfo> pipeline_layout_info);
		Expected<CMShared<GraphicsPipeline>> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& graphics_pipeline_create_info);
		Expected<CMShared<ComputePipeline>> CreateComputePipeline(const ComputePipelineCreateInfo& compute_pipeline_create_info);
		Expected<CMShared<RaytracingPipeline>> CreateRaytracingPipeline(const RaytracingPipelineCreateInfo& raytracing_pipeline_create_info);

		Expected<uint32_t> GetQueueFamilyIndex(QueueType queue_type);
		Expected<CMShared<CommandPool>> CreateCommandPool(const CommandPoolCreateInfo& command_pool_create_info);
		Expected<CMShared<CommandBuffer>> CreateCommandBuffer(const CommandBufferCreateInfo& command_buffer_create_info);
		Expected<CMShared<Fence>> CreateFence(const FenceCreateInfo& fence_create_info);
		Expected<CMShared<Semaphore>> CreateSemaphore(const SemaphoreCreateInfo& semaphore_create_info);
		Expected<CMShared<QueryPool>> CreateQueryPool(const QueryPoolCreateInfo& query_pool_create_info);

		Expected<AccelerationStructureWithBuffer> CreateAccelerationStructureWithBuffer(vk::AccelerationStructureTypeKHR acceleration_structure_type, vk::DeviceSize size);
		Expected<TransferJob> CreateBottomLevelAccelerationStructure(const BottomLevelAccelerationStructureCreateInfo& blas_create_info);
		Expected<TransferJob> UpdateBottomLevelAccelerationStructure(const BottomLevelAccelerationStructureUpdateInfo& blas_update_info);
		Expected<TransferJob> CompactBottomLevelAccelerationStructure(const TransferJob& blas_transfer_job);
		Error CleanupCompactBottomLevelAccelerationStructure(const TransferJob& compact_blas_transfer_job);
		Expected<TransferJob> CreateTopLevelAccelerationStructure(const TopLevelAccelerationStructureCreateInfo& tlas_create_info);

		Expected<CMShared<BottomLevelAccelerationStructure>> CreateBottomLevelAccelerationStructure(const BottomLevelAccelerationStructureCreateInfo& blas_create_info, CMShared<CommandBuffer> command_buffer);
		Expected<CMShared<TopLevelAccelerationStructure>> CreateTopLevelAccelerationStructure(const TopLevelAccelerationStructureCreateInfo& tlas_create_info, CMShared<CommandBuffer> command_buffer);

		Expected<CMShared<FrameBuffer>> CreateFrameBuffer(const FrameBufferCreateInfo& frame_buffer_create_info);

		RenderFrame& GetRenderFrameAt(std::size_t index);
		RenderFrame& GetRenderFrame();

		Error EnqueueDestroy(Buffer image);
		Error EnqueueDestroy(Image image);
		Error EnqueueDestroy(ImageView image);
		Error EnqueueDestroy(Texture texture);
		Error EnqueueDestroy(Sampler image);
		Error EnqueueDestroy(DescriptorPool image);
		Error EnqueueDestroy(DescriptorSetLayout image);
		Error EnqueueDestroy(DescriptorSet image);
		Error EnqueueDestroy(ShaderModule render_pass);
		Error EnqueueDestroy(RenderPass render_pass);
		Error EnqueueDestroy(PipelineLayout pipeline_layout);
		Error EnqueueDestroy(GraphicsPipeline pipeline_layout);
		Error EnqueueDestroy(ComputePipeline pipeline_layout);
		Error EnqueueDestroy(RaytracingPipeline pipeline_layout);
		Error EnqueueDestroy(Semaphore pipeline_layout);
		Error EnqueueDestroy(Fence pipeline_layout);
		Error EnqueueDestroy(CommandPool pipeline_layout);
		Error EnqueueDestroy(CommandBuffer pipeline_layout);
		Error EnqueueDestroy(BottomLevelAccelerationStructure blas);
		Error EnqueueDestroy(TopLevelAccelerationStructure tlas);
		Error EnqueueDestroy(QueryPool query_pool);
		Error EnqueueDestroy(FrameBuffer frame_buffer);

		Expected<DescriptorSetLayouts> Get(const DescriptorSetLayoutCreateInfos& descriptor_set_layout_create_info);
		Expected<CMShared<PipelineLayoutInfo>> Get(const PipelineLayoutCreateInfo& pipeline_layout_create_info);
		Expected<CMShared<PipelineLayout>> Get(CMShared<PipelineLayoutInfo> pipeline_layout_info);
		Expected<CMShared<GraphicsPipeline>> Get(const GraphicsPipelineCreateInfo& graphics_pipeline_create_info);
		Expected<CMShared<ComputePipeline>> Get(const ComputePipelineCreateInfo& compute_pipeline_create_info);
		Expected<CMShared<RaytracingPipeline>> Get(const RaytracingPipelineCreateInfo& raytracing_pipeline_create_info);
		Expected<CMShared<RenderPass>> Get(const RenderPassCreateInfo& render_pass_create_info);
		Expected<CMShared<ShaderModule>> Get(const ShaderModuleCreateInfo& shader_module_create_info);
		Expected<CMShared<DescriptorPool>> Get(const DescriptorPoolCreateInfo& descriptor_pool_create_info);
		Expected<CMShared<Sampler>> Get(const SamplerCreateInfo& sampler_create_info);
		Expected<CMShared<QueryPool>> Get(const QueryPoolCreateInfo& query_pool_create_info);

		// Not sure to keep
		Error Render();
		ShaderModuleManager& GetShaderModuleManager();
		ResourceManager& GetResourceManager();
		std::shared_ptr<Profiler> GetProfiler();
		
		Error ReleaseUnusedResources();
		Error EndFrame();
		void AdvanceFrame();

		Error Submit(QueueType queue_type, CMShared<CommandBuffer> command_buffer, CMShared<Fence> fence = nullptr, std::optional<vk::PipelineStageFlags> wait_dst_stage_mask = std::nullopt, CMShared<Semaphore> wait_semaphore = nullptr, CMShared<Semaphore> signal_semaphore = nullptr);
		Error Submit(QueueType queue_type, std::initializer_list<CMShared<CommandBuffer>> command_buffers, CMShared<Fence> fence = nullptr, std::optional<vk::PipelineStageFlags> wait_dst_stage_mask = std::nullopt, CMShared<Semaphore> wait_semaphore = nullptr, CMShared<Semaphore> signal_semaphore = nullptr);
		Error Submit(QueueType queue_type, std::vector<CMShared<CommandBuffer>> command_buffers, CMShared<Fence> fence = nullptr, std::vector<vk::PipelineStageFlags> wait_dst_stage_masks = {}, std::vector<CMShared<Semaphore>> wait_semaphores = {}, std::vector<CMShared<Semaphore>> signal_semaphores = {});
		Error Submit(QueueType queue_type, vk::SubmitInfo submit_info, CMShared<Fence> fence = nullptr);
		Error PrepareFrame(CMShared<Semaphore> acquire_image_semaphore);
		uint32_t GetNextFrameIndex() const;
		Error Present(CMShared<Semaphore> wait_semaphore, std::optional<uint32_t> image_index = std::nullopt);
		Error ResetFence(CMShared<Fence> fence);
		Error WaitForFence(CMShared<Fence> fence);
		Error ImmediateGraphicsSubmit(std::function<void(CommandBuffer& command_buffer)> f);
		Error ImmediateGraphicsSubmitPtr(std::function<void(CMShared<CommandBuffer> command_buffer)> f);

		CMShared<RenderPass> GetGlobalRenderPass() { return global_render_pass; }
		vk::Framebuffer GetFrameBuffer(std::size_t index) { return *(frame_buffers[index]); }
		vk::Framebuffer GetFrameCurrentBuffer() { return *(frame_buffers[next_image_index]); }

		template<typename T>
		void SetObjectName(const T& o, std::string_view s, vk::DebugReportObjectTypeEXT t) const
		{
			const static bool enable_profiler = Config["profiler"]["enable"];
			if (enable_profiler && debug_marker_enabled)
			{
				auto debug_marker_object_name_info = vk::DebugMarkerObjectNameInfoEXT{}
					.setObjectType(t)
					.setObject(reinterpret_cast<const uint64_t&>(o))
					.setPObjectName(s.data());

				device.debugMarkerSetObjectNameEXT(debug_marker_object_name_info);
			}
		}

		void SetName(CMShared<Buffer> buffer, std::string_view s) const;
		void SetName(CMShared<Image> image, std::string_view s) const;
		void SetName(CMShared<ImageView> image_view, std::string_view s) const;
		void SetName(CMShared<Texture> texture, std::string_view s) const;
		void SetName(CMShared<Sampler> sampler, std::string_view s) const;
		void SetName(CMShared<DescriptorPool> descriptor_pool, std::string_view s) const;
		void SetName(CMShared<DescriptorSetLayout> descriptor_set_layout, std::string_view s) const;
		void SetName(CMShared<DescriptorSet> descriptor_set, std::string_view s) const;
		void SetName(CMShared<ShaderModule> shader_module, std::string_view s) const;
		void SetName(CMShared<RenderPass> render_pass, std::string_view s) const;
		void SetName(CMShared<PipelineLayoutInfo> pipeline_layout_info, std::string_view s) const;
		void SetName(CMShared<PipelineLayout> pipeline_layout, std::string_view s) const;
		void SetName(CMShared<GraphicsPipeline> graphics_pipeline, std::string_view s) const;
		void SetName(CMShared<ComputePipeline> compute_pipeline, std::string_view s) const;
		void SetName(CMShared<RaytracingPipeline> raytracing_pipeline, std::string_view s) const;
		void SetName(CMShared<Semaphore> semaphore, std::string_view s) const;
		void SetName(CMShared<Fence> fence, std::string_view s) const;
		void SetName(CMShared<CommandPool> command_pool, std::string_view s) const;
		void SetName(CMShared<CommandBuffer> command_buffer, std::string_view s) const;
		void SetName(CMShared<BottomLevelAccelerationStructure> blas, std::string_view s) const;
		void SetName(CMShared<TopLevelAccelerationStructure> tlas, std::string_view s) const;
		void SetName(CMShared<QueryPool> query_pool, std::string_view s) const;
		void SetName(CMShared<FrameBuffer> frame_buffer, std::string_view s) const;

		FrameCounter GetFrame() const { return current_frame; }
		FrameCounter GetFrameIndex() const { return current_frame % GetNumFrames(); }
		FrameCounter GetPrevFrameIndex() const { return (int(GetFrameIndex()) - 1 < 0) ? (GetNumFrames() - 1) : (GetFrameIndex() - 1); }
		float GetLastFramePerSecond() const { return 1.0f / (static_cast<float>(last_frame_duration) / 1000000.0f); }

		VmaAllocator GetVmaAllocator() { return vma_allocator; }

		/*
		template<typename... Args>
		inline void ExecuteAsync(std::function<void(Args...)>&& f)
		{
			executor.silent_async(std::forward<F>(f));
		}
		*/

		template<typename F>
		inline void ExecuteAsync(F&& f)
		{
			executor.silent_async(std::forward<F>(f));
		}


		tf::Executor& GetExecutor() { return executor; }

	private:
		Error Resize();
		Error CreateSwapchain();
		Error CreateFrameBuffer();
		Error RecreateRenderFrames();
		Error InitCache();

	public:
		Error CreateGlobalRenderPass();

	private:
		// setup and necessary
		std::shared_ptr<VulkanWindow> window{};

		vk::SurfaceKHR surface{};

		vkb::Instance vkb_instance{};
		vkb::PhysicalDevice physical_device{};
		vkb::Device vkb_device{};
		vkb::Swapchain vkb_swapchain{};

		vk::Device device{};

		vk::Queue graphics_queue{};
		vk::Queue compute_queue{};
		vk::Queue transfer_queue{};
		vk::Queue present_queue{};

		// Allocation
		VmaAllocator vma_allocator{};

		// Mutable
		vk::Extent2D window_extent{};
		uint32_t next_image_index{};

		// Frame
		FrameCounter current_frame{};
		std::chrono::high_resolution_clock::time_point last_frame_time_point{};
		uint64_t last_frame_duration{};
		std::vector<RenderFrame> render_frames{};

		std::vector<vk::Image> swapchain_images{};
		std::vector<vk::ImageView> swapchain_image_views{};

		// Manager
		ShaderModuleManager shader_module_manager{};
		ResourceManager resource_manager{};
		DiskCacher disk_cacher{};
		std::shared_ptr<Profiler> profiler{};

		// Render
		CMShared<Texture> depth_image{};
		CMShared<RenderPass> global_render_pass{};
		std::vector<vk::UniqueFramebuffer> frame_buffers{};

		// cache
		CacheMappingWithFrameCounter<DescriptorSetLayoutCreateInfo, CMShared<DescriptorSetLayout>> descriptor_set_layout_cache;
		CacheMappingWithFrameCounter<PipelineLayoutCreateInfo, CMShared<PipelineLayoutInfo>> pipeline_layout_info_cache;
		CacheMappingWithFrameCounter<CMShared<PipelineLayoutInfo>, CMShared<PipelineLayout>> pipeline_layout_cache;
		CacheMappingWithFrameCounter<GraphicsPipelineCreateInfo, CMShared<GraphicsPipeline>> graphics_pipeline_cache;
		CacheMappingWithFrameCounter<ComputePipelineCreateInfo, CMShared<ComputePipeline>> compute_pipeline_cache;
		CacheMappingWithFrameCounter<RaytracingPipelineCreateInfo, CMShared<RaytracingPipeline>> raytracing_pipeline_cache;
		CacheMappingWithFrameCounter<RenderPassCreateInfo, CMShared<RenderPass>> render_pass_cache;
		CacheMappingWithFrameCounter<ShaderModuleCreateInfo, CMShared<ShaderModule>> shader_module_cache;
		CacheMappingWithFrameCounter<DescriptorPoolCreateInfo, CMShared<DescriptorPool>> descriptor_pool_cache;
		CacheMappingWithFrameCounter<SamplerCreateInfo, CMShared<Sampler>> sampler_cache;
		CacheMappingWithFrameCounter<QueryPoolCreateInfo, CMShared<QueryPool>> query_pool_cache;

		// Multithreading
		tf::Executor executor;

		std::unique_ptr<glslang::TPoolAllocator> glslang_pool_allocator{};

		bool debug_marker_enabled = false;
	};
}