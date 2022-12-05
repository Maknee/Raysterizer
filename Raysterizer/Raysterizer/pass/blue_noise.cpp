#include "blue_noise.h"

namespace Raysterizer
{
    namespace Pass
    {
        // -----------------------------------------------------------------------------------------------------------------------------------

        static const char* kSOBOL_TEXTURE = "resources/textures/blue_noise/sobol_256_4d.png";

        // -----------------------------------------------------------------------------------------------------------------------------------

        static const char* kSCRAMBLING_RANKING_TEXTURES[] = {
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_1spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_2spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_4spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_8spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_16spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_32spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_64spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_128spp.png",
            "resources/textures/blue_noise/scrambling_ranking_128x128_2d_256spp.png"
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        Error BlueNoise::Setup(std::shared_ptr<CommonResources> common_resources_)
        {
            auto nearest_sampler = common_resources_->nearest_sampler;

            sobol_image = CommonResources::CreateImageFromFile(kSOBOL_TEXTURE);
            c.SetName(sobol_image, fmt::format("Sobol Image"));

            sobol_image_view = CommonResources::CreateImageView(sobol_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
            c.SetName(sobol_image_view, fmt::format("Sobol Image View"));

            sobol_texture = std::make_shared<Texture>(sobol_image, sobol_image_view, nearest_sampler);

            for (int i = 0; i < 9; i++)
            {
                scrambling_ranking_image[i] = CommonResources::CreateImageFromFile(kSCRAMBLING_RANKING_TEXTURES[i]);
                c.SetName(scrambling_ranking_image[i], fmt::format("Sobol Ranking Image {}", i));

                scrambling_ranking_image_view[i] = CommonResources::CreateImageView(scrambling_ranking_image[i], vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                c.SetName(scrambling_ranking_image_view[i], fmt::format("Sobol Ranking Image View {}", i));

                scrambling_ranking_texture[i] = std::make_shared<Texture>(scrambling_ranking_image[i], scrambling_ranking_image_view[i], nearest_sampler);
            }

            return NoError();
        }
        
        CMShared<Texture> BlueNoise::GetSobolTexture()
        {
            return sobol_texture;
        }
        
        CMShared<Texture> BlueNoise::GetSobolRankingTexture(BlueNoiseSpp blue_noise_ssp)
        {
            return scrambling_ranking_texture[blue_noise_ssp];
        }
    }
}