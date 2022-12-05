#include "sky_environment.h"

namespace Raysterizer
{
	namespace Pass
	{
		Error SkyEnvironment::Setup(std::shared_ptr<CommonResources> common_resources_)
		{
			common_resources = common_resources_;

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			// Create procedural sky
			{
				ReturnIfError(hosek_wilkie_sky_model.Setup(common_resources));
				ReturnIfError(cubemap_sh_projection.Setup(common_resources, hosek_wilkie_sky_model.GetTexture()));
				ReturnIfError(cubemap_prefilter.Setup(common_resources, hosek_wilkie_sky_model.GetTexture()));
				ReturnIfError(brdf_integrate_lut.Setup(common_resources));
			}

			// Create blank SH image
			{
				blank_sh_image = CommonResources::CreateImage(vk::ImageType::e2D, 9, 1, 1, 1, 1,
					vk::Format::eR32G32B32A32Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
					vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst, vk::SampleCountFlagBits::e1,
					vk::ImageLayout::eUndefined);
				c.SetName(blank_sh_image, fmt::format("Blank SH Projection Image"));

				blank_sh_image_view = CommonResources::CreateImageView(blank_sh_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
				c.SetName(blank_sh_image_view, fmt::format("Blank SH Projection Image View"));

				blank_sh_texture = std::make_shared<Texture>(blank_sh_image, blank_sh_image_view, common_resources->nearest_sampler);

				std::vector<glm::vec4> sh_data(9);
				std::vector<size_t>    sh_sizes(1);

				for (int i = 0; i < 9; i++)
					sh_data[i] = glm::vec4(0.0f);

				sh_sizes[0] = sizeof(glm::vec4) * 9;

				CommonResources::UploadImageData(blank_sh_image, sh_data.data(), sh_sizes);
			}

			// Create blank environment map
			{
				blank_cubemap_image = CommonResources::CreateImage(vk::ImageType::e2D, 2, 2, 1, 1, 6,
					vk::Format::eR32G32B32A32Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
					vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SampleCountFlagBits::e1,
					vk::ImageLayout::eUndefined, 0, nullptr, vk::ImageCreateFlagBits::eCubeCompatible);
				c.SetName(blank_cubemap_image, fmt::format("Blank SH Projection Image"));

				blank_cubemap_image_view = CommonResources::CreateImageView(blank_cubemap_image, vk::ImageViewType::eCube, vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
				c.SetName(blank_sh_image_view, fmt::format("Blank SH Projection Image View"));

				blank_cubemap_texture = std::make_shared<Texture>(blank_cubemap_image, blank_cubemap_image_view, common_resources->trilinear_sampler);

				std::vector<glm::vec4> cubemap_data(2 * 2 * 6);
				std::vector<size_t>    cubemap_sizes(6);

				int idx = 0;

				for (int layer = 0; layer < 6; layer++)
				{
					cubemap_sizes[layer] = sizeof(glm::vec4) * 4;

					for (int i = 0; i < 4; i++)
						cubemap_data[idx++] = glm::vec4(0.0f);
				}

				CommonResources::UploadImageData(blank_cubemap_image, cubemap_data.data(), cubemap_sizes);
			}

			// Load environment maps
			std::vector<std::string> environment_map_images;
			environment_map_images = { "resources/textures/Arches_E_PineTree_3k.hdr" };
			//const std::vector<std::string> environment_map_images = { "resources/textures/Arches_E_PineTree_3k.hdr", "resources/textures/BasketballCourt_3k.hdr", "resources/textures/Etnies_Park_Center_3k.hdr", "resources/textures/LA_Downtown_Helipad_GoldenHour_3k.hdr" };

			EquirectangularToCubemap equirectangular_to_cubemap{};
			PanicIfError(equirectangular_to_cubemap.Setup(common_resources, vk::Format::eR32G32B32A32Sfloat));

			hdr_environments.resize(environment_map_images.size());

			for (int i = 0; i < environment_map_images.size(); i++)
			{
				HDREnvironment environment{};

				auto input_image = CommonResources::CreateImageFromFile(environment_map_images[i], true);

				environment.image = CommonResources::CreateImage(vk::ImageType::e2D, 1024, 1024, 1, 5, 6,
					vk::Format::eR32G32B32A32Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
					vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, vk::SampleCountFlagBits::e1,
					vk::ImageLayout::eUndefined, 0, nullptr, vk::ImageCreateFlagBits::eCubeCompatible);
				c.SetName(environment.image, fmt::format("HDR Image {}", environment_map_images[i]));

				environment.image_view = CommonResources::CreateImageView(blank_cubemap_image, vk::ImageViewType::eCube, vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
				c.SetName(environment.image_view, fmt::format("HDR Image View {}", environment_map_images[i]));

				environment.texture = std::make_shared<Texture>(environment.image, environment.image_view, common_resources->bilinear_sampler);

				PanicIfError(environment.cubemap_sh_projection.Setup(common_resources, environment.texture));
				PanicIfError(environment.cubemap_prefilter.Setup(common_resources, environment.texture));
				
				equirectangular_to_cubemap.Convert(input_image, environment.image);

				PanicIfError(c.ImmediateGraphicsSubmitPtr([&](CMShared<CommandBuffer> command_buffer)
					{
						ScopedGPUProfileRaysterizer(command_buffer, "Equirectangular To Cubemap");

						auto& cb = *command_buffer;
						PanicIfError(render_frame.GenerateMipMaps(command_buffer, environment.image));
						environment.cubemap_sh_projection.Update(command_buffer);
						environment.cubemap_prefilter.Update(command_buffer);
					}));

				hdr_environments[i] = environment;
			}

			return NoError();
		}

		std::vector<WriteDescriptorSetBindedResource> SkyEnvironment::OutputBindings()
		{
			auto brdf_integrate_lut_texture = brdf_integrate_lut.GetTexture();

			switch (common_resources->environment_type)
			{
			case ENVIRONMENT_TYPE_NONE:
			{
				blank_cubemap_texture->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				blank_sh_texture->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				blank_cubemap_texture->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				brdf_integrate_lut_texture->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				return { blank_cubemap_texture, blank_sh_texture, blank_cubemap_texture, brdf_integrate_lut_texture };
				break;
			}
			case ENVIRONMENT_TYPE_PROCEDURAL_SKY:
			{
				hosek_wilkie_sky_model.GetImage()->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				cubemap_sh_projection.GetImage()->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				cubemap_prefilter.GetImage()->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				brdf_integrate_lut_texture->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				return { hosek_wilkie_sky_model.GetTexture(), cubemap_sh_projection.GetTexture(), cubemap_prefilter.GetTexture(), brdf_integrate_lut_texture};
				break;
			}
			default:
			{
				auto index = common_resources->environment_type - 2;
				if (index >= hdr_environments.size())
				{
					PANIC("Index out of bounds for hdr env {} >= {}", index, hdr_environments.size());
				}
				auto& hdr_environment = hdr_environments[index];
				hdr_environment.image->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				hdr_environment.cubemap_sh_projection.GetImage()->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				hdr_environment.cubemap_prefilter.GetImage()->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				return { hdr_environment.texture, hdr_environment.cubemap_sh_projection.GetTexture(), hdr_environment.cubemap_prefilter.GetTexture(), brdf_integrate_lut_texture};
				break;
			}
			}
		}
	}
}