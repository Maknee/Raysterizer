#include "brdf_integrate_lut.h"

#define BRDF_LUT_SIZE 512
#define BRDF_WORK_GROUP_SIZE 8

namespace Raysterizer
{
	namespace Pass
	{
		Error BRDFIntegrateLUT::Setup(std::shared_ptr<CommonResources> common_resources_)
		{
			common_resources = common_resources_;

			size_t                size = BRDF_LUT_SIZE * BRDF_LUT_SIZE * sizeof(uint16_t) * 2;
			std::vector<uint16_t> buffer(size);

			std::fstream f("resources/textures/brdf_lut.bin", std::ios::in | std::ios::binary);
			if (!f)
			{
				PANIC("NOT READ LUT");
			}
			f.seekp(0);
			f.read((char*)buffer.data(), size);
			f.close();

			image = CommonResources::CreateImage(vk::ImageType::e2D, BRDF_LUT_SIZE, BRDF_LUT_SIZE, 1, 1, 1,
				vk::Format::eR16G16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1,
				vk::ImageLayout::eUndefined, size, buffer.data());
			c.SetName(image, fmt::format("BRDF LUT"));

			image_view = CommonResources::CreateImageView(image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
			c.SetName(image_view, fmt::format("BRDF LUT"));

			auto sampler = common_resources->bilinear_sampler;
			texture = std::make_shared<Texture>(image, image_view, sampler);

			return NoError();
		}
	}
}