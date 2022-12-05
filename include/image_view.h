#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct ImageViewCreateInfo
	{
		vk::ImageViewCreateInfo image_view_create_info;
	};

	struct ImageView
	{
		vk::ImageView image_view{};

	public:
		vk::ImageView operator*() const
		{
			return image_view;
		}

		operator vk::ImageView() noexcept
		{
			return image_view;
		}

	public:
		bool operator==(const ImageView& other) const noexcept {
			return image_view == other.image_view;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<ImageView>
	{
		size_t operator()(const ImageView& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.image_view));
			return h;
		}
	};

}
