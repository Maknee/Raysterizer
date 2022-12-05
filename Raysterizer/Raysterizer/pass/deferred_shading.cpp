#include "deferred_shading.h"

namespace Raysterizer
{
    namespace Pass
    {
        // -----------------------------------------------------------------------------------------------------------------------------------

        struct ShadingPushConstants
        {
            int shadows = 1;
            int ao = 1;
            int reflections = 1;
            int gi = 1;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct SkyboxPushConstants
        {
            glm::mat4 view_projection;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct VisualizeProbeGridPushConstants
        {
            float scale;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        Error DeferredShading::Setup(std::shared_ptr<CommonResources> common_resources_)
        {
            common_resources = common_resources_;

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            LoadSphereMesh();
            CreateCube();
            CreateImages();
            CreateRenderPass();
            CreateFrameBuffer();

            return NoError();
        }

        void DeferredShading::Render(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Deferred Shading");

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            RenderShading(command_buffer);
            RenderSkybox(command_buffer);
        }

        void DeferredShading::UpdateGui()
        {
            ImGui::Checkbox("Visualize Probe Grid", &visualize_probe_grid.enabled);
        }
        
        void DeferredShading::LoadSphereMesh()
        {
            RenderFrame& render_frame = c.GetRenderFrame();

            float cube_vertices[] = {
                // back face
                -1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                0.0f,
                -1.0f,
                0.0f,
                0.0f, // bottom-left
                1.0f,
                1.0f,
                -1.0f,
                0.0f,
                0.0f,
                -1.0f,
                1.0f,
                1.0f, // top-right
                1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                0.0f,
                -1.0f,
                1.0f,
                0.0f, // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                0.0f,
                0.0f,
                -1.0f,
                1.0f,
                1.0f, // top-right
                -1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                0.0f,
                -1.0f,
                0.0f,
                0.0f, // bottom-left
                -1.0f,
                1.0f,
                -1.0f,
                0.0f,
                0.0f,
                -1.0f,
                0.0f,
                1.0f, // top-left
                // front face
                -1.0f,
                -1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f,
                0.0f, // bottom-left
                1.0f,
                -1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                0.0f, // bottom-right
                1.0f,
                1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                1.0f, // top-right
                1.0f,
                1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                1.0f, // top-right
                -1.0f,
                1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f,
                1.0f, // top-left
                -1.0f,
                -1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f,
                0.0f, // bottom-left
                // left face
                -1.0f,
                1.0f,
                1.0f,
                -1.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f, // top-right
                -1.0f,
                1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f, // top-left
                -1.0f,
                -1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                0.0f,
                0.0f,
                1.0f, // bottom-left
                -1.0f,
                -1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                0.0f,
                0.0f,
                1.0f, // bottom-left
                -1.0f,
                -1.0f,
                1.0f,
                -1.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f, // bottom-right
                -1.0f,
                1.0f,
                1.0f,
                -1.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f, // top-right
                // right face
                1.0f,
                1.0f,
                1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f, // top-left
                1.0f,
                -1.0f,
                -1.0f,
                1.0f,
                0.0f,
                0.0f,
                0.0f,
                1.0f, // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f, // top-right
                1.0f,
                -1.0f,
                -1.0f,
                1.0f,
                0.0f,
                0.0f,
                0.0f,
                1.0f, // bottom-right
                1.0f,
                1.0f,
                1.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                0.0f, // top-left
                1.0f,
                -1.0f,
                1.0f,
                1.0f,
                0.0f,
                0.0f,
                0.0f,
                0.0f, // bottom-left
                // bottom face
                -1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                -1.0f,
                0.0f,
                0.0f,
                1.0f, // top-right
                1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                -1.0f,
                0.0f,
                1.0f,
                1.0f, // top-left
                1.0f,
                -1.0f,
                1.0f,
                0.0f,
                -1.0f,
                0.0f,
                1.0f,
                0.0f, // bottom-left
                1.0f,
                -1.0f,
                1.0f,
                0.0f,
                -1.0f,
                0.0f,
                1.0f,
                0.0f, // bottom-left
                -1.0f,
                -1.0f,
                1.0f,
                0.0f,
                -1.0f,
                0.0f,
                0.0f,
                0.0f, // bottom-right
                -1.0f,
                -1.0f,
                -1.0f,
                0.0f,
                -1.0f,
                0.0f,
                0.0f,
                1.0f, // top-right
                // top face
                -1.0f,
                1.0f,
                -1.0f,
                0.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f, // top-left
                1.0f,
                1.0f,
                1.0f,
                0.0f,
                1.0f,
                0.0f,
                1.0f,
                0.0f, // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                0.0f,
                1.0f,
                0.0f,
                1.0f,
                1.0f, // top-right
                1.0f,
                1.0f,
                1.0f,
                0.0f,
                1.0f,
                0.0f,
                1.0f,
                0.0f, // bottom-right
                -1.0f,
                1.0f,
                -1.0f,
                0.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f, // top-left
                -1.0f,
                1.0f,
                1.0f,
                0.0f,
                1.0f,
                0.0f,
                0.0f,
                0.0f // bottom-left
            };

            auto transfer_job = AssignOrPanic(render_frame.UploadDataToGPUBuffer(PointerView(cube_vertices), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst));
            visualize_probe_grid.sphere_vbo = transfer_job.gpu_buffer;
        }

        void DeferredShading::CreateCube()
        {
            RenderFrame& render_frame = c.GetRenderFrame();

            float cube_vertices[] = {
                // back face
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                1.0f,
                -1.0f,
                -1.0f,
                // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                // front face
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-right
                1.0f,
                1.0f,
                1.0f,
                // top-right
                1.0f,
                1.0f,
                1.0f,
                // top-right
                -1.0f,
                1.0f,
                1.0f,
                // top-left
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                // left face
                -1.0f,
                1.0f,
                1.0f,
                // top-right
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                -1.0f,
                -1.0f,
                -1.0f,
                // bottom-left
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-right
                -1.0f,
                1.0f,
                1.0f,
                // top-right
                // right face
                1.0f,
                1.0f,
                1.0f,
                // top-left
                1.0f,
                -1.0f,
                -1.0f,
                // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                1.0f,
                -1.0f,
                -1.0f,
                // bottom-right
                1.0f,
                1.0f,
                1.0f,
                // top-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                // bottom face
                -1.0f,
                -1.0f,
                -1.0f,
                // top-right
                1.0f,
                -1.0f,
                -1.0f,
                // top-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                1.0f,
                -1.0f,
                1.0f,
                // bottom-left
                -1.0f,
                -1.0f,
                1.0f,
                // bottom-right
                -1.0f,
                -1.0f,
                -1.0f,
                // top-right
                // top face
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                1.0f,
                1.0f,
                1.0f,
                // bottom-right
                1.0f,
                1.0f,
                -1.0f,
                // top-right
                1.0f,
                1.0f,
                1.0f,
                // bottom-right
                -1.0f,
                1.0f,
                -1.0f,
                // top-left
                -1.0f,
                1.0f,
                1.0f // bottom-left
            };

            auto cube_vbo_transfer_job = AssignOrPanic(render_frame.UploadDataToGPUBuffer(PointerView(cube_vertices), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst));
            skybox.cube_vbo = cube_vbo_transfer_job.gpu_buffer;
        }
        
        void DeferredShading::CreateImages()
        {
            auto nearest_sampler = common_resources->nearest_sampler;

            width = common_resources->width;
            height = common_resources->height;
            
            shading.image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::SampleCountFlagBits::e1);
            c.SetName(shading.image, fmt::format("Deferred Shading Image"));

            shading.view = CommonResources::CreateImageView(shading.image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
            c.SetName(shading.view, fmt::format("Deferred Shading Image View"));

            shading.texture = std::make_shared<Texture>(shading.image, shading.view, nearest_sampler);
        }

        void DeferredShading::CreateRenderPass()
        {
			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

            {
                // Deferred attachment
                std::vector<vk::AttachmentDescription> attachments(1);

                attachments[0].format = vk::Format::eR16G16B16A16Sfloat;
                attachments[0].samples = vk::SampleCountFlagBits::e1;
                attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
                attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
                attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
                attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
                attachments[0].initialLayout = vk::ImageLayout::eUndefined;
                attachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

                vk::AttachmentReference deferred_reference;

                deferred_reference.attachment = 0;
                deferred_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;

                std::vector<vk::SubpassDescription> subpass_description(1);

                subpass_description[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
                subpass_description[0].colorAttachmentCount = 1;
                subpass_description[0].pColorAttachments = &deferred_reference;
                subpass_description[0].pDepthStencilAttachment = nullptr;
                subpass_description[0].inputAttachmentCount = 0;
                subpass_description[0].pInputAttachments = nullptr;
                subpass_description[0].preserveAttachmentCount = 0;
                subpass_description[0].pPreserveAttachments = nullptr;
                subpass_description[0].pResolveAttachments = nullptr;

                // Subpass dependencies for layout transitions
                std::vector<vk::SubpassDependency> dependencies(2);

                dependencies[0] = vk::SubpassDependency{}
                    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);

                dependencies[1] = vk::SubpassDependency{}
                    .setSrcSubpass(0)
                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);


                RenderPassCreateInfo render_pass_create_info{};
                render_pass_create_info.attachment_descriptions = attachments;
                //render_pass_create_info.color_attachment_references = gbuffer_references;
                //render_pass_create_info.depth_stencil_reference = depth_reference;
                render_pass_create_info.subpass_descriptions = subpass_description;
                render_pass_create_info.subpass_dependencies = dependencies;

                shading.rp = AssignOrPanic(c.Get(render_pass_create_info));
            }


            {
                // Deferred attachment
                std::vector<vk::AttachmentDescription> attachments(2);

                attachments[0].format = vk::Format::eR16G16B16A16Sfloat;
                attachments[0].samples = vk::SampleCountFlagBits::e1;
                attachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
                attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
                attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
                attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
                attachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
                attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

                attachments[1].format = Constants::DEPTH_FORMAT;
                attachments[1].samples = vk::SampleCountFlagBits::e1;
                attachments[1].loadOp = vk::AttachmentLoadOp::eLoad;
                attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
                attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
                attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
                attachments[1].initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                attachments[1].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

                vk::AttachmentReference deferred_reference;

                deferred_reference.attachment = 0;
                deferred_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;

                vk::AttachmentReference depth_reference;

                depth_reference.attachment = 1;
                depth_reference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

                std::vector<vk::SubpassDescription> subpass_description(1);

                subpass_description[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
                subpass_description[0].colorAttachmentCount = 1;
                subpass_description[0].pColorAttachments = &deferred_reference;
                subpass_description[0].pDepthStencilAttachment = &depth_reference;
                subpass_description[0].inputAttachmentCount = 0;
                subpass_description[0].pInputAttachments = nullptr;
                subpass_description[0].preserveAttachmentCount = 0;
                subpass_description[0].pPreserveAttachments = nullptr;
                subpass_description[0].pResolveAttachments = nullptr;

                // Subpass dependencies for layout transitions
                std::vector<vk::SubpassDependency> dependencies(2);

                dependencies[0] = vk::SubpassDependency{}
                    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstSubpass(0)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);

                dependencies[1] = vk::SubpassDependency{}
                    .setSrcSubpass(0)
                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
                    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                    .setDependencyFlags(vk::DependencyFlagBits::eByRegion);


                RenderPassCreateInfo render_pass_create_info{};
                render_pass_create_info.attachment_descriptions = attachments;
                //render_pass_create_info.color_attachment_references = gbuffer_references;
                //render_pass_create_info.depth_stencil_reference = depth_reference;
                render_pass_create_info.subpass_descriptions = subpass_description;
                render_pass_create_info.subpass_dependencies = dependencies;

                skybox.rp = AssignOrPanic(c.Get(render_pass_create_info));
            }
        }

        void DeferredShading::CreateFrameBuffer()
		{
			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

            std::vector<vk::ImageView> attachments{ *shading.view };

            auto frame_buffer_create_info = vk::FramebufferCreateInfo{}
                .setRenderPass(*shading.rp)
                .setAttachments(attachments)
                .setWidth(width)
                .setHeight(height)
                .setLayers(1);

            auto frame_buffer = AssignOrPanicVkError(device.createFramebufferUnique(frame_buffer_create_info));
            shading.fbo = std::move(frame_buffer);

            /*
            for (auto i = 0; i < common_resources->ping_pong_size; i++)
            {
                std::vector<vk::ImageView> attachments{ *shading.view, *common_resources->g_buffer_pass->depth_fbo_image_views[i] };

                auto frame_buffer_create_info = vk::FramebufferCreateInfo{}
                    .setRenderPass(*skybox.rp)
                    .setAttachments(attachments)
                    .setWidth(width)
                    .setHeight(height)
                    .setLayers(1);

                auto frame_buffer = device.createFramebufferUnique(frame_buffer_create_info);
                skybox.fbo[i] = std::move(frame_buffer);
			}
            */
		}

        void DeferredShading::RenderShading(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Deferred Render Shading");

            auto& cb = **command_buffer;

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];
            command_buffer->Begin();

            auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
            {
                ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("other/triangle.vert")},
                ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("other/deferred.frag")},
            };

            auto plci = PipelineLayoutCreateInfo
            {
                shader_module_create_infos,
            };

            CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
            CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
            CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

            std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions = {
                vk::VertexInputBindingDescription{}
                    .setBinding(0)
                    .setStride(sizeof(glm::vec3))
                    .setInputRate(vk::VertexInputRate::eVertex)
            };

            std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions = {
                vk::VertexInputAttributeDescription{}
                    .setLocation(0)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(0)
            };

            auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{}
                .setVertexBindingDescriptions(vertex_input_binding_descriptions)
                .setVertexAttributeDescriptions(vertex_input_attribute_descriptions);

            auto pcbas = vk::PipelineColorBlendAttachmentState{}
                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                .setBlendEnable(VK_FALSE);

            std::vector<vk::PipelineColorBlendAttachmentState> pcbases{ pcbas };

            auto gpci = GraphicsPipelineCreateInfo
            {
                shading.rp,
                pl,
                vertex_input_state_create_info,
                vk::PipelineTessellationStateCreateInfo{},
                vk::PipelineInputAssemblyStateCreateInfo{}
                    .setTopology(vk::PrimitiveTopology::eTriangleList)
                    .setPrimitiveRestartEnable(VK_FALSE),
                vk::Viewport{}
                    .setX(0.0f)
                    .setY(0.0f)
                    .setWidth(c.GetWindowExtent().width)
                    .setHeight(c.GetWindowExtent().height)
                    .setMinDepth(0.0f)
                    .setMaxDepth(1.0f),
                vk::Rect2D{}
                    .setOffset({ 0, 0 })
                    .setExtent(c.GetWindowExtent()),
                vk::PipelineRasterizationStateCreateInfo{}
                    .setDepthClampEnable(VK_FALSE)
                    .setRasterizerDiscardEnable(VK_FALSE)
                    .setPolygonMode(vk::PolygonMode::eFill)
                    .setLineWidth(1.0f)
                    .setCullMode(vk::CullModeFlagBits::eNone)
                    .setFrontFace(vk::FrontFace::eCounterClockwise)
                    .setDepthBiasEnable(VK_FALSE)
                    .setDepthBiasConstantFactor(0.0f)
                    .setDepthBiasClamp(0.0f)
                    .setDepthBiasSlopeFactor(0.0f),
                vk::PipelineColorBlendStateCreateInfo{}
                    .setLogicOpEnable(VK_FALSE)
                    .setLogicOp(vk::LogicOp::eCopy)
                    .setAttachments(pcbases),
                vk::PipelineMultisampleStateCreateInfo{}
                    .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                    .setSampleShadingEnable(VK_FALSE)
                    .setMinSampleShading(1.0f)
                    .setPSampleMask(nullptr)
                    .setAlphaToCoverageEnable(VK_FALSE)
                    .setAlphaToOneEnable(VK_FALSE),
                vk::PipelineDepthStencilStateCreateInfo{}
                    .setDepthTestEnable(VK_TRUE)
                    .setDepthWriteEnable(VK_TRUE)
                    .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                    .setBack(vk::StencilOpState{}.setCompareOp(vk::CompareOp::eNever))
                    .setDepthBoundsTestEnable(VK_FALSE)
                    .setMinDepthBounds(0.0f)
                    .setMaxDepthBounds(1.0f)
                    .setStencilTestEnable(VK_FALSE),
                vk::PipelineDynamicStateCreateInfo{},
                vk::PipelineCache{}
            };

            CMShared<GraphicsPipeline> cp = AssignOrPanic(c.Get(gpci));

            shading.pipeline_layout = pl;
            shading.pipeline = cp;
            c.SetName(shading.pipeline_layout, "Deferred Shading Pipeline Layout");
            c.SetName(shading.pipeline, "Deferred Shading Compute Pipeline");

            vk::ClearValue clear_value;

            clear_value.color.float32[0] = 0.0f;
            clear_value.color.float32[1] = 0.0f;
            clear_value.color.float32[2] = 0.0f;
            clear_value.color.float32[3] = 1.0f;

            auto render_pass_begin_info = vk::RenderPassBeginInfo{}
                .setRenderPass(*shading.rp)
                .setRenderArea(vk::Rect2D{}
                    .setOffset({ 0, 0 })
                    .setExtent({ width, height })
                )
                .setFramebuffer(*shading.fbo)
                .setClearValues(clear_value);

            cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

            cb.setViewport(0, vk::Viewport{ 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f });
            cb.setScissor(0, vk::Rect2D({ 0, 0 }, { width, height }));

            cb.bindPipeline(vk::PipelineBindPoint::eGraphics, cp->pipeline);
            cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pl->pipeline_layout, 0, ds->descriptor_sets, {});

            {
                PanicIfError(ds->Bind(0, 0, common_resources->g_buffer_pass->OutputBindings()));
                PanicIfError(ds->Bind(1, 0, common_resources->ray_traced_ao_pass->OutputBindings()));
                PanicIfError(ds->Bind(2, 0, common_resources->ray_traced_shadows_pass->OutputBindings()));
                PanicIfError(ds->Bind(3, 0, common_resources->ray_traced_reflections_pass->OutputBindings()));
                PanicIfError(ds->Bind(4, 0, common_resources->ddgi_pass->OutputBindings()));
                PanicIfError(ds->Bind(5, 0, common_resources->GetUBOBuffer()));
                PanicIfError(ds->Bind(6, 0, common_resources->sky_environment->OutputBindings()));

                PanicIfError(render_frame.FlushPendingWrites(ds));

                ShadingPushConstants push_constants;

                push_constants.shadows = (float)shading.use_ray_traced_shadows;
                push_constants.ao = (float)shading.use_ray_traced_ao;
                push_constants.reflections = (float)shading.use_ray_traced_reflections;
                push_constants.gi = (float)shading.use_ddgi;

                PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
            }

            cb.draw(3, 1, 0, 0);
            cb.endRenderPass();
        }

        void DeferredShading::RenderSkybox(CMShared<CommandBuffer> command_buffer)
        {
            auto& cb = **command_buffer;

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            if (!common_resources->g_buffer_pass)
            {
                return;
            }
            
            if (current_gbuffer_pass != common_resources->g_buffer_pass)
            {
                current_gbuffer_pass = common_resources->g_buffer_pass;

                skybox.fbo.clear();
                for (auto i = 0; i < num_frames; i++)
                {
                    std::vector<vk::ImageView> attachments{ *shading.view, *common_resources->g_buffer_pass->depth_fbo_image_views[i] };

                    auto frame_buffer_create_info = FrameBufferCreateInfo
                    {
                        skybox.rp,
                        { shading.view, common_resources->g_buffer_pass->depth_fbo_image_views[i] },
                        width,
                        height,
                        1
                    };

                    auto frame_buffer = AssignOrPanic(c.CreateFrameBuffer(frame_buffer_create_info));
                    skybox.fbo.emplace_back(frame_buffer);
                }
            }

            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
            {
                ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("other/skybox.vert")},
                ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("other/skybox.frag")},
            };

            auto plci = PipelineLayoutCreateInfo
            {
                shader_module_create_infos,
            };

            CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
            CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
            CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

            std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions = {
                vk::VertexInputBindingDescription{}
                    .setBinding(0)
                    .setStride(sizeof(glm::vec3))
                    .setInputRate(vk::VertexInputRate::eVertex)
            };

            std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions = {
                vk::VertexInputAttributeDescription{}
                    .setLocation(0)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(0)
            };

            auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{}
                .setVertexBindingDescriptions(vertex_input_binding_descriptions)
                .setVertexAttributeDescriptions(vertex_input_attribute_descriptions);

            auto pcbas = vk::PipelineColorBlendAttachmentState{}
                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                .setBlendEnable(VK_FALSE);

            std::vector<vk::PipelineColorBlendAttachmentState> pcbases{ pcbas };

            auto gpci = GraphicsPipelineCreateInfo
            {
                skybox.rp,
                pl,
                vertex_input_state_create_info,
                vk::PipelineTessellationStateCreateInfo{},
                vk::PipelineInputAssemblyStateCreateInfo{}
                    .setTopology(vk::PrimitiveTopology::eTriangleList)
                    .setPrimitiveRestartEnable(VK_FALSE),
                vk::Viewport{}
                    .setX(0.0f)
                    .setY(0.0f)
                    .setWidth(width)
                    .setHeight(height)
                    .setMinDepth(0.0f)
                    .setMaxDepth(1.0f),
                vk::Rect2D{}
                    .setOffset({ 0, 0 })
                    .setExtent({ width, height }),
                vk::PipelineRasterizationStateCreateInfo{}
                    .setDepthClampEnable(VK_FALSE)
                    .setRasterizerDiscardEnable(VK_FALSE)
                    .setPolygonMode(vk::PolygonMode::eFill)
                    .setLineWidth(1.0f)
                    .setCullMode(vk::CullModeFlagBits::eNone)
                    .setFrontFace(vk::FrontFace::eCounterClockwise)
                    .setDepthBiasEnable(VK_FALSE)
                    .setDepthBiasConstantFactor(0.0f)
                    .setDepthBiasClamp(0.0f)
                    .setDepthBiasSlopeFactor(0.0f),
                vk::PipelineColorBlendStateCreateInfo{}
                    .setLogicOpEnable(VK_FALSE)
                    .setLogicOp(vk::LogicOp::eCopy)
                    .setAttachments(pcbases),
                vk::PipelineMultisampleStateCreateInfo{}
                    .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                    .setSampleShadingEnable(VK_FALSE)
                    .setMinSampleShading(1.0f)
                    .setPSampleMask(nullptr)
                    .setAlphaToCoverageEnable(VK_FALSE)
                    .setAlphaToOneEnable(VK_FALSE),
                vk::PipelineDepthStencilStateCreateInfo{}
                    .setDepthTestEnable(VK_TRUE)
                    .setDepthWriteEnable(VK_FALSE)
                    .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                    .setBack(vk::StencilOpState{}.setCompareOp(vk::CompareOp::eAlways))
                    .setDepthBoundsTestEnable(VK_FALSE)
                    .setMinDepthBounds(0.0f)
                    .setMaxDepthBounds(0.0f)
                    .setStencilTestEnable(VK_FALSE),
                vk::PipelineDynamicStateCreateInfo{},
                vk::PipelineCache{}
            };

            CMShared<GraphicsPipeline> cp = AssignOrPanic(c.Get(gpci));

            skybox.pipeline_layout = pl;
            skybox.pipeline = cp;
            c.SetName(skybox.pipeline_layout, "Skybox Pipeline Layout");
            c.SetName(skybox.pipeline, "Skybox Compute Pipeline");

            vk::ClearValue clear_value;

            clear_value.color.float32[0] = 0.0f;
            clear_value.color.float32[1] = 0.0f;
            clear_value.color.float32[2] = 0.0f;
            clear_value.color.float32[3] = 1.0f;

            auto render_pass_begin_info = vk::RenderPassBeginInfo{}
                .setRenderPass(*skybox.rp)
                .setRenderArea(vk::Rect2D{}
                    .setOffset({ 0, 0 })
                    .setExtent({ width, height })
                )
                .setFramebuffer(*skybox.fbo[current_frame_index])
                .setClearValues(clear_value);

            cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

            command_buffer->AddDependencyTo(skybox.fbo[current_frame_index]);

            cb.setViewport(0, vk::Viewport{ 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f });
            cb.setScissor(0, vk::Rect2D({ 0, 0 }, { width, height }));

            RenderProbes(command_buffer);

            ScopedGPUProfileRaysterizer(command_buffer, "Skybox");

            cb.bindPipeline(vk::PipelineBindPoint::eGraphics, cp->pipeline);
            cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pl->pipeline_layout, 0, ds->descriptor_sets, {});

            {
                PanicIfError(ds->Bind(0, 0, common_resources->sky_environment->OutputBindings()));

                PanicIfError(render_frame.FlushPendingWrites(ds));

                SkyboxPushConstants push_constants;

                auto& context = Raysterizer::OpenGL::Context::Get();
                auto& state = context.state;
                auto& raysterizer_info = state.GetRaysterizerInfo();

                push_constants.view_projection = raysterizer_info.projection_view;

                PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
            }

            cb.bindVertexBuffers(0, { **skybox.cube_vbo }, { 0 });

            cb.draw(36, 1, 0, 0);
            cb.endRenderPass();
        }


        void DeferredShading::RenderProbes(CMShared<CommandBuffer> command_buffer)
        {
            if (visualize_probe_grid.enabled)
            {
                ScopedGPUProfileRaysterizer(command_buffer, "Probes");

                auto& cb = **command_buffer;

                vk::Device device = c.GetDevice();
                RenderFrame& render_frame = c.GetRenderFrame();

                auto current_frame = c.GetFrame();
                auto current_frame_index = c.GetFrameIndex();
                auto num_frames = c.GetNumFrames();

                CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_probe_visualization.vert")},
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_probe_visualization.frag")},
                };

                auto plci = PipelineLayoutCreateInfo
                {
                    shader_module_create_infos,
                };

                CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
                CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
                CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

                struct SkyboxVertex
                {
                    glm::vec3 position;
                    glm::vec3 normal;
                    glm::vec2 texcoord;
                };

                std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions = {
                    vk::VertexInputBindingDescription{}
                        .setBinding(0)
                        .setStride(sizeof(SkyboxVertex))
                        .setInputRate(vk::VertexInputRate::eVertex)
                };

                std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions = {
                    vk::VertexInputAttributeDescription{}
                        .setLocation(0)
                        .setBinding(0)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(0),
                    vk::VertexInputAttributeDescription{}
                        .setLocation(1)
                        .setBinding(0)
                        .setFormat(vk::Format::eR32G32B32Sfloat)
                        .setOffset(offsetof(SkyboxVertex, normal)),
                    vk::VertexInputAttributeDescription{}
                        .setLocation(2)
                        .setBinding(0)
                        .setFormat(vk::Format::eR32G32Sfloat)
                        .setOffset(offsetof(SkyboxVertex, texcoord)),
                };

                auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{}
                    .setVertexBindingDescriptions(vertex_input_binding_descriptions)
                    .setVertexAttributeDescriptions(vertex_input_attribute_descriptions);

                auto pcbas = vk::PipelineColorBlendAttachmentState{}
                    .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                    .setBlendEnable(VK_FALSE);

                std::vector<vk::PipelineColorBlendAttachmentState> pcbases{ pcbas };

                auto gpci = GraphicsPipelineCreateInfo
                {
                    skybox.rp,
                    pl,
                    vertex_input_state_create_info,
                    vk::PipelineTessellationStateCreateInfo{},
                    vk::PipelineInputAssemblyStateCreateInfo{}
                        .setTopology(vk::PrimitiveTopology::eTriangleList)
                        .setPrimitiveRestartEnable(VK_FALSE),
                    vk::Viewport{}
                        .setX(0.0f)
                        .setY(0.0f)
                        .setWidth(width)
                        .setHeight(height)
                        .setMinDepth(0.0f)
                        .setMaxDepth(1.0f),
                    vk::Rect2D{}
                        .setOffset({ 0, 0 })
                        .setExtent({ width, height }),
                    vk::PipelineRasterizationStateCreateInfo{}
                        .setDepthClampEnable(VK_FALSE)
                        .setRasterizerDiscardEnable(VK_FALSE)
                        .setPolygonMode(vk::PolygonMode::eFill)
                        .setLineWidth(1.0f)
                        .setCullMode(vk::CullModeFlagBits::eNone)
                        .setFrontFace(vk::FrontFace::eCounterClockwise)
                        .setDepthBiasEnable(VK_FALSE)
                        .setDepthBiasConstantFactor(0.0f)
                        .setDepthBiasClamp(0.0f)
                        .setDepthBiasSlopeFactor(0.0f),
                    vk::PipelineColorBlendStateCreateInfo{}
                        .setLogicOpEnable(VK_FALSE)
                        .setLogicOp(vk::LogicOp::eCopy)
                        .setAttachments(pcbases),
                    vk::PipelineMultisampleStateCreateInfo{}
                        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                        .setSampleShadingEnable(VK_FALSE)
                        .setMinSampleShading(1.0f)
                        .setPSampleMask(nullptr)
                        .setAlphaToCoverageEnable(VK_FALSE)
                        .setAlphaToOneEnable(VK_FALSE),
                    vk::PipelineDepthStencilStateCreateInfo{}
                        .setDepthTestEnable(VK_TRUE)
                        .setDepthWriteEnable(VK_TRUE)
                        .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                        .setBack(vk::StencilOpState{}.setCompareOp(vk::CompareOp::eAlways))
                        .setDepthBoundsTestEnable(VK_FALSE)
                        .setMinDepthBounds(0.0f)
                        .setMaxDepthBounds(1.0f)
                        .setStencilTestEnable(VK_FALSE),
                    vk::PipelineDynamicStateCreateInfo{},
                    vk::PipelineCache{}
                };

                CMShared<GraphicsPipeline> cp = AssignOrPanic(c.Get(gpci));

                visualize_probe_grid.pipeline_layout = pl;
                visualize_probe_grid.pipeline = cp;
                c.SetName(visualize_probe_grid.pipeline_layout, "Probe Grid Pipeline Layout");
                c.SetName(visualize_probe_grid.pipeline, "Probe Grid Compute Pipeline");

                vk::ClearValue clear_value;

                clear_value.color.float32[0] = 0.0f;
                clear_value.color.float32[1] = 0.0f;
                clear_value.color.float32[2] = 0.0f;
                clear_value.color.float32[3] = 1.0f;

                cb.bindPipeline(vk::PipelineBindPoint::eGraphics, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, common_resources->GetUBOBuffer()));
                    PanicIfError(ds->Bind(1, 0, common_resources->ddgi_pass->CurrentReadBindings()));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    VisualizeProbeGridPushConstants push_constants;

                    push_constants.scale = visualize_probe_grid.scale;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                cb.bindVertexBuffers(0, { **visualize_probe_grid.sphere_vbo }, { 0 });

                auto probe_counts = common_resources->ddgi_pass->GetProbeCounts();
                uint32_t probe_count = probe_counts.x * probe_counts.y * probe_counts.z;

                cb.draw(36, probe_count, 0, 0);
            }
        }

        std::vector<WriteDescriptorSetBindedResource> DeferredShading::OutputBindings() const
        {
            return { shading.texture };
        }
    }
}
