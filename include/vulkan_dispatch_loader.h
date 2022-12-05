#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	class VulkanDispatchLoader
	{
	public:
		static Error Initialize()
		{
#ifdef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
			vk::DynamicLoader dynamic_loader{};
			PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dynamic_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
			VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
#endif
			return NoError();
		}

		static Error InitializeInstance(vk::Instance instance)
		{
#ifdef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
			VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#endif
			return NoError();
		}

		static Error InitializeDevice(vk::Device device)
		{
#ifdef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
			VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
#endif
			return NoError();
		}
	};
}