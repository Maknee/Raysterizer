#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	enum class MemoryUsage : uint8_t 
	{
		CpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
		CpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
		GpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
		GpuToCpu = VMA_MEMORY_USAGE_GPU_TO_CPU
	};

	struct BufferCreateInfo
	{
		VmaAllocationCreateInfo vma_allocation_create_info;
		vk::BufferUsageFlags buffer_usage_flags;
		std::size_t size;
	};

	struct Buffer
	{
	public:
		vk::Buffer buffer;
		VmaAllocation vma_allocation;
		VmaAllocationInfo vma_allocation_info;
		vk::DeviceAddress device_address;
		BufferCreateInfo buffer_create_info;

	public:
		const vk::Buffer& operator*() const noexcept
		{
			return buffer;
		}

		operator vk::Buffer() noexcept
		{
			return buffer;
		}

		bool operator==(const Buffer& o) const noexcept {
			return buffer == o.buffer;
		}

	public:
		Error Copy(PointerView data, std::size_t dst_offset = 0);
		uint8_t* GetMappedAddress() const;
		uint8_t* Map() const;
		template<typename T>
		T MapAs() const
		{
			T mapped_memory = reinterpret_cast<T>(vma_allocation_info.pMappedData);
			return mapped_memory;
		}

		vk::DeviceAddress GetAddress() const;
		vk::DeviceSize GetSize() const;
		PointerView GetPointerView() const;
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<Buffer>
	{
		size_t operator()(const Buffer& o) const noexcept
		{
			size_t h = reinterpret_cast<size_t>(static_cast<VkBuffer>(o.buffer));
			//HashCombine(h, Hash(o.buffer));
			return h;
		}
	};
}