#include "include/render_frame.h"

namespace RaysterizerEngine
{
    RenderFrame::RenderFrame(Context* c_) : c(c_)
    {
        pipeline_layout_info_cache.SetContext(c);
        descriptor_set_cache.SetContext(c);
        command_pool_cache.SetContext(c);
        /*
        for (auto i = 0; i < command_pools.size(); i++)
        {
            QueueType queue_type = static_cast<QueueType>(i);
            auto command_pool_create_info = CommandPoolCreateInfo{ queue_type, Constants::DEFAULT_COMMAND_POOL_CREATE_FLAGS };
            command_pools[i] = AssignOrPanic(c->CreateCommandPool(command_pool_create_info));
        }
        */
        command_buffer_cache.SetContext(c);
    }

    vk::Device RenderFrame::GetDevice() const
    {
        return c->GetDevice();
    }

    Expected<DescriptorSetLayouts> RenderFrame::Get(const DescriptorSetLayoutCreateInfos& descriptor_set_layout_create_info)
    {
        return c->Get(descriptor_set_layout_create_info);
    }

    Expected<CMShared<PipelineLayoutInfo>> RenderFrame::Get(const PipelineLayoutCreateInfo& pipeline_layout_create_info)
    {
        return c->Get(pipeline_layout_create_info);
        if (auto result = pipeline_layout_info_cache.Get(pipeline_layout_create_info))
        {
            return result->Get();
        }
        else
        {
            ConsumeError(result.takeError());

            GrowingPool<PipelineLayoutInfo> growing_pipeline_layout_info;
            growing_pipeline_layout_info.SetCreateFunction([this, pipeline_layout_create_info]()
                {
                    CMShared<PipelineLayoutInfo> pipeline_layout_info = AssignOrPanic(c->CreatePipelineLayoutInfo(pipeline_layout_create_info));
                    return pipeline_layout_info;
                });
            auto& pipeline_layout_info_pool = pipeline_layout_info_cache.Emplace(pipeline_layout_create_info, std::move(growing_pipeline_layout_info));
            auto pipeline_layout_info = pipeline_layout_info_pool.Get();
            return pipeline_layout_info;
        }
    }

    Expected<CMShared<GraphicsPipeline>> RenderFrame::Get(const GraphicsPipelineCreateInfo& graphics_pipeline_create_info)
    {
        return c->Get(graphics_pipeline_create_info);
    }

    Expected<CMShared<RenderPass>> RenderFrame::Get(const RenderPassCreateInfo& render_pass_create_info)
    {
        return c->Get(render_pass_create_info);
    }

    Expected<CMShared<ShaderModule>> RenderFrame::Get(const ShaderModuleCreateInfo& shader_module_create_info)
    {
        return c->Get(shader_module_create_info);
    }
    
    Expected<CMShared<DescriptorPool>> RenderFrame::Get(const DescriptorPoolCreateInfo& descriptor_pool_create_info)
    {
        return c->Get(descriptor_pool_create_info);
    }

    Expected<CMShared<DescriptorSet>> RenderFrame::Get(const DescriptorSetCreateInfo& descriptor_set_create_info)
    {
        if (auto result = descriptor_set_cache.Get(descriptor_set_create_info))
        {
            return result->Get();
        }
        else
        {
            ConsumeError(result.takeError());

            GrowingPool<DescriptorSet> growing_descriptor_set;
            growing_descriptor_set.SetCreateFunction([this, descriptor_set_create_info]()
                {
                    CMShared<DescriptorSet> descriptor_set = AssignOrPanic(c->CreateDescriptorSet(descriptor_set_create_info));
                    return descriptor_set;
                });
            auto& descriptor_set_pool = descriptor_set_cache.Emplace(descriptor_set_create_info, std::move(growing_descriptor_set));
            auto descriptor_set = descriptor_set_pool.Get();
            return descriptor_set;
        }
    }

    Expected<CMShared<CommandPool>> RenderFrame::Get(const CommandPoolCreateInfo& command_pool_create_info)
    {
        /*
        const auto& queue_type = command_pool_create_info.queue_type;
        auto queue_index = static_cast<std::size_t>(queue_type);
        const auto& command_pool = command_pools[queue_index];
        return command_pool;
        */
        if (auto result = command_pool_cache.Get(command_pool_create_info))
        {
            return result;
        }
        else
        {
            ConsumeError(result.takeError());

            AssignOrReturnError(CMShared<CommandPool> cm_command_pool, c->CreateCommandPool(command_pool_create_info));
            auto& command_pool = command_pool_cache.Emplace(command_pool_create_info, std::move(cm_command_pool));
            return command_pool;
        }
    }

    Expected<CMShared<CommandBuffer>> RenderFrame::Get(const CommandBufferCreateInfo& command_buffer_create_info)
    {
        if (auto result = command_buffer_cache.Get(command_buffer_create_info))
        {
            return result->Get();
        }
        else
        {
            ConsumeError(result.takeError());

            GrowingPool<CommandBuffer> growing_command_buffer_pool;
            growing_command_buffer_pool.SetCreateFunction([this, command_buffer_create_info]()
                {
                    CMShared<CommandBuffer> command_buffer = AssignOrPanic(c->CreateCommandBuffer(command_buffer_create_info));
                    return command_buffer;
                });

            auto& command_buffer_pool = command_buffer_cache.Emplace(command_buffer_create_info, std::move(growing_command_buffer_pool));
            auto command_buffer = command_buffer_pool.Get();
            return command_buffer;
        }
    }


    Expected<CMShared<Texture>> RenderFrame::Get(const TextureCreateInfo& texture_create_info)
    {
        return StringError("Not implemented");
    }

    Expected<CMShared<Buffer>> RenderFrame::Get(const BufferCreateInfo& buffer_create_info)
    {
        return StringError("Not implemented");
    }

    Expected<CMShared<Fence>> RenderFrame::GetFence()
    {
        fence_pool.SetCreateFunction([this]()
            {
                auto fence_create_info = FenceCreateInfo
                {
                    Constants::DEFAULT_FENCE_CREATE_INFO
                };

                CMShared<Fence> fence = AssignOrPanic(c->CreateFence(fence_create_info));
                return fence;
            });

        auto fence = fence_pool.Get();
        //ReturnIfError(c->WaitForFence(fence));
        //ReturnIfError(c->ResetFence(fence));
        return fence;
    }

    Error RenderFrame::WaitForFences(uint64_t timeout)
    {
        /*
        std::vector<vk::Fence> fences(fence_pool.size());
        std::transform(std::begin(fence_pool), std::end(fence_pool), std::begin(fences),
            [](const auto& e) { return e->fence; }
        );
        c->GetDevice().waitForFences(fences, VK_TRUE, timeout);
        */
        return StringError("Not implemented");
        return NoError();
    }

    Error RenderFrame::ResetFences()
    {
        /*
        std::vector<vk::Fence> fences(fence_pool.size());
        std::transform(std::begin(fence_pool), std::end(fence_pool), std::begin(fences),
            [](const auto& e) { return e->fence; }
        );
        c->GetDevice().resetFences(fences);

        fence_pool_index = 0;
        */
        return StringError("Not implemented");
        return NoError();
    }

    Expected<CMShared<Semaphore>> RenderFrame::GetSemaphore()
    {
        semaphore_pool.SetCreateFunction([this]()
            {
                auto vk_semaphore_create_info = Constants::DEFAULT_SEMAPHORE_CREATE_INFO;
                vk_semaphore_create_info
                    .setPNext(&Constants::DEFAULT_SEMAPHORE_TYPE_CREATE_INFO);

                auto semaphore_create_info = SemaphoreCreateInfo
                {
                    vk_semaphore_create_info
                };

                CMShared<Semaphore> semaphore = AssignOrPanic(c->CreateSemaphore(semaphore_create_info));
                return semaphore;
            });

        auto semaphore = semaphore_pool.Get();
        return semaphore;
    }

    Error RenderFrame::ResetSemaphores()
    {
        //semaphore_pool_index = 0;

        return StringError("Not implemented");
        return NoError();
    }

    Expected<CMShared<Semaphore>> RenderFrame::GetBinarySemaphore()
    {
        binary_semaphore_pool.SetCreateFunction([this]()
            {
                auto semaphore_type_create_info = vk::SemaphoreTypeCreateInfo{ vk::SemaphoreType::eBinary, Constants::DEFAULT_SEMAPHORE_INITIAL_VALUE };
                auto vk_semaphore_create_info = Constants::DEFAULT_SEMAPHORE_CREATE_INFO;
                vk_semaphore_create_info
                    .setPNext(&semaphore_type_create_info);

                auto semaphore_create_info = SemaphoreCreateInfo
                {
                    vk_semaphore_create_info
                };

                CMShared<Semaphore> semaphore = AssignOrPanic(c->CreateSemaphore(semaphore_create_info));
                return semaphore;
            });

        auto semaphore = binary_semaphore_pool.Get();
        return semaphore;
    }

    Expected<CMShared<Texture>> RenderFrame::CreateTexture(CommandBuffer command_buffer, vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t layer_count)
    {
        vk::PhysicalDevice physical_device = c->GetPhysicalDevice();
        vk::FormatProperties format_properties = physical_device.getFormatProperties(format);

        {
            AssignOrReturnError(auto staging_buffer, CreateStagingBuffer(data.GetTotalSize()));
            ReturnIfError(staging_buffer->Copy(data));

            auto extent = vk::Extent3D{ width, height, 1 };

            auto mip_levels = 1;
            vk::ImageUsageFlags image_usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;

            auto vk_image_create_info = vk::ImageCreateInfo{}
                .setImageType(vk::ImageType::e2D)
                .setFormat(format)
                .setExtent(extent)
                .setMipLevels(mip_levels)
                .setArrayLayers(layer_count)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setUsage(image_usage_flags);

            VmaAllocationCreateInfo vma_allocation_create_info{};
            vma_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vma_allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            ImageCreateInfo image_create_info{ vk_image_create_info, vma_allocation_create_info };

            auto image_view_create_info = ImageViewCreateInfo
            {
                vk::ImageViewCreateInfo{}
                    .setFlags(vk::ImageViewCreateFlags{})
                    //.setImage(*image)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(format)
                    .setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
                    .setSubresourceRange
                    (
                        vk::ImageSubresourceRange{}
                            .setAspectMask(vk::ImageAspectFlagBits::eColor)
                            .setBaseMipLevel(0)
                            .setLevelCount(mip_levels)
                            .setBaseArrayLayer(0)
                            .setLayerCount(layer_count)
                    )
            };

            auto vk_sampler_create_info = vk::SamplerCreateInfo{}
                .setMagFilter(vk::Filter::eLinear)
                .setMinFilter(vk::Filter::eLinear)
                .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                .setMipLodBias(0.0f)
                .setCompareOp(vk::CompareOp::eNever)
                .setMinLod(0.0f)
                .setMaxLod(static_cast<float>(mip_levels))
                .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
                .setMaxAnisotropy(1.0f)
                .setAnisotropyEnable(VK_FALSE);

            auto physical_device_features = c->GetPhysicalDeviceFeatures();
            if (physical_device_features.samplerAnisotropy)
            {
                auto physical_device_properties = c->GetPhysicalDeviceProperties();
                vk_sampler_create_info
                    .setMaxAnisotropy(physical_device_properties.limits.maxSamplerAnisotropy)
                    .setAnisotropyEnable(VK_TRUE);
            }

            auto sampler_create_info = SamplerCreateInfo{ vk_sampler_create_info };

            TextureCreateInfo texture_create_info
            {
                image_create_info,
                image_view_create_info,
                sampler_create_info
            };

            AssignOrReturnError(CMShared<Texture> texture, c->CreateTexture(texture_create_info));
            auto& image = texture->image;
            image->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

            {
                AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Graphics));

                command_buffer->Begin();
                const vk::CommandBuffer& cb = **command_buffer;

                // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                auto image_subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(layer_count);

                auto transfer_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer->InsertImageMemoryBarrier(transfer_image_memory_barrier, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer));

                // Copy the first mip of the chain, remaining mips will be generated
                std::vector<vk::BufferImageCopy> buffer_image_copies;
                for (uint32_t layer = 0; layer < layer_count; layer++)
                {
                    auto image_subresource_layers = vk::ImageSubresourceLayers{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(0)
                        .setBaseArrayLayer(layer)
                        .setLayerCount(1);

                    auto buffer_image_copy = vk::BufferImageCopy{}
                        .setImageSubresource(image_subresource_layers)
                        .setImageExtent(extent)
                        .setBufferOffset(layer * width * height * data.GetStride());

                    buffer_image_copies.emplace_back(buffer_image_copy);
                }

                // Copy the first mip of the chain, remaining mips will be generated
                cb.copyBufferToImage(*staging_buffer, *image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copies);

                // Transition first mip level to transfer source so we can blit(read) from it
                auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer->InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader));

                AssignOrReturnError(CMShared<Fence> fence, GetFence());

                ReturnIfError(c->Submit(QueueType::Graphics, command_buffer, fence));
                ReturnIfError(c->WaitForFence(fence));
            }

            return texture;
        }

        if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) || 
            !(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst))
        {
            // Run software mip mapping
            //stb_load

            return StringError("Format not supported for vulkan runtime mipmapping");
        }
        else
        {
            AssignOrReturnError(auto staging_buffer, CreateStagingBuffer(data.GetTotalSize()));
            ReturnIfError(staging_buffer->Copy(data));

            auto extent = vk::Extent3D{ width, height, 1 };

            // Calculate number of mip levels as per Vulkan specs:
            // numLevels = 1 + floor(log2(max(w, h, d)))
            auto mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
            vk::ImageUsageFlags image_usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;

            auto vk_image_create_info = vk::ImageCreateInfo{}
                .setImageType(vk::ImageType::e2D)
                .setFormat(format)
                .setExtent(extent)
                .setMipLevels(mip_levels)
                .setArrayLayers(layer_count)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setUsage(image_usage_flags);

            VmaAllocationCreateInfo vma_allocation_create_info{};
            vma_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vma_allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            ImageCreateInfo image_create_info{ vk_image_create_info, vma_allocation_create_info };
            AssignOrReturnError(CMShared<Image> image, c->CreateImage(image_create_info));

            {
                AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Graphics));

                command_buffer->Begin();
                const vk::CommandBuffer& cb = **command_buffer;

                // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                auto image_subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(layer_count);

                auto transfer_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer->InsertImageMemoryBarrier(transfer_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer));

                // Copy the first mip of the chain, remaining mips will be generated
                std::vector<vk::BufferImageCopy> buffer_image_copies;
                for (uint32_t layer = 0; layer < layer_count; layer++)
                {
                    auto image_subresource_layers = vk::ImageSubresourceLayers{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(0)
                        .setBaseArrayLayer(layer)
                        .setLayerCount(1);

                    auto buffer_image_copy = vk::BufferImageCopy{}
                        .setImageSubresource(image_subresource_layers)
                        .setImageExtent(extent)
                        .setBufferOffset(layer * width * height * data.GetStride());

                    buffer_image_copies.emplace_back(buffer_image_copy);
                }

                // Copy the first mip of the chain, remaining mips will be generated
                cb.copyBufferToImage(*staging_buffer, *image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copies);

                // Transition first mip level to transfer source so we can blit(read) from it
                auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer->InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer));

                AssignOrReturnError(CMShared<Fence> fence, GetFence());

                ReturnIfError(c->Submit(QueueType::Graphics, command_buffer, fence));
                ReturnIfError(c->WaitForFence(fence));
            }

            AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Graphics));

            command_buffer->Begin();
            const vk::CommandBuffer& cb = **command_buffer;

            // Generate the mip chain
            // ---------------------------------------------------------------
            // We copy down the whole mip chain doing a blit from mip-1 to mip
            // An alternative way would be to always blit from the first mip level and sample that one down
            for (uint32_t i = 1; i < mip_levels; i++)
            {
                // Prepare current mip level as image blit destination
                auto image_subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(i)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                auto dst_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer->InsertImageMemoryBarrier(dst_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer));

                auto src_image_subresource_layers = vk::ImageSubresourceLayers{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(i - 1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                auto dst_image_subresource_layers = vk::ImageSubresourceLayers{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(i)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                auto src_mip_width = static_cast<int32_t>(width >> (i - 1));
                auto src_mip_height = static_cast<int32_t>(height >> (i - 1));

                auto dst_mip_width = std::max(static_cast<int32_t>(width >> i), 1);
                auto dst_mip_height = std::max(static_cast<int32_t>(height >> i), 1);

                auto image_blit = vk::ImageBlit{}
                    .setSrcSubresource(src_image_subresource_layers)
                    .setDstSubresource(dst_image_subresource_layers)
                    .setSrcOffsets({ vk::Offset3D{}, vk::Offset3D{src_mip_width, src_mip_height, 1} })
                    .setDstOffsets({ vk::Offset3D{}, vk::Offset3D{dst_mip_width, dst_mip_height, 1} });

                // Blit from previous level
                cb.blitImage(*image, vk::ImageLayout::eTransferSrcOptimal, *image, vk::ImageLayout::eTransferDstOptimal, image_blit, vk::Filter::eLinear);

                // Prepare current mip level as image blit source for next level
                auto src_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer->InsertImageMemoryBarrier(src_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer));
            }

            // After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
            auto image_subresource_range = vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(mip_levels - 1)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            auto src_image_memory_barrier = vk::ImageMemoryBarrier{}
                .setSrcQueueFamilyIndex({})
                .setDstQueueFamilyIndex({})
                .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSubresourceRange(image_subresource_range)
                .setImage(**image);

            PanicIfError(command_buffer->InsertImageMemoryBarrier(src_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader));

            AssignOrReturnError(CMShared<Fence> fence, GetFence());

            ReturnIfError(c->Submit(QueueType::Graphics, command_buffer, fence));
            ReturnIfError(c->WaitForFence(fence));

            image->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);



            auto image_view_create_info = ImageViewCreateInfo
            {
                vk::ImageViewCreateInfo{}
                    .setFlags(vk::ImageViewCreateFlags{})
                    .setImage(*image)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(format)
                    .setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
                    .setSubresourceRange
                    (
                        vk::ImageSubresourceRange{}
                            .setAspectMask(vk::ImageAspectFlagBits::eColor)
                            .setBaseMipLevel(0)
                            .setLevelCount(mip_levels)
                            .setBaseArrayLayer(0)
                            .setLayerCount(1)
                    )
            };

            AssignOrReturnError(CMShared<ImageView> image_view, c->CreateImageView(image_view_create_info));

            auto vk_sampler_create_info = vk::SamplerCreateInfo{}
                .setMagFilter(vk::Filter::eLinear)
                .setMinFilter(vk::Filter::eLinear)
                .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                .setAddressModeU(vk::SamplerAddressMode::eRepeat)
                .setAddressModeV(vk::SamplerAddressMode::eRepeat)
                .setAddressModeW(vk::SamplerAddressMode::eRepeat)
                .setMipLodBias(0.0f)
                .setCompareOp(vk::CompareOp::eNever)
                .setMinLod(0.0f)
                .setMaxLod(static_cast<float>(mip_levels))
                .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
                .setMaxAnisotropy(1.0f)
                .setAnisotropyEnable(VK_FALSE);

            auto physical_device_features = c->GetPhysicalDeviceFeatures();
            if (physical_device_features.samplerAnisotropy)
            {
                auto physical_device_properties = c->GetPhysicalDeviceProperties();
                vk_sampler_create_info
                    .setMaxAnisotropy(physical_device_properties.limits.maxSamplerAnisotropy)
                    .setAnisotropyEnable(VK_TRUE);
            }

            auto sampler_create_info = SamplerCreateInfo{ vk_sampler_create_info };

            TextureCreateInfo texture_create_info
            {
                image_create_info,
                image_view_create_info,
                sampler_create_info
            };

            AssignOrReturnError(CMShared<Texture> texture, c->CreateTexture(texture_create_info));
            return texture;
        }
    }

    Error RenderFrame::UploadData(CMShared<Image> image, PointerView data, uint32_t array_index, uint32_t mip_level, vk::ImageLayout src_image_layout, vk::ImageLayout dst_image_layout)
    {
        auto image_subresource_layers = vk::ImageSubresourceLayers{}
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setMipLevel(mip_level)
            .setBaseArrayLayer(array_index)
            .setLayerCount(1);

        const auto& image_create_info = image->image_create_info.image_create_info;
        const auto& extent = image_create_info.extent;

        auto buffer_image_copy = vk::BufferImageCopy{}
            .setImageSubresource(image_subresource_layers)
            .setImageExtent(extent);

        AssignOrReturnError(auto staging_buffer, CreateStagingBuffer(data.GetTotalSize()));
        ReturnIfError(staging_buffer->Copy(data));

        auto subresource_range = vk::ImageSubresourceRange{}
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(mip_level)
            .setLevelCount(1)
            .setBaseArrayLayer(array_index)
            .setLayerCount(1);

        image->SetImageLayout(dst_image_layout);
        PanicIfError(c->ImmediateGraphicsSubmit([&](CommandBuffer& command_buffer)
            {
                auto& cb = *command_buffer;
                if (src_image_layout != vk::ImageLayout::eTransferDstOptimal)
                {
                    PanicIfError(command_buffer.SetImageLayout(image, src_image_layout, vk::ImageLayout::eTransferDstOptimal, subresource_range));
                }

                cb.copyBufferToImage(*staging_buffer, *image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copy);

                if (dst_image_layout != vk::ImageLayout::eTransferDstOptimal)
                {
                    PanicIfError(command_buffer.SetImageLayout(image, vk::ImageLayout::eTransferDstOptimal, dst_image_layout, subresource_range));
                }
            }));

        return NoError();
    }

    Error RenderFrame::GenerateMipMaps(CommandBuffer& command_buffer, CMShared<Image> image, vk::ImageLayout src_image_layout, vk::ImageLayout dst_image_layout, vk::ImageAspectFlags aspect_flags, vk::Filter filter)
    {
        const auto& image_create_info = image->image_create_info.image_create_info;

        auto mip_level = image_create_info.mipLevels;
        auto array_layers = image_create_info.arrayLayers;

        auto width = image_create_info.extent.width;
        auto height = image_create_info.extent.height;

        auto initial_subresource_range = vk::ImageSubresourceRange{}
            .setAspectMask(aspect_flags)
            .setBaseMipLevel(1)
            .setLevelCount(mip_level - 1)
            .setBaseArrayLayer(0)
            .setLayerCount(array_layers);

        command_buffer.Record([&](CommandBuffer& command_buffer)
            {
                auto& cb = *command_buffer;
                if (src_image_layout != vk::ImageLayout::eTransferDstOptimal)
                {
                    PanicIfError(command_buffer.SetImageLayout(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, initial_subresource_range));
                }

                auto subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(aspect_flags)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                for (auto array_index = 0; array_index < array_layers; array_index++)
                {
                    auto mip_width = width;
                    auto mip_height = height;

                    for (auto mip_index = 1; mip_index < mip_level; mip_index++)
                    {
                        subresource_range.setBaseMipLevel(mip_index - 1);
                        subresource_range.setBaseArrayLayer(array_index);

                        vk::ImageLayout layout = vk::ImageLayout::eTransferDstOptimal;

                        if (mip_index == 1)
                        {
                            layout = src_image_layout;
                        }

                        if (layout != vk::ImageLayout::eTransferSrcOptimal)
                        {
                            PanicIfError(command_buffer.SetImageLayout(image, layout, vk::ImageLayout::eTransferSrcOptimal, subresource_range));
                        }

                        auto src_image_subresource_layers = vk::ImageSubresourceLayers{}
                            .setAspectMask(aspect_flags)
                            .setMipLevel(mip_index - 1)
                            .setBaseArrayLayer(array_index)
                            .setLayerCount(1);

                        auto dst_image_subresource_layers = vk::ImageSubresourceLayers{}
                            .setAspectMask(aspect_flags)
                            .setMipLevel(mip_index)
                            .setBaseArrayLayer(array_index)
                            .setLayerCount(1);

                        auto src_mip_width = static_cast<int32_t>(mip_width);
                        auto src_mip_height = static_cast<int32_t>(mip_height);
                            
                        auto dst_mip_width = std::max(static_cast<int32_t>(mip_width / 2), 1);
                        auto dst_mip_height = std::max(static_cast<int32_t>(mip_height / 2), 1);

                        auto image_blit = vk::ImageBlit{}
                            .setSrcSubresource(src_image_subresource_layers)
                            .setDstSubresource(dst_image_subresource_layers)
                            .setSrcOffsets({ vk::Offset3D{}, vk::Offset3D{src_mip_width, src_mip_height, 1} })
                            .setDstOffsets({ vk::Offset3D{}, vk::Offset3D{dst_mip_width, dst_mip_height, 1} });

                        cb.blitImage(*image, vk::ImageLayout::eTransferSrcOptimal, *image, vk::ImageLayout::eTransferDstOptimal, image_blit, filter);

                        PanicIfError(command_buffer.SetImageLayout(image, vk::ImageLayout::eTransferSrcOptimal, dst_image_layout, subresource_range));

                        if (mip_width > 1) mip_width /= 2;
                        if (mip_height > 1) mip_height /= 2;
                    }

                    subresource_range.setBaseMipLevel(mip_level - 1);

                    PanicIfError(command_buffer.SetImageLayout(image, vk::ImageLayout::eTransferDstOptimal, dst_image_layout, subresource_range));
                }
            });

        return NoError();
    }


    Error RenderFrame::GenerateMipMaps(CMShared<CommandBuffer> command_buffer, CMShared<Image> image, vk::ImageLayout src_image_layout, vk::ImageLayout dst_image_layout, vk::ImageAspectFlags aspect_flags, vk::Filter filter)
    {
        return GenerateMipMaps(*command_buffer, image, src_image_layout, dst_image_layout, aspect_flags, filter);
    }

    Error RenderFrame::GenerateMipMaps(CMShared<Image> image, vk::ImageLayout src_image_layout, vk::ImageLayout dst_image_layout, vk::ImageAspectFlags aspect_flags, vk::Filter filter)
    {
        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Graphics));

        image->SetImageLayout(dst_image_layout);
        PanicIfError(GenerateMipMaps(command_buffer, image, src_image_layout, dst_image_layout, aspect_flags, filter));

        AssignOrReturnError(CMShared<Fence> fence, GetFence());
        ReturnIfError(c->Submit(QueueType::Graphics, command_buffer, fence));
        ReturnIfError(c->WaitForFence(fence));

        return NoError();
    }

    Expected<CMShared<Buffer>> RenderFrame::CreateBuffer(MemoryUsage memory_usage, vk::BufferUsageFlags buffer_usage_flags, std::size_t size, bool map)
    {
        VmaAllocationCreateInfo vma_allocation_create_info{};
        vma_allocation_create_info.usage = static_cast<VmaMemoryUsage>(memory_usage);

        if (map)
        {
            vma_allocation_create_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        BufferCreateInfo buffer_create_info
        {
            vma_allocation_create_info,
            buffer_usage_flags,
            size
        };

        return c->CreateBuffer(buffer_create_info);
    }

    Expected<CMShared<Buffer>> RenderFrame::ResizeBuffer(CMShared<Buffer> buffer, std::size_t size)
    {
        auto buffer_create_info = buffer->buffer_create_info;
        buffer_create_info.size = size;

        AssignOrReturnError(auto new_buffer, c->CreateBuffer(buffer_create_info));
        PanicIfError(new_buffer->Copy(buffer->GetPointerView()));

        return new_buffer;
    }

    Expected<CMShared<Buffer>> RenderFrame::CopyBuffer(CMShared<Buffer> buffer)
    {
        const auto& buffer_create_info = buffer->buffer_create_info;

        AssignOrReturnError(auto new_buffer, c->CreateBuffer(buffer_create_info));
        PanicIfError(new_buffer->Copy(buffer->GetPointerView()));

        return new_buffer;
    }

    Expected<CMShared<Buffer>> RenderFrame::CreateStagingBuffer(std::size_t size)
    {
        MemoryUsage memory_usage = MemoryUsage::CpuOnly;
        vk::BufferUsageFlags buffer_usage_flags = vk::BufferUsageFlagBits::eTransferSrc;
        bool map = true;

        return CreateBuffer(memory_usage, buffer_usage_flags, size, map);
    }

    Expected<TransferJob> RenderFrame::UploadDataToGPUBuffer(CMShared<Buffer> upload_buffer, vk::BufferUsageFlags buffer_usage_flags)
    {
        auto cpu_buffer_usage_flags = upload_buffer->buffer_create_info.buffer_usage_flags;
        auto size = upload_buffer->buffer_create_info.size;

        if (!(cpu_buffer_usage_flags & vk::BufferUsageFlagBits::eTransferSrc))
        {
            return StringError("Cpu buffer is not suppose to {} does not have {}!", vk::to_string(cpu_buffer_usage_flags), vk::to_string(buffer_usage_flags));
        }

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Transfer));
        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, GetSemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, GetFence());

        c->SetName(command_buffer, "Upload GPU buffer command buffer");
        c->SetName(upload_semaphore, "Upload GPU buffer semaphore");
        c->SetName(upload_fence, "Upload GPU buffer fence");

        AssignOrReturnError(CMShared<Buffer> gpu_buffer, CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eTransferDst | buffer_usage_flags, size));

        c->SetName(upload_fence, "Upload GPU buffer");

        command_buffer->AddDependencyTo(upload_buffer);
        command_buffer->AddDependencyTo(gpu_buffer);
        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(c->GetProfiler(), command_buffer, "Upload buffer to gpu");
                auto& cb = *command_buffer;
                auto buffer_copy = vk::BufferCopy{}
                    .setDstOffset(0)
                    .setSrcOffset(0)
                    .setSize(size);

                cb.copyBuffer(*upload_buffer, *gpu_buffer, buffer_copy);
                CollectGPUProfile(c->GetProfiler(), command_buffer);
            });
        ReturnIfError(c->Submit(QueueType::Transfer, command_buffer, upload_fence, {}, {}, upload_semaphore));

        FrameCounter frame_counter = c->GetFrame();
        TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, upload_buffer, gpu_buffer };
        return transfer_job;
    }

    Expected<TransferJob> RenderFrame::UploadDataToGPUBuffer(PointerView data, vk::BufferUsageFlags buffer_usage_flags)
    {
        AssignOrReturnError(CMShared<Buffer> upload_buffer, CreateBuffer(MemoryUsage::CpuOnly, vk::BufferUsageFlagBits::eTransferSrc, data.GetTotalSize(), true));
        PanicIfError(upload_buffer->Copy(data));

        return UploadDataToGPUBuffer(upload_buffer, buffer_usage_flags);
    }

    Expected<TransferJob> RenderFrame::UploadDataToGPUTexture(vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t depth)
    {
        FrameCounter frame_counter = c->GetFrame();

        vk::PhysicalDevice physical_device = c->GetPhysicalDevice();
        vk::FormatProperties format_properties = physical_device.getFormatProperties(format);

        uint32_t mip_levels = 1;
        bool can_be_runtime_mipmapped = (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) || (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst);
        can_be_runtime_mipmapped = false;
        if (can_be_runtime_mipmapped)
        {
            mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        }

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Transfer));
        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, GetSemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, GetFence());

        c->SetName(command_buffer, "Upload GPU texture command buffer");
        c->SetName(upload_semaphore, "Upload GPU texture semaphore");
        c->SetName(upload_fence, "Upload GPU texture fence");

        AssignOrReturnError(CMShared<Buffer> upload_buffer, CreateStagingBuffer(data.GetTotalSize()));
        
        c->SetName(upload_fence, "Upload GPU texture buffer");

        PanicIfError(upload_buffer->Copy(data));

        auto extent = vk::Extent3D{ width, height, 1 };

        vk::ImageUsageFlags image_usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;

        auto vk_image_create_info = vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setFormat(format)
            .setExtent(extent)
            .setMipLevels(mip_levels)
            .setArrayLayers(depth)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(image_usage_flags);

        VmaAllocationCreateInfo vma_allocation_create_info{};
        vma_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vma_allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        ImageCreateInfo image_create_info{ vk_image_create_info, vma_allocation_create_info };

        auto image_view_create_info = ImageViewCreateInfo
        {
            vk::ImageViewCreateInfo{}
                .setFlags(vk::ImageViewCreateFlags{})
            //.setImage(*image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(format)
            .setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
            .setSubresourceRange
            (
                vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(mip_levels)
                    .setBaseArrayLayer(0)
                    .setLayerCount(depth)
            )
        };

        auto vk_sampler_create_info = vk::SamplerCreateInfo{}
            .setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setMipLodBias(0.0f)
            .setCompareOp(vk::CompareOp::eNever)
            .setMinLod(0.0f)
            .setMaxLod(static_cast<float>(mip_levels))
            .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
            .setMaxAnisotropy(1.0f)
            .setAnisotropyEnable(VK_FALSE);

        auto physical_device_features = c->GetPhysicalDeviceFeatures();
        if (physical_device_features.samplerAnisotropy)
        {
            auto physical_device_properties = c->GetPhysicalDeviceProperties();
            vk_sampler_create_info
                .setMaxAnisotropy(physical_device_properties.limits.maxSamplerAnisotropy)
                .setAnisotropyEnable(VK_TRUE);
        }

        auto sampler_create_info = SamplerCreateInfo{ vk_sampler_create_info };

        TextureCreateInfo texture_create_info
        {
            image_create_info,
            image_view_create_info,
            sampler_create_info
        };

        AssignOrReturnError(CMShared<Texture> texture, c->CreateTexture(texture_create_info));
        auto& image = texture->image;
        image->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        command_buffer->AddDependencyTo(upload_buffer);
        command_buffer->AddDependencyTo(texture);
        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(c->GetProfiler(), command_buffer, "Upload buffer to texture");
                auto& cb = *command_buffer;

                // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                auto image_subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(depth);

                auto transfer_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer.InsertImageMemoryBarrier(transfer_image_memory_barrier, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer));

                // Copy the first mip of the chain, remaining mips will be generated
                std::vector<vk::BufferImageCopy> buffer_image_copies;
                for (uint32_t layer = 0; layer < depth; layer++)
                {
                    auto image_subresource_layers = vk::ImageSubresourceLayers{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(0)
                        .setBaseArrayLayer(layer)
                        .setLayerCount(1);

                    auto buffer_image_copy = vk::BufferImageCopy{}
                        .setImageSubresource(image_subresource_layers)
                        .setImageExtent(extent)
                        .setBufferOffset(layer * width * height * data.GetStride());

                    buffer_image_copies.emplace_back(buffer_image_copy);
                }

                // Copy the first mip of the chain, remaining mips will be generated
                cb.copyBufferToImage(*upload_buffer, *image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copies);

                if (c->IsUnifiedGraphicsAndTransferQueue())
                {
                    // Transition first mip level to transfer source so we can blit(read) from it
                    auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                        .setSrcQueueFamilyIndex({})
                        .setDstQueueFamilyIndex({})
                        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setSubresourceRange(image_subresource_range)
                        .setImage(**image);

                    PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader));
                }
                else
                {
                    // Transition first mip level to transfer source so we can blit(read) from it
                    if (!can_be_runtime_mipmapped)
                    {
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                            .setDstAccessMask({})
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe));
                    }
                    else
                    {
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe));
                    }
                }
                //CollectGPUProfile(c->GetProfiler(), command_buffer);
            });

        ReturnIfError(c->Submit(QueueType::Transfer, command_buffer, upload_fence, {}, {}, upload_semaphore));

        if (!can_be_runtime_mipmapped)
        {
            if (c->IsUnifiedGraphicsAndTransferQueue())
            {
                TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, upload_buffer, {}, texture };
                return transfer_job;
            }
            else
            {
                CMShared<Semaphore> graphics_sync_semaphore;
                CMShared<Fence> graphics_sync_fence;
                CMShared<CommandBuffer> graphics_sync_command_buffer;

                AssignOrReturnError(graphics_sync_command_buffer, GetCommandBuffer(QueueType::Graphics));
                AssignOrReturnError(graphics_sync_semaphore, GetSemaphore());
                AssignOrReturnError(graphics_sync_fence, GetFence());


                c->SetName(command_buffer, "Upload GPU texture sync command buffer");
                c->SetName(upload_semaphore, "Upload GPU texture sync semaphore");
                c->SetName(upload_fence, "Upload GPU texture sync fence");

                graphics_sync_fence->AddCompletionTo(graphics_sync_command_buffer);
                graphics_sync_command_buffer->AddDependencyTo(upload_buffer);
                graphics_sync_command_buffer->AddDependencyTo(texture);
                graphics_sync_command_buffer->Record([=](CommandBuffer& command_buffer)
                    {
                        auto& cb = *command_buffer;

                        auto image_subresource_range = vk::ImageSubresourceRange{}
                            .setAspectMask(vk::ImageAspectFlagBits::eColor)
                            .setBaseMipLevel(0)
                            .setLevelCount(1)
                            .setBaseArrayLayer(0)
                            .setLayerCount(depth);

                        // Transition first mip level to transfer source so we can blit(read) from it
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask({})
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eFragmentShader));
                    }
                );
                ReturnIfError(c->Submit(QueueType::Graphics, graphics_sync_command_buffer, graphics_sync_fence, {}, {}, graphics_sync_semaphore));

                TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, upload_buffer, {}, texture, graphics_sync_command_buffer, graphics_sync_semaphore, graphics_sync_fence };
                return transfer_job;
            }
        }
        else
        {
            CMShared<CommandBuffer> graphics_sync_command_buffer;
            CMShared<Semaphore> graphics_sync_semaphore;
            CMShared<Fence> graphics_sync_fence;

            AssignOrReturnError(graphics_sync_command_buffer, GetCommandBuffer(QueueType::Graphics));
            AssignOrReturnError(graphics_sync_semaphore, GetSemaphore());
            AssignOrReturnError(graphics_sync_fence, GetFence());
            ReturnIfError(c->ResetFence(graphics_sync_fence));

            c->SetName(command_buffer, "Upload GPU texture sync command buffer");
            c->SetName(upload_semaphore, "Upload GPU texture sync semaphore");
            c->SetName(upload_fence, "Upload GPU texture sync fence");

            graphics_sync_fence->AddCompletionTo(graphics_sync_command_buffer);
            graphics_sync_command_buffer->AddDependencyTo(upload_buffer);
            graphics_sync_command_buffer->AddDependencyTo(texture);

            if (c->IsUnifiedGraphicsAndTransferQueue())
            {
            }
            else
            {
                graphics_sync_command_buffer->Record([=](CommandBuffer& command_buffer)
                    {
                        auto& cb = *command_buffer;

                        auto image_subresource_range = vk::ImageSubresourceRange{}
                            .setAspectMask(vk::ImageAspectFlagBits::eColor)
                            .setBaseMipLevel(0)
                            .setLevelCount(1)
                            .setBaseArrayLayer(0)
                            .setLayerCount(depth);

                        // Transition first mip level to transfer source so we can blit(read) from it
                        /*
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask({})
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);
                        */

                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask({})
                            .setDstAccessMask({})
                            .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                            .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        //PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eFragmentShader));
                    }
                );
            }

            graphics_sync_command_buffer->Begin();
            const vk::CommandBuffer& cb = **graphics_sync_command_buffer;

            auto image_subresource_range = vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                //.setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            auto src_image_subresource_layers = vk::ImageSubresourceLayers{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                //.setMipLevel(i - 1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            auto dst_image_subresource_layers = vk::ImageSubresourceLayers{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                //.setMipLevel(i)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            // Generate the mip chain
            // ---------------------------------------------------------------
            // We copy down the whole mip chain doing a blit from mip-1 to mip
            // An alternative way would be to always blit from the first mip level and sample that one down
            for (uint32_t i = 1; i < mip_levels; i++)
            {
                // Prepare current mip level as image blit destination
                image_subresource_range.setBaseMipLevel(i);

                auto dst_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(graphics_sync_command_buffer->InsertImageMemoryBarrier(dst_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer));

                src_image_subresource_layers.setMipLevel(i - 1);
                dst_image_subresource_layers.setMipLevel(i);

                auto src_mip_width = static_cast<int32_t>(width >> (i - 1));
                auto src_mip_height = static_cast<int32_t>(height >> (i - 1));

                auto dst_mip_width = std::max(static_cast<int32_t>(width >> i), 1);
                auto dst_mip_height = std::max(static_cast<int32_t>(height >> i), 1);

                auto image_blit = vk::ImageBlit{}
                    .setSrcSubresource(src_image_subresource_layers)
                    .setDstSubresource(dst_image_subresource_layers)
                    .setSrcOffsets({ vk::Offset3D{}, vk::Offset3D{src_mip_width, src_mip_height, 1} })
                    .setDstOffsets({ vk::Offset3D{}, vk::Offset3D{dst_mip_width, dst_mip_height, 1} });

                // Blit from previous level
                cb.blitImage(*image, vk::ImageLayout::eTransferSrcOptimal, *image, vk::ImageLayout::eTransferDstOptimal, image_blit, vk::Filter::eLinear);

                // Prepare current mip level as image blit source for next level
                auto src_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(graphics_sync_command_buffer->InsertImageMemoryBarrier(src_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer));
            }

            // After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
            image_subresource_range.setBaseMipLevel(mip_levels - 1);

            auto src_image_memory_barrier = vk::ImageMemoryBarrier{}
                .setSrcQueueFamilyIndex({})
                .setDstQueueFamilyIndex({})
                .setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSubresourceRange(image_subresource_range)
                .setImage(**image);

            PanicIfError(graphics_sync_command_buffer->InsertImageMemoryBarrier(src_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader));
            
            ReturnIfError(c->Submit(QueueType::Graphics, graphics_sync_command_buffer, graphics_sync_fence, {}, {}, graphics_sync_semaphore));

            TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, upload_buffer, {}, texture, graphics_sync_command_buffer, graphics_sync_semaphore, graphics_sync_fence };
            return transfer_job;
        }
    }
    
    Expected<TransferJob> RenderFrame::UploadDataToGPUTexture2DArray(vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t depth)
    {
        FrameCounter frame_counter = c->GetFrame();

        vk::PhysicalDevice physical_device = c->GetPhysicalDevice();
        vk::FormatProperties format_properties = physical_device.getFormatProperties(format);

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Graphics));
        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, GetSemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, GetFence());

        c->SetName(command_buffer, "Upload GPU texture command buffer");
        c->SetName(upload_semaphore, "Upload GPU texture semaphore");
        c->SetName(upload_fence, "Upload GPU texture fence");

        AssignOrReturnError(CMShared<Buffer> upload_buffer, CreateStagingBuffer(data.GetTotalSize()));

        c->SetName(upload_fence, "Upload GPU texture buffer");

        PanicIfError(upload_buffer->Copy(data));

        auto extent = vk::Extent3D{ width, height, 1 };
        auto mip_levels = 1;

        vk::ImageUsageFlags image_usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;

        auto vk_image_create_info = vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setFormat(format)
            .setExtent(extent)
            .setMipLevels(mip_levels)
            .setArrayLayers(depth)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(image_usage_flags);

        VmaAllocationCreateInfo vma_allocation_create_info{};
        vma_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vma_allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        ImageCreateInfo image_create_info{ vk_image_create_info, vma_allocation_create_info };

        auto image_view_create_info = ImageViewCreateInfo
        {
            vk::ImageViewCreateInfo{}
                .setFlags(vk::ImageViewCreateFlags{})
                .setViewType(vk::ImageViewType::e2DArray)
                .setFormat(format)
                .setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
                .setSubresourceRange
                (
                    vk::ImageSubresourceRange{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setBaseMipLevel(0)
                        .setLevelCount(mip_levels)
                        .setBaseArrayLayer(0)
                        .setLayerCount(depth)
                )
        };

        auto vk_sampler_create_info = vk::SamplerCreateInfo{}
            .setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
            .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
            .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
            .setMipLodBias(0.0f)
            .setCompareOp(vk::CompareOp::eNever)
            .setMinLod(0.0f)
            .setMaxLod(12.0f)
            .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
            .setMaxAnisotropy(1.0f)
            .setAnisotropyEnable(VK_FALSE);

        auto sampler_create_info = SamplerCreateInfo{ vk_sampler_create_info };

        TextureCreateInfo texture_create_info
        {
            image_create_info,
            image_view_create_info,
            sampler_create_info
        };

        AssignOrReturnError(CMShared<Texture> texture, c->CreateTexture(texture_create_info));
        auto& image = texture->image;
        image->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        command_buffer->AddDependencyTo(upload_buffer);
        command_buffer->AddDependencyTo(texture);
        command_buffer->Record([&](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(c->GetProfiler(), command_buffer, "Upload buffer to texture");
                auto& cb = *command_buffer;

                // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                auto image_subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(depth);

                auto transfer_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer.InsertImageMemoryBarrier(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, image_subresource_range));
                //PanicIfError(command_buffer.InsertImageMemoryBarrier(transfer_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));

                // Copy the first mip of the chain, remaining mips will be generated
                std::vector<vk::BufferImageCopy> buffer_image_copies;
                for (uint32_t layer = 0; layer < depth; layer++)
                {
                    auto image_subresource_layers = vk::ImageSubresourceLayers{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(0)
                        .setBaseArrayLayer(layer)
                        .setLayerCount(1);

                    auto buffer_image_copy = vk::BufferImageCopy{}
                        .setImageSubresource(image_subresource_layers)
                        .setImageExtent(extent)
                        .setBufferOffset(layer * width * height * data.GetStride());

                    buffer_image_copies.emplace_back(buffer_image_copy);
                }

                // Copy the first mip of the chain, remaining mips will be generated
                cb.copyBufferToImage(*upload_buffer, *image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copies);

                // Transition first mip level to transfer source so we can blit(read) from it
                auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer.InsertImageMemoryBarrier(image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, image_subresource_range));
                //PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
            });

        ReturnIfError(c->Submit(QueueType::Graphics, command_buffer, upload_fence, {}, {}, upload_semaphore));

        TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, upload_buffer, {}, texture, {}, {}, {} };
        return transfer_job;
    }

    Expected<TransferJob> RenderFrame::UploadDataToGPUTexture3D(vk::Format format, PointerView data, uint32_t width, uint32_t height, uint32_t depth)
    {
        FrameCounter frame_counter = c->GetFrame();

        vk::PhysicalDevice physical_device = c->GetPhysicalDevice();
        vk::FormatProperties format_properties = physical_device.getFormatProperties(format);

        uint32_t mip_levels = 1;
        bool can_be_runtime_mipmapped = (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) || (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst);
        can_be_runtime_mipmapped = false;
        if (can_be_runtime_mipmapped)
        {
            mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        }

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Transfer));
        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, GetSemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, GetFence());

        c->SetName(command_buffer, "Upload GPU texture command buffer");
        c->SetName(upload_semaphore, "Upload GPU texture semaphore");
        c->SetName(upload_fence, "Upload GPU texture fence");

        AssignOrReturnError(CMShared<Buffer> upload_buffer, CreateStagingBuffer(data.GetTotalSize()));

        c->SetName(upload_fence, "Upload GPU texture buffer");

        PanicIfError(upload_buffer->Copy(data));

        auto extent = vk::Extent3D{ width, height, depth };

        vk::ImageUsageFlags image_usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;

        auto vk_image_create_info = vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e3D)
            .setFormat(format)
            .setExtent(extent)
            .setMipLevels(mip_levels)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(image_usage_flags);

        VmaAllocationCreateInfo vma_allocation_create_info{};
        vma_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vma_allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        ImageCreateInfo image_create_info{ vk_image_create_info, vma_allocation_create_info };

        auto image_view_create_info = ImageViewCreateInfo
        {
            vk::ImageViewCreateInfo{}
                .setFlags(vk::ImageViewCreateFlags{})
            //.setImage(*image)
            .setViewType(vk::ImageViewType::e3D)
            .setFormat(format)
            .setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
            .setSubresourceRange
            (
                vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(mip_levels)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)
            )
        };

        auto vk_sampler_create_info = vk::SamplerCreateInfo{}
            .setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setMipLodBias(0.0f)
            .setCompareOp(vk::CompareOp::eNever)
            .setMinLod(0.0f)
            .setMaxLod(static_cast<float>(mip_levels))
            .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
            .setMaxAnisotropy(1.0f)
            .setAnisotropyEnable(VK_FALSE);

        auto physical_device_features = c->GetPhysicalDeviceFeatures();
        if (physical_device_features.samplerAnisotropy)
        {
            auto physical_device_properties = c->GetPhysicalDeviceProperties();
            vk_sampler_create_info
                .setMaxAnisotropy(physical_device_properties.limits.maxSamplerAnisotropy)
                .setAnisotropyEnable(VK_TRUE);
        }

        auto sampler_create_info = SamplerCreateInfo{ vk_sampler_create_info };

        TextureCreateInfo texture_create_info
        {
            image_create_info,
            image_view_create_info,
            sampler_create_info
        };

        AssignOrReturnError(CMShared<Texture> texture, c->CreateTexture(texture_create_info));
        auto& image = texture->image;
        image->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        command_buffer->AddDependencyTo(upload_buffer);
        command_buffer->AddDependencyTo(texture);
        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(c->GetProfiler(), command_buffer, "Upload buffer to texture");
                auto& cb = *command_buffer;

                // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                auto image_subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(depth);

                auto transfer_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer.InsertImageMemoryBarrier(transfer_image_memory_barrier, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer));

                // Copy the first mip of the chain, remaining mips will be generated
                std::vector<vk::BufferImageCopy> buffer_image_copies;
                {
                    auto image_subresource_layers = vk::ImageSubresourceLayers{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(0)
                        .setBaseArrayLayer(0)
                        .setLayerCount(1);

                    auto buffer_image_copy = vk::BufferImageCopy{}
                        .setImageSubresource(image_subresource_layers)
                        .setImageExtent(extent)
                        .setBufferOffset(0);

                    buffer_image_copies.emplace_back(buffer_image_copy);
                }

                // Copy the first mip of the chain, remaining mips will be generated
                cb.copyBufferToImage(*upload_buffer, *image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copies);

                if (c->IsUnifiedGraphicsAndTransferQueue())
                {
                    // Transition first mip level to transfer source so we can blit(read) from it
                    auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                        .setSrcQueueFamilyIndex({})
                        .setDstQueueFamilyIndex({})
                        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setSubresourceRange(image_subresource_range)
                        .setImage(**image);

                    PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader));
                }
                else
                {
                    // Transition first mip level to transfer source so we can blit(read) from it
                    if (!can_be_runtime_mipmapped)
                    {
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                            .setDstAccessMask({})
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe));
                    }
                    else
                    {
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe));
                    }
                }
                //CollectGPUProfile(c->GetProfiler(), command_buffer);
            });

        ReturnIfError(c->Submit(QueueType::Transfer, command_buffer, upload_fence, {}, {}, upload_semaphore));

        TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, upload_buffer, {}, texture };
        return transfer_job;
    }

    Expected<TransferJob> RenderFrame::UploadDataToGPUTextureCubeMap(vk::Format format, PointerView data, uint32_t width, uint32_t height)
    {
        FrameCounter frame_counter = c->GetFrame();

        vk::PhysicalDevice physical_device = c->GetPhysicalDevice();
        vk::FormatProperties format_properties = physical_device.getFormatProperties(format);

        uint32_t mip_levels = 1;
        bool can_be_runtime_mipmapped = (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) || (format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst);
        can_be_runtime_mipmapped = false;
        if (can_be_runtime_mipmapped)
        {
            mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        }

        auto depth = 6;

        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, GetCommandBuffer(QueueType::Transfer));
        AssignOrReturnError(CMShared<Semaphore> upload_semaphore, GetSemaphore());
        AssignOrReturnError(CMShared<Fence> upload_fence, GetFence());

        c->SetName(command_buffer, "Upload GPU texture command buffer");
        c->SetName(upload_semaphore, "Upload GPU texture semaphore");
        c->SetName(upload_fence, "Upload GPU texture fence");

        AssignOrReturnError(CMShared<Buffer> upload_buffer, CreateStagingBuffer(data.GetTotalSize()));

        c->SetName(upload_fence, "Upload GPU texture buffer");

        PanicIfError(upload_buffer->Copy(data));

        auto extent = vk::Extent3D{ width, height, 1 };

        vk::ImageUsageFlags image_usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;

        auto vk_image_create_info = vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e3D)
            .setFormat(format)
            .setExtent(extent)
            .setMipLevels(mip_levels)
            .setArrayLayers(depth)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(image_usage_flags);

        VmaAllocationCreateInfo vma_allocation_create_info{};
        vma_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vma_allocation_create_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        ImageCreateInfo image_create_info{ vk_image_create_info, vma_allocation_create_info };

        auto image_view_create_info = ImageViewCreateInfo
        {
            vk::ImageViewCreateInfo{}
                .setFlags(vk::ImageViewCreateFlags{})
            //.setImage(*image)
            .setViewType(vk::ImageViewType::eCube)
            .setFormat(format)
            .setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
            .setSubresourceRange
            (
                vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(mip_levels)
                    .setBaseArrayLayer(0)
                    .setLayerCount(depth)
            )
        };

        auto vk_sampler_create_info = vk::SamplerCreateInfo{}
            .setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setMipLodBias(0.0f)
            .setCompareOp(vk::CompareOp::eNever)
            .setMinLod(0.0f)
            .setMaxLod(static_cast<float>(mip_levels))
            .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
            .setMaxAnisotropy(1.0f)
            .setAnisotropyEnable(VK_FALSE);

        auto physical_device_features = c->GetPhysicalDeviceFeatures();
        if (physical_device_features.samplerAnisotropy)
        {
            auto physical_device_properties = c->GetPhysicalDeviceProperties();
            vk_sampler_create_info
                .setMaxAnisotropy(physical_device_properties.limits.maxSamplerAnisotropy)
                .setAnisotropyEnable(VK_TRUE);
        }

        auto sampler_create_info = SamplerCreateInfo{ vk_sampler_create_info };

        TextureCreateInfo texture_create_info
        {
            image_create_info,
            image_view_create_info,
            sampler_create_info
        };

        AssignOrReturnError(CMShared<Texture> texture, c->CreateTexture(texture_create_info));
        auto& image = texture->image;
        image->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        command_buffer->AddDependencyTo(upload_buffer);
        command_buffer->AddDependencyTo(texture);
        command_buffer->Record([=](CommandBuffer& command_buffer)
            {
                ScopedGPUProfile(c->GetProfiler(), command_buffer, "Upload buffer to texture");
                auto& cb = *command_buffer;

                // Optimal image will be used as destination for the copy, so we must transfer from our initial undefined image layout to the transfer destination layout
                auto image_subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(depth);

                auto transfer_image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setSrcQueueFamilyIndex({})
                    .setDstQueueFamilyIndex({})
                    .setSrcAccessMask({})
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSubresourceRange(image_subresource_range)
                    .setImage(**image);

                PanicIfError(command_buffer.InsertImageMemoryBarrier(transfer_image_memory_barrier, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer));

                // Copy the first mip of the chain, remaining mips will be generated
                std::vector<vk::BufferImageCopy> buffer_image_copies;
                for (uint32_t layer = 0; layer < depth; layer++)
                {
                    auto image_subresource_layers = vk::ImageSubresourceLayers{}
                        .setAspectMask(vk::ImageAspectFlagBits::eColor)
                        .setMipLevel(0)
                        .setBaseArrayLayer(layer)
                        .setLayerCount(1);

                    auto buffer_image_copy = vk::BufferImageCopy{}
                        .setImageSubresource(image_subresource_layers)
                        .setImageExtent(extent)
                        .setBufferOffset(layer * width * height * data.GetStride());

                    buffer_image_copies.emplace_back(buffer_image_copy);
                }

                // Copy the first mip of the chain, remaining mips will be generated
                cb.copyBufferToImage(*upload_buffer, *image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copies);

                if (c->IsUnifiedGraphicsAndTransferQueue())
                {
                    // Transition first mip level to transfer source so we can blit(read) from it
                    auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                        .setSrcQueueFamilyIndex({})
                        .setDstQueueFamilyIndex({})
                        .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setSubresourceRange(image_subresource_range)
                        .setImage(**image);

                    PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader));
                }
                else
                {
                    // Transition first mip level to transfer source so we can blit(read) from it
                    if (!can_be_runtime_mipmapped)
                    {
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                            .setDstAccessMask({})
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe));
                    }
                    else
                    {
                        auto copy_image_memory_barrier = vk::ImageMemoryBarrier{}
                            .setSrcQueueFamilyIndex(c->GetTransferQueueFamily())
                            .setDstQueueFamilyIndex(c->GetGraphicsQueueFamily())
                            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                            .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                            .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                            .setSubresourceRange(image_subresource_range)
                            .setImage(**image);

                        PanicIfError(command_buffer.InsertImageMemoryBarrier(copy_image_memory_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe));
                    }
                }
                //CollectGPUProfile(c->GetProfiler(), command_buffer);
            });

        ReturnIfError(c->Submit(QueueType::Transfer, command_buffer, upload_fence, {}, {}, upload_semaphore));

        TransferJob transfer_job{ frame_counter, command_buffer, upload_semaphore, upload_fence, upload_buffer, {}, texture };
        return transfer_job;
    }

    Expected<CMShared<CommandBuffer>> RenderFrame::GetCommandBuffer(QueueType queue_type, vk::CommandPoolCreateFlags command_pool_create_flags)
    {
        auto command_pool_create_info = CommandPoolCreateInfo{ queue_type, command_pool_create_flags };
        AssignOrReturnError(CMShared<CommandPool> command_pool, Get(command_pool_create_info));

        CommandBufferCreateInfo command_buffer_create_info{ command_pool, vk::CommandBufferLevel::ePrimary };
        AssignOrReturnError(CMShared<CommandBuffer> command_buffer, Get(command_buffer_create_info));

        return command_buffer;
    }

    Expected<CMShared<CommandBuffer>> RenderFrame::GetCommandBufferFromResetPool(QueueType queue_type)
    {
        vk::CommandPoolCreateFlags command_pool_create_flags = vk::CommandPoolCreateFlagBits::eTransient;
        auto command_pool_create_info = CommandPoolCreateInfo{ queue_type, command_pool_create_flags };
        AssignOrReturnError(CMShared<CommandPool> command_pool, Get(command_pool_create_info));

        CommandBufferCreateInfo command_buffer_create_info{ command_pool, vk::CommandBufferLevel::ePrimary };
        CMShared<CommandBuffer> command_buffer = AssignOrPanic(c->CreateCommandBuffer(command_buffer_create_info));

        reset_command_pool_to_buffers[command_pool].emplace_back(command_buffer);

        return command_buffer;
    }

    Error RenderFrame::ResetCommandBufferPools()
    {
        for (auto& [reset_command_pool, reset_command_buffers] : reset_command_pool_to_buffers)
        {
            for (auto& reset_command_buffer : reset_command_buffers)
            {
                if (reset_command_buffer->HasInUse())
                {
                    if (reset_command_buffer->IsInUse())
                    {
                        return StringError("Command buffer is in use -> need to be not used to reset");
                    }
                    else
                    {
                        reset_command_buffer->Reset();
                    }
                }
                else
                {
                    return StringError("Command buffer has no dependency -> required for reset");
                }
            }
            reset_command_buffers.clear();

            ReturnIfVkError(c->GetDevice().resetCommandPool(reset_command_pool->command_pool, vk::CommandPoolResetFlagBits::eReleaseResources));
        }


        /*
        for (const auto& [command_pool_create_info, command_pool] : command_pool_cache)
        {
            if (c->GetFrameIndex() != command_pool.frame_counter)
            {
                const CommandPool& pool = *command_pool.t;
                auto reset_flags = vk::CommandPoolResetFlags{}; //vk::CommandPoolResetFlagBits::eReleaseResources
                c->GetDevice().resetCommandPool(*pool, reset_flags);
            }
        }
        */

        return NoError();
    }

    Expected<CMShared<CommandBuffer>> RenderFrame::GetSecondaryCommandBuffer(QueueType queue_type, vk::CommandPoolCreateFlags command_pool_create_flags)
    {
        auto command_pool_create_info = CommandPoolCreateInfo{ queue_type, command_pool_create_flags };
        AssignOrReturnError(CMShared<CommandPool> command_pool, Get(command_pool_create_info));

        CommandBufferCreateInfo command_buffer_create_info{ command_pool, vk::CommandBufferLevel::eSecondary };
        auto command_buffer = AssignOrPanic(Get(command_buffer_create_info));

        return command_buffer;
    }

    Error RenderFrame::AddDependencyBetween(CMShared<Fence> fence, CMShared<CommandBuffer> command_buffer)
    {
        PANIC("Not supported anymore");
        /*
        command_buffer->SetInUse(true);
        ReturnIfError(fence->AddCompletionCallback([command_buffer]()
            {
                command_buffer->SetInUse(false);
            }));
        */
        return NoError();
    }

    Error RenderFrame::AddBeginningOfFrameUpdate(std::function<void()> callback)
    {
        beginning_of_frame_updates.emplace_back(callback);
        return NoError();
    }

    Error RenderFrame::PerformBeginningOfFrameUpdates()
    {
        for (const auto& callback : beginning_of_frame_updates)
        {
            callback();
        }
        beginning_of_frame_updates.clear();

        vk::Device device = c->GetDevice();

        {
            auto& fence_pool_used = fence_pool.GetUsed();
            auto& fence_pool_unused = fence_pool.GetUnused();
            for (auto iter = std::begin(fence_pool_used); iter != std::end(fence_pool_used);)
            {
                const auto& fence = *iter;
                vk::Result result = device.getFenceStatus(*fence);
                if (result == vk::Result::eSuccess)
                {
                    // signaled
                    ReturnIfError(c->WaitForFence(fence));
                    ReturnIfError(c->ResetFence(fence));
                    ReturnIfError(fence->PerformCompletionCallbacks());

                    fence_pool_unused.insert(fence);
                    iter = fence_pool_used.erase(iter);
                    continue;
                }
                else if (result == vk::Result::eNotReady)
                {
                    // not signaled
                }
#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
                else if (result == vk::Result::eErrorDeviceLost)
                {
                    auto tdr_termination_timeout = std::chrono::seconds(3);
                    auto start = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::milliseconds::zero();

                    GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
                    AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

                    while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
                        status != GFSDK_Aftermath_CrashDump_Status_Finished &&
                        elapsed < tdr_termination_timeout)
                    {
                        // Sleep 50ms and poll the status again until timeout or Aftermath finished processing the crash dump.
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

                        auto end = std::chrono::steady_clock::now();
                        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    }

                    if (status != GFSDK_Aftermath_CrashDump_Status_Finished)
                    {
                        PANIC("Aftermath Error, unexpected dump error {}", status);
                    }
                    ReturnIfVkError(result);
                }
#endif
                else
                {
                    ReturnIfVkError(result);
                }
                iter++;
            }
        }

        for (auto& [command_buffer_create_info, command_buffer_pools] : command_buffer_cache)
        {
            auto& command_buffer_pool = command_buffer_pools.t;
            auto& command_buffer_pool_used = command_buffer_pool.GetUsed();
            auto& command_buffer_pool_unused = command_buffer_pool.GetUnused();

            for (auto iter = std::begin(command_buffer_pool_used); iter != std::end(command_buffer_pool_used);)
            {
                auto& command_buffer = *iter;

                // Manual in use switch
                if (command_buffer->HasInUse())
                {
                    if (!command_buffer->IsInUse())
                    {
                        command_buffer->Reset();
                        command_buffer_pool_unused.insert(command_buffer);
                        iter = command_buffer_pool_used.erase(iter);
                        continue;
                    }
                    else
                    {
                        // Still in use, do nothing
                    }
                }
                else
                {
                    // Reset automatically, no dependency
                    command_buffer->Reset();
                    command_buffer_pool_unused.insert(command_buffer);
                    iter = command_buffer_pool_used.erase(iter);
                    continue;
                }
                iter++;
            }
        }

        {
            for (auto& [descriptor_set_create_info, descriptor_set_pools] : descriptor_set_cache)
            {
                auto& descriptor_set_pool = descriptor_set_pools.t;
                auto& descriptor_set_pool_used = descriptor_set_pool.GetUsed();
                auto& descriptor_set_pool_unused = descriptor_set_pool.GetUnused();

                for (auto iter = std::begin(descriptor_set_pool_used); iter != std::end(descriptor_set_pool_used);)
                {
                    auto& descriptor_set = *iter;

                    // Manual in use switch
                    if (descriptor_set->HasInUse())
                    {
                        if (!descriptor_set->IsInUse())
                        {
                            descriptor_set->ResetInUse();
                            descriptor_set_pool_unused.insert(descriptor_set);
                            iter = descriptor_set_pool_used.erase(iter);
                            continue;
                        }
                        else
                        {
                            // Still in use, do nothing
                        }
                    }
                    else
                    {
                        // Reset automatically, no dependency
                        descriptor_set->ResetInUse();
                        descriptor_set_pool_unused.insert(descriptor_set);
                        iter = descriptor_set_pool_used.erase(iter);
                        continue;
                    }
                    iter++;
                }
            }
        }

        {
            auto& semaphore_pool_used = semaphore_pool.GetUsed();
            auto& semaphore_pool_unused = semaphore_pool.GetUnused();

            if (0 && !semaphore_pool_unused.empty())
            {
                std::vector<vk::Semaphore> semaphore_pool_unused_vec(semaphore_pool_unused.size());
                std::transform(std::begin(semaphore_pool_unused), std::end(semaphore_pool_unused), std::begin(semaphore_pool_unused_vec), [](const auto& e) { return **e; });
                std::vector<uint64_t> semaphore_wait_values(semaphore_pool_unused.size(), Constants::DEFAULT_SEMAPHORE_TIMEOUT_VALUE);
                auto semaphore_wait_info = vk::SemaphoreWaitInfo{}
                    .setSemaphores(semaphore_pool_unused_vec)
                    .setValues(semaphore_wait_values);
                device.waitSemaphores(semaphore_wait_info, Constants::DEFAULT_SEMAPHORE_TIMEOUT_VALUE);
            }

            for (auto iter = std::begin(semaphore_pool_used); iter != std::end(semaphore_pool_used);)
            {
                auto& semaphore = *iter;
                AssignOrReturnVkError(auto counter_value, device.getSemaphoreCounterValue(*semaphore));
                if (counter_value > 0)
                {
                    semaphore->value = counter_value;
                    semaphore_pool_unused.insert(semaphore);
                    iter = semaphore_pool_used.erase(iter);
                }
                else
                {
                    iter++;
                }
            }
        }

        {
            auto& semaphore_pool_used = binary_semaphore_pool.GetUsed();
            auto& semaphore_pool_unused = binary_semaphore_pool.GetUnused();

            if (0 && !semaphore_pool_unused.empty())
            {
                std::vector<vk::Semaphore> semaphore_pool_unused_vec(semaphore_pool_unused.size());
                std::transform(std::begin(semaphore_pool_unused), std::end(semaphore_pool_unused), std::begin(semaphore_pool_unused_vec), [](const auto& e) { return **e; });
                std::vector<uint64_t> semaphore_wait_values(semaphore_pool_unused.size(), Constants::DEFAULT_SEMAPHORE_TIMEOUT_VALUE);
                auto semaphore_wait_info = vk::SemaphoreWaitInfo{}
                    .setSemaphores(semaphore_pool_unused_vec)
                    .setValues(semaphore_wait_values);
                device.waitSemaphores(semaphore_wait_info, Constants::DEFAULT_SEMAPHORE_TIMEOUT_VALUE);
            }

            binary_semaphore_pool.Clear();
        }

        {
            for (auto& [pipeline_layout_create_info, pipeline_layout_info_pools] : pipeline_layout_info_cache)
            {
                auto& pipeline_layout_info_pool = pipeline_layout_info_pools.t;
                auto& pipeline_layout_info_pool_used = pipeline_layout_info_pool.GetUsed();
                auto& pipeline_layout_info_pool_unused = pipeline_layout_info_pool.GetUnused();

                for (auto iter = std::begin(pipeline_layout_info_pool_used); iter != std::end(pipeline_layout_info_pool_used);)
                {
                    auto& pipeline_layout_info = *iter;

                    pipeline_layout_info_pool_unused.insert(pipeline_layout_info);
                    iter = pipeline_layout_info_pool_used.erase(iter);
                    continue;
                }
            }
        }

        return NoError();
    }

    Error RenderFrame::AddEndOfFrameUpdate(std::function<void()> callback)
    {
        end_of_frame_updates.emplace_back(callback);
        return NoError();
    }

    Error RenderFrame::PerformEndOfFrameUpdates()
    {
        for (const auto& callback : end_of_frame_updates)
        {
            callback();
        }
        end_of_frame_updates.clear();

        return NoError();
    }

    Error RenderFrame::EndFrame()
    {
        ReturnIfError(ReleaseUnusedResources());
        ReturnIfError(PerformEndOfFrameUpdates());
        return NoError();
    }

    Error RenderFrame::ReleaseUnusedResources()
    {
        // Deallocate any resource unused over x frames
        FrameCounter render_frame_release_entries_past_frame_counter_difference = Config["cache"]["render_frame_release_entries_past_frame_counter_difference"];
        descriptor_set_cache.ClearEntriesPastFrameCounterDifference(render_frame_release_entries_past_frame_counter_difference);
        //command_pool_cache.ClearEntriesPastFrameCounterDifference(render_frame_release_entries_past_frame_counter_difference);
        command_buffer_cache.ClearEntriesPastFrameCounterDifference(render_frame_release_entries_past_frame_counter_difference);

        return NoError();
    }

    Error RenderFrame::FlushCommandBuffers()
    {
        command_buffer_cache.Clear();
        return NoError();
    }

    Error RenderFrame::FlushPendingWrites(CMShared<DescriptorSet> descriptor_sets)
    {
        ReturnIfError(descriptor_sets->FlushPendingWrites(c->GetDevice()));
        return NoError();
    }

    Expected<CMShared<DescriptorSet>> RenderFrame::Copy(CMShared<DescriptorSet> descriptor_sets)
    {
        AssignOrReturnError(auto copied_descriptor_sets, Get(descriptor_sets->descriptor_set_create_info));
        // Copy contents from write descriptor set

        auto& original_write_descriptor_sets = descriptor_sets->GetWriteDescriptorSets();
        auto& copied_write_descriptor_sets = copied_descriptor_sets->GetWriteDescriptorSets();
        for (auto& [set, bindings_to_write_descriptor_sets] : original_write_descriptor_sets)
        {
            for (auto& [binding, original_write_descriptor_set] : bindings_to_write_descriptor_sets)
            {
                //auto& original_write_descriptor_set = original_write_descriptor_sets[set][binding];
                //write_descriptor_set.binded_resource = original_write_descriptor_set.binded_resource;
                //write_descriptor_set.descriptor_info = original_write_descriptor_set.descriptor_info;
                
                // Flush to descriptor set only if the resource is valid
                bool flush_resource = true;
                auto& original_binded_resource = original_write_descriptor_set.binded_resource;

                auto& copied_write_descriptor_set = copied_write_descriptor_sets[set][binding];
                auto& copied_binded_resource = copied_write_descriptor_set.binded_resource;
                //if (copied_binded_resource != original_binded_resource)
                if(1)
                {
                    if (auto buffer_ = std::get_if<CMShared<Buffer>>(&original_binded_resource))
                    {
                        auto& buffer = *buffer_;
                        if (buffer)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, buffer));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else if (auto texture = std::get_if<CMShared<Texture>>(&original_binded_resource))
                    {
                        if (*texture)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, *texture));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else if (auto sampler = std::get_if<CMShared<Sampler>>(&original_binded_resource))
                    {
                        if (*sampler)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, *sampler));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else if (auto tlas = std::get_if<CMShared<TopLevelAccelerationStructure>>(&original_binded_resource))
                    {
                        if (*tlas)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, *tlas));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else if (auto buffers = std::get_if<std::vector<CMShared<Buffer>>>(&original_binded_resource))
                    {
                        if (buffers)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, *buffers));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else if (auto textures = std::get_if<std::vector<CMShared<Texture>>>(&original_binded_resource))
                    {
                        if (textures)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, *textures));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else if (auto samplers = std::get_if<std::vector<CMShared<Sampler>>>(&original_binded_resource))
                    {
                        if (samplers)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, *samplers));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else if (auto tlases = std::get_if<std::vector<CMShared<TopLevelAccelerationStructure>>>(&original_binded_resource))
                    {
                        if (tlases)
                        {
                            ReturnIfError(copied_descriptor_sets->Bind(set, binding, *tlases));
                        }
                        else
                        {
                            flush_resource = false;
                        }
                    }
                    else
                    {
                        return StringError("Couldn't sync descriptor set with descriptor info");
                    }

                    // Mark this to be flushed to descriptor set
                    copied_write_descriptor_set.pending_write = flush_resource;

                    PanicIfError(FlushPendingWrites(copied_descriptor_sets));
                }
            }
        }

        return copied_descriptor_sets;
    }

    Error RenderFrame::BindPushConstant(CMShared<CommandBuffer> command_buffer, CMShared<PipelineLayout> pipeline_layout, PointerView data)
    {
        return BindPushConstant(*command_buffer, pipeline_layout, data);
    }

    Error RenderFrame::BindPushConstant(CommandBuffer& command_buffer, CMShared<PipelineLayout> pipeline_layout, PointerView data)
    {
        auto size = data.GetTotalSize();
        if (size % sizeof(float) != 0)
        {
            return StringError("Push constants not splittable {} % {} != 0", size, sizeof(float));
        }
        //size = Util::AlignUp(size, sizeof(glm::mat4));

        auto max_push_constants_size = c->GetPhysicalDeviceProperties().limits.maxPushConstantsSize;
        if (size > max_push_constants_size)
        {
            return StringError("Push constant buffer is greater than expected {} > {}", size, max_push_constants_size);
        }

        auto push_constant_stage_flags = vk::ShaderStageFlags{};
        for (const auto& push_constant_range : pipeline_layout->pipeline_layout_info->combined_shader_reflection.push_constant_ranges)
        {
            push_constant_stage_flags |= static_cast<vk::ShaderStageFlagBits>(push_constant_range.stageFlags);
        }

        command_buffer.command_buffer.pushConstants(pipeline_layout->pipeline_layout, push_constant_stage_flags, 0, size, data.GetData());

        return NoError();
    }
}