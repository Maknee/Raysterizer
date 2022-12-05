#include "include/resource_manager.h"

namespace RaysterizerEngine
{
    ResourceManager::ResourceManager()
    {

    }

    ResourceManager::ResourceManager(Context* c_) : c(std::move(c_))
    {

    }

    ResourceManager::~ResourceManager()
    {
        PanicIfError(Flush());
    }

    Error ResourceManager::EnqueueDestroy(Buffer buffer)
    {
#define MAYBE_EMPLACE_BUFFER 1
        if (MAYBE_EMPLACE_BUFFER)
        {
            buffers.emplace_back(CacheFrameCounterEntryWithCompleted<Buffer>{ buffer });
        }
        else
        {
            if (*buffer)
            {
                if (buffer.buffer_create_info.vma_allocation_create_info.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
                {
                    vmaUnmapMemory(c->GetVmaAllocator(), buffer.vma_allocation);
                }
                vmaDestroyBuffer(c->GetVmaAllocator(), *buffer, buffer.vma_allocation);
                buffer.buffer = vk::Buffer{};
            }
        }
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(Image image)
    {
        //vmaDestroyImage(c->GetVmaAllocator(), *image, image.vma_allocation);
        images.emplace_back(CacheFrameCounterEntryWithCompleted<Image>{ image });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(ImageView image_view)
    {
        image_views.emplace_back(CacheFrameCounterEntryWithCompleted<ImageView>{ image_view });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(Texture texture)
    {
        textures.emplace_back(CacheFrameCounterEntryWithCompleted<Texture>{ texture });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(Sampler sampler)
    {
        samplers.emplace_back(CacheFrameCounterEntryWithCompleted<Sampler>{ sampler });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(DescriptorPool descriptor_pool)
    {
        descriptor_pools.emplace_back(CacheFrameCounterEntryWithCompleted<DescriptorPool>{ descriptor_pool });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(DescriptorSetLayout descriptor_set_layout)
    {
        descriptor_set_layouts.emplace_back(CacheFrameCounterEntryWithCompleted<DescriptorSetLayout>{ descriptor_set_layout });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(DescriptorSet descriptor_set)
    {
        descriptor_sets.emplace_back(CacheFrameCounterEntryWithCompleted<DescriptorSet>{ descriptor_set });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(ShaderModule shader_module)
    {
        shader_modules.emplace_back(CacheFrameCounterEntryWithCompleted<ShaderModule>{ shader_module });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(RenderPass render_pass)
    {
        render_passes.emplace_back(CacheFrameCounterEntryWithCompleted<RenderPass>{ render_pass });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(PipelineLayoutInfo pipeline_layout_info)
    {
        pipeline_layout_infos.emplace_back(CacheFrameCounterEntryWithCompleted<PipelineLayoutInfo>{ pipeline_layout_info });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(PipelineLayout pipeline_layout)
    {
        pipeline_layouts.emplace_back(CacheFrameCounterEntryWithCompleted<PipelineLayout>{ pipeline_layout });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(GraphicsPipeline graphics_pipeline)
    {
        graphics_pipelines.emplace_back(CacheFrameCounterEntryWithCompleted<GraphicsPipeline>{ graphics_pipeline });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(ComputePipeline compute_pipeline)
    {
        compute_pipelines.emplace_back(CacheFrameCounterEntryWithCompleted<ComputePipeline>{ compute_pipeline });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(RaytracingPipeline raytracing_pipeline)
    {
        raytracing_pipelines.emplace_back(CacheFrameCounterEntryWithCompleted<RaytracingPipeline>{ raytracing_pipeline });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(Semaphore semaphore)
    {
        semaphores.emplace_back(CacheFrameCounterEntryWithCompleted<Semaphore>{ semaphore });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(Fence fence)
    {
        fences.emplace_back(CacheFrameCounterEntryWithCompleted<Fence>{ fence });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(CommandPool command_pool)
    {
        command_pools.emplace_back(CacheFrameCounterEntryWithCompleted<CommandPool>{ command_pool });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(CommandBuffer command_buffer)
    {
        command_buffers.emplace_back(CacheFrameCounterEntryWithCompleted<CommandBuffer>{ command_buffer });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(BottomLevelAccelerationStructure bottom_level_acceleration_structure)
    {
        bottom_level_acceleration_structures.emplace_back(CacheFrameCounterEntryWithCompleted<BottomLevelAccelerationStructure>{ bottom_level_acceleration_structure });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(TopLevelAccelerationStructure top_level_acceleration_structure)
    {
        top_level_acceleration_structures.emplace_back(CacheFrameCounterEntryWithCompleted<TopLevelAccelerationStructure>{ top_level_acceleration_structure });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(QueryPool query_pool)
    {
        query_pools.emplace_back(CacheFrameCounterEntryWithCompleted<QueryPool>{ query_pool });
        return NoError();
    }

    Error ResourceManager::EnqueueDestroy(FrameBuffer frame_buffer)
    {
        frame_buffers.emplace_back(CacheFrameCounterEntryWithCompleted<FrameBuffer>{ frame_buffer });
        return NoError();
    }

    template<typename T>
    void UpdateCounterAndClearElementsInVector(std::vector<CacheFrameCounterEntryWithCompleted<T>>& v)
    {
        const static int resource_manager_deallocate_frame_counter = Config["raysterizer"]["resource_manager"]["deallocate_frame_counter"];
        v.erase(std::remove_if(std::begin(v), std::end(v), [&](const auto& e) {
            if (e.frame_counter > resource_manager_deallocate_frame_counter && e.completed)
            {
                return true;
            }
            return false;
            }), std::end(v));
        for (auto& e : v)
        {
            e.frame_counter++;
        }
    }

    Error ResourceManager::Flush()
    {
        if (!c)
        {
            return NoError();
        }

        const static int resource_manager_deallocate_frame_counter = Config["raysterizer"]["resource_manager"]["deallocate_frame_counter"];
        CallOnce
        {
            if (resource_manager_deallocate_frame_counter < 1)
            {
                //PANIC("Resource manager deallocate frame counter must be at least 1 [{} < 1]", resource_manager_deallocate_frame_counter);
            }
        };

        auto device = c->GetDevice();
        auto DestroyBuffer = [&](Buffer& buffer)
        {
            if (*buffer)
            {
                if (buffer.buffer_create_info.vma_allocation_create_info.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)
                {
                    vmaUnmapMemory(c->GetVmaAllocator(), buffer.vma_allocation);
                }
                vmaDestroyBuffer(c->GetVmaAllocator(), *buffer, buffer.vma_allocation);
                buffer.buffer = vk::Buffer{};
            }
        };

        for (auto& buffer_ : buffers)
        {
            auto& [buffer, frame_counter, completed] = buffer_;
            if (*buffer)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    DestroyBuffer(buffer);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(buffers);

        for (auto& image_ : images)
        {
            auto& [image, frame_counter, completed] = image_;
            if (*image)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    vmaDestroyImage(c->GetVmaAllocator(), *image, image.vma_allocation);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(images);

        for (auto& image_view_ : image_views)
        {
            auto& [image_view, frame_counter, completed] = image_view_;
            if (*image_view)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyImageView(*image_view);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(image_views);

        for (auto& texture_ : textures)
        {
            auto& [texture, frame_counter, completed] = texture_;
            if (texture.image)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    // Let the resource manager handle everything instead
                    /*
                    vmaDestroyImage(c->GetVmaAllocator(), texture.image->image, texture.image->vma_allocation);
                    device.destroyImageView(*texture.image_view);
                    device.destroySampler(*texture.sampler);

                    texture.image->image = vk::Image{};
                    texture.image_view->image_view = vk::ImageView{};
                    texture.sampler->sampler = vk::Sampler{};
                    */
                    
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(textures);

        for (auto& sampler_ : samplers)
        {
            auto& [sampler, frame_counter, completed] = sampler_;
            if (*sampler)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroySampler(*sampler);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(samplers);

        for (auto& descriptor_pool_ : descriptor_pools)
        {
            auto& [descriptor_pool, frame_counter, completed] = descriptor_pool_;
            if (*descriptor_pool)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyDescriptorPool(*descriptor_pool);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(descriptor_pools);

        for (auto& descriptor_set_layout_ : descriptor_set_layouts)
        {
            auto& [descriptor_set_layout, frame_counter, completed] = descriptor_set_layout_;
            if (*descriptor_set_layout)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyDescriptorSetLayout(*descriptor_set_layout);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(descriptor_set_layouts);

        for (auto& descriptor_set : descriptor_sets)
        {
            // Handled by descriptor pool
            /*
            for (const auto& vk_descriptor_set : descriptor_set.descriptor_sets)
            {
                device.destroyDescriptorSet(vk_descriptor_set);
            }
            */
        }
        UpdateCounterAndClearElementsInVector(descriptor_sets);
        descriptor_sets.clear();

        for (auto& shader_module_ : shader_modules)
        {
            auto& [shader_module, frame_counter, completed] = shader_module_;
            if (*shader_module)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyShaderModule(*shader_module);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(shader_modules);

        for (auto& render_pass_ : render_passes)
        {
            auto& [render_pass, frame_counter, completed] = render_pass_;
            if (*render_pass)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyRenderPass(*render_pass);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(render_passes);

        for (auto& pipeline_layout_info_ : pipeline_layout_infos)
        {
            auto& [pipeline_layout_info, frame_counter, completed] = pipeline_layout_info_;
            if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
            {
                completed = true;
            }
            /*
            if (*pipeline_layout)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyPipelineLayout(*pipeline_layout);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
            */
        }
        UpdateCounterAndClearElementsInVector(pipeline_layouts);


        for (auto& pipeline_layout_ : pipeline_layouts)
        {
            auto& [pipeline_layout, frame_counter, completed] = pipeline_layout_;
            if (*pipeline_layout)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyPipelineLayout(*pipeline_layout);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(pipeline_layouts);

        for (auto& graphics_pipeline_ : graphics_pipelines)
        {
            auto& [graphics_pipeline, frame_counter, completed] = graphics_pipeline_;
            if (*graphics_pipeline)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyPipeline(*graphics_pipeline);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(graphics_pipelines);

        for (auto& compute_pipeline_ : compute_pipelines)
        {
            auto& [compute_pipeline, frame_counter, completed] = compute_pipeline_;
            if (*compute_pipeline)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyPipeline(*compute_pipeline);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(compute_pipelines);

        for (auto& raytracing_pipeline_ : raytracing_pipelines)
        {
            auto& [raytracing_pipeline, frame_counter, completed] = raytracing_pipeline_;
            if (*raytracing_pipeline)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyPipeline(*raytracing_pipeline);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(raytracing_pipelines);

        for (auto& semaphore_ : semaphores)
        {
            auto& [semaphore, frame_counter, completed] = semaphore_;
            if (*semaphore)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroySemaphore(*semaphore);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(semaphores);

        for (auto& fence_ : fences)
        {
            auto& [fence, frame_counter, completed] = fence_;
            if (*fence)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyFence(*fence);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(fences);

        for (auto& command_pool_ : command_pools)
        {
            auto& [command_pool, frame_counter, completed] = command_pool_;
            if (*command_pool)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyCommandPool(*command_pool);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(command_pools);

        for (auto& command_buffer : command_buffers)
        {
            // Handled by command pool
            //device.destroyCommandBuffer(*command_buffer);
        }
        UpdateCounterAndClearElementsInVector(command_buffers);
        command_buffers.clear();

        for (auto& bottom_level_acceleration_structure_ : bottom_level_acceleration_structures)
        {
            auto& [bottom_level_acceleration_structure, frame_counter, completed] = bottom_level_acceleration_structure_;
            if (*bottom_level_acceleration_structure)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    bottom_level_acceleration_structure.create_info = {};
                    if (bottom_level_acceleration_structure.compact_acceleration_structure_with_buffer.acceleration_structure && 
                        bottom_level_acceleration_structure.compact_acceleration_structure_with_buffer.acceleration_structure != bottom_level_acceleration_structure.acceleration_structure_with_buffer.acceleration_structure)
                    {
                        device.destroyAccelerationStructureKHR(bottom_level_acceleration_structure.compact_acceleration_structure_with_buffer.acceleration_structure);
                        bottom_level_acceleration_structure.compact_acceleration_structure_with_buffer.acceleration_structure = vk::AccelerationStructureKHR{};
                    }
                    if (bottom_level_acceleration_structure.acceleration_structure_with_buffer.acceleration_structure)
                    {
                        device.destroyAccelerationStructureKHR(bottom_level_acceleration_structure.acceleration_structure_with_buffer.acceleration_structure);
                        bottom_level_acceleration_structure.acceleration_structure_with_buffer.acceleration_structure = vk::AccelerationStructureKHR{};
                    }

                    // Let the resource manager handle everything instead
                    bottom_level_acceleration_structure.compact_acceleration_structure_with_buffer.buffer = nullptr;
                    bottom_level_acceleration_structure.acceleration_structure_with_buffer.buffer = nullptr;
                    /*
                    if (bottom_level_acceleration_structure.acceleration_structure_with_buffer.buffer->buffer)
                    {
                        DestroyBuffer(*bottom_level_acceleration_structure.acceleration_structure_with_buffer.buffer);
                    }
                    if (bottom_level_acceleration_structure.compact_acceleration_structure_with_buffer.buffer)
                    {
                        DestroyBuffer(*bottom_level_acceleration_structure.compact_acceleration_structure_with_buffer.buffer);
                    }
                    */
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
            /*
            if (bottom_level_acceleration_structure.scratch_buffer)
            {
                vmaDestroyBuffer(c->GetVmaAllocator(), bottom_level_acceleration_structure.scratch_buffer->buffer, bottom_level_acceleration_structure.scratch_buffer->vma_allocation);
                bottom_level_acceleration_structure.scratch_buffer->buffer = vk::Buffer{};
            }
            */
            // Might be shared, can use by other parts
            /*
            if (blas->query_pool)
            {
                device.destroyQueryPool(*blas->query_pool);
            }
            */
        }
        UpdateCounterAndClearElementsInVector(bottom_level_acceleration_structures);

        for (auto& top_level_acceleration_structure_ : top_level_acceleration_structures)
        {
            auto& [top_level_acceleration_structure, frame_counter, completed] = top_level_acceleration_structure_;
            if (*top_level_acceleration_structure)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    top_level_acceleration_structure.tlas_create_info = {};
                    if (*top_level_acceleration_structure)
                    {
                        device.destroyAccelerationStructureKHR(*top_level_acceleration_structure);
                    }
                    // Let the resource manager handle everything instead
                    top_level_acceleration_structure.acceleration_structure_with_buffer.buffer = nullptr;
                    top_level_acceleration_structure.instance_buffer = nullptr;
                    /*
                    if (top_level_acceleration_structure.acceleration_structure_with_buffer.buffer->buffer)
                    {
                        DestroyBuffer(*top_level_acceleration_structure.acceleration_structure_with_buffer.buffer);
                    }
                    if (top_level_acceleration_structure.instance_buffer->buffer)
                    {
                        DestroyBuffer(*top_level_acceleration_structure.instance_buffer);
                    }
                    */
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(top_level_acceleration_structures);

        for (auto& query_pool_ : query_pools)
        {
            auto& [query_pool, frame_counter, completed] = query_pool_;
            if (*query_pool)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyQueryPool(*query_pool);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(query_pools);

        for (auto& frame_buffer_ : frame_buffers)
        {
            auto& [frame_buffer, frame_counter, completed] = frame_buffer_;
            if (*frame_buffer)
            {
                if (frame_counter > resource_manager_deallocate_frame_counter && !completed)
                {
                    device.destroyFramebuffer(*frame_buffer);
                    completed = true;
                }
            }
            else
            {
                completed = true;
            }
        }
        UpdateCounterAndClearElementsInVector(frame_buffers);

        return NoError();
    }

    Error ResourceManager::FlushEntirely()
    {
        while (
            !buffers.empty() ||
            !images.empty() ||
            !image_views.empty() ||
            !textures.empty() ||
            !samplers.empty() ||
            !descriptor_pools.empty() ||
            !descriptor_set_layouts.empty() ||
            !descriptor_sets.empty() ||
            !shader_modules.empty() ||
            !render_passes.empty() ||
            !pipeline_layout_infos.empty() ||
            !pipeline_layouts.empty() ||
            !graphics_pipelines.empty() ||
            !compute_pipelines.empty() ||
            !raytracing_pipelines.empty() ||
            !semaphores.empty() ||
            !fences.empty() ||
            !command_pools.empty() ||
            !command_buffers.empty() ||
            !bottom_level_acceleration_structures.empty() ||
            !top_level_acceleration_structures.empty() ||
            !query_pools.empty() ||
            !frame_buffers.empty()
            )
        {
            ReturnIfError(Flush());
        }
        return NoError();
    }
}