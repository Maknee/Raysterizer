#include "common_resources.h"
#include "gbuffer.h"

namespace Raysterizer
{
	namespace Pass
	{
		Error CommonResources::Setup()
		{
			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			width = c.GetWindowExtent().width;
			height = c.GetWindowExtent().height;

			auto sampler_create_info = vk::SamplerCreateInfo{}
				.setMagFilter(vk::Filter::eLinear)
				.setMinFilter(vk::Filter::eLinear)
				.setMipmapMode(vk::SamplerMipmapMode::eNearest)
				.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
				.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
				.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
				.setMipLodBias(0.0f)
				.setAnisotropyEnable(VK_TRUE)
				.setMaxAnisotropy(1.0f)
				.setCompareEnable(VK_FALSE)
				.setCompareOp(vk::CompareOp::eNever)
				.setMinLod(0.0f)
				.setMaxLod(12.0f)
				.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
				.setUnnormalizedCoordinates({});

			bilinear_sampler = AssignOrPanic(c.Get(SamplerCreateInfo{ sampler_create_info }));

			sampler_create_info.setMipmapMode(vk::SamplerMipmapMode::eLinear);

			trilinear_sampler = AssignOrPanic(c.Get(SamplerCreateInfo{ sampler_create_info }));

			sampler_create_info
				.setMagFilter(vk::Filter::eNearest)
				.setMinFilter(vk::Filter::eNearest);

			nearest_sampler = AssignOrPanic(c.Get(SamplerCreateInfo{ sampler_create_info }));

			for (auto i = 0; i < num_frames; i++)
			{
				auto buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(UBO), true));
				per_frame_ubo_buffers.emplace_back(buffer);
			}

			ddgi.id = 1;

			SetupBRDFIntegrateLUT();

			return NoError();
		}

		void CommonResources::SetupBRDFIntegrateLUT()
		{
		}

		CMShared<Buffer> CommonResources::GetUBOBuffer()
		{
			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			return per_frame_ubo_buffers[current_frame_index];
		}

		std::vector<WriteDescriptorSetBindedResource> CommonResources::OutputBindings() const
		{
			auto tlas = pipeline_manager->GetTLAS();
			auto instance_datas_buffer = pipeline_manager->instance_data_pool.GetBuffer();
			auto materials_buffer = pipeline_manager->materials_buffer;
			auto raysterizer_out_colors = pipeline_manager->out_color_buffers;
			return { tlas, instance_datas_buffer, materials_buffer, raysterizer_out_colors };
		}

		CMShared<Image> CommonResources::CreateImage(vk::ImageType type, uint32_t width, uint32_t height, uint32_t depth,
			uint32_t mip_levels, uint32_t array_size, vk::Format format,
			VmaMemoryUsage memory_usage, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sample_count,
			vk::ImageLayout initial_layout, size_t size, void* data, vk::ImageCreateFlags flags, vk::ImageTiling tiling)
		{
			if (mip_levels == 0)
			{
				mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
			}

			auto vk_image_create_info = vk::ImageCreateInfo{}
				.setImageType(type)
				.setFormat(format)
				.setExtent(vk::Extent3D{ width, height, depth })
				.setMipLevels(mip_levels)
				.setArrayLayers(array_size)
				.setSamples(sample_count)
				.setTiling(tiling)
				.setSharingMode(vk::SharingMode::eExclusive)
				.setInitialLayout(initial_layout)
				.setUsage(usage)
				.setFlags(flags);

			VmaAllocationCreateInfo vma_allocation_create_info{};
			vma_allocation_create_info.usage = memory_usage;
			vma_allocation_create_info.flags = (memory_usage == VMA_MEMORY_USAGE_CPU_ONLY || memory_usage == VMA_MEMORY_USAGE_GPU_TO_CPU) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

			ImageCreateInfo image_create_info{ vk_image_create_info, vma_allocation_create_info };
			auto image = AssignOrPanic(c.CreateImage(image_create_info));

            RenderFrame& render_frame = c.GetRenderFrame();
			if (data)
			{
				auto subresource_range = vk::ImageSubresourceRange{}
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(mip_levels)
					.setBaseArrayLayer(0)
					.setLayerCount(array_size);
				
                PanicIfError(c.ImmediateGraphicsSubmit([&](CommandBuffer& command_buffer)
                    {
                        PanicIfError(command_buffer.SetImageLayout(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresource_range));
                    }));

				PanicIfError(render_frame.UploadData(image, PointerView(data, 1, size), 0, 0, vk::ImageLayout::eTransferDstOptimal));

				if (mip_levels > 1)
				{
					PanicIfError(render_frame.GenerateMipMaps(image));
				}
			}

			return image;
		}
		
		CMShared<ImageView> CommonResources::CreateImageView(CMShared<Image> image, vk::ImageViewType view_type, vk::ImageAspectFlags aspect_flags, uint32_t base_mip_level, uint32_t level_count, uint32_t base_array_layer, uint32_t layer_count)
		{
			auto image_view_create_info = ImageViewCreateInfo
			{
				vk::ImageViewCreateInfo{}
				.setFlags(vk::ImageViewCreateFlags{})
				.setImage(*image)
				.setViewType(view_type)
				.setFormat(image->image_create_info.image_create_info.format)
				.setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
				.setSubresourceRange
				(
					vk::ImageSubresourceRange{}
						.setAspectMask(aspect_flags)
						.setBaseMipLevel(base_mip_level)
						.setLevelCount(level_count)
						.setBaseArrayLayer(base_array_layer)
						.setLayerCount(layer_count)
				)
			};

			auto image_view = AssignOrPanic(c.CreateImageView(image_view_create_info));
			return image_view;
		}

		CMShared<Image> CommonResources::CreateImageFromFile(fs::path path, bool flip_vertical, bool srgb)
		{
			int x, y, n;
			stbi_set_flip_vertically_on_load(flip_vertical);

			std::string ext = path.extension().string();
			auto path_string = path.string();

			if (ext == "hdr")
			{
				float* data = stbi_loadf(path_string.c_str(), &x, &y, &n, 4);

				if (!data)
				{
					PANIC("FILE FOR IMAGE COULD NOT BE FOUND {}", path_string);
					return nullptr;
				}

				auto image = CreateImage(vk::ImageType::e2D, (uint32_t)x, (uint32_t)y, 1, 0, 1, vk::Format(VK_FORMAT_R32G32B32A32_SFLOAT), VMA_MEMORY_USAGE_GPU_ONLY, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, vk::SampleCountFlagBits::e1, vk::ImageLayout::eUndefined, x * y * sizeof(float) * 4, data);

				stbi_image_free(data);

				return image;
			}
			else
			{
				stbi_uc* data = stbi_load(path_string.c_str(), &x, &y, &n, 0);

				if (!data)
				{
					PANIC("FILE FOR IMAGE COULD NOT BE FOUND {}", path_string);
					return nullptr;
				}

				if (n == 3)
				{
					stbi_image_free(data);
					data = stbi_load(path_string.c_str(), &x, &y, &n, 4);
					n = 4;
				}

				VkFormat format;

				if (n == 1)
					format = VK_FORMAT_R8_UNORM;
				else
				{
					if (srgb)
						format = VK_FORMAT_R8G8B8A8_SRGB;
					else
						format = VK_FORMAT_R8G8B8A8_UNORM;
				}

				auto image = CreateImage(vk::ImageType::e2D, (uint32_t)x, (uint32_t)y, 1, 0, 1, vk::Format(format), VMA_MEMORY_USAGE_GPU_ONLY, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, vk::SampleCountFlagBits::e1, vk::ImageLayout::eUndefined, x * y * n, data);

				stbi_image_free(data);

				return image;
			}
		}

		void CommonResources::UploadImageData(CMShared<Image> image, void* data, const std::vector<size_t>& mip_level_sizes, VkImageLayout src_layout, VkImageLayout dst_layout)
		{
			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			size_t size = 0;

			for (const auto& region_size : mip_level_sizes)
				size += region_size;

			auto buffer = AssignOrPanic(render_frame.CreateStagingBuffer(size));
			PanicIfError(buffer->Copy(PointerView(data, size, 1)));

			std::vector<VkBufferImageCopy> copy_regions;
			size_t                         offset = 0;
			uint32_t                       region_idx = 0;

			auto image_create_info = image->image_create_info.image_create_info;

			for (int array_idx = 0; array_idx < image_create_info.arrayLayers; array_idx++)
			{
				int width = image_create_info.extent.width;
				int height = image_create_info.extent.height;

				for (int i = 0; i < image_create_info.mipLevels; i++)
				{
					VkBufferImageCopy buffer_copy_region{};

					buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					buffer_copy_region.imageSubresource.mipLevel = i;
					buffer_copy_region.imageSubresource.baseArrayLayer = array_idx;
					buffer_copy_region.imageSubresource.layerCount = 1;
					buffer_copy_region.imageExtent.width = width;
					buffer_copy_region.imageExtent.height = height;
					buffer_copy_region.imageExtent.depth = 1;
					buffer_copy_region.bufferOffset = offset;

					copy_regions.push_back(buffer_copy_region);

					width = std::max(1, width / 2);
					height = std::max(1, (height / 2));

					offset += mip_level_sizes[region_idx++];
				}
			}

			PanicIfError(c.ImmediateGraphicsSubmit([&](CommandBuffer& command_buffer)
				{
					auto& cb = *command_buffer;

					VkImageSubresourceRange subresource_range{};

					subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					subresource_range.baseMipLevel = 0;
					subresource_range.levelCount = image_create_info.mipLevels;
					subresource_range.layerCount = image_create_info.arrayLayers;
					subresource_range.baseArrayLayer = 0;

					if (src_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
					{
						// Image barrier for optimal image (target)
						// Optimal image will be used as destination for the copy
						PanicIfError(command_buffer.SetImageLayout(image, (vk::ImageLayout)src_layout, vk::ImageLayout::eTransferDstOptimal, subresource_range));
					}

					// Copy mip levels from staging buffer
					vkCmdCopyBufferToImage(cb,
						**buffer,
						**image,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						copy_regions.size(),
						copy_regions.data());

					if (dst_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
					{
						// Change texture image layout to shader read after all mip levels have been copied
						PanicIfError(command_buffer.SetImageLayout(image, vk::ImageLayout::eTransferDstOptimal, (vk::ImageLayout)dst_layout, subresource_range));
					}
				}));
		}

	}
}