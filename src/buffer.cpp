#include "include/buffer.h"
#include <vk_mem_alloc.h>

namespace RaysterizerEngine
{
	Error Buffer::Copy(PointerView data, std::size_t dst_offset)
	{
		auto* mapped_memory = reinterpret_cast<uint8_t*>(vma_allocation_info.pMappedData);
		if (!mapped_memory)
		{
			return StringError("Buffer is not mapped!");
		}

		auto size = data.GetTotalSize();

		auto* src_begin = data.GetDataAs<uint8_t*>();
		auto* src_end = src_begin + size;
		auto* dst = mapped_memory + dst_offset;
		std::copy(src_begin, src_end, dst);

		return NoError();
	}

	uint8_t* Buffer::GetMappedAddress() const
	{
		auto* mapped_memory = reinterpret_cast<uint8_t*>(vma_allocation_info.pMappedData);
		if (mapped_memory == nullptr)
		{
			PANIC("Buffer mapped memory is null -- most likely this is a GPU buffer");
		}
		return mapped_memory;
	}

	uint8_t* Buffer::Map() const
	{
		return GetMappedAddress();
	}

	vk::DeviceAddress Buffer::GetAddress() const
	{
		return device_address;
	}

	vk::DeviceSize Buffer::GetSize() const
	{
		//return vma_allocation_info.size;
		return buffer_create_info.size;
	}

	PointerView Buffer::GetPointerView() const
	{
		PointerView pointer_view(Map(), GetSize(), 1);
		return pointer_view;
	}
}