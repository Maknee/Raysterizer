#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{

        enum BlueNoiseSpp
        {
            BLUE_NOISE_1SPP,
            BLUE_NOISE_2SPP,
            BLUE_NOISE_4SPP,
            BLUE_NOISE_8SPP,
            BLUE_NOISE_16SPP,
            BLUE_NOISE_32SPP,
            BLUE_NOISE_64SPP,
            BLUE_NOISE_128SPP
        };

        struct BlueNoise
        {
            CMShared<Image> sobol_image;
            CMShared<ImageView> sobol_image_view;
            CMShared<Texture> sobol_texture;

            CMShared<Image> scrambling_ranking_image[9];
            CMShared<ImageView> scrambling_ranking_image_view[9];
            CMShared<Texture> scrambling_ranking_texture[9];

            explicit BlueNoise() {}
            Error Setup(std::shared_ptr<CommonResources> common_resources_);

            CMShared<Texture> GetSobolTexture();
            CMShared<Texture> GetSobolRankingTexture(BlueNoiseSpp blue_noise_ssp);
        };
	}
}