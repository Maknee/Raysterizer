#pragma once

#include "pch.h"

namespace Raysterizer
{
    namespace Pass
    {
        class DeferredShading
        {
        public:
            explicit DeferredShading() {}
            Error Setup(std::shared_ptr<CommonResources> common_resources_);
            void Render(CMShared<CommandBuffer> command_buffer);
            void UpdateGui();

            void SetUseRayTracedAO(bool use) { shading.use_ray_traced_ao = use; }
            bool GetUseRayTracedAO() const { return shading.use_ray_traced_ao; }
            void SetUseRayTracedShadows(bool use) { shading.use_ray_traced_shadows = use; }
            bool GetUseRayTracedShadows() const { return shading.use_ray_traced_shadows; }
            void SetUseRayTracedReflections(bool use) { shading.use_ray_traced_reflections = use; }
            bool GetUseRayTracedReflections() const { return shading.use_ray_traced_reflections; }
            void SetUseDDGI(bool use) { shading.use_ddgi = use; }
            bool GetUseDDGI() const { return shading.use_ddgi; }

        private:
            void LoadSphereMesh();
            void CreateCube();
            void CreateImages();
            void CreateRenderPass();
            void CreateFrameBuffer();
            void RenderShading(CMShared<CommandBuffer> command_buffer);
            void RenderSkybox(CMShared<CommandBuffer> command_buffer);
            void RenderProbes(CMShared<CommandBuffer> command_buffer);

        public:
            std::vector<WriteDescriptorSetBindedResource> OutputBindings() const;

        private:
            struct Shading
            {
                bool                          use_ray_traced_ao = true;
                bool                          use_ray_traced_shadows = true;
                bool                          use_ray_traced_reflections = true;
                bool                          use_ddgi = true;
                CMShared<RenderPass>       rp;
                vk::UniqueFramebuffer fbo;
                CMShared<Image>            image;
                CMShared<ImageView>        view;
                CMShared<Texture>        texture;
                CMShared<GraphicsPipeline> pipeline;
                CMShared<PipelineLayout>   pipeline_layout;
                CMShared<DescriptorSet>    read_ds;
            };

            struct Skybox
            {
                CMShared<Buffer> cube_vbo;
                CMShared<GraphicsPipeline> pipeline;
                CMShared<PipelineLayout>   pipeline_layout;
                CMShared<RenderPass>       rp;
                std::vector<CMShared<FrameBuffer>>      fbo;
            };

            struct VisualizeProbeGrid
            {
                bool                          enabled = false;
                float                         scale = 1.0f;
                CMShared<Buffer> sphere_vbo;
                CMShared<GraphicsPipeline> pipeline;
                CMShared<PipelineLayout>   pipeline_layout;
            };

            uint32_t                       width;
            uint32_t                       height;
            std::shared_ptr<CommonResources> common_resources;
            Shading                        shading;
            Skybox skybox;
            VisualizeProbeGrid             visualize_probe_grid;

            std::shared_ptr<GBufferPass> current_gbuffer_pass;
        };
    }
}