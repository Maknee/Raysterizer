#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct TextureCreateInfo
	{
		ImageCreateInfo image_create_info;
		ImageViewCreateInfo image_view_create_info;
		SamplerCreateInfo sampler_create_info;
	};

	class Texture
	{
	public:
		explicit Texture() {}
		explicit Texture(CMShared<Image> image_, CMShared<ImageView> image_view_, CMShared<Sampler> sampler_) :
			image(image_), image_view(image_view_), sampler(sampler_)
		{
		}
		explicit Texture(CMShared<Image> image_, CMShared<ImageView> image_view_, TextureCreateInfo texture_create_info_, CMShared<Sampler> sampler_) :
			image(image_), image_view(image_view_), texture_create_info(std::move(texture_create_info_)), sampler(sampler_)
		{
		}

	public:
		void SetImageLayout(vk::ImageLayout image_layout_);
		const vk::ImageLayout& GetImageLayout() const;

		CMShared<Image> image{};
		CMShared<ImageView> image_view{};
		TextureCreateInfo texture_create_info{};

		CMShared<Sampler> sampler{};

	public:
		bool operator==(const Texture& o) const noexcept {
			return CheckEquality(image, o.image) &&
				CheckEquality(image_view, o.image_view) &&
				CheckEquality(sampler, o.sampler);
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<Texture>
	{
		size_t operator()(const Texture& o) const noexcept
		{
			size_t h{};
			if (o.image)
			{
				HashCombine(h, StdHash(*o.image));
			}
			if (o.image_view)
			{
				HashCombine(h, StdHash(*o.image_view));
			}
			if (o.sampler)
			{
				HashCombine(h, StdHash(*o.sampler));
			}
			
			return h;
		}
	};
}