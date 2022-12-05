#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	namespace Constants
	{
        constexpr std::string_view CONFIG_PATH = "raysterizer_config.json";

		constexpr std::string_view VULKAN_APPLICATION_NAME = "RaysterizerEngine";
        constexpr auto RAYSTERIZER_VULKAN_VERSION = VK_MAKE_API_VERSION(0, 1, 3, 0);

#define RAYTRACING_ENABLED

        const std::vector<std::string_view> INSTANCE_EXTENSIONS = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
            //VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef WIN32
            //VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,

            // OpenGL interlop
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
        };

        const std::vector<std::string_view> DEVICE_EXTENSIONS = {      
            VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,

            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,

            VK_KHR_SPIRV_1_4_EXTENSION_NAME,

            VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,

#ifdef RAYTRACING_ENABLED
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_RAY_QUERY_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
#endif
            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,

            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,

            // Ray Tracing
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,

            // GLSL
            VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
            VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,

            // OpenGL interlop
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,

            VK_KHR_SHADER_CLOCK_EXTENSION_NAME,

#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
            VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME,
            VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
#endif

            //VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME,

            VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME,
        };

        // Allocation
        constexpr vk::CommandPoolCreateFlags DEFAULT_COMMAND_POOL_CREATE_FLAGS = vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient;
        constexpr std::string_view DEFAULT_SHADER_MAIN = "main";

        const uint64_t DEFAULT_SEMAPHORE_INITIAL_VALUE = 0;
        const uint64_t DEFAULT_FENCE_TIMEOUT_VALUE = ~uint64_t(0);
        const uint64_t DEFAULT_SEMAPHORE_TIMEOUT_VALUE = ~uint64_t(0);

        //const vk::FenceCreateInfo default_fence_create_info = vk::FenceCreateInfo{ vk::FenceCreateFlagBits::eSignaled };
        const vk::FenceCreateInfo DEFAULT_FENCE_CREATE_INFO = vk::FenceCreateInfo{};
        const vk::SemaphoreTypeCreateInfo DEFAULT_SEMAPHORE_TYPE_CREATE_INFO = vk::SemaphoreTypeCreateInfo{ vk::SemaphoreType::eTimeline, DEFAULT_SEMAPHORE_INITIAL_VALUE };
        const vk::SemaphoreCreateInfo DEFAULT_SEMAPHORE_CREATE_INFO = vk::SemaphoreCreateInfo{};

        // const
        const VmaAllocatorCreateFlags VMA_ALLOCATOR_CREATE_FLAGS = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        // Always add shader device address bit for every allocation
        const VmaAllocationCreateFlags VMA_ALLOCATION_CREATE_FLAGS_ADDED = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        //const vk::Format DEPTH_FORMAT = vk::Format::eD24UnormS8Uint;
        const vk::Format DEPTH_FORMAT = vk::Format::eD32Sfloat;
        const vk::ImageUsageFlags DEPTH_IMAGE_USAGE_FLAGS = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        const vk::ImageAspectFlags DEPTH_IMAGE_ASPECT_MASK = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

        const vk::DescriptorBindingFlags VARIABLE_BINDING_FLAGS = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound;

        constexpr vk::DescriptorPoolCreateFlags DEFAULT_DESCRIPTOR_POOL_CREATE_FLAGS = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
        constexpr vk::DescriptorSetLayoutCreateFlags DEFAULT_DESCRIPTOR_SET_LAYOUT_CREATE_FLAGS = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
        constexpr vk::DescriptorBindingFlags DEFAULT_DESCRIPTOR_BINDING_FLAGS = vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending | vk::DescriptorBindingFlagBits::ePartiallyBound;

        const std::vector<vk::SurfaceFormatKHR> DESIRED_SURFACE_FORMATS =
        {
            vk::SurfaceFormatKHR{ vk::Format::eR32G32B32A32Sfloat, vk::ColorSpaceKHR::eSrgbNonlinear }
        };

        const std::vector<vk::SurfaceFormatKHR> FALLBACK_SURFACE_FORMATS =
        {
            vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
            vk::SurfaceFormatKHR{ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear }
        };

        const vk::ImageUsageFlags SWAPCHAIN_IMAGE_USAGE_FLAGS = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;


        // PROFILER
        constexpr uint32_t fnv1a(const char* s, uint32_t h = 0x811C9DC5) {
            return !*s ? h : fnv1a(s + 1, (h ^ (uint8_t)*s) * 0x01000193);
        }

#define DEFAULT_PROFILER_DEPTH 10
#define OPENGL_PROFILE_COLOR 0x00000000
#define RAYSTERIZER_PROFILE_COLOR 0xFFFFFFFF

#define ScopedCPUProfileRaysterizerConst(x) ScopedCPUProfileColorDepth(x, RAYSTERIZER_PROFILE_COLOR, DEFAULT_PROFILER_DEPTH);
#define ScopedCPUProfileRaysterizer(x) ScopedCPUProfileColorDepth(x, RAYSTERIZER_PROFILE_COLOR ^ RaysterizerEngine::Constants::fnv1a(x), DEFAULT_PROFILER_DEPTH);
#define ScopedCPUProfileRaysterizerCurrentFunction() ScopedCPUProfileRaysterizer(__FUNCTION__)

#define ScopedGPUProfileRaysterizer(cmdbuf, name) ScopedGPUProfile(c.GetProfiler(), *cmdbuf, name); ScopedCPUProfileRaysterizerCurrentFunction()

    }
}
