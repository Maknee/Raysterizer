#include "include/image.h"

namespace RaysterizerEngine
{
	void Image::SetImageLayout(vk::ImageLayout image_layout_)
	{
		image_layout = image_layout_;
	}

	const vk::ImageLayout& Image::GetImageLayout() const
	{
		return image_layout;
	}
}