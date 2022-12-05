#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
    struct FossilizeState : Fossilize::StateCreatorInterface
    {
        explicit FossilizeState(Context* c_);
        bool enqueue_create_sampler(Fossilize::Hash hash, const VkSamplerCreateInfo* create_info, VkSampler* sampler) final;
        bool enqueue_create_descriptor_set_layout(Fossilize::Hash hash, const VkDescriptorSetLayoutCreateInfo* create_info, VkDescriptorSetLayout* layout) final;
        bool enqueue_create_pipeline_layout(Fossilize::Hash hash, const VkPipelineLayoutCreateInfo* create_info, VkPipelineLayout* layout) final;
        bool enqueue_create_shader_module(Fossilize::Hash hash, const VkShaderModuleCreateInfo* create_info, VkShaderModule* shader_module) final;
        bool enqueue_create_render_pass(Fossilize::Hash hash, const VkRenderPassCreateInfo* create_info, VkRenderPass* render_pass) final;
        bool enqueue_create_render_pass2(Fossilize::Hash hash, const VkRenderPassCreateInfo2* create_info, VkRenderPass* render_pass) final;
        bool enqueue_create_compute_pipeline(Fossilize::Hash hash, const VkComputePipelineCreateInfo* create_info, VkPipeline* pipeline) final;
        bool enqueue_create_graphics_pipeline(Fossilize::Hash hash, const VkGraphicsPipelineCreateInfo* create_info, VkPipeline* pipeline) final;
        bool enqueue_create_raytracing_pipeline(Fossilize::Hash hash, const VkRayTracingPipelineCreateInfoKHR* create_info, VkPipeline* pipeline) final;
        
        void sync_threads() final;
        void sync_shader_modules() final;
        void notify_replayed_resources_for_type() final;

        Context* c{};
        flat_hash_map<Fossilize::Hash, ShaderModuleCreateInfo> shader_module_create_info_mapping;
        flat_hash_map<Fossilize::Hash, CMShared<DescriptorSetLayout>> descriptor_set_layout_mapping;
        flat_hash_map<Fossilize::Hash, CMShared<PipelineLayout>> pipeline_layout_mapping;
        flat_hash_map<Fossilize::Hash, CMShared<RenderPass>> render_pass_mapping;

        inline static const vk::ShaderStageFlags SHADER_MODULE_FLAGS = vk::ShaderStageFlags(vk::ShaderStageFlagBits::eAllGraphics);
        inline static const vk::ShaderStageFlags VARIABLE_BINDING_FLAGS = vk::ShaderStageFlags(vk::ShaderStageFlagBits::eAll);
    };

	class DiskCacher
	{
    public:
		explicit DiskCacher();
        ~DiskCacher();
        Error Init(Context* c_);
        Error Record(CMShared<DescriptorSetLayout> descriptor_set_layout);
        Error Record(CMShared<PipelineLayout> pipeline_layout);
        Error Record(CMShared<ShaderModule> shader_module);
        Error Record(CMShared<GraphicsPipeline> graphics_pipeline);
        Error Record(CMShared<ComputePipeline> compute_pipeline);
        Error Record(CMShared<RaytracingPipeline> raytracing_pipeline);
        Error Record(CMShared<RenderPass> render_pass);
        Error Record(CMShared<Sampler> sampler);

        template<typename T, typename... Args>
        inline Error Record(Args&&... args)
        {
            bool disk_cache_enabled = Config["cache"]["disk"]["enable"];
            if (disk_cache_enabled)
            {
                ReturnIfError(Record(std::forward<Args>(args)...));
            }
            return NoError();
        }

        Error SyncFlush();
        Error AsyncFlush();
        bool IsEnabled();

	private:
        Context* c{};
		std::unique_ptr<Fossilize::DatabaseInterface> db{};
		Fossilize::StateRecorder recorder;
    };
}