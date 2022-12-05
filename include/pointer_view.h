#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct PointerView
	{
	public:
		explicit PointerView() = default;
		explicit PointerView(void* data_, std::size_t stride_, std::size_t num_elements_);
		explicit PointerView(const void* data_, std::size_t stride_, std::size_t num_elements_);
		template<typename T>
		explicit PointerView(const std::vector<T>& vec) :
			PointerView(vec.data(), sizeof(T), vec.size())
		{
		}

		template<typename T>
		explicit PointerView(const T& t) :
			PointerView(&t, sizeof(T), 1)
		{
		}

		PointerView(const PointerView& other) = default;
		PointerView& operator=(const PointerView& other) = default;
		PointerView(PointerView&& other) = default;
		PointerView& operator=(PointerView&& other) = default;
		bool operator==(const PointerView& other) const;
		bool operator!=(const PointerView& other) const;
		explicit operator bool() const;

		void Invalidate();
		bool Valid() const;
		void* GetData() const;
		
		template<typename T>
		T GetDataAs()
		{
			return reinterpret_cast<T>(GetData());
		}

		template<typename T>
		T GetDataAs() const
		{
			return reinterpret_cast<T>(GetData());
		}
		
		std::size_t GetStride() const;
		std::size_t GetNumElements() const;
		std::size_t GetTotalSize() const;
		bool ChangeStride(std::size_t new_stride);

		template<typename T>
		std::vector<T> ExtractSubdata(std::size_t data_offset, std::size_t data_size = 0)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto stride = GetStride();
			uintptr_t base = reinterpret_cast<uintptr_t>(GetData());

			auto new_stride = stride - data_offset;

			if (new_stride > stride)
			{
				PanicMessageBox("New stride is invalid");
			}

			if (data_size != 0)
			{
				new_stride = data_size;
			}

			std::vector<T> sub_data;
			sub_data.reserve(num_elements);
			for (std::size_t i = 0; i < num_elements; i++)
			{
				auto start = base + stride * i;
				auto offset = start + data_offset;
				T* data = reinterpret_cast<T*>(offset);

				sub_data.emplace_back(*data);
			}

			return sub_data;
		}

		template<>
		std::vector<uint8_t> ExtractSubdata(std::size_t data_offset, std::size_t data_size)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			if (!Valid())
			{
				PANIC("Not valid");
			}

			auto num_elements = GetNumElements();
			auto current_stride = GetStride();
			uintptr_t base = reinterpret_cast<uintptr_t>(GetData());

			auto new_stride = current_stride - data_offset;

			if (new_stride > current_stride)
			{
				PanicMessageBox("New stride is invalid");
			}

			if (data_size != 0)
			{
				new_stride = data_size;
			}

			std::vector<uint8_t> sub_data;
			sub_data.reserve(num_elements * new_stride);
			for (std::size_t i = 0; i < num_elements; i++)
			{
				auto start = base + current_stride * i;
				auto offset = start + data_offset;
				uint8_t* data = reinterpret_cast<uint8_t*>(offset);

				std::copy(data, data + new_stride, std::back_inserter(sub_data));
			}

			return sub_data;
		}

		void ExtractSubdataInto(void* target, std::size_t target_data_offset, std::size_t target_data_stride,
			std::size_t source_data_offset, std::size_t source_data_size = 0, std::size_t start_element = 0, std::size_t num_elements = 0,
			std::optional<std::function<void(uint8_t*, uint8_t*)>> custom_copy_function = std::nullopt);

		std::vector<uint8_t> ExtractData() const;

		//Adds padding
		std::vector<uint8_t> ExtractDataWithNewStride(std::size_t new_stride, uint8_t fill_data = 0) const;
		
		template<typename T>
		void CopyBytesFrom(T* other_data, std::size_t size, std::size_t offset = 0)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto* src = (uint8_t*)other_data;
			auto* dst = data;
			memcpy(reinterpret_cast<uint8_t*>(dst) + offset, src, size);
		}

		template<typename T>
		void CopyBytesInto(T* other_data, std::size_t size, std::size_t offset = 0)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto* src = (uint8_t*)data;
			auto* dst = (uint8_t*)other_data;
			/*
			for (auto i = 0; i < size; i++)
			{
				dst[i] = (src + offset)[i];
			}
			*/
			memcpy(dst, src + offset, size);
		}

	protected:
		void* data = nullptr;
		std::size_t stride = 0;
		std::size_t num_elements = 0;
	};

	struct HashedPointerView : public PointerView
	{
	public:
		explicit HashedPointerView() = default;
		explicit HashedPointerView(void* data_, std::size_t stride_, std::size_t num_elements_, std::optional<std::size_t> existing_hash = std::nullopt);
		explicit HashedPointerView(const void* data_, std::size_t stride_, std::size_t num_elements_, std::optional<std::size_t> existing_hash = std::nullopt);
		template<typename T>
		explicit HashedPointerView(const std::vector<T>&vec) :
			HashedPointerView(vec.data(), sizeof(T), vec.size())
		{
		}

		template<typename T>
		explicit HashedPointerView(const T & t) :
			HashedPointerView(&t, sizeof(T), 1)
		{
		}

		explicit operator PointerView() const;

		HashedPointerView(const HashedPointerView& other) = default;
		HashedPointerView& operator=(const HashedPointerView& other) = default;
		HashedPointerView(HashedPointerView&& other) = default;
		HashedPointerView& operator=(HashedPointerView&& other) = default;
		bool operator==(const HashedPointerView& other) const;
		bool operator!=(const HashedPointerView& other) const;

		XXH64_hash_t Hash() const;

		template<typename T>
		void CopyBytesFrom(T* other_data, std::size_t size, std::size_t offset = 0, bool perform_hash = true)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto* src = (uint8_t*)other_data;
			auto* dst = data;
			memcpy(reinterpret_cast<uint8_t*>(dst) + offset, src, size);
			if (perform_hash)
			{
				ComputeHash();
			}
		}

	private:
		void ComputeHash();

	private:
		XXH64_hash_t hash{};
		const static XXH64_hash_t hash_seed = 0;
	};

	template<typename T>
	class ResourceHashedPointerView : public HashedPointerView
	{
	public:
		explicit ResourceHashedPointerView() = default;
		explicit ResourceHashedPointerView(void* data_, std::size_t stride_, std::size_t num_elements_, std::optional<std::size_t> existing_hash = std::nullopt) :
			HashedPointerView(data_, stride_, num_elements_), resource(std::move(resource_))
		{
		}
		explicit ResourceHashedPointerView(const void* data_, std::size_t stride_, std::size_t num_elements_, std::optional<std::size_t> existing_hash = std::nullopt) :
			HashedPointerView(const_cast<void*>(data_), stride_, num_elements_, existing_hash), resource(std::move(resource_))
		{
		}
		ResourceHashedPointerView(const ResourceHashedPointerView& other) = default;
		ResourceHashedPointerView& operator=(const ResourceHashedPointerView& other) = default;
		ResourceHashedPointerView(ResourceHashedPointerView&& other) = default;
		ResourceHashedPointerView& operator=(ResourceHashedPointerView&& other) = default;
		bool operator==(const ResourceHashedPointerView& other)
		{
			return stride == other.stride &&
				num_elements == other.num_elements &&
				Hash() == other.Hash() &&
				resource == other.resource;
		}
		bool operator!=(const ResourceHashedPointerView& other) const
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			hash = 0;
			HashCombine(hash, reinterpret_cast<std::size_t>(data));
			auto total_size = GetTotalSize();
			HashCombine(hash, total_size);
			hash ^= XXH64(data, GetTotalSize(), hash_seed);
			hash ^= StdHash(resource);
		}

		XXH64_hash_t Hash() const;
	private:
		void ComputeHash();

	private:
		T resource;
	};

	struct BlockHashedPointerView : public PointerView
	{
	public:
		explicit BlockHashedPointerView() = default;
		explicit BlockHashedPointerView(void* data_, std::size_t stride_, std::size_t num_elements_);
		explicit BlockHashedPointerView(const void* data_, std::size_t stride_, std::size_t num_elements_);
		template<typename T>
		explicit BlockHashedPointerView(const std::vector<T>& vec) :
			BlockHashedPointerView(vec.data(), sizeof(T), vec.size())
		{
		}

		template<typename T>
		explicit BlockHashedPointerView(const T& t) :
			BlockHashedPointerView(&t, sizeof(T), 1)
		{
		}

		explicit operator PointerView() const;
		explicit operator HashedPointerView() const;

		HashedPointerView AsHashedPointerView() const { return HashedPointerView(data, stride, num_elements, hash); };

		BlockHashedPointerView(const BlockHashedPointerView& other) = default;
		BlockHashedPointerView& operator=(const BlockHashedPointerView& other) = default;
		BlockHashedPointerView(BlockHashedPointerView&& other) = default;
		BlockHashedPointerView& operator=(BlockHashedPointerView&& other) = default;
		bool operator==(const BlockHashedPointerView& other) const;
		bool operator!=(const BlockHashedPointerView& other) const;

		XXH64_hash_t Hash() const;

		template<typename T>
		void CopyBytesFrom(T* other_data, std::size_t size, std::size_t offset = 0, bool perform_hash = true)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto* src = (uint8_t*)other_data;
			auto* dst = reinterpret_cast<uint8_t*>(data) + offset;
			memcpy(dst, src, size);
			if (perform_hash)
			{
				RehashRegion(dst, size);
				//ComputeHash();
			}
		}

		template<typename T>
		void RehashRegion(T* ptr_, std::size_t size)
		{
			auto ptr = reinterpret_cast<uintptr_t>(ptr_);
			auto base_ptr = reinterpret_cast<uintptr_t>(data);
			auto ptr_difference = (ptr - base_ptr);
			auto begin_hash_block_index = ptr_difference / hash_block_size;
			auto end_hash_block_index = (ptr_difference + size + hash_block_size) / hash_block_size;

			if (begin_hash_block_index >= block_hashes.size())
			{
				PANIC("Hash block indexing out of bounds {} >= {}", begin_hash_block_index, block_hashes.size());
			}
			if (end_hash_block_index > block_hashes.size())
			{
				PANIC("Hash block indexing out of bounds {} >= {}", end_hash_block_index, block_hashes.size());
			}

			for (auto i = begin_hash_block_index; i < end_hash_block_index; i++)
			{
				auto& block_hash = block_hashes[i];

				// undo hash
				hash ^= block_hash;

				auto index_ptr = base_ptr + (hash_block_size * i);

				auto block_size = hash_block_size;
				if (last_block_size > 0 && i == block_hashes.size() - 1)
				{
					block_size = last_block_size;
				}
				block_hash = XXH64((void*)index_ptr, block_size, hash_seed);

				// redo hash
				hash ^= block_hash;
			}
		}

	private:
		void ComputeHash();

	private:
		XXH64_hash_t hash{};
		const static XXH64_hash_t hash_seed = 0;
		const static std::size_t hash_block_size = 1024;
		std::vector<std::size_t> block_hashes;
		std::size_t last_block_size{};
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template <>
	struct hash<PointerView>
	{
		std::size_t operator()(const PointerView& o) const
		{
			return o.GetDataAs<std::size_t>();
		}
	};

	template <>
	struct hash<HashedPointerView>
	{
		std::size_t operator()(const HashedPointerView& o) const
		{
			return o.Hash();
		}
	};

	template <typename T>
	struct hash<ResourceHashedPointerView<T>>
	{
		std::size_t operator()(const ResourceHashedPointerView<T>& o) const
		{
			return o.Hash();
		}
	};
}

