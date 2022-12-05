#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct ImageCreateInfo
	{
		vk::ImageCreateInfo image_create_info;
		VmaAllocationCreateInfo vma_allocation_create_info;
	};

	struct Image
	{
		vk::Image image{};
		VmaAllocation vma_allocation{};

		ImageCreateInfo image_create_info{};
		vk::ImageLayout image_layout = vk::ImageLayout::eUndefined;

	public:
		void SetImageLayout(vk::ImageLayout image_layout_);
		const vk::ImageLayout& GetImageLayout() const;

	public:
		vk::Image operator*() const
		{
			return image;
		}

		operator vk::Image() noexcept
		{
			return image;
		}

	public:
		bool operator==(const Image& other) const noexcept {
			return image == other.image;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<Image>
	{
		size_t operator()(const Image& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.image));
			return h;
		}
	};
}
