#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
		struct HDREnvironment
		{
			CMShared<Image>                    image;
			CMShared<ImageView>                image_view;
			CMShared<Texture>                texture;
			CubemapSHProjection cubemap_sh_projection;
			CubemapPrefiler     cubemap_prefilter;
		};

		class SkyEnvironment
		{
		public:
			Error Setup(std::shared_ptr<CommonResources> common_resources_);
			std::vector<WriteDescriptorSetBindedResource> OutputBindings();

			auto& GetHosekWilkieSkyModel() { return hosek_wilkie_sky_model; }
			auto& GetCubemapShProjection() { return cubemap_sh_projection; }
			auto& GetCubemapPrefilter() { return cubemap_prefilter; }

		private:
			std::shared_ptr<CommonResources> common_resources{};
			
			HosekWilkieSkyModel hosek_wilkie_sky_model;
			CubemapSHProjection cubemap_sh_projection;
			CubemapPrefiler cubemap_prefilter;
			BRDFIntegrateLUT brdf_integrate_lut;

			CMShared<Image>                    blank_sh_image;
			CMShared<ImageView>                blank_sh_image_view;
			CMShared<Texture>                blank_sh_texture;
			CMShared<Image>                    blank_cubemap_image;
			CMShared<ImageView>                blank_cubemap_image_view;
			CMShared<Texture>                blank_cubemap_texture;

			std::vector<HDREnvironment> hdr_environments;
		};
	}
}