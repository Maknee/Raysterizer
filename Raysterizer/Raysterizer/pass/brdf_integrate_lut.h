#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
		class BRDFIntegrateLUT
		{
		public:
			Error Setup(std::shared_ptr<CommonResources> common_resources_);

			CMShared<Image> GetImage() { return image; }
			CMShared<ImageView> GetImageView() { return image_view; }
			CMShared<Texture> GetTexture() const { return texture; }

		private:
			std::shared_ptr<CommonResources> common_resources{};

			CMShared<Image>           image;
			CMShared<ImageView>       image_view;
			CMShared<Texture>       texture;
		};
	}
}