#include "include/texture.h"

namespace RaysterizerEngine
{
	void Texture::SetImageLayout(vk::ImageLayout image_layout_)
	{
		image->image_layout = image_layout_;
	}

	const vk::ImageLayout& Texture::GetImageLayout() const
	{
		return image->image_layout;
	}
}