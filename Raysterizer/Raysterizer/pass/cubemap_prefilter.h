#pragma once

#include "pch.h"

namespace Raysterizer
{
    namespace Pass
    {
        class CubemapPrefiler
        {
        public:
            Error Setup(std::shared_ptr<CommonResources> common_resources_, CMShared<Texture> cubemap);
            void Update(CMShared<CommandBuffer> command_buffer);

            void SetSampleCount(uint32_t count);
            uint32_t GetSampleCount() const { return sample_count; }

            CMShared<Image> GetImage() { return image; }
            CMShared<ImageView> GetImageView() { return image_view; }
            CMShared<Texture> GetTexture() const { return texture; }

        private:
            void PrecomputePrefilterConstants();

        private:
            std::shared_ptr<CommonResources> common_resources{};

            int sample_count = 32;
            uint32_t size;

            std::vector<CMShared<DescriptorSet>> descriptor_sets;
            CMShared<ImageView>           cubemap_image_view;
            CMShared<Texture>           cubemap_texture;
            CMShared<Image>               image;
            CMShared<ImageView>           image_view;
            CMShared<Texture>           texture;
            std::vector<CMShared<ImageView>>           mip_image_views;
            std::vector<CMShared<Texture>>           mip_textures;
            std::vector<CMShared<Buffer>> sample_directions_ubos;
        };
    }
}