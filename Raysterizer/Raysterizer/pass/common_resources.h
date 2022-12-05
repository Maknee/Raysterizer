#pragma once

#include "pch.h"
#include <winnt.h>

#define RAYSTERIZER_ALIGN(x) __declspec(align(x))

namespace Raysterizer
{
	namespace MiddleWare
	{
		class PipelineManager;
		class RaysterizerVulkanState;
	}

	namespace Pass
	{
		enum VisualizationType
		{
			VISUALIZATION_TYPE_FINAL,
			VISUALIZATION_TYPE_SHADOWS,
			VISUALIZATION_TYPE_AMBIENT_OCCLUSION,
			VISUALIZATION_TYPE_REFLECTIONS,
			VISUALIZATION_TYPE_GLOBAL_ILLUIMINATION,
			VISUALIZATION_TYPE_GROUND_TRUTH,
			VISUALIZATION_TYPE_NORMALS,
		};

		enum LightType
		{
			LIGHT_TYPE_DIRECTIONAL,
			LIGHT_TYPE_POINT,
			LIGHT_TYPE_SPOT,
			LIGHT_TYPE_COUNT
		};

		struct Light
		{
			glm::vec4 data0;
			glm::vec4 data1;
			glm::vec4 data2;
			glm::vec4 data3;

			inline void set_light_direction(glm::vec3 value)
			{
				data0.x = value.x;
				data0.y = value.y;
				data0.z = value.z;
			}

			inline void set_light_position(glm::vec3 value)
			{
				data1.x = value.x;
				data1.y = value.y;
				data1.z = value.z;
			}

			inline void set_light_color(glm::vec3 value)
			{
				data2.x = value.x;
				data2.y = value.y;
				data2.z = value.z;
			}

			inline void set_light_intensity(float value)
			{
				data0.w = value;
			}

			inline void set_light_radius(float value)
			{
				data1.w = value;
			}

			inline void set_light_type(LightType value)
			{
				data3.x = value;
			}

			inline void set_light_cos_theta_outer(float value)
			{
				data3.y = value;
			}

			inline void set_light_cos_theta_inner(float value)
			{
				data3.z = value;
			}
		};

		// Uniform buffer data structure.
		struct UBO
		{
			RAYSTERIZER_ALIGN(16)
				glm::mat4 view_inverse;
			RAYSTERIZER_ALIGN(16)
				glm::mat4 proj_inverse;
			RAYSTERIZER_ALIGN(16)
				glm::mat4 view_proj_inverse;
			RAYSTERIZER_ALIGN(16)
				glm::mat4 prev_view_proj;
			RAYSTERIZER_ALIGN(16)
				glm::mat4 view_proj;
			RAYSTERIZER_ALIGN(16)
				glm::vec4 cam_pos;
			RAYSTERIZER_ALIGN(16)
				glm::vec4 near_far;
			RAYSTERIZER_ALIGN(16)
				glm::vec4 current_prev_jitter;
			RAYSTERIZER_ALIGN(16)
				Light light;
		};

		enum EnvironmentType
		{
			ENVIRONMENT_TYPE_NONE,
			ENVIRONMENT_TYPE_PROCEDURAL_SKY,
			ENVIRONMENT_TYPE_ARCHES_PINE_TREE,
			ENVIRONMENT_TYPE_BASKETBALL_COURT,
			ENVIRONMENT_TYPE_ETNIES_PART_CENTRAL,
			ENVIRONMENT_TYPE_LA_DOWNTOWN_HELIPAD
		};

		enum RayTraceScale
		{
			RAY_TRACE_SCALE_FULL_RES,
			RAY_TRACE_SCALE_HALF_RES,
			RAY_TRACE_SCALE_QUARTER_RES
		};

		class GBufferPass;
		class BlueNoise;
		class RayTracedShadows;
		class RayTracedAO;
		class DDGI;
		class RayTracedReflections;
		class DeferredShading;
		class TemporalAA;
		class ToneMapPass;
		class BRDFIntegrateLUT;
		class SkyEnvironment;

		class CommonResources
		{
		public:
			Error Setup();
			void SetupBRDFIntegrateLUT();
			CMShared<Buffer> GetUBOBuffer();

			uint32_t width;
			uint32_t height;
			std::vector<CMShared<DescriptorPool>> global_descriptor_pools{};

			std::shared_ptr<GBufferPass> g_buffer_pass;

			Raysterizer::MiddleWare::PipelineManager* pipeline_manager;

			CMShared<Sampler> nearest_sampler;
			CMShared<Sampler> bilinear_sampler;
			CMShared<Sampler> trilinear_sampler;

			BlueNoise* blue_noise;
			RayTracedShadows* ray_traced_shadows_pass;
			RayTracedAO* ray_traced_ao_pass;
			DDGI* ddgi_pass;
			RayTracedReflections* ray_traced_reflections_pass;
			DeferredShading* deferred_shading_pass;
			TemporalAA* temporal_aa_pass;
			ToneMapPass* tone_map_pass;
			BRDFIntegrateLUT* brdf_integrate_lut;
			SkyEnvironment* sky_environment;

			// Shadows
			struct RaytracingShadows
			{
				RayTraceScale scale = RayTraceScale::RAY_TRACE_SCALE_FULL_RES;
			} shadows;

			// AO
			struct RaytracingAO
			{
				RayTraceScale scale = RayTraceScale::RAY_TRACE_SCALE_FULL_RES;
				glm::vec4 z_buffer_params;
			} ao;

			// DDGI
			struct DDGI
			{
				RayTraceScale scale = RayTraceScale::RAY_TRACE_SCALE_FULL_RES;
				uint32_t id = 0;
				glm::vec3 min_extents = glm::vec3(-25, -0.5, -25);
				glm::vec3 max_extents = glm::vec3(25, 10, 25);
				bool sample_from_vertices = false;
				uint32_t sample_from_vertices_amount = 64;
			} ddgi;

			// Reflections
			struct RaytracingReflections
			{
				RayTraceScale scale = RayTraceScale::RAY_TRACE_SCALE_FULL_RES;
			} reflections;

			VisualizationType                            current_visualization_type = VISUALIZATION_TYPE_FINAL;
			std::vector<CMShared<Buffer>> per_frame_ubo_buffers;
			const std::size_t ping_pong_size = 2;
			bool ping_pong = false;

			EnvironmentType environment_type = ENVIRONMENT_TYPE_PROCEDURAL_SKY;

			struct LightInfo {
				ImGuizmo::OPERATION light_transform_operation = ImGuizmo::OPERATION::ROTATE;
				glm::mat4           light_transform = glm::mat4(1.0f);
				float               light_radius = 0.1f;
				glm::vec3           light_direction = glm::normalize(glm::vec3(-0.260, -0.248, -0.955));
				glm::vec3           light_position = glm::vec3(5.0f);
				glm::vec3           light_color = glm::vec3(1.0f);
				float               light_intensity = 1.0f;
				float               light_cone_angle_inner = 40.0f;
				float               light_cone_angle_outer = 50.0f;
				float               light_animation_time = 0.0f;
				bool                light_animation = false;
				LightType           light_type = LIGHT_TYPE_DIRECTIONAL;
			} light;

			struct View {
				glm::vec3                                    prev_position;
				glm::mat4                                    view;
				glm::mat4                                    projection;
				glm::mat4                                    prev_view_projection;
			} view;
			bool                                         first_frame = true;

			glm::vec3           procedural_sky_direction = glm::normalize(glm::vec3(-0.260, -0.248, -0.955));

			std::vector<WriteDescriptorSetBindedResource> OutputBindings() const;

			static CMShared<Image> CreateImage(vk::ImageType type, uint32_t width, uint32_t height, uint32_t depth,
				uint32_t mip_levels, uint32_t array_size, vk::Format format, 
				VmaMemoryUsage memory_usage, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sample_count, 
				vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined, size_t size = 0, void* data = nullptr, vk::ImageCreateFlags flags = {}, vk::ImageTiling tiling = vk::ImageTiling::eOptimal);

			static CMShared<ImageView> CreateImageView(CMShared<Image> image, vk::ImageViewType view_type, vk::ImageAspectFlags aspect_flags,
				uint32_t base_mip_level = 0, uint32_t level_count = 1, uint32_t base_array_layer = 0, uint32_t layer_count = 1);

			static CMShared<Image> CreateImageFromFile(fs::path path, bool flip_vertical = false, bool srgb = false);

			static void UploadImageData(CMShared<Image> image, void* data, const std::vector<size_t>& mip_level_sizes, VkImageLayout src_layout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout dst_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		};
	}
}