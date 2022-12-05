#include "include/context.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace RaysterizerEngine
{
	Context::Context()
	{
	}

    Context::~Context()
    {
        if(device)
        {
            device.waitIdle();

            render_frames.clear();

            frame_buffers.clear();
            global_render_pass = nullptr;
            depth_image = nullptr;

            descriptor_set_layout_cache.Clear();
            pipeline_layout_info_cache.Clear();
            pipeline_layout_cache.Clear();
            graphics_pipeline_cache.Clear();
            compute_pipeline_cache.Clear();
            raytracing_pipeline_cache.Clear();
            render_pass_cache.Clear();
            shader_module_cache.Clear();
            descriptor_pool_cache.Clear();
            sampler_cache.Clear();
            query_pool_cache.Clear();

            PanicIfError(shader_module_manager.Flush());
            PanicIfError(resource_manager.FlushEntirely());

            vmaDestroyAllocator(vma_allocator);
        }

    }

	Error Context::Setup(std::shared_ptr<VulkanWindow> window_)
	{
        if (!window_)
        {
            return StringError("Window is invalid:");
        }

        window = window_;

        ReturnIfError(VulkanDispatchLoader::Initialize());
        vkb::InstanceBuilder builder;

#ifdef ENABLE_PROFILER
        const static bool enable_profiler = Config["profiler"]["enable"];
        if (enable_profiler)
        {
            profiler = std::make_shared<Profiler>(this);
#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
            profiler->SetupNvidiaNSightAfterMath();
#endif
        }
#endif

        builder = builder
            .set_app_name(Constants::VULKAN_APPLICATION_NAME.data());

        if (Config["validation_callback"])
        {
            builder = builder
                .set_debug_callback(
                    [](
                        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                        void* pUserData) -> VkBool32 {
                            auto ms = vkb::to_string_message_severity(messageSeverity);
                            auto mt = vkb::to_string_message_type(messageType);
                            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT || messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                            {
                                auto error = fmt::format("[{}: {}] | {}\n", ms, mt, pCallbackData->pMessage);
                                spdlog::error(error);
                                PANIC(error);
                            }
                            return VK_FALSE;
                    });
        }
        if (Config["validation_layers"])
        {
            builder = builder.request_validation_layers();
        }

        builder.desire_api_version(Constants::RAYSTERIZER_VULKAN_VERSION);

        auto instance_extensions = window->GetVulkanExtensions();
        for (const auto& extension : instance_extensions)
        {
            builder = builder.enable_extension(extension.c_str());
        }

        for (const auto& extension : Constants::INSTANCE_EXTENSIONS)
        {
            builder = builder.enable_extension(extension.data());
        }

        auto inst_ret = builder
            .build();

        if (!inst_ret)
        {
            return StringError("Failed to create Vulkan instance. Error: {}", inst_ret.error().message());
        }
        vkb_instance = inst_ret.value();

        ReturnIfError(VulkanDispatchLoader::InitializeInstance(vkb_instance.instance));

        window->SetResizeCallback([this](vk::Extent2D new_window_extent)
            {
            });

        window->SetShutdownCallback([this]()
            {
            });

        surface = window->CreateSurface(vkb_instance.instance);
        window_extent = window->GetSize();
        
        vkb::PhysicalDeviceSelector vkb_physical_device_selector = vkb::PhysicalDeviceSelector{ vkb_instance }
            .set_surface(surface)
            .set_desired_version(VK_API_VERSION_MAJOR(Constants::RAYSTERIZER_VULKAN_VERSION), VK_API_VERSION_MINOR(Constants::RAYSTERIZER_VULKAN_VERSION))
            .require_separate_compute_queue()
            .require_separate_transfer_queue();

        for (const auto& extension : Constants::DEVICE_EXTENSIONS)
        {
            vkb_physical_device_selector = vkb_physical_device_selector.add_required_extension(extension.data());
        }

        ////////////////////////////////////////////////////////
        // Device Extensions
        ////////////////////////////////////////////////////////

        vk::PhysicalDeviceBufferDeviceAddressFeatures address_features{};
        address_features.setBufferDeviceAddress(true);

        vk::PhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{};
        acceleration_structure_features.setAccelerationStructure(true);
        //acceleration_structure_features.setAccelerationStructureCaptureReplay(true);
        //acceleration_structure_features.setAccelerationStructureHostCommands(true);
        //acceleration_structure_features.setAccelerationStructureIndirectBuild(true);
        acceleration_structure_features.setDescriptorBindingAccelerationStructureUpdateAfterBind(true);

        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_pipeline_features{};
        raytracing_pipeline_features.setRayTracingPipeline(true);
        //raytracing_pipeline_features.setRayTracingPipelineShaderGroupHandleCaptureReplay(true);
        //raytracing_pipeline_features.setRayTracingPipelineShaderGroupHandleCaptureReplayMixed(true);
        raytracing_pipeline_features.setRayTracingPipelineTraceRaysIndirect(true);
        raytracing_pipeline_features.setRayTraversalPrimitiveCulling(true);

        auto ray_query_features = vk::PhysicalDeviceRayQueryFeaturesKHR{}
            .setRayQuery(true);

        vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{};
        descriptor_indexing_features.setRuntimeDescriptorArray(true);
        descriptor_indexing_features.setDescriptorBindingVariableDescriptorCount(true);
        descriptor_indexing_features.setDescriptorBindingPartiallyBound(true);
        descriptor_indexing_features.setShaderUniformBufferArrayNonUniformIndexing(true);
        descriptor_indexing_features.setShaderStorageBufferArrayNonUniformIndexing(true);
        descriptor_indexing_features.setShaderSampledImageArrayNonUniformIndexing(true);
        descriptor_indexing_features.setShaderInputAttachmentArrayNonUniformIndexing(true);
        descriptor_indexing_features.setShaderStorageTexelBufferArrayNonUniformIndexing(true);
        descriptor_indexing_features.setShaderUniformBufferArrayNonUniformIndexing(true);
        descriptor_indexing_features.setShaderUniformTexelBufferArrayNonUniformIndexing(true);

        vk::PhysicalDevice16BitStorageFeaturesKHR physical_device_16_bit_storage_features{};
        physical_device_16_bit_storage_features
            .setStorageBuffer16BitAccess(true)
            .setStorageInputOutput16(false)
            .setStoragePushConstant16(true)
            .setUniformAndStorageBuffer16BitAccess(true);

        vk::PhysicalDevice8BitStorageFeaturesKHR physical_device_8_bit_storage_features{};
        physical_device_8_bit_storage_features
            .setStorageBuffer8BitAccess(true)
            .setStoragePushConstant8(true)
            .setUniformAndStorageBuffer8BitAccess(true);

        vk::PhysicalDeviceShaderFloat16Int8Features physical_device_shader_float_16_int_8_features{};
        physical_device_shader_float_16_int_8_features
            .setShaderInt8(true)
            .setShaderFloat16(true);

        vk::PhysicalDeviceScalarBlockLayoutFeatures physical_device_scalar_block_layout_features{};
        physical_device_scalar_block_layout_features
            .setScalarBlockLayout(true)
            .setPNext(&physical_device_shader_float_16_int_8_features);

        vk::PhysicalDeviceRobustness2FeaturesEXT physical_device_robustness2_features{};
        physical_device_robustness2_features
            .setRobustBufferAccess2(true)
            .setRobustImageAccess2(true)
            .setNullDescriptor(true);

        vk::PhysicalDeviceShaderClockFeaturesKHR physical_device_shader_clock{};
        physical_device_shader_clock
            .setShaderSubgroupClock(true)
            .setShaderDeviceClock(true);

        /*
        vkb_physical_device_selector.add_required_extension_features(address_features);
        vkb_physical_device_selector.add_required_extension_features(acceleration_structure_features);
        vkb_physical_device_selector.add_required_extension_features(raytracing_pipeline_features);
        vkb_physical_device_selector.add_required_extension_features(descriptor_indexing_features);
        vkb_physical_device_selector.add_required_extension_features(physical_device_16_bit_storage_features);
        vkb_physical_device_selector.add_required_extension_features(physical_device_8_bit_storage_features);
        vkb_physical_device_selector.add_required_extension_features(physical_device_shader_float_16_int_8_features);
        vkb_physical_device_selector.add_required_extension_features(physical_device_scalar_block_layout_features);
        vkb_physical_device_selector.add_required_extension_features(physical_device_robustness2_features);
        */

        auto physical_device_depth_clip_control_features_ext = vk::PhysicalDeviceDepthClipControlFeaturesEXT{}
            .setDepthClipControl(true);

        //vkb_physical_device_selector.add_required_extension_features(physical_device_depth_clip_control_features_ext);

        ////////////////////////////////////////////////////////
        // Physical device features
        ////////////////////////////////////////////////////////

        auto features = vk::PhysicalDeviceFeatures{}
            .setDualSrcBlend(true)
            .setIndependentBlend(true)
            .setGeometryShader(true)
            .setTessellationShader(true)
            .setRobustBufferAccess(true)
            .setSamplerAnisotropy(true)
            .setPipelineStatisticsQuery(true)
            .setVertexPipelineStoresAndAtomics(true)
            .setFragmentStoresAndAtomics(true)
            .setShaderTessellationAndGeometryPointSize(true)
            .setShaderImageGatherExtended(true)
            .setShaderStorageImageExtendedFormats(true)
            .setShaderStorageImageMultisample(true)
            .setShaderStorageImageReadWithoutFormat(true)
            .setShaderStorageImageWriteWithoutFormat(true)
            .setShaderUniformBufferArrayDynamicIndexing(true)
            .setShaderSampledImageArrayDynamicIndexing(true)
            .setShaderStorageBufferArrayDynamicIndexing(true)
            .setShaderStorageImageArrayDynamicIndexing(true)
            .setFillModeNonSolid(true)
            .setShaderClipDistance(true)
            .setShaderCullDistance(true)
            .setShaderFloat64(true)
            .setShaderInt64(true)
            .setShaderInt16(true);

        auto features_11 = vk::PhysicalDeviceVulkan11Features{}
            .setStorageBuffer16BitAccess(true)
            .setUniformAndStorageBuffer16BitAccess(true);

        auto features_12 = vk::PhysicalDeviceVulkan12Features{}
            .setSamplerMirrorClampToEdge(true)
            .setDrawIndirectCount(true)
            .setStorageBuffer8BitAccess(true)
            .setUniformAndStorageBuffer8BitAccess(true)
            .setStoragePushConstant8(true)
            .setShaderBufferInt64Atomics(true)
            .setShaderSharedInt64Atomics(true)
            .setShaderFloat16(true)
            .setShaderInt8(true)
            .setDescriptorIndexing(true)
            .setShaderInputAttachmentArrayDynamicIndexing(true)
            .setShaderUniformTexelBufferArrayDynamicIndexing(true)
            .setShaderStorageTexelBufferArrayDynamicIndexing(true)
            .setShaderUniformBufferArrayNonUniformIndexing(true)
            .setShaderSampledImageArrayNonUniformIndexing(true)
            .setShaderStorageBufferArrayNonUniformIndexing(true)
            .setShaderStorageImageArrayNonUniformIndexing(true)
            .setShaderInputAttachmentArrayNonUniformIndexing(true)
            .setShaderUniformTexelBufferArrayNonUniformIndexing(true)
            .setShaderStorageTexelBufferArrayNonUniformIndexing(true)
            .setDescriptorBindingUniformBufferUpdateAfterBind(true)
            .setDescriptorBindingSampledImageUpdateAfterBind(true)
            .setDescriptorBindingStorageImageUpdateAfterBind(true)
            .setDescriptorBindingStorageBufferUpdateAfterBind(true)
            .setDescriptorBindingUniformTexelBufferUpdateAfterBind(true)
            .setDescriptorBindingStorageTexelBufferUpdateAfterBind(true)
            .setDescriptorBindingUpdateUnusedWhilePending(true)
            .setDescriptorBindingPartiallyBound(true)
            .setDescriptorBindingVariableDescriptorCount(true)
            .setRuntimeDescriptorArray(true)
            .setSamplerFilterMinmax(true)
            .setScalarBlockLayout(true)
            .setImagelessFramebuffer(true)
            .setUniformBufferStandardLayout(true)
            .setShaderSubgroupExtendedTypes(true)
            .setSeparateDepthStencilLayouts(true)
            .setHostQueryReset(true)
            .setTimelineSemaphore(true)
            .setBufferDeviceAddress(true)
            .setBufferDeviceAddressCaptureReplay(true)
            .setBufferDeviceAddressMultiDevice(true)
            .setVulkanMemoryModel(true)
            .setVulkanMemoryModelDeviceScope(true)
            .setVulkanMemoryModelAvailabilityVisibilityChains(true)
            .setShaderOutputViewportIndex(true)
            .setShaderOutputLayer(true);

        auto features_13 = vk::PhysicalDeviceVulkan13Features{}
            .setRobustImageAccess(true)
            .setInlineUniformBlock(true)
            .setDynamicRendering(true)
            .setSynchronization2(true)
            .setPipelineCreationCacheControl(true)
            .setShaderTerminateInvocation(true);

        vkb_physical_device_selector.set_required_features(static_cast<VkPhysicalDeviceFeatures>(features));
        vkb_physical_device_selector.set_required_features_11(static_cast<VkPhysicalDeviceVulkan11Features>(features_11));
        vkb_physical_device_selector.set_required_features_12(static_cast<VkPhysicalDeviceVulkan12Features>(features_12));
        //vkb_physical_device_selector.set_required_features_13(static_cast<VkPhysicalDeviceVulkan13Features>(features_13));

        auto device_selection_mode = vkb::DeviceSelectionMode::partially_and_fully_suitable;
        auto vkb_physical_device_selector_with_no_debug_marker = vkb_physical_device_selector;

        if (Config["validation_layers"])
        {
            vkb_physical_device_selector = vkb_physical_device_selector.add_required_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }

        auto physical_device_names_result = vkb_physical_device_selector.select_device_names(device_selection_mode);
        std::vector<std::string> physical_device_names;
        if (!physical_device_names_result)
        {
            vkb_physical_device_selector = vkb_physical_device_selector_with_no_debug_marker;
            auto physical_device_names_result = vkb_physical_device_selector.select_device_names(device_selection_mode);
            if (!physical_device_names_result)
            {
                if (GetModuleHandleA("renderdoc.dll"))
                {

                }
                else
                {
                    return StringError("Failed to enumerate Vulkan Physical Device names. Error: {}", physical_device_names_result.error().message());
                }
            }
            else
            {
                physical_device_names = physical_device_names_result.value();
            }
        }
        else
        {
            debug_marker_enabled = true;
            physical_device_names = physical_device_names_result.value();
        }

        for (const auto& physical_device_name : physical_device_names)
        {
            DEBUG("Physical device: {}", physical_device_name);
        }

        auto phys_ret = vkb_physical_device_selector
            .select(device_selection_mode);

        if (!phys_ret)
        {
            if (GetModuleHandleA("renderdoc.dll"))
            {
                vkb_physical_device_selector = vkb_physical_device_selector.select_first_device_unconditionally();
                auto phys_ret = vkb_physical_device_selector
                    .select(device_selection_mode);
                physical_device = phys_ret.value();
            }
            else
            {
                return StringError("Failed to select Vulkan Physical Device. Error: {}", phys_ret.error().message());
            }
        }
        else
        {
            physical_device = phys_ret.value();
        }

        vkb::DeviceBuilder vkb_device_builder = vkb::DeviceBuilder{ physical_device };
        //vkb_device_builder.add_pNext(&address_features);
        vkb_device_builder.add_pNext(&acceleration_structure_features);
#ifdef RAYTRACING_ENABLED
        vkb_device_builder.add_pNext(&raytracing_pipeline_features);
        vkb_device_builder.add_pNext(&ray_query_features);
#endif
        //vkb_device_builder.add_pNext(&descriptor_indexing_features);
        //vkb_device_builder.add_pNext(&physical_device_16_bit_storage_features);
        //vkb_device_builder.add_pNext(&physical_device_8_bit_storage_features);
        //vkb_device_builder.add_pNext(&physical_device_shader_float_16_int_8_features);
        //vkb_device_builder.add_pNext(&physical_device_scalar_block_layout_features);
        vkb_device_builder.add_pNext(&physical_device_robustness2_features);
        vkb_device_builder.add_pNext(&physical_device_shader_clock);

#ifdef ENABLE_PROFILER
#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
        auto device_diagnostics_config_create_info_nv = vk::DeviceDiagnosticsConfigCreateInfoNV{}
            .setFlags(
                vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableResourceTracking |
                vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableAutomaticCheckpoints |
                vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableShaderDebugInfo);

        vkb_device_builder.add_pNext(&device_diagnostics_config_create_info_nv);
#endif
#endif
        // automatically propagate needed data from instance & physical device
        auto dev_ret = vkb_device_builder
            .build();
        if (!dev_ret) {
            return StringError("Failed to create Vulkan device. Error: {}", dev_ret.error().message());
        }
        vkb_device = dev_ret.value();

        // Get the VkDevice handle used in the rest of a vulkan application
        device = vkb_device.device;
        ReturnIfError(VulkanDispatchLoader::InitializeDevice(device));

        ////////////////////////////////////////////////////////
        // Queues
        ////////////////////////////////////////////////////////

        // Get the graphics queue with a helper function
        auto present_queue_ret = vkb_device.get_queue(vkb::QueueType::graphics);
        if (!present_queue_ret) {
            return StringError("Failed to get present queue. Error: {}", present_queue_ret.error().message());
        }
        present_queue = present_queue_ret.value();

        auto graphics_queue_ret = vkb_device.get_queue(vkb::QueueType::graphics);
        if (!graphics_queue_ret) {
            return StringError("Failed to get graphics queue. Error: {}", graphics_queue_ret.error().message());
        }
        graphics_queue = graphics_queue_ret.value();

        auto compute_queue_ret = vkb_device.get_queue(vkb::QueueType::compute);
        if (!compute_queue_ret) {
            return StringError("Failed to get compute queue. Error: {}", compute_queue_ret.error().message());
        }
        compute_queue = compute_queue_ret.value();

        auto transfer_queue_ret = vkb_device.get_queue(vkb::QueueType::transfer);
        if (!transfer_queue_ret) {
            return StringError("Failed to get transfer queue. Error: {}", transfer_queue_ret.error().message());
        }
        transfer_queue = transfer_queue_ret.value();

        SetObjectName(present_queue, "Present Queue", vk::DebugReportObjectTypeEXT::eQueue);
        SetObjectName(graphics_queue, "Graphics Queue", vk::DebugReportObjectTypeEXT::eQueue);
        SetObjectName(compute_queue, "Compute Queue", vk::DebugReportObjectTypeEXT::eQueue);
        SetObjectName(transfer_queue, "Transfer Queue", vk::DebugReportObjectTypeEXT::eQueue);

        VmaAllocatorCreateInfo vma_allocator_create_info{};
        vma_allocator_create_info.physicalDevice = GetPhysicalDevice();
        vma_allocator_create_info.device = device;
        vma_allocator_create_info.instance = vkb_instance.instance;
        vma_allocator_create_info.flags = Constants::VMA_ALLOCATOR_CREATE_FLAGS;
        vma_allocator_create_info.vulkanApiVersion = Constants::RAYSTERIZER_VULKAN_VERSION;
        vmaCreateAllocator(&vma_allocator_create_info, &vma_allocator);

        ////////////////////////////////////////////////////////
        // Creation
        ////////////////////////////////////////////////////////

        ReturnIfError(InitCache());

        ReturnIfError(CreateSwapchain());
        ReturnIfError(CreateGlobalRenderPass());
        ReturnIfError(CreateFrameBuffer());
        ReturnIfError(RecreateRenderFrames());
        
#ifdef ENABLE_PROFILER
        if (enable_profiler)
        {
            profiler->Setup();
        }
#endif

        shader_module_manager = ShaderModuleManager(this);
        resource_manager = ResourceManager(this);
        ReturnIfError(disk_cacher.Init(this));

        glslang::InitializeProcess();
        glslang_pool_allocator = std::make_unique<glslang::TPoolAllocator>();
        glslang::SetThreadPoolAllocator(glslang_pool_allocator.get());

        return NoError();
	}

    Error Context::Resize()
    {
        AssignOrReturnVkError(auto surface_capabilities, GetPhysicalDevice().getSurfaceCapabilities2KHR(surface));
        auto current_extent = surface_capabilities.surfaceCapabilities.currentExtent;

        // Minimized
        if (current_extent == vk::Extent2D{ 0, 0 })
        {
            return NoError();
        }

        if (current_extent == window_extent)
        {
            return NoError();
            //return StringError("Expected new extent to match sufarce!");
        }
        //surface = window->CreateSurface(vkb_instance.instance);
        window_extent = current_extent;

        // Reset state
        device.waitIdle();

        ReturnIfError(CreateSwapchain());
        //ReturnIfError(CreateGlobalRenderPass());
        ReturnIfError(CreateFrameBuffer());

        for (auto& render_frame : render_frames)
        {
            PanicIfError(render_frame.FlushCommandBuffers());
        }

        device.waitIdle();

        // Flush
        PanicIfError(resource_manager.Flush());

        device.waitIdle();

        return NoError();
    }

    Error Context::CreateSwapchain()
    {
        ////////////////////////////////////////////////////////
        // Swapchain
        ////////////////////////////////////////////////////////

        auto present_mode = GetPresentMode();

        vkb::SwapchainBuilder swapchain_builder{ physical_device, device, surface };

        AssignOrReturnVkError(std::vector<vk::SurfaceFormatKHR> surface_formats, GetPhysicalDevice().getSurfaceFormatsKHR(surface));

        swapchain_builder
            //.use_default_format_selection()
            .set_image_usage_flags(static_cast<VkImageUsageFlags>(Constants::SWAPCHAIN_IMAGE_USAGE_FLAGS))
            .set_desired_present_mode(static_cast<VkPresentModeKHR>(present_mode))
            .set_desired_extent(window_extent.width, window_extent.height);

        for (const auto& surface_format : Constants::DESIRED_SURFACE_FORMATS)
        {
            swapchain_builder.set_desired_format(static_cast<VkSurfaceFormatKHR>(surface_format));
        }

        for (const auto& surface_format : surface_formats)
        {
            swapchain_builder.set_desired_format(static_cast<VkSurfaceFormatKHR>(surface_format));
        }

        for (const auto& surface_format : Constants::FALLBACK_SURFACE_FORMATS)
        {
            swapchain_builder.add_fallback_format(static_cast<VkSurfaceFormatKHR>(surface_format));
        }

        if (vkb_swapchain)
        {
            swapchain_builder
                .set_old_swapchain(vkb_swapchain);
        }

        auto vkb_swapchain_result = swapchain_builder
            .build();

        if (!vkb_swapchain_result)
        {
            return StringError("Failed to create swapchain. Error: {}", vkb_swapchain_result.error().message());
        }

        vkb_swapchain = vkb_swapchain_result.value();

        auto images_result = vkb_swapchain.get_images();
        if (!images_result)
        {
            return StringError("Failed to create swapchain images. Error: {}", images_result.error().message());
        }

        swapchain_images.clear();
        auto swapchain_images_ = images_result.value();
        std::transform(std::begin(swapchain_images_), std::end(swapchain_images_), std::back_inserter(swapchain_images), [](const VkImage& image) { return vk::Image{ image }; });

        auto image_views_result = vkb_swapchain.get_image_views();
        if (!image_views_result)
        {
            return StringError("Failed to create swapchain image views. Error: {}", image_views_result.error().message());
        }

        swapchain_image_views.clear();
        auto swapchain_image_views_ = image_views_result.value();
        std::transform(std::begin(swapchain_image_views_), std::end(swapchain_image_views_), std::back_inserter(swapchain_image_views), [](const VkImageView& image_view) { return vk::ImageView{ image_view }; });

        auto depth_image_create_info = vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setFormat(Constants::DEPTH_FORMAT)
            .setExtent(GetWindowExtent3D())
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(Constants::DEPTH_IMAGE_USAGE_FLAGS);

        VmaAllocationCreateInfo depth_vma_allocation_create_info{};
        depth_vma_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        depth_vma_allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        auto depth_image_view_create_info = vk::ImageViewCreateInfo{}
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(Constants::DEPTH_FORMAT)
            .setSubresourceRange
            (
                vk::ImageSubresourceRange{}
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)
                    .setAspectMask(Constants::DEPTH_IMAGE_ASPECT_MASK)
            );

        TextureCreateInfo texture_create_info
        { 
            ImageCreateInfo{depth_image_create_info, depth_vma_allocation_create_info},
            ImageViewCreateInfo{depth_image_view_create_info}
        };

        const static bool global_render_pass_create_depth_buffer = Config["raysterizer"]["global_render_pass_create_depth_buffer"];
        if (global_render_pass_create_depth_buffer)
        {
            AssignOrReturnError(depth_image, CreateTexture(texture_create_info));
        }

        return NoError();
    }

    Error Context::CreateFrameBuffer()
    {
        const static bool global_render_pass_create_depth_buffer = Config["raysterizer"]["global_render_pass_create_depth_buffer"];

        std::vector<vk::ImageView> attachments(1 + (global_render_pass_create_depth_buffer ? 1 : 0));

        if (global_render_pass_create_depth_buffer)
        {
            // Depth/Stencil attachment is the same for all frame buffers
            attachments[1] = depth_image->image_view->image_view;
        }

        // Create frame buffers for every swap chain image
        frame_buffers.resize(GetNumFrames());
        for (auto i = 0; i < frame_buffers.size(); i++)
        {
            attachments[0] = GetSwapchainImageViews()[i];

            auto frame_buffer_create_info = vk::FramebufferCreateInfo{}
                .setRenderPass(global_render_pass->render_pass)
                .setAttachments(attachments)
                .setWidth(GetWindowExtent().width)
                .setHeight(GetWindowExtent().height)
                .setLayers(1);

            frame_buffers[i] = AssignOrPanicVkError(device.createFramebufferUnique(frame_buffer_create_info));
        }

        return NoError();
    }

    Error Context::RecreateRenderFrames()
    {
        // Make the frames
        render_frames.clear();
        for (auto i = 0; i < GetNumFrames(); i++)
        {
            render_frames.emplace_back(RenderFrame{ this });
        }
        //render_frames.resize(GetNumFrames());

        return NoError();
    }

    Error Context::InitCache()
    {
        descriptor_set_layout_cache.SetContext(this);
        pipeline_layout_info_cache.SetContext(this);
        pipeline_layout_cache.SetContext(this);
        graphics_pipeline_cache.SetContext(this);
        compute_pipeline_cache.SetContext(this);
        raytracing_pipeline_cache.SetContext(this);
        render_pass_cache.SetContext(this);
        shader_module_cache.SetContext(this);
        descriptor_pool_cache.SetContext(this);
        sampler_cache.SetContext(this);
        query_pool_cache.SetContext(this);

        return NoError();
    }

    Error Context::CreateGlobalRenderPass()
    {
        const static bool global_render_pass_create_depth_buffer = Config["raysterizer"]["global_render_pass_create_depth_buffer"];
        const static bool load_op_clear = Config["raysterizer"]["global_render_pass_color_load_op_clear"];

        std::vector<vk::AttachmentDescription> attachments;
        attachments.emplace_back(
            vk::AttachmentDescription{}
            .setFormat(GetSwapchainFormat())
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(load_op_clear ? vk::AttachmentLoadOp::eDontCare : vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
        );

        if (global_render_pass_create_depth_buffer)
        {
            attachments.emplace_back(
                vk::AttachmentDescription{}
                .setFormat(Constants::DEPTH_FORMAT)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eClear)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
            );
        }

        std::vector<vk::AttachmentReference> color_references = {
            vk::AttachmentReference{}
                .setAttachment(0)
                .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
        };

        vk::AttachmentReference depth_reference = vk::AttachmentReference{}
            .setAttachment(1)
            .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        auto subpass_description = vk::SubpassDescription{}
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(color_references);
            //.setInputAttachments(nullptr)
            //.setPreserveAttachments(nullptr)
            //.setResolveAttachments(nullptr)

        if (global_render_pass_create_depth_buffer)
        {
            subpass_description
                .setPDepthStencilAttachment(&depth_reference);
        }

        auto subpass_descriptions = std::vector<vk::SubpassDescription>
        {
            subpass_description
        };

        // Subpass dependencies for layout transitions
        std::vector<vk::SubpassDependency> dependencies;

        dependencies.emplace_back(
            vk::SubpassDependency{}
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion)
        );

        dependencies.emplace_back(
            vk::SubpassDependency{}
            .setSrcSubpass(0)
            .setDstSubpass(VK_SUBPASS_EXTERNAL)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
            .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion)
        );

        RenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.attachment_descriptions = attachments;
        render_pass_create_info.color_attachment_references = color_references;
        render_pass_create_info.depth_stencil_reference = depth_reference;
        render_pass_create_info.subpass_descriptions = subpass_descriptions;
        render_pass_create_info.subpass_dependencies = dependencies;

        AssignOrReturnError(global_render_pass, Get(render_pass_create_info));

        return NoError();
    }

    vk::Instance Context::GetInstance() const
    {
        return vkb_instance.instance;
    }

    vk::PhysicalDevice Context::GetPhysicalDevice() const
    {
        return physical_device.physical_device;
    }

    vk::Device Context::GetDevice() const
    {
        return vkb_device.device;
    }

    uint32_t Context::GetGraphicsQueueFamily() const
    {
        auto index_ret = vkb_device.get_queue_index(vkb::QueueType::graphics);
        if (!index_ret) {
            PANIC("Failed to get queue family. Error: {}", index_ret.error().message());
        }
        auto index = index_ret.value();
        return index;
    }

    uint32_t Context::GetComputeQueueFamily() const
    {
        auto index_ret = vkb_device.get_queue_index(vkb::QueueType::compute);
        if (!index_ret) {
            PANIC("Failed to get queue family. Error: {}", index_ret.error().message());
        }
        auto index = index_ret.value();
        return index;
    }

    uint32_t Context::GetTransferQueueFamily() const
    {
        auto index_ret = vkb_device.get_queue_index(vkb::QueueType::transfer);
        if (!index_ret) {
            PANIC("Failed to get queue family. Error: {}", index_ret.error().message());
        }
        auto index = index_ret.value();
        return index;
    }

    uint32_t Context::GetPresentQueueFamily() const
    {
        auto index_ret = vkb_device.get_queue_index(vkb::QueueType::transfer);
        if (!index_ret) {
            PANIC("Failed to get queue family. Error: {}", index_ret.error().message());
        }
        auto index = index_ret.value();
        return index;
    }

    bool Context::IsUnifiedGraphicsAndTransferQueue() const
    {
        return graphics_queue == transfer_queue;
    }

    vk::Queue Context::GetGraphicsQueue() const
    {
        return graphics_queue;
    }

    vk::Queue Context::GetComputeQueue() const
    {
        return compute_queue;
    }

    vk::Queue Context::GetTransferQueue() const
    {
        return transfer_queue;
    }

    vk::Queue Context::GetPresentQueue() const
    {
        return present_queue;
    }

    vk::Queue Context::GetQueue(QueueType queue_type) const
    {
        switch (queue_type)
        {
        case QueueType::Graphics:
            return graphics_queue;
        case QueueType::Compute:
            return compute_queue;
        case QueueType::Transfer:
            return transfer_queue;
        case QueueType::Present:
            return present_queue;
        default:
            PANIC("Not possible option!");
        }
        return vk::Queue{};
    }

    vk::PresentModeKHR Context::GetPresentMode() const
    {
        bool vsync = Config["vsync"];
        if (vsync)
        {
            return vk::PresentModeKHR::eFifo;
        }
        else
        {
            return vk::PresentModeKHR::eMailbox;
        }
    }

    vk::SwapchainKHR Context::GetSwapchain() const
    {
        return vkb_swapchain.swapchain;
    }

    const std::vector<vk::Image>& Context::GetSwapchainImages() const
    {
        return swapchain_images;
    }

    const std::vector<vk::ImageView>& Context::GetSwapchainImageViews() const
    {
        return swapchain_image_views;
    }

    uint32_t Context::GetNumFrames() const
    {
        return static_cast<uint32_t>(GetSwapchainImages().size());
    }

    vk::Format Context::GetSwapchainFormat() const
    {
        return vk::Format{ vkb_swapchain.image_format };
    }

    vk::Extent2D Context::GetWindowExtent() const
    {
        return window_extent;
    }

    vk::Extent3D Context::GetWindowExtent3D() const
    {
        return vk::Extent3D
        {
            GetWindowExtent(),
            1
        };
    }

    vk::PhysicalDeviceFeatures Context::GetPhysicalDeviceFeatures() const
    {
        auto physical_device_features = GetPhysicalDevice().getFeatures();
        return physical_device_features;
    }

    vk::PhysicalDeviceProperties Context::GetPhysicalDeviceProperties() const
    {
        auto physical_device_properties = GetPhysicalDevice().getProperties();
        return physical_device_properties;
    }

    template<typename T, typename... Args>
    inline CMShared<T> Context::MakeContextManaged(Args&&... args)
    {
        auto cm_shared = CMShared<T>(new T{ std::forward<Args>(args)... }, [this](T* ptr)
            {
                PanicIfError(this->resource_manager.EnqueueDestroy(std::move(*ptr)));
                delete ptr;
            });
        return cm_shared;
    }

    Expected<CMShared<Image>> Context::CreateImage(const ImageCreateInfo& image_create_info)
    {
        VmaAllocationCreateInfo vma_allocation_create_info = image_create_info.vma_allocation_create_info;
        vma_allocation_create_info.flags |= Constants::VMA_ALLOCATION_CREATE_FLAGS_ADDED;

        VkImage vk_image{};
        VmaAllocation vma_allocation{};
        vmaCreateImage(vma_allocator, &static_cast<VkImageCreateInfo>(image_create_info.image_create_info), &vma_allocation_create_info, &vk_image, &vma_allocation, nullptr);

        auto image = MakeContextManaged<Image>(vk_image, vma_allocation, image_create_info);
        return image;
    }

    Expected<CMShared<ImageView>> Context::CreateImageView(const ImageViewCreateInfo& image_view_create_info)
    {
        AssignOrReturnVkError(auto vulkan_image_view, device.createImageView(image_view_create_info.image_view_create_info));

        auto image_view = MakeContextManaged<ImageView>(vulkan_image_view);
        return image_view;
    }

    Expected<CMShared<Texture>> Context::CreateTexture(TextureCreateInfo& texture_create_info)
    {
        CMShared<Image> image{};
        CMShared<ImageView> image_view{};

        const auto& image_create_info = texture_create_info.image_create_info;
        auto& image_view_create_info = texture_create_info.image_view_create_info;

        AssignOrReturnError(image, CreateImage(image_create_info));
        
        image_view_create_info.image_view_create_info.image = image->image;
        AssignOrReturnError(image_view, CreateImageView(image_view_create_info));

        AssignOrReturnError(CMShared<Sampler> sampler, Get(texture_create_info.sampler_create_info));

        auto texture = MakeContextManaged<Texture>(image, image_view, texture_create_info, sampler);
        return texture;
    }

    Expected<CMShared<Sampler>> Context::CreateSampler(const SamplerCreateInfo& sampler_create_info)
    {
        AssignOrReturnVkError(auto vk_sampler, device.createSampler(sampler_create_info.sampler_create_info));

        auto sampler = MakeContextManaged<Sampler>(vk_sampler, sampler_create_info);
        ReturnIfError(disk_cacher.Record(sampler));
        return sampler;
    }

    Expected<CMShared<Buffer>> Context::CreateBuffer(const BufferCreateInfo& buffer_create_info)
    {
        if (buffer_create_info.size == 0)
        {
            return StringError("Creating buffer size of 0 is an error");
        }

        auto buffer_size = buffer_create_info.size;
        static auto uniform_buffer_alignment = GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        if (buffer_create_info.buffer_usage_flags & vk::BufferUsageFlagBits::eUniformBuffer && buffer_size < uniform_buffer_alignment)
        {
            buffer_size = Util::AlignUp(buffer_size, uniform_buffer_alignment);
        }

        auto vulkan_buffer_create_info = vk::BufferCreateInfo{}
            .setSize(buffer_size)
            .setUsage(buffer_create_info.buffer_usage_flags | vk::BufferUsageFlagBits::eShaderDeviceAddress);

        VmaAllocationCreateInfo vma_allocation_create_info = buffer_create_info.vma_allocation_create_info;
        vma_allocation_create_info.flags |= Constants::VMA_ALLOCATION_CREATE_FLAGS_ADDED;

        VkBuffer vulkan_buffer;
        VmaAllocation vma_allocation;
        VmaAllocationInfo vma_allocation_info;
        vmaCreateBuffer(
            vma_allocator,
            &static_cast<VkBufferCreateInfo>(vulkan_buffer_create_info),
            &vma_allocation_create_info,
            &vulkan_buffer,
            &vma_allocation,
            &vma_allocation_info
        );

        auto buffer_device_address_info = vk::BufferDeviceAddressInfo{}
            .setBuffer(vulkan_buffer);
        
        VkBufferDeviceAddressInfo vk;
        vk.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        vk.pNext = nullptr;
        vk.buffer = vulkan_buffer;

        vk::DeviceAddress device_address = device.getBufferAddress(buffer_device_address_info);
        if (buffer_create_info.vma_allocation_create_info.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
        {
            // This map memory will be internally ref counted by vma, so it does not 
            uint8_t* mapped_memory{};
            ReturnIfVkError(vmaMapMemory(vma_allocator, vma_allocation, reinterpret_cast<void**>(&mapped_memory)));
        }

        auto buffer = MakeContextManaged<Buffer>(vulkan_buffer, vma_allocation, vma_allocation_info, device_address, buffer_create_info);
        return buffer;
    }

    Expected<CMShared<DescriptorPool>> Context::CreateDescriptorPool(const DescriptorPoolCreateInfo& descriptor_pool_create_info)
    {
        const std::vector<DescriptorSetLayoutCreateInfo>& descriptor_set_layout_create_infos = descriptor_pool_create_info.descriptor_set_layout_create_infos.GetCreateInfos();

        auto max_sets = descriptor_set_layout_create_infos.size();
        
        flat_hash_map<vk::DescriptorType, uint32_t> descriptor_type_to_count;

        for (const auto& descriptor_set_layout_create_info : descriptor_set_layout_create_infos)
        {
            const auto& bindings = descriptor_set_layout_create_info.GetBindings();
            for (const auto& [binding_index, descriptor_binding] : bindings)
            {
                const auto& descriptor_set_layout_binding = descriptor_binding.descriptor_set_layout_binding;

                auto type = descriptor_set_layout_binding.descriptorType;
                auto count = descriptor_set_layout_binding.descriptorCount;
                descriptor_type_to_count[type] += count;
            }

            const auto& variable_binding = descriptor_set_layout_create_info.GetVariableBinding();
            if (variable_binding)
            {
                if (auto found = bindings.find(variable_binding->binding); found != std::end(bindings))
                {
                    const auto& [binding_index, descriptor_binding] = *found;
                    const auto& descriptor_set_layout_binding = descriptor_binding.descriptor_set_layout_binding;

                    auto type = descriptor_set_layout_binding.descriptorType;
                    auto count = variable_binding->num_bindings;
                    descriptor_type_to_count[type] += count;
                }
                else
                {
                    PANIC("Could not find variable binding!");
                }
            }
        }

        std::vector<vk::DescriptorPoolSize> pool_sizes(descriptor_type_to_count.size());
        auto pool_size_index = 0;
        for (const auto& [descriptor_type, count] : descriptor_type_to_count)
        {
            auto& pool_size = pool_sizes[pool_size_index];

            auto descriptor_count = count * Config["descriptor_pool_size_multipler"];
            pool_size
                .setType(descriptor_type)
                .setDescriptorCount(descriptor_count);
           
            pool_size_index++;
        }

        // Means there's no buffers used in the shader (probably only used for vertex location inputs)
        // We have at least one entry so this does not crash...
        if (max_sets == 0)
        {
            return StringError("Creating descriptor pool from descriptor set layouts -- descriptor set layouts are none (most likely, the shader does not use any external buffers (uniform, storage, ...)");
            max_sets = 1;
        }
        if (pool_sizes.empty())
        {
            auto dummy_pool_size = vk::DescriptorPoolSize{}
                .setType(vk::DescriptorType::eStorageBuffer)
                .setDescriptorCount(1);
            pool_sizes.emplace_back(dummy_pool_size);
        }

        auto vk_descriptor_pool_create_info = vk::DescriptorPoolCreateInfo{}
            .setMaxSets(max_sets)
            .setFlags(Constants::DEFAULT_DESCRIPTOR_POOL_CREATE_FLAGS)
            .setPoolSizes(pool_sizes);

        AssignOrReturnVkError(auto vk_descriptor_pool, device.createDescriptorPool(vk_descriptor_pool_create_info));

        auto descriptor_pool = MakeContextManaged<DescriptorPool>(vk_descriptor_pool);
        return descriptor_pool;
    }

    Expected<CMShared<DescriptorPool>> Context::CreateDescriptorPool(const vk::DescriptorPoolCreateInfo& descriptor_pool_create_info)
    {
        AssignOrReturnVkError(auto vk_descriptor_pool, device.createDescriptorPool(descriptor_pool_create_info));

        auto descriptor_pool = MakeContextManaged<DescriptorPool>(vk_descriptor_pool);
        return descriptor_pool;
    }

    Expected<CMShared<DescriptorSetLayout>> Context::CreateDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& descriptor_set_layout_create_info)
    {
        const auto& bindings = descriptor_set_layout_create_info.GetBindings();
        const auto& variable_binding = descriptor_set_layout_create_info.GetVariableBinding();

        std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings(bindings.size());

        std::transform(std::begin(bindings), std::end(bindings), std::begin(descriptor_set_layout_bindings), [](const auto& it) { return it.second.descriptor_set_layout_binding; });
        std::sort(std::begin(descriptor_set_layout_bindings), std::end(descriptor_set_layout_bindings), [](const auto& e1, const auto& e2) { return e1.binding < e2.binding; });

        vk::DescriptorSetLayout vulkan_descriptor_set_layout;
        if (variable_binding)
        {
            uint32_t max_binding = 0;
            for (const auto& b : descriptor_set_layout_bindings)
            {
                max_binding = std::max(b.binding, max_binding);
            }

            //auto descriptor_binding_flags_size = max_binding + 1;
            //auto descriptor_variable_binding = variable_binding->binding;
            auto descriptor_binding_flags_size = std::min(static_cast<std::size_t>(max_binding + 1), descriptor_set_layout_bindings.size());
            auto descriptor_variable_binding = std::min(static_cast<std::size_t>(variable_binding->binding), descriptor_set_layout_bindings.size() - 1);

            std::vector<vk::DescriptorBindingFlags> descriptor_binding_flags(descriptor_binding_flags_size, Constants::DEFAULT_DESCRIPTOR_BINDING_FLAGS);
            descriptor_binding_flags[descriptor_variable_binding] |= Constants::VARIABLE_BINDING_FLAGS;
            auto variable_binding_flags = vk::DescriptorSetLayoutBindingFlagsCreateInfo{ descriptor_binding_flags };

            auto vulkan_descriptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo{}
                .setFlags(Constants::DEFAULT_DESCRIPTOR_SET_LAYOUT_CREATE_FLAGS)
                .setBindings(descriptor_set_layout_bindings)
                .setPNext(&variable_binding_flags);

            AssignOrReturnVkError(vulkan_descriptor_set_layout, device.createDescriptorSetLayout(vulkan_descriptor_set_layout_create_info));
        }
        else
        {
            uint32_t max_binding = 0;
            for (const auto& b : descriptor_set_layout_bindings)
            {
                max_binding = std::max(b.binding + 1, max_binding);
            }

            if (descriptor_set_layout_bindings.size() == 1)
            {
                max_binding = 1;
            }

            if (max_binding != descriptor_set_layout_bindings.size())
            {
                max_binding = descriptor_set_layout_bindings.size();
            }

            std::vector<vk::DescriptorBindingFlags> descriptor_binding_flags(max_binding, Constants::DEFAULT_DESCRIPTOR_BINDING_FLAGS);
            auto variable_binding_flags = vk::DescriptorSetLayoutBindingFlagsCreateInfo{ descriptor_binding_flags };

            auto vulkan_descriptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo{}
                .setFlags(Constants::DEFAULT_DESCRIPTOR_SET_LAYOUT_CREATE_FLAGS)
                .setBindings(descriptor_set_layout_bindings)
                .setPNext(&variable_binding_flags);

            AssignOrReturnVkError(vulkan_descriptor_set_layout, device.createDescriptorSetLayout(vulkan_descriptor_set_layout_create_info));
        }

        auto descriptor_set_layout = MakeContextManaged<DescriptorSetLayout>(vulkan_descriptor_set_layout, descriptor_set_layout_create_info);
        ReturnIfError(disk_cacher.Record(descriptor_set_layout));
        return descriptor_set_layout;
    }

    Expected<CMShared<DescriptorSet>> Context::CreateDescriptorSet(const DescriptorSetCreateInfo& descriptor_set_create_info)
    {
        std::vector<vk::DescriptorSetLayout> vulkan_descriptor_set_layouts = descriptor_set_create_info.pipeline_layout_info->descriptor_set_layouts.GetDescriptorSetLayouts();
        auto variable_bindings = descriptor_set_create_info.pipeline_layout_info->descriptor_set_layouts.GetVariableBindings();

        std::vector<std::optional<uint32_t>> descriptor_sets_counts(vulkan_descriptor_set_layouts.size());
        for (const auto& [set, variable_binding] : variable_bindings)
        {
            descriptor_sets_counts[set] = variable_binding.num_bindings;
        }

        /*
        auto descriptor_set_variable_descriptor_count_allocate_info = vk::DescriptorSetVariableDescriptorCountAllocateInfo{}
            .setDescriptorCounts(descriptor_sets_countsb

        auto info = vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool(descriptor_set_create_info.descriptor_pool.descriptor_pool)
            .setSetLayouts(vulkan_descriptor_set_layouts)
            .setPNext(&descriptor_set_variable_descriptor_count_allocate_info);

        auto vulkan_descriptor_sets = device.allocateDescriptorSets(info);
        */
        std::vector<vk::DescriptorSet> vulkan_descriptor_sets(vulkan_descriptor_set_layouts.size());
        for (auto i = 0; i < vulkan_descriptor_sets.size(); i++)
        {
            bool has_variable_binding = descriptor_sets_counts[i].has_value();

            uint32_t descriptor_set_count{};
            if (has_variable_binding)
            {
                descriptor_set_count = *descriptor_sets_counts[i];
            }

            auto descriptor_set_variable_descriptor_count_allocate_info = vk::DescriptorSetVariableDescriptorCountAllocateInfo{}
                .setDescriptorCounts(descriptor_set_count);

            auto info = vk::DescriptorSetAllocateInfo{}
                .setDescriptorPool(*descriptor_set_create_info.descriptor_pool)
                .setSetLayouts(vulkan_descriptor_set_layouts[i]);

            if (has_variable_binding)
            {
                info.setPNext(&descriptor_set_variable_descriptor_count_allocate_info);
            }

            AssignOrReturnVkError(auto vulkan_descriptor_set, device.allocateDescriptorSets(info));
            vulkan_descriptor_sets[i] = vulkan_descriptor_set[0];
        }

        auto descriptor_set = MakeContextManaged<DescriptorSet>(vulkan_descriptor_sets, descriptor_set_create_info);
        ReturnIfError(descriptor_set->InitializeWriteDescriptorSets());

        return descriptor_set;
    }

    Expected<CMShared<ShaderModule>> Context::CreateShaderModule(const ShaderModuleCreateInfo& shader_module_create_info)
    {
        auto shader_module = ShaderModule{};
        ReturnIfError(shader_module.Create(shader_module_create_info, device));
        auto context_managed_shader_module = MakeContextManaged<ShaderModule>(std::move(shader_module));
        ReturnIfError(disk_cacher.Record(context_managed_shader_module));

        /*
        // Write to disk
        std::size_t shader_hash = StdHash(shader_module_create_info);
        fs::path cache_path = Config["shader"]["compiler"]["cache_path"];
        if (!fs::exists(cache_path))
        {
            fs::create_directories(cache_path);
        }
        auto shader_cache_path = cache_path / fmt::format("{}.spv", shader_hash);
        if (!fs::exists(shader_cache_path))
        {
            ExecuteAsync([=]()
                {
                    std::ofstream file(shader_cache_path, std::ios::binary);
                    if (!file)
                    {
                        PANIC("Could not open shader cache path for writing: {}", shader_cache_path.string());
                    }
                    const auto& spirv = context_managed_shader_module->spirv;
                    file.write((char*)spirv.data(), spirv.size() * sizeof(spirv[0]));
                });
        }
        */

        return context_managed_shader_module;
    }

    Expected<CMShared<RenderPass>> Context::CreateRenderPass(const RenderPassCreateInfo& render_pass_create_info)
    {
        auto create_info = render_pass_create_info.CreateVulkanRenderPassCreateInfo();
        AssignOrReturnVkError(auto vulkan_render_pass, device.createRenderPass(create_info));

        auto render_pass = MakeContextManaged<RenderPass>(vulkan_render_pass, render_pass_create_info);
        ReturnIfError(disk_cacher.Record(render_pass));

        return render_pass;
    }

    Expected<CMShared<PipelineLayoutInfo>> Context::CreatePipelineLayoutInfo(const PipelineLayoutCreateInfo& pipeline_layout_create_info)
    {
        auto pipeline_layout_info = MakeContextManaged<PipelineLayoutInfo>();
        std::vector<RayTracingShaderGroupCreateInfoWithShaderStageFlags> raytracing_shader_group_create_infos{};

        for (const auto& shader_module_create_info : pipeline_layout_create_info.shader_module_create_infos)
        {
            //CMShared<ShaderModule> shader_module = shader_module_manager.LoadShader(shader_module_create_info);
            AssignOrReturnError(CMShared<ShaderModule> shader_module, Get(shader_module_create_info));
            const auto& shader_reflection = shader_module->shader_reflection;

            auto stage = shader_module->GetShaderStageFlagBits();
            auto pipeline_shader_stage_create_info = vk::PipelineShaderStageCreateInfo{}
                .setStage(stage)
                .setModule(shader_module->shader_module)
                .setPName(Constants::DEFAULT_SHADER_MAIN.data());

            pipeline_layout_info->pipeline_shader_stages.emplace_back(pipeline_shader_stage_create_info);

            ReturnIfError(pipeline_layout_info->combined_shader_reflection.Merge(shader_reflection));
            pipeline_layout_info->shader_modules.emplace_back(shader_module);

            // Raytracing
            auto shader_index = static_cast<uint32_t>(pipeline_layout_info->shader_modules.size() - 1);
            if (stage & vk::ShaderStageFlagBits::eRaygenKHR || stage & vk::ShaderStageFlagBits::eMissKHR)
            {
                auto raytracing_shader_group_create_info = vk::RayTracingShaderGroupCreateInfoKHR{}
                    .setType(vk::RayTracingShaderGroupTypeKHR::eGeneral)
                    .setGeneralShader(shader_index)
                    .setClosestHitShader(VK_SHADER_UNUSED_KHR)
                    .setAnyHitShader(VK_SHADER_UNUSED_KHR)
                    .setIntersectionShader(VK_SHADER_UNUSED_KHR);

                raytracing_shader_group_create_infos.emplace_back(RayTracingShaderGroupCreateInfoWithShaderStageFlags{ raytracing_shader_group_create_info, stage});
            }
            else if (stage & vk::ShaderStageFlagBits::eClosestHitKHR)
            {
                auto raytracing_shader_group_create_info = vk::RayTracingShaderGroupCreateInfoKHR{}
                    .setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup)
                    .setGeneralShader(VK_SHADER_UNUSED_KHR)
                    .setClosestHitShader(shader_index)
                    .setAnyHitShader(VK_SHADER_UNUSED_KHR)
                    .setIntersectionShader(VK_SHADER_UNUSED_KHR);

                raytracing_shader_group_create_infos.emplace_back(RayTracingShaderGroupCreateInfoWithShaderStageFlags{ raytracing_shader_group_create_info, stage });
            }
            else if (stage & vk::ShaderStageFlagBits::eAnyHitKHR)
            {
                auto raytracing_shader_group_create_info = vk::RayTracingShaderGroupCreateInfoKHR{}
                    .setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup)
                    .setGeneralShader(VK_SHADER_UNUSED_KHR)
                    .setClosestHitShader(VK_SHADER_UNUSED_KHR)
                    .setAnyHitShader(shader_index)
                    .setIntersectionShader(VK_SHADER_UNUSED_KHR);

                raytracing_shader_group_create_infos.emplace_back(RayTracingShaderGroupCreateInfoWithShaderStageFlags{ raytracing_shader_group_create_info, stage });
            }
            else if (stage & vk::ShaderStageFlagBits::eIntersectionKHR)
            {
                auto raytracing_shader_group_create_info = vk::RayTracingShaderGroupCreateInfoKHR{}
                    .setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup)
                    .setGeneralShader(VK_SHADER_UNUSED_KHR)
                    .setClosestHitShader(VK_SHADER_UNUSED_KHR)
                    .setAnyHitShader(VK_SHADER_UNUSED_KHR)
                    .setIntersectionShader(shader_index);

                raytracing_shader_group_create_infos.emplace_back(RayTracingShaderGroupCreateInfoWithShaderStageFlags{ raytracing_shader_group_create_info, stage });
            }
        }

        if (!raytracing_shader_group_create_infos.empty())
        {
            pipeline_layout_info->raytracing_shader_group_create_infos = raytracing_shader_group_create_infos;
        }

        DescriptorSetLayoutCreateInfos descriptor_set_layout_create_infos{};
        AssignOrReturnError(descriptor_set_layout_create_infos, pipeline_layout_create_info.BuildDescriptorSetLayoutCreateInfos(pipeline_layout_info->combined_shader_reflection));

        const auto& push_constant_ranges = pipeline_layout_info->combined_shader_reflection.push_constant_ranges;
        std::transform(std::begin(push_constant_ranges), std::end(push_constant_ranges), std::back_inserter(pipeline_layout_info->push_constant_ranges), [](const VkPushConstantRange& range) { return range; });

        AssignOrReturnError(pipeline_layout_info->descriptor_set_layouts, Get(descriptor_set_layout_create_infos));

        return pipeline_layout_info;
    }

    Expected<CMShared<PipelineLayout>> Context::CreatePipelineLayout(CMShared<PipelineLayoutInfo> pipeline_layout_info)
    {
        auto pipeline_layout_create_info = pipeline_layout_info->GetPipelineLayoutCreateInfo();
        /*
        for (auto i = 0; i < pipeline_layout_create_info.pushConstantRangeCount; i++)
        {
            auto& pp = ((vk::PushConstantRange*)pipeline_layout_create_info.pPushConstantRanges)[i];
            pp.stageFlags = vk::ShaderStageFlags(vk::ShaderStageFlagBits::eAll);
        }
        auto pcps = std::vector<vk::PushConstantRange>(pipeline_layout_create_info.setLayoutCount);
        if (pipeline_layout_create_info.pushConstantRangeCount)
        {
            for (auto& p : pcps)
            {
                p = pipeline_layout_create_info.pPushConstantRanges[0];
            }
            pipeline_layout_create_info.setPushConstantRanges(pcps);
        }
        */

        AssignOrReturnVkError(auto vk_pipeline_layout, device.createPipelineLayout(pipeline_layout_create_info));

        auto pipeline_layout = MakeContextManaged<PipelineLayout>(vk_pipeline_layout, pipeline_layout_info);
        ReturnIfError(disk_cacher.Record(pipeline_layout));

        return pipeline_layout;
    }

    Expected<CMShared<GraphicsPipeline>> Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& graphics_pipeline_create_info)
    {
        const auto& pipeline_layout_info = graphics_pipeline_create_info.pipeline_layout->pipeline_layout_info;
        const auto& combined_shader_reflection = pipeline_layout_info->combined_shader_reflection;
        const auto& shader_modules = pipeline_layout_info->shader_modules;
        const auto& stages = pipeline_layout_info->pipeline_shader_stages;

        auto viewports = std::vector<vk::Viewport>{ graphics_pipeline_create_info.viewport };
        auto scissors = std::vector<vk::Rect2D>{ graphics_pipeline_create_info.scissor };
        auto pipeline_viewport_state_create_info = vk::PipelineViewportStateCreateInfo{}
            .setViewports(viewports)
            .setScissors(scissors)
            .setPNext(graphics_pipeline_create_info.pipeline_viewport_state_create_info_next);

        auto vulkan_graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo{}
            .setStages(stages)
            .setPVertexInputState(&graphics_pipeline_create_info.pipeline_vertex_input_state_create_info)
            .setPInputAssemblyState(&graphics_pipeline_create_info.pipeline_input_assembly_state_create_info)
            .setPTessellationState(&graphics_pipeline_create_info.pipeline_tessellation_state_create_info)
            .setPViewportState(&pipeline_viewport_state_create_info)
            .setPRasterizationState(&graphics_pipeline_create_info.pipeline_rasterization_state_create_info)
            .setPMultisampleState(&graphics_pipeline_create_info.pipeline_multisample_state_create_info)
            .setPDepthStencilState(&graphics_pipeline_create_info.pipeline_depth_stencil_state_create_info)
            .setPColorBlendState(&graphics_pipeline_create_info.pipeline_color_blend_state_create_info)
            .setPDynamicState(&graphics_pipeline_create_info.pipeline_dynamic_state_create_info)
            .setLayout(graphics_pipeline_create_info.pipeline_layout->pipeline_layout)
            .setRenderPass(graphics_pipeline_create_info.render_pass->render_pass)
            .setBasePipelineHandle(VK_NULL_HANDLE);

        auto vulkan_graphics_pipeline_result = device.createGraphicsPipeline(graphics_pipeline_create_info.pipeline_cache, vulkan_graphics_pipeline_create_info);
        if (vulkan_graphics_pipeline_result.result != vk::Result::eSuccess)
        {
            return StringError("{}", vk::to_string(vulkan_graphics_pipeline_result.result));
        }

        auto graphics_pipeline = MakeContextManaged<GraphicsPipeline>(vulkan_graphics_pipeline_result.value, graphics_pipeline_create_info);
        ReturnIfError(disk_cacher.Record(graphics_pipeline));
        return graphics_pipeline;
    }

    Expected<CMShared<ComputePipeline>> Context::CreateComputePipeline(const ComputePipelineCreateInfo& compute_pipeline_create_info)
    {
        const auto& pipeline_layout_info = compute_pipeline_create_info.pipeline_layout->pipeline_layout_info;
        const auto& stages = pipeline_layout_info->pipeline_shader_stages;

        if (stages.size() != 1)
        {
            return StringError("Compute pipeline creation expected stages to be 1 when it is actually {}", stages.size());
        }

        auto vulkan_compute_pipeline_create_info = vk::ComputePipelineCreateInfo{}
            .setStage(stages[0])
            .setLayout(compute_pipeline_create_info.pipeline_layout->pipeline_layout);

        auto vulkan_compute_pipeline_result = device.createComputePipeline(compute_pipeline_create_info.pipeline_cache, vulkan_compute_pipeline_create_info);
        if (vulkan_compute_pipeline_result.result != vk::Result::eSuccess)
        {
            return StringError("{}", vk::to_string(vulkan_compute_pipeline_result.result));
        }

        auto compute_pipeline = MakeContextManaged<ComputePipeline>(vulkan_compute_pipeline_result.value, compute_pipeline_create_info);
        ReturnIfError(disk_cacher.Record(compute_pipeline));
        return compute_pipeline;
    }

    Expected<CMShared<RaytracingPipeline>> Context::CreateRaytracingPipeline(const RaytracingPipelineCreateInfo& raytracing_pipeline_create_info)
    {
        const auto& pipeline_layout_info = raytracing_pipeline_create_info.pipeline_layout->pipeline_layout_info;
        const auto& stages = pipeline_layout_info->pipeline_shader_stages;
        AssignOrReturnError(auto raytracing_shader_group_create_infos, pipeline_layout_info->GetRaytracingShaderGroupCreateInfos());

        auto vulkan_raytracing_pipeline_create_info = vk::RayTracingPipelineCreateInfoKHR{}
            .setStages(stages)
            .setGroups(raytracing_shader_group_create_infos)
            .setMaxPipelineRayRecursionDepth(raytracing_pipeline_create_info.recursion_depth)
            .setLayout(raytracing_pipeline_create_info.pipeline_layout->pipeline_layout);

        auto vulkan_raytracing_pipeline_result = device.createRayTracingPipelineKHR(raytracing_pipeline_create_info.deferred_operation, raytracing_pipeline_create_info.pipeline_cache, vulkan_raytracing_pipeline_create_info);
        if (vulkan_raytracing_pipeline_result.result != vk::Result::eSuccess)
        {
            return StringError("{}", vk::to_string(vulkan_raytracing_pipeline_result.result));
        }

        auto raytracing_pipeline = MakeContextManaged<RaytracingPipeline>(vulkan_raytracing_pipeline_result.value, raytracing_pipeline_create_info);
        ReturnIfError(disk_cacher.Record(raytracing_pipeline));
        return raytracing_pipeline;
    }

    Expected<uint32_t> Context::GetQueueFamilyIndex(QueueType queue_type)
    {
        vkb::QueueType vkb_queue_type{};
        switch (queue_type)
        {
        case QueueType::Graphics:
            vkb_queue_type = vkb::QueueType::graphics;
            break;
        case QueueType::Compute:
            vkb_queue_type = vkb::QueueType::compute;
            break;
        case QueueType::Transfer:
            vkb_queue_type = vkb::QueueType::transfer;
            break;
        case QueueType::Present:
            vkb_queue_type = vkb::QueueType::present;
            break;
        default:
            return StringError("Not possible!");
        }
        auto queue_index_ret = vkb_device.get_queue_index(vkb_queue_type);
        if (!queue_index_ret) {
            return StringError("Failed to get queue. Error: {}", queue_index_ret.error().message());
        }
        auto queue_index = queue_index_ret.value();

        return queue_index;
    }

    Expected<CMShared<CommandPool>> Context::CreateCommandPool(const CommandPoolCreateInfo& command_pool_create_info)
    {
        AssignOrReturnError(uint32_t queue_family_index, GetQueueFamilyIndex(command_pool_create_info.queue_type));

        auto vk_commmand_pool_create_info = vk::CommandPoolCreateInfo{}
            .setQueueFamilyIndex(queue_family_index)
            .setFlags(command_pool_create_info.flags);

        AssignOrReturnVkError(auto vk_command_pool, device.createCommandPool(vk_commmand_pool_create_info));

        auto command_pool = MakeContextManaged<CommandPool>(vk_command_pool, command_pool_create_info);
        return command_pool;
    }

    Expected<CMShared<CommandBuffer>> Context::CreateCommandBuffer(const CommandBufferCreateInfo& command_buffer_create_info)
    {
        auto command_buffer_allocate_info = vk::CommandBufferAllocateInfo{}
            .setCommandPool(command_buffer_create_info.command_pool->command_pool)
            .setLevel(command_buffer_create_info.command_buffer_level)
            .setCommandBufferCount(1);

        AssignOrReturnVkError(auto vk_command_buffer, device.allocateCommandBuffers(command_buffer_allocate_info));
        auto command_buffer = MakeContextManaged<CommandBuffer>(vk_command_buffer[0], command_buffer_create_info);
        return command_buffer;
    }

    Expected<CMShared<Fence>> Context::CreateFence(const FenceCreateInfo& fence_create_info)
    {
        AssignOrReturnVkError(auto vk_fence, device.createFence(fence_create_info.fence_create_info));
        auto fence = MakeContextManaged<Fence>(vk_fence);
        return fence;
    }

    Expected<CMShared<Semaphore>> Context::CreateSemaphore(const SemaphoreCreateInfo& semaphore_create_info)
    {
        AssignOrReturnVkError(auto vk_semaphore, device.createSemaphore(semaphore_create_info.semaphore_create_info));

        auto value = Constants::DEFAULT_SEMAPHORE_INITIAL_VALUE;
        const auto* vk_semphore_type_create_info = reinterpret_cast<const vk::SemaphoreTypeCreateInfo*>(semaphore_create_info.semaphore_create_info.pNext);
        vk::SemaphoreType semaphore_type{};
        if (vk_semphore_type_create_info)
        {
            value = vk_semphore_type_create_info->initialValue;
            semaphore_type = vk_semphore_type_create_info->semaphoreType;
        }
        auto semaphore = MakeContextManaged<Semaphore>(vk_semaphore, value, semaphore_type);
        return semaphore;
    }

    Expected<CMShared<QueryPool>> Context::CreateQueryPool(const QueryPoolCreateInfo& query_pool_create_info)
    {
        AssignOrReturnVkError(auto vk_query_pool, device.createQueryPool(query_pool_create_info.query_pool_create_info));
        auto query_pool = MakeContextManaged<QueryPool>(vk_query_pool);
        return query_pool;
    }

    Expected<AccelerationStructureWithBuffer> Context::CreateAccelerationStructureWithBuffer(vk::AccelerationStructureTypeKHR acceleration_structure_type, vk::DeviceSize size)
    {
        AssignOrReturnError(auto acceleration_structure_buffer, GetRenderFrame().CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR, size));

        auto acceleration_structure_create_info = vk::AccelerationStructureCreateInfoKHR()
            .setType(acceleration_structure_type)
            .setSize(size)
            .setBuffer(*acceleration_structure_buffer)
            .setOffset(0);

        AssignOrReturnVkError(auto acceleration_structure, device.createAccelerationStructureKHR(acceleration_structure_create_info));

        auto acceleration_structure_device_address_info = vk::AccelerationStructureDeviceAddressInfoKHR{}
            .setAccelerationStructure(acceleration_structure);
        auto acceleration_structure_address = device.getAccelerationStructureAddressKHR(acceleration_structure_device_address_info);

        auto acceleration_structure_with_buffer = AccelerationStructureWithBuffer{ acceleration_structure_buffer, acceleration_structure, acceleration_structure_address };
        return acceleration_structure_with_buffer;
    }

    Expected<TransferJob> Context::CreateBottomLevelAccelerationStructure(const BottomLevelAccelerationStructureCreateInfo& blas_create_info)
    {
        auto& render_frame = GetRenderFrame();

        CMShared<BottomLevelAccelerationStructure> blas = MakeContextManaged<BottomLevelAccelerationStructure>();
        blas->create_info = blas_create_info;

        auto build_flags = blas_create_info.build_flags;

        auto num_geometries = blas_create_info.geometry_descriptions.size();
        std::vector<vk::AccelerationStructureGeometryKHR> geometries(num_geometries);
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> build_range_infos(num_geometries);
        std::vector<uint32_t> primitive_counts(num_geometries);

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, render_frame.GetCommandBuffer(QueueType::Compute));
        SetName(command_buffer, "Create blas command buffer");

        if (num_geometries != 1)
        {
            return StringError("Expected 1 geometry...");
        }

        for (auto i = 0; i < num_geometries; i++)
        {
            const auto& geometry_description = blas_create_info.geometry_descriptions[i];

            auto geometry_data = geometry_description.GetGeometryTrianglesData();
            geometries[i] = geometry_data;

            auto primitive_count = 0;
            if (geometry_description.index_buffer.index_type == vk::IndexType::eNoneNV)
            {
                primitive_count = geometry_description.vertex_buffer.count / 3;
            }
            else
            {
                primitive_count = geometry_description.index_buffer.count / 3;
            }
            
            primitive_counts[i] = primitive_count;
         
            auto build_range_info = vk::AccelerationStructureBuildRangeInfoKHR{}
                .setFirstVertex(0)
                .setPrimitiveCount(primitive_count)
                .setPrimitiveOffset(0)
                .setTransformOffset(0);

            build_range_infos[i] = build_range_info;

            command_buffer->AddDependencyTo(geometry_description.vertex_buffer.buffer);
            command_buffer->AddDependencyTo(geometry_description.index_buffer.buffer);
        }

        auto acceleration_structure_geometry_build_info = vk::AccelerationStructureBuildGeometryInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
            .setGeometries(geometries)
            .setFlags(build_flags);
        
        blas->build_sizes = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, acceleration_structure_geometry_build_info, primitive_counts);
        AssignOrReturnError(blas->acceleration_structure_with_buffer, CreateAccelerationStructureWithBuffer(vk::AccelerationStructureTypeKHR::eBottomLevel, blas->build_sizes.accelerationStructureSize));
        
        auto scratch_size = blas->build_sizes.buildScratchSize;
        AssignOrReturnError(auto scratch_buffer, render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress, scratch_size));

        acceleration_structure_geometry_build_info
            .setScratchData(vk::DeviceOrHostAddressKHR{}.setDeviceAddress(scratch_buffer->GetAddress()))
            .setDstAccelerationStructure(blas->acceleration_structure_with_buffer.acceleration_structure);

        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, render_frame.GetBinarySemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, render_frame.GetFence());

        SetName(upload_semaphore, "Create blas upload semaphore");
        SetName(upload_fence, "Create blas upload fence");

        if (build_flags & vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction)
        {
            blas->request_compaction = true;
            blas->compacted = false;

            /*
            auto query_pool_create_info = QueryPoolCreateInfo
            {
                vk::QueryPoolCreateInfo{}
                    .setQueryType(vk::QueryType::eAccelerationStructureCompactedSizeKHR)
                    .setQueryCount(blas->query_count)
            };

            AssignOrReturnError(blas->query_pool, Get(query_pool_create_info));
            device.resetQueryPool(*blas->query_pool, blas->first_query, blas->query_count);
            */
        }

        command_buffer->AddDependencyTo(scratch_buffer);
        command_buffer->AddDependencyTo(blas);

        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(profiler, command_buffer, "Build bottom acceleration structure");
                const vk::CommandBuffer& cb = *command_buffer;
                cb.buildAccelerationStructuresKHR(acceleration_structure_geometry_build_info, build_range_infos.data());

                /*
                if (blas->request_compaction)
                {
                    cb.writeAccelerationStructuresPropertiesKHR(blas->acceleration_structure_with_buffer.acceleration_structure, vk::QueryType::eAccelerationStructureCompactedSizeKHR, blas->query_pool->query_pool, blas->first_query);
                }
                */
                CollectGPUProfile(profiler, command_buffer);
            });
        ReturnIfError(Submit(QueueType::Compute, command_buffer, upload_fence, {}, {}, upload_semaphore));

        FrameCounter frame_counter = GetFrame();
        TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, {}, {}, {}, {}, {}, {}, blas, {} };
        return transfer_job;
    }

    Expected<TransferJob> Context::UpdateBottomLevelAccelerationStructure(const BottomLevelAccelerationStructureUpdateInfo& blas_update_info)
    {
        auto& render_frame = GetRenderFrame();
        auto build_flags = blas_update_info.build_flags;
        auto blas = blas_update_info.blas;

        if (!(build_flags & vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate))
        {
            return StringError("Expected update build flag");
        }

        auto num_geometries = blas_update_info.geometry_descriptions.size();
        std::vector<vk::AccelerationStructureGeometryKHR> geometries(num_geometries);
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> build_range_infos(num_geometries);
        std::vector<uint32_t> primitive_counts(num_geometries);

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, render_frame.GetCommandBuffer(QueueType::Compute));
        SetName(command_buffer, "Update blas command buffer");

        if (num_geometries != 1)
        {
            return StringError("Expected 1 geometry...");
        }

        for (auto i = 0; i < num_geometries; i++)
        {
            const auto& geometry_description = blas_update_info.geometry_descriptions[i];

            auto geometry_data = geometry_description.GetGeometryTrianglesData();
            geometries[i] = geometry_data;

            auto primitive_count = 0;
            if (geometry_description.index_buffer.index_type == vk::IndexType::eNoneNV)
            {
                primitive_count = geometry_description.vertex_buffer.count / 3;
            }
            else
            {
                primitive_count = geometry_description.index_buffer.count / 3;
            }

            primitive_counts[i] = primitive_count;

            auto build_range_info = vk::AccelerationStructureBuildRangeInfoKHR{}
                .setFirstVertex(0)
                .setPrimitiveCount(primitive_count)
                .setPrimitiveOffset(0)
                .setTransformOffset(0);

            build_range_infos[i] = build_range_info;

            command_buffer->AddDependencyTo(geometry_description.vertex_buffer.buffer);
            command_buffer->AddDependencyTo(geometry_description.index_buffer.buffer);
        }

        auto acceleration_structure_geometry_build_info = vk::AccelerationStructureBuildGeometryInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setMode(vk::BuildAccelerationStructureModeKHR::eUpdate)
            .setGeometries(geometries)
            .setFlags(build_flags);

        blas->build_sizes = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, acceleration_structure_geometry_build_info, primitive_counts);

        auto scratch_size = blas->build_sizes.updateScratchSize;
        AssignOrReturnError(auto scratch_buffer, render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress, scratch_size));

        if (!blas->acceleration_structure_with_buffer.acceleration_structure)
        {
            return StringError("Expected update to have existing acceleration structure");
        }
        acceleration_structure_geometry_build_info
            .setScratchData(vk::DeviceOrHostAddressKHR{}.setDeviceAddress(scratch_buffer->GetAddress()))
            .setSrcAccelerationStructure(blas->acceleration_structure_with_buffer.acceleration_structure)
            .setDstAccelerationStructure(blas->acceleration_structure_with_buffer.acceleration_structure);

        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, render_frame.GetSemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, render_frame.GetFence());

        SetName(upload_semaphore, "Update blas upload semaphore");
        SetName(upload_fence, "Update blas upload fence");

        command_buffer->AddDependencyTo(scratch_buffer);
        command_buffer->AddDependencyTo(blas);

        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(profiler, command_buffer, "Update bottom acceleration structure");
                const vk::CommandBuffer& cb = *command_buffer;
                cb.buildAccelerationStructuresKHR(acceleration_structure_geometry_build_info, build_range_infos.data());
                CollectGPUProfile(profiler, command_buffer);
            });
        ReturnIfError(Submit(QueueType::Compute, command_buffer, upload_fence, {}, {}, upload_semaphore));

        FrameCounter frame_counter = GetFrame();
        TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, {}, {}, {}, {}, {}, {}, blas, {} };
        return transfer_job;
    }

    Expected<TransferJob> Context::CompactBottomLevelAccelerationStructure(const TransferJob& blas_transfer_job)
    {
        auto& render_frame = GetRenderFrame();

        CMShared<BottomLevelAccelerationStructure> blas = blas_transfer_job.blas;
        if (blas && blas->request_compaction && !blas->compacted)
        {
            blas->request_compaction = true;
            blas->compacted = true;

            ReturnIfError(WaitForFence(blas_transfer_job.fence));

            AssignOrReturnError(CMShared<CommandBuffer> command_buffer, render_frame.GetCommandBuffer(QueueType::Compute));
            AssignOrReturnError(CMShared<Semaphore> upload_semaphore, render_frame.GetSemaphore());
            AssignOrReturnError(CMShared<Fence> upload_fence, render_frame.GetFence());

            SetName(command_buffer, "Compact blas upload command buffer");
            SetName(upload_semaphore, "Compact blas upload semaphore");
            SetName(upload_fence, "Compact blas upload fence");

            command_buffer->AddDependencyTo(blas);

            auto query_pool_create_info = QueryPoolCreateInfo
            {
                vk::QueryPoolCreateInfo{}
                    .setQueryType(vk::QueryType::eAccelerationStructureCompactedSizeKHR)
                    .setQueryCount(blas->query_count)
            };

            AssignOrReturnError(auto query_pool, Get(query_pool_create_info));
            SetName(command_buffer, "Compact blas query pool");
            device.resetQueryPool(*query_pool, blas->first_query, blas->query_count);

            command_buffer->Record([=](CommandBuffer& command_buffer)
                {
                    ScopedGPUProfile(profiler, command_buffer, "Compact bottom acceleration structure write acceleration structure");
                    const vk::CommandBuffer& cb = *command_buffer;
                    cb.writeAccelerationStructuresPropertiesKHR(blas->acceleration_structure_with_buffer.acceleration_structure, vk::QueryType::eAccelerationStructureCompactedSizeKHR, query_pool->query_pool, blas->first_query);
                    CollectGPUProfile(profiler, command_buffer);
                });
            ReturnIfError(Submit(QueueType::Compute, command_buffer, upload_fence, {}, {}, upload_semaphore));

            using QueryPoolType = vk::DeviceSize;
            auto stride = sizeof(QueryPoolType);
            AssignOrReturnVkError(auto compact_sizes, device.getQueryPoolResults<QueryPoolType>(*query_pool, blas->first_query, blas->query_count, blas->query_count * stride, stride, vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::e64));

            auto new_compact_size = compact_sizes[0];
            AssignOrReturnError(auto compact_acceleration_structure_with_buffer, CreateAccelerationStructureWithBuffer(vk::AccelerationStructureTypeKHR::eBottomLevel, new_compact_size));

            auto copy_acceleration_structure_info = vk::CopyAccelerationStructureInfoKHR()
                .setMode(vk::CopyAccelerationStructureModeKHR::eCompact)
                .setSrc(blas->acceleration_structure_with_buffer.acceleration_structure)
                .setDst(compact_acceleration_structure_with_buffer.acceleration_structure);

            AssignOrReturnError(CMShared<CommandBuffer> blas_compact_command_buffer, render_frame.GetCommandBuffer(QueueType::Compute));
            AssignOrReturnError(CMShared<Semaphore> blas_compact_semaphore, render_frame.GetSemaphore());
            AssignOrReturnError(CMShared<Fence> blas_compact_fence, render_frame.GetFence());

            SetName(blas_compact_command_buffer, "Compact blas compact command buffer");
            SetName(blas_compact_semaphore, "Compact blas compact semaphore");
            SetName(blas_compact_fence, "Compact blas compact fence");

            blas_compact_fence->AddCompletionTo(blas_compact_command_buffer);
            blas_compact_command_buffer->AddDependencyTo(blas);
            blas_compact_command_buffer->AddDependencyTo(compact_acceleration_structure_with_buffer.buffer);

            blas_compact_command_buffer->Record([=](CommandBuffer& command_buffer)
                {
                    ScopedGPUProfile(profiler, command_buffer, "Compact bottom acceleration structure copy acceleration structure");
                    const vk::CommandBuffer& cb = *command_buffer;
                    cb.copyAccelerationStructureKHR(copy_acceleration_structure_info);
                    CollectGPUProfile(profiler, command_buffer);
                });
            ReturnIfError(Submit(QueueType::Compute, blas_compact_command_buffer, blas_compact_fence, {}, {}, blas_compact_semaphore));

            blas->compact_acceleration_structure_with_buffer = compact_acceleration_structure_with_buffer;

            DEBUG("BLAS compacted Old: {:08X} New: {:08X} New percentage of original size: {}", blas->build_sizes.accelerationStructureSize, new_compact_size, (static_cast<float>(new_compact_size) / static_cast<float>(blas->build_sizes.accelerationStructureSize)));

            FrameCounter frame_counter = GetFrame();
            TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, {}, {}, {}, {}, {}, {}, blas, {}, blas_compact_command_buffer, blas_compact_semaphore, blas_compact_fence };
            return transfer_job;
        }

        return StringError("No compaction requested for acceleration structure");
    }

    Error Context::CleanupCompactBottomLevelAccelerationStructure(const TransferJob& compact_blas_transfer_job)
    {
        CMShared<BottomLevelAccelerationStructure> blas = compact_blas_transfer_job.blas;
        if (blas && blas->request_compaction && blas->compacted)
        {
            //ReturnIfError(WaitForFence(compact_blas_transfer_job.fence));
            if (!compact_blas_transfer_job.blas_compact_fence)
            {
                return StringError("Expected BLAS compact fence");
            }
            ReturnIfError(WaitForFence(compact_blas_transfer_job.blas_compact_fence));

            blas->request_compaction = true;
            blas->compacted = false;

            blas->acceleration_structure_with_buffer.buffer = nullptr;
            device.destroyAccelerationStructureKHR(blas->acceleration_structure_with_buffer.acceleration_structure);
            blas->acceleration_structure_with_buffer = AccelerationStructureWithBuffer{};

            std::swap(blas->acceleration_structure_with_buffer, blas->compact_acceleration_structure_with_buffer);

            return NoError();
        }

        return StringError("No compaction requested for acceleration structure");
    }

    Expected<TransferJob> Context::CreateTopLevelAccelerationStructure(const TopLevelAccelerationStructureCreateInfo& tlas_create_info)
    {
        auto& render_frame = GetRenderFrame();
        CMShared<TopLevelAccelerationStructure> tlas = MakeContextManaged<TopLevelAccelerationStructure>();
        tlas->tlas_create_info = tlas_create_info;

        auto update = tlas_create_info.update;

        const std::vector<vk::AccelerationStructureInstanceKHR>& instances = tlas_create_info.instances;
        auto instances_size = instances.size() * sizeof(instances[0]);
        std::array<uint32_t, 1> primitive_counts = { static_cast<uint32_t>(instances.size()) };

        const auto& existing_instance_buffer = tlas_create_info.existing_instance_buffer;
        if (existing_instance_buffer != nullptr && existing_instance_buffer->GetSize() >= instances_size)
        {
            tlas->instance_buffer = existing_instance_buffer;
        }
        else
        {
            AssignOrReturnError(tlas->instance_buffer, render_frame.CreateBuffer(MemoryUsage::CpuToGpu,
                vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress, instances_size, true));
        }

        ReturnIfError(tlas->instance_buffer->Copy(PointerView(instances)));

        auto instance_address = tlas->instance_buffer->GetAddress();

        auto geometry = vk::AccelerationStructureGeometryKHR()
            .setGeometryType(vk::GeometryTypeKHR::eInstances)
            .setFlags(tlas_create_info.geometry_flags);

        geometry.geometry.setInstances
        (
            vk::AccelerationStructureGeometryInstancesDataKHR()
                .setData(instance_address)
                .setArrayOfPointers(false)
        );

        auto build_acceleration_structure_mode = update ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild;
        auto acceleration_structure_geometry_build_info = vk::AccelerationStructureBuildGeometryInfoKHR{}
            .setFlags(tlas_create_info.build_flags)
            .setGeometries(geometry)
            .setMode(build_acceleration_structure_mode)
            .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
            .setSrcAccelerationStructure(vk::AccelerationStructureKHR{})
            .setDstAccelerationStructure(vk::AccelerationStructureKHR{});

        vk::AccelerationStructureBuildSizesInfoKHR build_sizes = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, acceleration_structure_geometry_build_info, primitive_counts);

        if (!update)
        {
            AssignOrReturnError(tlas->acceleration_structure_with_buffer, CreateAccelerationStructureWithBuffer(vk::AccelerationStructureTypeKHR::eTopLevel, build_sizes.accelerationStructureSize));
        }

        auto scratch_size = update ? build_sizes.updateScratchSize : build_sizes.buildScratchSize;
        AssignOrReturnError(auto scratch_buffer, render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress, scratch_size));

        acceleration_structure_geometry_build_info
            .setScratchData(vk::DeviceOrHostAddressKHR{}.setDeviceAddress(scratch_buffer->GetAddress()))
            .setGeometries(geometry)
            .setFlags(tlas_create_info.build_flags)
            .setDstAccelerationStructure(tlas->acceleration_structure_with_buffer.acceleration_structure);
        
        if (update)
        {
            acceleration_structure_geometry_build_info
                .setSrcAccelerationStructure(tlas->acceleration_structure_with_buffer.acceleration_structure);
        }

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, render_frame.GetCommandBuffer(QueueType::Compute));
        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, render_frame.GetSemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, render_frame.GetFence());

        SetName(command_buffer, "Tlas command buffer");
        SetName(upload_semaphore, "Tlas upload semaphore");
        SetName(upload_fence, "Tlas upload fence");

        command_buffer->AddDependencyTo(scratch_buffer);
        for (const auto& blas : tlas_create_info.blases)
        {
            command_buffer->AddDependencyTo(blas);
        }
        command_buffer->AddDependencyTo(tlas);

        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(profiler, command_buffer, "Compact top acceleration structure build");
                auto& cb = *command_buffer;

                auto build_range_info = vk::AccelerationStructureBuildRangeInfoKHR{}
                    .setFirstVertex(0)
                    .setPrimitiveCount(instances.size())
                    .setPrimitiveOffset(0)
                    .setTransformOffset(0);

                cb.buildAccelerationStructuresKHR(acceleration_structure_geometry_build_info, &build_range_info);
                CollectGPUProfile(profiler, command_buffer);
            });
        ReturnIfError(Submit(QueueType::Compute, command_buffer, upload_fence, {}, {}, upload_semaphore));

        FrameCounter frame_counter = GetFrame();
        TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, {}, {}, {}, {}, {}, {}, {}, tlas };
        return transfer_job;
    }

    Expected<CMShared<BottomLevelAccelerationStructure>> Context::CreateBottomLevelAccelerationStructure(const BottomLevelAccelerationStructureCreateInfo& blas_create_info, CMShared<CommandBuffer> command_buffer)
    {
        auto& render_frame = GetRenderFrame();

        CMShared<BottomLevelAccelerationStructure> blas = MakeContextManaged<BottomLevelAccelerationStructure>();
        blas->create_info = blas_create_info;

        auto build_flags = blas_create_info.build_flags;

        auto num_geometries = blas_create_info.geometry_descriptions.size();
        std::vector<vk::AccelerationStructureGeometryKHR> geometries(num_geometries);
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> build_range_infos(num_geometries);
        std::vector<uint32_t> primitive_counts(num_geometries);

        if (num_geometries != 1)
        {
            return StringError("Expected 1 geometry...");
        }

        for (auto i = 0; i < num_geometries; i++)
        {
            const auto& geometry_description = blas_create_info.geometry_descriptions[i];

            auto geometry_data = geometry_description.GetGeometryTrianglesData();
            geometries[i] = geometry_data;

            auto primitive_count = 0;
            if (geometry_description.index_buffer.index_type == vk::IndexType::eNoneNV)
            {
                primitive_count = geometry_description.vertex_buffer.count / 3;
            }
            else
            {
                primitive_count = geometry_description.index_buffer.count / 3;
            }

            primitive_counts[i] = primitive_count;

            auto build_range_info = vk::AccelerationStructureBuildRangeInfoKHR{}
                .setFirstVertex(0)
                .setPrimitiveCount(primitive_count)
                .setPrimitiveOffset(0)
                .setTransformOffset(0);

            build_range_infos[i] = build_range_info;

            command_buffer->AddDependencyTo(geometry_description.vertex_buffer.buffer);
            command_buffer->AddDependencyTo(geometry_description.index_buffer.buffer);
        }

        auto acceleration_structure_geometry_build_info = vk::AccelerationStructureBuildGeometryInfoKHR()
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
            .setGeometries(geometries)
            .setFlags(build_flags);

        blas->build_sizes = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, acceleration_structure_geometry_build_info, primitive_counts);
        AssignOrReturnError(blas->acceleration_structure_with_buffer, CreateAccelerationStructureWithBuffer(vk::AccelerationStructureTypeKHR::eBottomLevel, blas->build_sizes.accelerationStructureSize));

        auto scratch_size = blas->build_sizes.buildScratchSize;
        AssignOrReturnError(auto scratch_buffer, render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress, scratch_size));

        acceleration_structure_geometry_build_info
            .setScratchData(vk::DeviceOrHostAddressKHR{}.setDeviceAddress(scratch_buffer->GetAddress()))
            .setDstAccelerationStructure(blas->acceleration_structure_with_buffer.acceleration_structure);

        if (build_flags & vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction)
        {
            blas->request_compaction = true;
            blas->compacted = false;
        }

        command_buffer->AddDependencyTo(scratch_buffer);
        command_buffer->AddDependencyTo(blas);

        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(profiler, command_buffer, "Build bottom acceleration structure");
                const vk::CommandBuffer& cb = *command_buffer;
                cb.buildAccelerationStructuresKHR(acceleration_structure_geometry_build_info, build_range_infos.data());

                CollectGPUProfile(profiler, command_buffer);
            });

        return blas;
    }

    Expected<CMShared<TopLevelAccelerationStructure>> Context::CreateTopLevelAccelerationStructure(const TopLevelAccelerationStructureCreateInfo& tlas_create_info, CMShared<CommandBuffer> command_buffer)
    {
        auto& render_frame = GetRenderFrame();
        CMShared<TopLevelAccelerationStructure> tlas = MakeContextManaged<TopLevelAccelerationStructure>();
        tlas->tlas_create_info = tlas_create_info;

        auto update = tlas_create_info.update;

        const std::vector<vk::AccelerationStructureInstanceKHR>& instances = tlas_create_info.instances;
        auto instances_size = instances.size() * sizeof(instances[0]);
        std::array<uint32_t, 1> primitive_counts = { static_cast<uint32_t>(instances.size()) };

        AssignOrReturnError(tlas->instance_buffer, render_frame.CreateBuffer(MemoryUsage::CpuToGpu,
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress, instances_size, true));

        ReturnIfError(tlas->instance_buffer->Copy(PointerView(instances)));

        auto instance_address = tlas->instance_buffer->GetAddress();

        auto geometry = vk::AccelerationStructureGeometryKHR()
            .setGeometryType(vk::GeometryTypeKHR::eInstances)
            .setFlags(tlas_create_info.geometry_flags);

        geometry.geometry.setInstances
        (
            vk::AccelerationStructureGeometryInstancesDataKHR()
            .setData(instance_address)
            .setArrayOfPointers(false)
        );

        auto build_acceleration_structure_mode = update ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild;
        auto acceleration_structure_geometry_build_info = vk::AccelerationStructureBuildGeometryInfoKHR{}
            .setFlags(tlas_create_info.build_flags)
            .setGeometries(geometry)
            .setMode(build_acceleration_structure_mode)
            .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
            .setSrcAccelerationStructure(vk::AccelerationStructureKHR{})
            .setDstAccelerationStructure(vk::AccelerationStructureKHR{});

        vk::AccelerationStructureBuildSizesInfoKHR build_sizes = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, acceleration_structure_geometry_build_info, primitive_counts);

        if (!update)
        {
            AssignOrReturnError(tlas->acceleration_structure_with_buffer, CreateAccelerationStructureWithBuffer(vk::AccelerationStructureTypeKHR::eTopLevel, build_sizes.accelerationStructureSize));
        }

        auto scratch_size = update ? build_sizes.updateScratchSize : build_sizes.buildScratchSize;
        AssignOrReturnError(auto scratch_buffer, render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress, scratch_size));

        acceleration_structure_geometry_build_info
            .setScratchData(vk::DeviceOrHostAddressKHR{}.setDeviceAddress(scratch_buffer->GetAddress()))
            .setGeometries(geometry)
            .setFlags(tlas_create_info.build_flags)
            .setDstAccelerationStructure(tlas->acceleration_structure_with_buffer.acceleration_structure);

        if (update)
        {
            acceleration_structure_geometry_build_info
                .setSrcAccelerationStructure(tlas->acceleration_structure_with_buffer.acceleration_structure);
        }

        command_buffer->AddDependencyTo(scratch_buffer);
        for (const auto& blas : tlas_create_info.blases)
        {
            command_buffer->AddDependencyTo(blas);
        }
        command_buffer->AddDependencyTo(tlas);

        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(profiler, command_buffer, "Build top acceleration structure");
                auto& cb = *command_buffer;

                auto build_range_info = vk::AccelerationStructureBuildRangeInfoKHR{}
                    .setFirstVertex(0)
                    .setPrimitiveCount(instances.size())
                    .setPrimitiveOffset(0)
                    .setTransformOffset(0);

                cb.buildAccelerationStructuresKHR(acceleration_structure_geometry_build_info, &build_range_info);
                CollectGPUProfile(profiler, command_buffer);
            });
        
        return tlas;
    }

    Expected<CMShared<FrameBuffer>> Context::CreateFrameBuffer(const FrameBufferCreateInfo& frame_buffer_create_info)
    {
        std::vector<vk::ImageView> vk_attachments;
        std::transform(std::begin(frame_buffer_create_info.attachments), std::end(frame_buffer_create_info.attachments), std::back_inserter(vk_attachments), [](const auto& e)
            {
                return *e;
            });
        auto vk_frame_buffer_create_info = vk::FramebufferCreateInfo{}
            .setRenderPass(*frame_buffer_create_info.render_pass)
            .setAttachments(vk_attachments)
            .setWidth(frame_buffer_create_info.width)
            .setHeight(frame_buffer_create_info.height)
            .setLayers(frame_buffer_create_info.layers);

        AssignOrReturnVkError(auto vk_frame_buffer, device.createFramebuffer(vk_frame_buffer_create_info));
        auto frame_buffer = MakeContextManaged<FrameBuffer>(vk_frame_buffer, frame_buffer_create_info);
        return frame_buffer;
    }
    
    RenderFrame& Context::GetRenderFrameAt(std::size_t index)
    {
        if (render_frames.empty())
        {
            PANIC("Empty render frames");
        }

        auto& render_frame = render_frames[index];
        return render_frame;
    }

    RenderFrame& Context::GetRenderFrame()
    {
        const auto total_render_frames = render_frames.size();
        const auto frame_index = current_frame % total_render_frames;
        return GetRenderFrameAt(frame_index);
    }

    Error Context::EnqueueDestroy(Buffer image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(Image image)
    {
        return NoError();
    }
    
    Error Context::EnqueueDestroy(ImageView image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(Texture image)
    {
        return NoError();
    }
    
    Error Context::EnqueueDestroy(Sampler image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(DescriptorPool image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(DescriptorSetLayout image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(DescriptorSet image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(ShaderModule image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(RenderPass image)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(PipelineLayout pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(GraphicsPipeline pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(ComputePipeline pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(RaytracingPipeline pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(Semaphore pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(Fence pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(CommandPool pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(CommandBuffer pipeline_layout)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(BottomLevelAccelerationStructure blas)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(TopLevelAccelerationStructure tlas)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(QueryPool query_pool)
    {
        return NoError();
    }

    Error Context::EnqueueDestroy(FrameBuffer frame_buffer)
    {
        return NoError();
    }

    Expected<DescriptorSetLayouts> Context::Get(const DescriptorSetLayoutCreateInfos& descriptor_set_layout_create_info)
    {
        DescriptorSetLayouts descriptor_set_layouts{};

        for (const auto& descriptor_set_layout_create_info : descriptor_set_layout_create_info.GetCreateInfos())
        {
#define MAYBE_DESCRIPTOR_SET_CACHE_GET 1

            if (MAYBE_DESCRIPTOR_SET_CACHE_GET)
            {
                if (auto result = descriptor_set_layout_cache.Get(descriptor_set_layout_create_info))
                {
                    descriptor_set_layouts.descriptor_set_layouts.emplace_back(*result);
                }
                else
                {
                    ConsumeError(result.takeError());

                    AssignOrReturnError(CMShared<DescriptorSetLayout> cm_descriptor_set_layout, CreateDescriptorSetLayout(descriptor_set_layout_create_info));
                    auto& descriptor_set_layout = descriptor_set_layout_cache.Emplace(descriptor_set_layout_create_info, std::move(cm_descriptor_set_layout));
                
                    descriptor_set_layouts.descriptor_set_layouts.emplace_back(descriptor_set_layout);
                }
            }
            else
            {
                AssignOrReturnError(CMShared<DescriptorSetLayout> cm_descriptor_set_layout, CreateDescriptorSetLayout(descriptor_set_layout_create_info));
                descriptor_set_layouts.descriptor_set_layouts.emplace_back(cm_descriptor_set_layout);
            }
        }

        return descriptor_set_layouts;
    }

    Expected<CMShared<PipelineLayoutInfo>> Context::Get(const PipelineLayoutCreateInfo& pipeline_layout_create_info)
    {
        if (auto result = pipeline_layout_info_cache.Get(pipeline_layout_create_info))
        {
            auto& pipeline_layout_info = *result;
            return pipeline_layout_info;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<PipelineLayoutInfo> pipeline_layout_info, CreatePipelineLayoutInfo(pipeline_layout_create_info));
            auto& pipeline_layout_info_ref = pipeline_layout_info_cache.Emplace(pipeline_layout_create_info, std::move(pipeline_layout_info));

            return pipeline_layout_info_ref;
        }
    }

    Expected<CMShared<PipelineLayout>> Context::Get(CMShared<PipelineLayoutInfo> pipeline_layout_info)
    {
        if (auto result = pipeline_layout_cache.Get(pipeline_layout_info))
        {
            auto& pipeline_layout_info = *result;
            return pipeline_layout_info;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<PipelineLayout> cm_pipeline_layout, CreatePipelineLayout(pipeline_layout_info));
            auto& pipeline_layout = pipeline_layout_cache.Emplace(pipeline_layout_info, std::move(cm_pipeline_layout));
            return pipeline_layout;
        }
    }

    Expected<CMShared<GraphicsPipeline>> Context::Get(const GraphicsPipelineCreateInfo& graphics_pipeline_create_info)
    {
        for (const auto& [k, v] : graphics_pipeline_cache)
        {
            const auto& vv = v.t;
            if (vv->pipeline)
            {

            }
            const GraphicsPipelineCreateInfo& gp = k;
            if (gp.pipeline_layout->pipeline_layout_info->pipeline_create_info.flags)
            {
                fmt::print("{}", (uint64_t)k.render_pass->render_pass.operator VkRenderPass());
            }
        }
        if (auto result = graphics_pipeline_cache.Get(graphics_pipeline_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());
            
            AssignOrReturnError(CMShared<GraphicsPipeline> cm_graphics_pipeline, CreateGraphicsPipeline(graphics_pipeline_create_info));
            auto& graphics_pipeline = graphics_pipeline_cache.Emplace(graphics_pipeline_create_info, std::move(cm_graphics_pipeline));
            return graphics_pipeline;
        }
    }

    Expected<CMShared<ComputePipeline>> Context::Get(const ComputePipelineCreateInfo& compute_pipeline_create_info)
    {
        if (auto result = compute_pipeline_cache.Get(compute_pipeline_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<ComputePipeline> cm_compute_pipeline, CreateComputePipeline(compute_pipeline_create_info));
            auto& compute_pipeline = compute_pipeline_cache.Emplace(compute_pipeline_create_info, std::move(cm_compute_pipeline));
            return compute_pipeline;
        }
    }

    Expected<CMShared<RaytracingPipeline>> Context::Get(const RaytracingPipelineCreateInfo& raytracing_pipeline_create_info)
    {
        if (auto result = raytracing_pipeline_cache.Get(raytracing_pipeline_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<RaytracingPipeline> cm_raytracing_pipeline, CreateRaytracingPipeline(raytracing_pipeline_create_info));
            auto& raytracing_pipeline = raytracing_pipeline_cache.Emplace(raytracing_pipeline_create_info, std::move(cm_raytracing_pipeline));
            return raytracing_pipeline;
        }
    }

    Expected<CMShared<RenderPass>> Context::Get(const RenderPassCreateInfo& render_pass_create_info)
    {
        if (auto result = render_pass_cache.Get(render_pass_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<RenderPass> cm_render_pass, CreateRenderPass(render_pass_create_info));
            auto& render_pass = render_pass_cache.Emplace(render_pass_create_info, std::move(cm_render_pass));
            return render_pass;
        }
    }

    Expected<CMShared<ShaderModule>> Context::Get(const ShaderModuleCreateInfo& shader_module_create_info)
    {
        if (auto result = shader_module_cache.Get(shader_module_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<ShaderModule> cm_shader_module, shader_module_manager.LoadShader(shader_module_create_info));
            //AssignOrReturnError(CMShared<ShaderModule> cm_shader_module, CreateShaderModule(shader_module_create_info));
            auto& shader_module = shader_module_cache.Emplace(shader_module_create_info, std::move(cm_shader_module));
            return shader_module;
        }
    }

    Expected<CMShared<DescriptorPool>> Context::Get(const DescriptorPoolCreateInfo& descriptor_pool_create_info)
    {
        if (auto result = descriptor_pool_cache.Get(descriptor_pool_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<DescriptorPool> cm_descriptor_pool, CreateDescriptorPool(descriptor_pool_create_info));
            auto& descriptor_pool = descriptor_pool_cache.Emplace(descriptor_pool_create_info, std::move(cm_descriptor_pool));
            return descriptor_pool;
        }
    }

    Expected<CMShared<Sampler>> Context::Get(const SamplerCreateInfo& sampler_create_info)
    {
        if (auto result = sampler_cache.Get(sampler_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<Sampler> cm_sampler, CreateSampler(sampler_create_info));
            auto& sampler = sampler_cache.Emplace(sampler_create_info, std::move(cm_sampler));
            return sampler;
        }
    }

    Expected<CMShared<QueryPool>> Context::Get(const QueryPoolCreateInfo& query_pool_create_info)
    {
        if (auto result = query_pool_cache.Get(query_pool_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<QueryPool> cm_query_pool, CreateQueryPool(query_pool_create_info));
            auto& query_pool = query_pool_cache.Emplace(query_pool_create_info, std::move(cm_query_pool));
            return query_pool;
        }
    }

    Error Context::Render()
    {
        return NoError();
    }

    Error Context::PrepareFrame(CMShared<Semaphore> acquire_image_semaphore)
    {
        auto acquire_result = device.acquireNextImageKHR(GetSwapchain(), Constants::DEFAULT_FENCE_TIMEOUT_VALUE, *acquire_image_semaphore, nullptr);
        vk::Result result = acquire_result.result;
        if (result != vk::Result::eSuccess)
        {
            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
            {
                ReturnIfError(Resize());
            }
            else
            {
                return StringError("Acquired image error {}", vk::to_string(result));
            }
        }
        next_image_index = acquire_result.value;
        if (next_image_index > GetNumFrames())
        {
            return llvm::make_error<SwapchainOutOfDateError>();
        }

        return NoError();
    }

    uint32_t Context::GetNextFrameIndex() const
    {
        return next_image_index;
    }

    Error Context::Present(CMShared<Semaphore> wait_semaphore, std::optional<uint32_t> image_index)
    {
        auto image_index_ = next_image_index;
        if (image_index)
        {
            image_index_ = *image_index;
        }

        auto swapchain = GetSwapchain();
        auto pi = vk::PresentInfoKHR{}
            .setSwapchains(swapchain)
            .setImageIndices(image_index_);

        if (wait_semaphore)
        {
            pi.setWaitSemaphores(**wait_semaphore);
        }

        vk::Result present_result = GetGraphicsQueue().presentKHR(pi);
        if (present_result != vk::Result::eSuccess)
        {
            if (present_result == vk::Result::eErrorOutOfDateKHR)
            {
                ReturnIfError(Resize());
                return llvm::make_error<SwapchainOutOfDateError>();
            }
            else
            {
                return StringError("Present error {}", vk::to_string(present_result));
            }
        }

        return NoError();
    }

    ShaderModuleManager& Context::GetShaderModuleManager()
    {
        return shader_module_manager;
    }

    ResourceManager& Context::GetResourceManager()
    {
        return resource_manager;
    }

#ifdef ENABLE_PROFILER
    std::shared_ptr<Profiler> Context::GetProfiler()
    {
        return profiler;
    }
#endif

    Error Context::ReleaseUnusedResources()
    {
        // Deallocate any resource unused over x frames
        const static FrameCounter context_release_entries_past_frame_counter_difference = Config["cache"]["context_release_entries_past_frame_counter_difference"];
        descriptor_set_layout_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        pipeline_layout_info_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        pipeline_layout_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        graphics_pipeline_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        compute_pipeline_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        raytracing_pipeline_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        render_pass_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        shader_module_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        descriptor_pool_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        sampler_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);
        query_pool_cache.ClearEntriesPastFrameCounterDifference(context_release_entries_past_frame_counter_difference);

        PanicIfError(resource_manager.Flush());
        return NoError();
    }

    Error Context::EndFrame()
    {
        ReturnIfError(ReleaseUnusedResources());

        profiler->EndFrame();
        return NoError();
    }

    void Context::AdvanceFrame()
    {
        auto current_frame_time_point = std::chrono::high_resolution_clock::now();
        last_frame_duration = std::chrono::duration_cast<std::chrono::microseconds>(current_frame_time_point - last_frame_time_point).count();
        last_frame_time_point = current_frame_time_point;

        current_frame++;
#ifdef ENABLE_PROFILER
        const static bool enable_profiler = Config["profiler"]["enable"];
        if (enable_profiler)
        {
            profiler->SetFrame(current_frame);
        }
#endif
    }

    Error Context::Submit(QueueType queue_type, CMShared<CommandBuffer> command_buffer, CMShared<Fence> fence, std::optional<vk::PipelineStageFlags> wait_dst_stage_mask, CMShared<Semaphore> wait_semaphore, CMShared<Semaphore> signal_semaphore)
    {
        ReturnIfError(Submit(queue_type, { command_buffer }, fence, wait_dst_stage_mask, wait_semaphore, signal_semaphore));
        return NoError();
    }

    Error Context::Submit(QueueType queue_type, std::initializer_list<CMShared<CommandBuffer>> command_buffers, CMShared<Fence> fence, std::optional<vk::PipelineStageFlags> wait_dst_stage_mask, CMShared<Semaphore> wait_semaphore, CMShared<Semaphore> signal_semaphore)
    {
        for (auto& command_buffer : command_buffers)
        {
            command_buffer->End();
        }

        if (fence)
        {
            for (auto& command_buffer : command_buffers)
            {
                fence->AddCompletionTo(command_buffer);
            }
        }

        std::vector<vk::CommandBuffer> vk_command_buffers(command_buffers.size());
        std::transform(std::begin(command_buffers), std::end(command_buffers), std::begin(vk_command_buffers), [](const auto& e) { return **e; });

        auto timeline_semaphore_submit_info = vk::TimelineSemaphoreSubmitInfo{};
        std::vector<uint64_t> signal_values;
        std::vector<uint64_t> wait_values;

        auto submit_info = vk::SubmitInfo{}
            .setCommandBuffers(vk_command_buffers);

        if (wait_dst_stage_mask)
        {
            submit_info.setWaitDstStageMask(*wait_dst_stage_mask);
        }

        if (wait_semaphore)
        {
            submit_info.setWaitSemaphores(**wait_semaphore);
            //if (wait_semaphore->semaphore_type == vk::SemaphoreType::eTimeline)
            {
                wait_values.emplace_back(0);
                timeline_semaphore_submit_info.setWaitSemaphoreValues(wait_values);
            }
        }

        if (signal_semaphore)
        {
            submit_info.setSignalSemaphores(**signal_semaphore);
            //if (signal_semaphore->semaphore_type == vk::SemaphoreType::eTimeline)
            {
                auto signal_value = signal_semaphore->value + 1;
                signal_values.emplace_back(signal_value);
                timeline_semaphore_submit_info.setSignalSemaphoreValues(signal_values);
            }
        }

        if (timeline_semaphore_submit_info.pWaitSemaphoreValues || timeline_semaphore_submit_info.pSignalSemaphoreValues)
        {
            submit_info.setPNext(&timeline_semaphore_submit_info);
        }

        ReturnIfError(Submit(queue_type, submit_info, fence));
        return NoError();
    }

    Error Context::Submit(QueueType queue_type, std::vector<CMShared<CommandBuffer>> command_buffers, CMShared<Fence> fence, std::vector<vk::PipelineStageFlags> wait_dst_stage_masks, std::vector<CMShared<Semaphore>> wait_semaphores, std::vector<CMShared<Semaphore>> signal_semaphores)
    {
        for (auto& command_buffer : command_buffers)
        {
            command_buffer->End();
        }

        if (fence)
        {
            for (auto& command_buffer : command_buffers)
            {
                fence->AddCompletionTo(command_buffer);
            }
        }

        std::vector<vk::CommandBuffer> vk_command_buffers(command_buffers.size());
        std::transform(std::begin(command_buffers), std::end(command_buffers), std::begin(vk_command_buffers), [](const auto& e) { return **e; });

        std::vector<vk::Semaphore> vk_wait_semaphores(wait_semaphores.size());
        std::transform(std::begin(wait_semaphores), std::end(wait_semaphores), std::begin(vk_wait_semaphores), [](const auto& e) { return **e; });

        std::vector<vk::Semaphore> vk_signal_semaphores(signal_semaphores.size());
        std::transform(std::begin(signal_semaphores), std::end(signal_semaphores), std::begin(vk_signal_semaphores), [](const auto& e) { return **e; });

        auto submit_info = vk::SubmitInfo{}
            .setCommandBuffers(vk_command_buffers)
            .setWaitDstStageMask(wait_dst_stage_masks)
            .setWaitSemaphores(vk_wait_semaphores)
            .setSignalSemaphores(vk_signal_semaphores);

        auto timeline_semaphore_submit_info = vk::TimelineSemaphoreSubmitInfo{};
        std::vector<uint64_t> wait_values;
        std::vector<uint64_t> signal_values;

        for(const auto& wait_semaphore : wait_semaphores)
        {
            //if (wait_semaphore->semaphore_type == vk::SemaphoreType::eTimeline)
            {
                wait_values.emplace_back(0);
            }
        }

        for (const auto& signal_semaphore : signal_semaphores)
        {
            //if (signal_semaphore->semaphore_type == vk::SemaphoreType::eTimeline)
            {
                auto signal_value = signal_semaphore->value + 1;
                signal_values.emplace_back(signal_value);
            }
        }

        if (!wait_values.empty())
        {
            timeline_semaphore_submit_info.setWaitSemaphoreValues(wait_values);
        }

        if (!signal_values.empty())
        {
            timeline_semaphore_submit_info.setSignalSemaphoreValues(signal_values);
        }

        if (timeline_semaphore_submit_info.pWaitSemaphoreValues || timeline_semaphore_submit_info.pSignalSemaphoreValues)
        {
            submit_info.setPNext(&timeline_semaphore_submit_info);
        }

        ReturnIfError(Submit(queue_type, submit_info, fence));
        return NoError();
    }

    Error Context::Submit(QueueType queue_type, vk::SubmitInfo submit_info, CMShared<Fence> fence)
    {
        vk::Fence vk_fence = {};
        if (fence)
        {
            vk_fence = fence->fence;
        }

        switch (queue_type)
        {
        case QueueType::Present:
        {
            ReturnIfVkError(present_queue.submit(submit_info, vk_fence));
            break;
        }
        case QueueType::Graphics:
        {
            ReturnIfVkError(graphics_queue.submit(submit_info, vk_fence));
            break;
        }
        case QueueType::Compute:
        {
            ReturnIfVkError(compute_queue.submit(submit_info, vk_fence));
            break;
        }
        case QueueType::Transfer:
        {
            ReturnIfVkError(transfer_queue.submit(submit_info, vk_fence));
            break;
        }
        default:
            return StringError("Queue not supported!");
        }

        return NoError();
    }

    Error Context::ResetFence(CMShared<Fence> fence)
    {
        device.resetFences(**fence);
        return NoError();
    }

    Error Context::WaitForFence(CMShared<Fence> fence)
    {
        device.waitForFences(**fence, VK_TRUE, Constants::DEFAULT_FENCE_TIMEOUT_VALUE);
        return NoError();
    }

    Error Context::ImmediateGraphicsSubmit(std::function<void(CommandBuffer& command_buffer)> f)
    {
        auto& render_frame = GetRenderFrame();

        auto command_pool_create_info = CommandPoolCreateInfo{ QueueType::Graphics, vk::CommandPoolCreateFlagBits::eResetCommandBuffer };
        AssignOrReturnError(CMShared<CommandPool> command_pool, render_frame.Get(command_pool_create_info));

        CommandBufferCreateInfo command_buffer_create_info{ command_pool, vk::CommandBufferLevel::ePrimary };
        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, render_frame.Get(command_buffer_create_info));

        command_buffer->RecordAndEnd(f);

        /*
        auto fence_create_info = FenceCreateInfo{};
        AssignOrReturnError(CMShared<Fence> fence, CreateFence(fence_create_info));
        */

        AssignOrReturnError(CMShared<Fence> fence, render_frame.GetFence());

        auto submit_info = vk::SubmitInfo{}
            .setCommandBuffers(command_buffer->command_buffer);

        ReturnIfError(Submit(QueueType::Graphics, submit_info, fence));
        ReturnIfError(WaitForFence(fence));

        return NoError();
    }

    Error Context::ImmediateGraphicsSubmitPtr(std::function<void(CMShared<CommandBuffer> command_buffer)> f)
    {
        auto& render_frame = GetRenderFrame();

        auto command_pool_create_info = CommandPoolCreateInfo{ QueueType::Graphics, vk::CommandPoolCreateFlagBits::eResetCommandBuffer };
        AssignOrReturnError(CMShared<CommandPool> command_pool, render_frame.Get(command_pool_create_info));

        CommandBufferCreateInfo command_buffer_create_info{ command_pool, vk::CommandBufferLevel::ePrimary };
        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, render_frame.Get(command_buffer_create_info));

        command_buffer->Begin();
        f(command_buffer);
        command_buffer->End();

        /*
        auto fence_create_info = FenceCreateInfo{};
        AssignOrReturnError(CMShared<Fence> fence, CreateFence(fence_create_info));
        */

        AssignOrReturnError(CMShared<Fence> fence, render_frame.GetFence());

        auto submit_info = vk::SubmitInfo{}
            .setCommandBuffers(command_buffer->command_buffer);

        ReturnIfError(Submit(QueueType::Graphics, submit_info, fence));
        ReturnIfError(WaitForFence(fence));

        return NoError();
    }

    void Context::SetName(CMShared<Buffer> buffer, std::string_view s) const
    {
        if (buffer)
        {
            SetObjectName(buffer->buffer, s, vk::DebugReportObjectTypeEXT::eBuffer);
        }
    }

    void Context::SetName(CMShared<Image> image, std::string_view s) const
    {
        if (image)
        {
            SetObjectName(image->image, s, vk::DebugReportObjectTypeEXT::eImage);
        }
    }

    void Context::SetName(CMShared<ImageView> image_view, std::string_view s) const
    {
        if (image_view)
        {
            SetObjectName(image_view->image_view, s, vk::DebugReportObjectTypeEXT::eImageView);
        }
    }

    void Context::SetName(CMShared<Texture> texture, std::string_view s) const
    {
        if (texture)
        {
            SetName(texture->image, fmt::format("{} image", s));
            SetName(texture->image_view, fmt::format("{} image view", s));
            SetName(texture->sampler, fmt::format("{} sampler", s));
        }
    }

    void Context::SetName(CMShared<Sampler> sampler, std::string_view s) const
    {
        if (sampler)
        {
            SetObjectName(sampler->sampler, s, vk::DebugReportObjectTypeEXT::eSampler);
        }
    }

    void Context::SetName(CMShared<DescriptorPool> descriptor_pool, std::string_view s) const
    {
        if (descriptor_pool)
        {
            SetObjectName(descriptor_pool->descriptor_pool, s, vk::DebugReportObjectTypeEXT::eDescriptorPool);
        }
    }

    void Context::SetName(CMShared<DescriptorSetLayout> descriptor_set_layout, std::string_view s) const
    {
        if (descriptor_set_layout)
        {
            SetObjectName(descriptor_set_layout->descriptor_set_layout, s, vk::DebugReportObjectTypeEXT::eDescriptorSetLayout);
        }
    }

    void Context::SetName(CMShared<DescriptorSet> descriptor_set, std::string_view s) const
    {
        if (descriptor_set)
        {
            const auto& descriptor_sets = descriptor_set->descriptor_sets;
            for (auto i = 0; i < descriptor_sets.size(); i++)
            {
                auto& ds = descriptor_sets[i];
                SetObjectName(ds, fmt::format("{} {}", s, i), vk::DebugReportObjectTypeEXT::eDescriptorSet);
            }
        }
    }

    void Context::SetName(CMShared<ShaderModule> shader_module, std::string_view s) const
    {
        if (shader_module)
        {
            SetObjectName(shader_module->shader_module, s, vk::DebugReportObjectTypeEXT::eShaderModule);
        }
    }

    void Context::SetName(CMShared<RenderPass> render_pass, std::string_view s) const
    {
        if (render_pass)
        {
            SetObjectName(render_pass->render_pass, s, vk::DebugReportObjectTypeEXT::eRenderPass);
        }
    }

    void Context::SetName(CMShared<PipelineLayoutInfo> pipeline_layout_info, std::string_view s) const
    {
        if (pipeline_layout_info)
        {
            const auto& descriptor_set_layouts = pipeline_layout_info->descriptor_set_layouts.descriptor_set_layouts;
            for (auto i = 0; i < descriptor_set_layouts.size(); i++)
            {
                const auto& descriptor_set_layout = descriptor_set_layouts[i];
                SetName(descriptor_set_layout, fmt::format("{} descriptor set layout {}", s, i));
            }
            const auto& shader_modules = pipeline_layout_info->shader_modules;
            for (auto i = 0; i < shader_modules.size(); i++)
            {
                const auto& shader_module = shader_modules[i];
                SetName(shader_module, fmt::format("{} shader module {}", s, i));
            }
        }
    }

    void Context::SetName(CMShared<PipelineLayout> pipeline_layout, std::string_view s) const
    {
        if (pipeline_layout)
        {
            SetObjectName(pipeline_layout->pipeline_layout, s, vk::DebugReportObjectTypeEXT::ePipelineLayout);
        }
    }

    void Context::SetName(CMShared<GraphicsPipeline> graphics_pipeline, std::string_view s) const
    {
        if (graphics_pipeline)
        {
            SetObjectName(graphics_pipeline->pipeline, s, vk::DebugReportObjectTypeEXT::ePipeline);
        }
    }

    void Context::SetName(CMShared<ComputePipeline> compute_pipeline, std::string_view s) const
    {
        if (compute_pipeline)
        {
            SetObjectName(compute_pipeline->pipeline, s, vk::DebugReportObjectTypeEXT::ePipeline);
        }
    }

    void Context::SetName(CMShared<RaytracingPipeline> raytracing_pipeline, std::string_view s) const
    {
        if (raytracing_pipeline)
        {
            SetObjectName(raytracing_pipeline->pipeline, s, vk::DebugReportObjectTypeEXT::ePipeline);
        }
    }

    void Context::SetName(CMShared<Semaphore> semaphore, std::string_view s) const
    {
        if (semaphore)
        {
            SetObjectName(semaphore->semaphore, s, vk::DebugReportObjectTypeEXT::eSemaphore);
        }
    }

    void Context::SetName(CMShared<Fence> fence, std::string_view s) const
    {
        if (fence)
        {
            SetObjectName(fence->fence, s, vk::DebugReportObjectTypeEXT::eFence);
        }
    }

    void Context::SetName(CMShared<CommandPool> command_pool, std::string_view s) const
    {
        if (command_pool)
        {
            SetObjectName(command_pool->command_pool, s, vk::DebugReportObjectTypeEXT::eCommandPool);
        }
    }

    void Context::SetName(CMShared<CommandBuffer> command_buffer, std::string_view s) const
    {
        if (command_buffer)
        {
            SetObjectName(command_buffer->command_buffer, s, vk::DebugReportObjectTypeEXT::eCommandBuffer);
        }
    }

    void Context::SetName(CMShared<BottomLevelAccelerationStructure> blas, std::string_view s) const
    {
        if (blas)
        {
            if (blas->acceleration_structure_with_buffer.acceleration_structure)
            {
                SetObjectName(blas->acceleration_structure_with_buffer.acceleration_structure, s, vk::DebugReportObjectTypeEXT::eAccelerationStructureKHR);
            }
            SetName(blas->acceleration_structure_with_buffer.buffer, s);
            auto compact_name = fmt::format("{} Compact", s);
            if (blas->compact_acceleration_structure_with_buffer.acceleration_structure)
            {
                SetObjectName(blas->compact_acceleration_structure_with_buffer.acceleration_structure, compact_name, vk::DebugReportObjectTypeEXT::eAccelerationStructureKHR);
            }
            SetName(blas->compact_acceleration_structure_with_buffer.buffer, compact_name);
        }
    }

    void Context::SetName(CMShared<TopLevelAccelerationStructure> tlas, std::string_view s) const
    {
        if (tlas)
        {
            if (tlas->acceleration_structure_with_buffer.acceleration_structure)
            {
                SetObjectName(tlas->acceleration_structure_with_buffer.acceleration_structure, s, vk::DebugReportObjectTypeEXT::eAccelerationStructureKHR);
            }
            SetName(tlas->acceleration_structure_with_buffer.buffer, s);
            auto instance_name = fmt::format("{} TLAS instance ", s);
            SetName(tlas->instance_buffer, instance_name);
        }
    }

    void Context::SetName(CMShared<QueryPool> query_pool, std::string_view s) const
    {
        if (query_pool)
        {
            SetObjectName(query_pool->query_pool, s, vk::DebugReportObjectTypeEXT::eQueryPool);
        }
    }

    void Context::SetName(CMShared<FrameBuffer> frame_buffer, std::string_view s) const
    {
        if (frame_buffer)
        {
            SetObjectName(frame_buffer->frame_buffer, s, vk::DebugReportObjectTypeEXT::eFramebuffer);
        }
    }
}