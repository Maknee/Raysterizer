#include "include/pointer_view.h"

namespace RaysterizerEngine
{
	PointerView::PointerView(void* data_, std::size_t stride_, std::size_t num_elements_) :
		data(data_), stride(stride_), num_elements(num_elements_)
	{
	}

	PointerView::PointerView(const void* data_, std::size_t stride_, std::size_t num_elements_) :
		PointerView(const_cast<void*>(data_), stride_, num_elements_)
	{
	}

	bool PointerView::operator==(const PointerView& other) const
	{
		return data == other.data &&
			stride == other.stride &&
			num_elements == other.num_elements;
	}

	bool PointerView::operator!=(const PointerView& other) const
	{
		return !(*this == other);
	}

	PointerView::operator bool() const
	{
		return Valid();
	}

	void PointerView::Invalidate()
	{
		data = nullptr;
		stride = 0;
		num_elements = 0;
	}

	bool PointerView::Valid() const
	{
		return data != nullptr && stride > 0 && num_elements > 0;
	}

	void* PointerView::GetData() const
	{
		return data;
	}

	std::size_t PointerView::GetStride() const
	{
		return stride;
	}

	std::size_t PointerView::GetNumElements() const
	{
		return num_elements;
	}

	std::size_t PointerView::GetTotalSize() const
	{
		return stride * num_elements;
	}

	bool PointerView::ChangeStride(std::size_t new_stride)
	{
		auto total_size = GetTotalSize();
		if (stride == 0 || num_elements == 0 || new_stride == 0)
		{
			//PANIC("Cannot change stride!");
			return false;
		}

		stride = new_stride;
		num_elements = total_size / stride;
		return true;
	}

	void PointerView::ExtractSubdataInto(void* target, std::size_t target_data_offset, std::size_t target_data_stride,
		std::size_t source_data_offset, std::size_t source_data_size, std::size_t start_element, std::size_t num_elements,
		std::optional<std::function<void(uint8_t*, uint8_t*)>> custom_copy_function)
	{
		ScopedCPUProfileRaysterizerCurrentFunction();

		if (!Valid())
		{
			PANIC("Not valid");
		}
		if (num_elements == 0)
		{
			num_elements = GetNumElements();
		}

		auto current_stride = GetStride();
		uintptr_t base = reinterpret_cast<uintptr_t>(GetData());

		/*
		auto new_stride = stride - source_data_offset;

		if (new_stride > stride)
		{
			PanicMessageBox("New stride is invalid");
		}

		if (source_data_size != 0)
		{
			new_stride = source_data_size;
		}
		*/

		for (std::size_t i = 0; i < num_elements; i++)
		{
			auto start = base + current_stride * (i + start_element);
			auto offset = start + source_data_offset;
			uint8_t* data = reinterpret_cast<uint8_t*>(offset);

			uint8_t* target_start = reinterpret_cast<uint8_t*>(target);
			auto total_target_offset = (i * target_data_stride) + target_data_offset;
			uint8_t* target_data = target_start + total_target_offset;

			if (custom_copy_function)
			{
				(*custom_copy_function)(target_data, data);
			}
			else
			{
				std::copy(data, data + source_data_size, target_data);
			}
		}
	}

	std::vector<uint8_t> PointerView::ExtractData() const
	{
		ScopedCPUProfileRaysterizerCurrentFunction();

		if (!Valid())
		{
			PANIC("Not valid");
		}

		auto total_size = GetTotalSize();
		uint8_t* base = reinterpret_cast<uint8_t*>(GetData());

		std::vector<uint8_t> extracted_data(total_size);
		std::copy(base, base + total_size, std::begin(extracted_data));
		return extracted_data;
	}

	std::vector<uint8_t> PointerView::ExtractDataWithNewStride(std::size_t new_stride, uint8_t fill_data) const
	{
		if (!Valid())
		{
			PANIC("Not valid");
		}

		auto total_size = GetTotalSize();
		uint8_t* base = reinterpret_cast<uint8_t*>(GetData());

		if (new_stride <= stride)
		{
			PanicMessageBox("New stride must be at least stride, this increases size");
		}

		std::vector<uint8_t> extracted_data(num_elements * new_stride, fill_data);
		for (auto i = 0; i < num_elements; i++)
		{
			auto start = base + stride * i;

			auto extracted_data_offset = std::begin(extracted_data) + new_stride * i;

			std::copy(start, start + stride, extracted_data_offset);
		}
		return extracted_data;
	}

	HashedPointerView::HashedPointerView(void* data_, std::size_t stride_, std::size_t num_elements_, std::optional<std::size_t> existing_hash) :
		PointerView(data_, stride_, num_elements_)
	{
		if (existing_hash)
		{
			hash = *existing_hash;
		}
		else if (data)
		{
			ComputeHash();
		}
	}

	HashedPointerView::HashedPointerView(const void* data_, std::size_t stride_, std::size_t num_elements_, std::optional<std::size_t> existing_hash) :
		HashedPointerView(const_cast<void*>(data_), stride_, num_elements_, existing_hash)
	{
	}

	bool HashedPointerView::operator==(const HashedPointerView& other) const
	{
		return stride == other.stride &&
			num_elements == other.num_elements &&
			Hash() == other.Hash();
	}

	bool HashedPointerView::operator!=(const HashedPointerView& other) const
	{
		return !(*this == other);
	}

	HashedPointerView::operator PointerView() const
	{
		return PointerView(data, stride, num_elements);
	}

	XXH64_hash_t HashedPointerView::Hash() const
	{
		return hash;
	}

	void HashedPointerView::ComputeHash()
	{
		ScopedCPUProfileRaysterizerCurrentFunction();

		hash = 0;
		HashCombine(hash, reinterpret_cast<std::size_t>(data));
		auto total_size = GetTotalSize();
		HashCombine(hash, total_size);
		hash ^= XXH64(data, GetTotalSize(), hash_seed);
	};

	BlockHashedPointerView::BlockHashedPointerView(void* data_, std::size_t stride_, std::size_t num_elements_) :
		PointerView(data_, stride_, num_elements_)
	{
		if (data)
		{
			const auto total_size = GetTotalSize();
			const auto aligned_total_size = Util::AlignUp(total_size, hash_block_size);
			const auto num_block_hashes = aligned_total_size / hash_block_size;
			last_block_size = total_size % hash_block_size;
			block_hashes.resize(num_block_hashes);

			ComputeHash();
		}
	}

	BlockHashedPointerView::BlockHashedPointerView(const void* data_, std::size_t stride_, std::size_t num_elements_) :
		BlockHashedPointerView(const_cast<void*>(data_), stride_, num_elements_)
	{
	}

	bool BlockHashedPointerView::operator==(const BlockHashedPointerView& other) const
	{
		return stride == other.stride &&
			num_elements == other.num_elements &&
			Hash() == other.Hash();
	}

	bool BlockHashedPointerView::operator!=(const BlockHashedPointerView& other) const
	{
		return !(*this == other);
	}

	BlockHashedPointerView::operator PointerView() const
	{
		return PointerView(data, stride, num_elements);
	}

	BlockHashedPointerView::operator HashedPointerView() const
	{
		return HashedPointerView(data, stride, num_elements, hash);
	}

	XXH64_hash_t BlockHashedPointerView::Hash() const
	{
		return hash;
	}

	void BlockHashedPointerView::ComputeHash()
	{
		ScopedCPUProfileRaysterizerCurrentFunction();

		for (auto i = 0; i < block_hashes.size(); i++)
		{
			auto& block_hash = block_hashes[i];

			auto ptr = (uint8_t*)data + (hash_block_size * i);
			auto block_size = hash_block_size;
			if (last_block_size > 0 && i == block_hashes.size() - 1)
			{
				block_size = last_block_size;
			}
			block_hash = XXH64(ptr, block_size, hash_seed);
		}

		hash = 0;
		HashCombine(hash, reinterpret_cast<std::size_t>(data));
		auto total_size = GetTotalSize();
		HashCombine(hash, total_size);
		for (const auto& block_hash : block_hashes)
		{
			hash ^= block_hash;
		}
	}
}