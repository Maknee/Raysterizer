#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace MiddleWare
	{
		class RenderState
		{
		public:
			void SetDepthTest(bool b) { depth_test = b; }
			bool GetDepthTest() const { return depth_test; }
			void SetDepthWriting(bool b) { depth_writing = b; }
			bool GetDepthWriting() const { return depth_writing; }
			void SetCullFace(bool b) { cull_face = b; }
			bool GetCullFace() const { return cull_face; }
			void SetBlend(bool b) { blend = b; }
			bool GetBlend() const { return blend; }
			void SetColorLogicOpEnabled(bool b) { color_logic_op_enabled = b; }
			bool GetColorLogicOpEnabled() const { return color_logic_op_enabled; }
			
			void SetClearColor(glm::vec4 color) { clear_color = color; }
			const glm::vec4& GetClearColor() const { return clear_color; }
			void SetClearDepth(float depth) { clear_depth = depth; }
			float GetClearDepth() const { return clear_depth; }

			void SetDepthFunc(vk::CompareOp depth_func_) { depth_func = depth_func_; }
			vk::CompareOp GetDepthFunc() const { return depth_func; }
			void SetDepthRange(float depth_near_, float depth_far_) { depth_near = depth_near_; depth_far = depth_far_; }
			float GetDepthNear() const { return depth_near; }
			float GetDepthFar() const { return depth_far; }

			void SetBlendColor(glm::vec4 blend_color_) { blend_color = blend_color_; }
			const glm::vec4& GetBlendColor() const { return blend_color; }

			void SetColorBlendOp(vk::BlendOp blend_op) { color_blend_op = blend_op; }
			vk::BlendOp GetColorBlendOp() const { return color_blend_op; }
			void SetAlphaBlendOp(vk::BlendOp blend_op) { alpha_blend_op = blend_op; }
			vk::BlendOp GetAlphaBlendOp() const { return alpha_blend_op; }
			void SetSrcColorBlendFactor(vk::BlendFactor blend_factor) { src_color_blend_factor = blend_factor; }
			vk::BlendFactor GetSrcColorBlendFactor() const { return src_color_blend_factor; }
			void SetDstColorBlendFactor(vk::BlendFactor blend_factor) { dst_color_blend_factor = blend_factor; }
			vk::BlendFactor GetDstColorBlendFactor() const { return dst_color_blend_factor; }
			void SetSrcAlphaBlendFactor(vk::BlendFactor blend_factor) { src_alpha_blend_factor = blend_factor; }
			vk::BlendFactor GetSrcAlphaBlendFactor() const { return src_alpha_blend_factor; }
			void SetDstAlphaBlendFactor(vk::BlendFactor blend_factor) { dst_alpha_blend_factor = blend_factor; }
			vk::BlendFactor GetDstAlphaBlendFactor() const { return dst_alpha_blend_factor; }
			void SetColorWriteMask(vk::ColorComponentFlags color_component_mask) { color_write_mask = color_component_mask; }
			vk::ColorComponentFlags GetColorWriteMask() const { return color_write_mask; }

			void SetColorLogicOp(vk::LogicOp logic_op) { color_logic_op = logic_op; }
			vk::LogicOp GetColorLogicOp() const { return color_logic_op; }

		private:
			bool depth_test = true;
			bool depth_writing = true;
			bool cull_face = true;
			bool blend = false;
			bool color_logic_op_enabled = false;

			glm::vec4 clear_color{ 0.0f, 0.0f, 0.0f, 0.0f };
			float clear_depth = 1.0f;

			vk::CompareOp depth_func = vk::CompareOp::eLessOrEqual;
			float depth_near = 0.0f;
			float depth_far = 1.0f;
			
			glm::vec4 blend_color{};

			vk::BlendOp color_blend_op;
			vk::BlendOp alpha_blend_op;
			vk::BlendFactor src_color_blend_factor;
			vk::BlendFactor dst_color_blend_factor;
			vk::BlendFactor src_alpha_blend_factor;
			vk::BlendFactor dst_alpha_blend_factor;

			vk::ColorComponentFlags color_write_mask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

			vk::LogicOp color_logic_op;
		};

		struct RenderCallRecord
		{
			std::shared_ptr<Pass::GBufferPass> g_buffer_pass;
			uint32_t first;
			uint64_t count;
			uint32_t instance_count;
		};

		class IndexPool
		{
		public:
			explicit IndexPool(std::size_t starting_limit = 0) :
				current_limit(starting_limit)
			{}

			std::size_t Get()
			{
				if (unused.empty())
				{
					auto item = current_limit++;
					used.emplace(item);

					return item;
				}
				else
				{
					auto iter = std::begin(unused);
					auto item = *iter;

					used.emplace(item);
					unused.erase(iter);
					return item;
				}
			}

			void Remove(std::size_t index)
			{
				if (index >= current_limit)
				{
					PANIC("Above limit {} > {}", index, current_limit);
				}
				used.erase(index);
				unused.emplace(index);
			}

			void Clear()
			{
				used.clear();
				unused.clear();
				current_limit = 0;
			}

			std::size_t GetLimit() const { return current_limit; }

		private:
			phmap::flat_hash_set<std::size_t> used;
			phmap::flat_hash_set<std::size_t> unused;
			std::size_t current_limit = 0;
		};

#define RAYSTERIZER_DATA_TYPE_UNKNOWN 0
#define RAYSTERIZER_DATA_TYPE_INT 1
#define RAYSTERIZER_DATA_TYPE_IVEC2 2
#define RAYSTERIZER_DATA_TYPE_IVEC3 3
#define RAYSTERIZER_DATA_TYPE_IVEC4 4
#define RAYSTERIZER_DATA_TYPE_UINT 5
#define RAYSTERIZER_DATA_TYPE_UVEC2 6
#define RAYSTERIZER_DATA_TYPE_UVEC3 7
#define RAYSTERIZER_DATA_TYPE_UVEC4 8
#define RAYSTERIZER_DATA_TYPE_FLOAT 9
#define RAYSTERIZER_DATA_TYPE_VEC2 10
#define RAYSTERIZER_DATA_TYPE_VEC3 11
#define RAYSTERIZER_DATA_TYPE_VEC4 12

		using UInt8Ptr = vk::DeviceAddress;
		struct InstanceData
		{
			RAYSTERIZER_ALIGN(4) uint64_t material_index;

			RAYSTERIZER_ALIGN(4) UInt8Ptr vertices;
			RAYSTERIZER_ALIGN(4) UInt8Ptr indices;

			RAYSTERIZER_ALIGN(4) uint64_t vertex_stride;
			RAYSTERIZER_ALIGN(4) uint64_t index_stride;

			RAYSTERIZER_ALIGN(4) uint64_t position_offset;
			RAYSTERIZER_ALIGN(4) uint64_t normal_offset;
			RAYSTERIZER_ALIGN(4) uint64_t tex_coord_offset;

			RAYSTERIZER_ALIGN(4) uint64_t position_data_type;
			RAYSTERIZER_ALIGN(4) uint64_t normal_data_type;
			RAYSTERIZER_ALIGN(4) uint64_t tex_coord_data_type;

			RAYSTERIZER_ALIGN(4) uint64_t out_color_buffer_index;
		};

		struct InstanceDataOffsets
		{
			RAYSTERIZER_ALIGN(4) uint64_t position_offset = -1;
			RAYSTERIZER_ALIGN(4) uint64_t normal_offset = -1;
			RAYSTERIZER_ALIGN(4) uint64_t tex_coord_offset = -1;

			RAYSTERIZER_ALIGN(4) uint64_t position_data_type = RAYSTERIZER_DATA_TYPE_UNKNOWN;
			RAYSTERIZER_ALIGN(4) uint64_t normal_data_type = RAYSTERIZER_DATA_TYPE_UNKNOWN;
			RAYSTERIZER_ALIGN(4) uint64_t tex_coord_data_type = RAYSTERIZER_DATA_TYPE_UNKNOWN;
		};

		struct MeshData
		{
			RAYSTERIZER_ALIGN(4) UInt8Ptr vertices;
			RAYSTERIZER_ALIGN(4) UInt8Ptr indices;
		};

		template<typename T>
		class BufferWithIndexPool
		{
		public:
			T& Get(std::size_t hash)
			{
				RenderFrame& render_frame = c.GetRenderFrame();

				if (auto found = hash_to_index.find(hash); found != std::end(hash_to_index))
				{
					const auto& index = found->second;
					auto& t = buffer->MapAs<T*>()[index];
					return t;
				}
				else
				{
					auto index = index_pool.Get();

					if (buffer)
					{
						if ((buffer->GetSize() / sizeof(T)) < index_pool.GetLimit())
						{
							buffer = AssignOrPanic(render_frame.ResizeBuffer(buffer, sizeof(T) * index_pool.GetLimit()));
						}
					}
					else
					{
						buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eStorageBuffer, sizeof(T), true));
					}

					hash_to_index[hash] = index;
					auto& t = buffer->MapAs<T*>()[index];
					return t;
				}
			}

			void Clear()
			{
				hash_to_index.clear();
				index_pool.Clear();
			}
			
			auto GetBuffer()
			{
				return buffer;
			}

			void SetName(std::string_view name)
			{
				c.SetName(buffer, name);
			}

		private:
			flat_hash_map<std::size_t, std::size_t> hash_to_index;
			CMShared<Buffer> buffer;
			IndexPool index_pool;
		};

		struct Material
		{
			RAYSTERIZER_ALIGN(16) vec4 highlight_color;
			RAYSTERIZER_ALIGN(16) bool highlight;

			RAYSTERIZER_ALIGN(16) vec4 albedo;
			RAYSTERIZER_ALIGN(16) vec4 roughness_metallic;
			RAYSTERIZER_ALIGN(16) vec4 emissive;
		};

		struct MaterialInfo
		{
			std::size_t hash;
			std::string name;

			Material material;
		};
		void to_json(json& j, const Material& material);
		void from_json(const json& j, Material& material);
		void to_json(json& j, const MaterialInfo& material_override);
		void from_json(const json& j, MaterialInfo& material_override);

		struct MaterialInfoWithIndex
		{
			std::size_t index;
			MaterialInfo m;
		};

		enum GameType
		{
			Default,
			OSRS,
			Dolphin,
			Roblox,
			PPSSPP,
		};

		// This keeps track of descriptor sets, SBT and vulkan pipeline state.
		// It should hold the draw calls (for the buffers and shaders)
		class PipelineManager
		{
		public:
			explicit PipelineManager();
			void Setup();

			Error InsertProgramToDrawCall(GLuint program_id,
										  Raysterizer::OpenGL::VertexShader& vertex_shader, 
										  Raysterizer::OpenGL::FragmentShader& fragment_shader);
			Error RemoveProgramToDrawCall(GLuint program_id);

			flat_hash_map<GLuint, std::shared_ptr<DrawCalls>>& GetCurrentProgramToDrawCalls();
			const flat_hash_map<GLuint, std::shared_ptr<DrawCalls>>& GetCurrentProgramToDrawCalls() const;
			Expected<DrawCalls&> GetProgramToDrawCalls(GLuint program);
			void BeginFrame();
			void BuildResourcesForRaytracing();
			void Draw();

			auto& GetDrawCallToBindingIndex() { return draw_call_to_binding_index; }
			auto& GetDrawCallStorageBufferBindingIndexPool() { return draw_call_storage_buffer_binding_index_pool; }
			auto& GetDrawCallUniformBufferBindingIndexPool() { return draw_call_uniform_buffer_binding_index_pool; }
			auto& GetDrawCallCombinedSamplerBufferBindingIndexPool() { return draw_call_combined_sampler_binding_index_pool; }

			auto& GetGlobalDescriptorPool() { return global_descriptor_pools[c.GetFrameIndex()]; }

			Error CreateGBufferPass(std::size_t id);
			Error DeleteGBufferPass(std::size_t id);
			Error SetActiveGBufferPass(std::size_t id);
			Expected<std::shared_ptr<Pass::GBufferPass>> GetGBufferPass(std::size_t id);

			void RecordRenderCall(RenderCallRecord render_call_record);

			MaterialInfoWithIndex& GetMaterialInfoWithIndex(std::size_t hash);
			Material& GetMaterialWithIndex(std::size_t hash);
			void RemoveMaterialInfoWithIndex(std::size_t hash);
			CMShared<Buffer> GetMaterialsBuffer() { return materials_buffer; }

			auto GetTLAS() { return tlases[c.GetFrameIndex()]; }

			auto GetGameType() { return game_type; }

		private:
			void GenerateRaytracingStorageTexture();

		private:
			flat_hash_map<GLuint, std::shared_ptr<DrawCalls>> program_to_draw_calls{};
			const std::size_t draw_call_to_set_starting_index = 0;
			IndexPool draw_call_to_binding_index = IndexPool(draw_call_to_set_starting_index);

			IndexPool draw_call_storage_buffer_binding_index_pool = IndexPool{};
			IndexPool draw_call_uniform_buffer_binding_index_pool = IndexPool{};
			IndexPool draw_call_combined_sampler_binding_index_pool = IndexPool{};

			// Vulkan
			CMShared<Buffer> raysterizer_info_buffer{};

			CacheMappingWithFrameCounter<XXH64_hash_t, CMShared<BottomLevelAccelerationStructure>> blas_cache;
			std::vector<CMShared<TopLevelAccelerationStructure>> tlases{};

			CMShared<Texture> raytracing_read_storage_texture{};
			CMShared<Texture> raytracing_write_storage_texture{};

			CMShared<Texture> default_image{};
			CMShared<Sampler> default_sampler{};

			ShaderBindingTable sbt;
			std::vector<CMShared<Buffer>> sbt_buffers;
			std::vector<std::size_t> sbt_shader_hashes{};

			CMShared<CommandBuffer> imgui_command_buffer{};

			const vk::ShaderStageFlags accessible_stages = vk::ShaderStageFlagBits::eRaygenKHR 
				| vk::ShaderStageFlagBits::eClosestHitKHR 
				| vk::ShaderStageFlagBits::eMissKHR
				| vk::ShaderStageFlagBits::eAnyHitKHR;

			std::vector<ShaderModuleCreateInfo> base_shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
			{
				ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("raysterizer.rgen")},
				ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("raysterizer.rmiss")}
			};

			std::vector<CMShared<Semaphore>> render_semaphores;
			std::vector<CMShared<Fence>> render_fences;
			std::vector<bool> render_fence_ready;
			std::vector<CMShared<CommandBuffer>> command_buffers;

			CMShared<PipelineLayoutInfo> base_pli{};
			DescriptorSetLayouts global_descriptor_set_layouts;
			std::vector<CMShared<DescriptorPool>> global_descriptor_pools{};
			std::vector<CMShared<DescriptorSet>> global_descriptor_sets{};
			CMShared<PipelineLayout> global_pipeline_layout{};
			std::vector<CMShared<ShaderModule>> global_raytracing_shaders{};
			CMShared<RaytracingPipeline> global_raytracing_pipeline{};

			vk::PhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties{};

		public:
			std::shared_ptr<Pass::CommonResources> common_resources;
			flat_hash_map<std::size_t, std::shared_ptr<Pass::GBufferPass>> id_to_g_buffer_pass;
			std::vector<std::shared_ptr<Pass::GBufferPass>> to_be_deleted_g_buffer_pass;
			std::shared_ptr<Pass::GBufferPass> active_g_buffer_pass;
			Pass::BlueNoise blue_noise;
			Pass::RayTracedShadows ray_traced_shadows_pass;
			Pass::RayTracedAO ray_traced_ao_pass;
			Pass::DDGI ddgi_pass;
			Pass::RayTracedReflections ray_traced_reflections_pass;
			Pass::DeferredShading deferred_shading_pass;
			Pass::TemporalAA temporal_aa_pass;
			Pass::ToneMapPass tone_map_pass;
			Pass::SkyEnvironment sky_environment;
			std::vector<RenderCallRecord> render_call_records;

			std::vector<CMShared<Buffer>> out_color_buffers;

			flat_hash_map<std::size_t, MaterialInfoWithIndex> material_info_mapping;
			CMShared<Buffer> materials_buffer;
			IndexPool materials_buffer_index_pool;

			BufferWithIndexPool<InstanceData> instance_data_pool;
			BufferWithIndexPool<MeshData> mesh_data_pool;

			flat_hash_map<GLuint, InstanceDataOffsets> program_to_instance_data_offsets{};

			const std::size_t INSTANCE_CUSTOM_INDEX_MATERIAL_UNUSED = -1;
			const std::size_t INSTANCE_CUSTOM_INDEX_MATERIAL_START_INDEX = 0;

			QueueBatchManager transfer_qbm;
			QueueBatchManager compute_qbm;
			QueueBatchManager graphics_qbm;

			CMShared<CommandBuffer> frame_command_buffer;
			CMShared<Semaphore> present_semaphore;
			CMShared<Semaphore> render_semaphore;
			CMShared<Fence> render_fence;

			bool skip_frame = false;

			GameType game_type;
		};

		struct RaysterizerVulkanState
		{
		public:
			static RaysterizerVulkanState& Get()
			{
				static RaysterizerVulkanState raysterizer_vulkan_state{};
				return raysterizer_vulkan_state;
			}

			void Setup();
			PipelineManager& GetPipelineManager();
			std::vector<PipelineManager>& GetPipelineManagers() { return pipeline_managers; }
			auto& GetRenderState() { return render_state; }
			const auto& GetRenderState() const { return render_state; }

		private:
			std::vector<PipelineManager> pipeline_managers;
			RenderState render_state;
		};
	}
}