#include "pipeline_manager.h"

namespace Raysterizer
{
	namespace MiddleWare
	{
		inline std::array<float, 4> ConvertToArray(glm::vec4 v)
		{
			return std::array<float, 4>{v[0], v[1], v[2], v[3]};
		}

		inline glm::vec4 ConvertFromArray(std::array<float, 4> v)
		{
			return glm::vec4(v[0], v[1], v[2], v[3]);
		}

		void to_json(json& j, const Material& material)
		{
			/*
			j = json
			{
				{ "highlight_color", ConvertToArray(material.highlight_color) },
				{ "highlight", material.highlight },
				{ "albedo", ConvertToArray(material.albedo) },
				{ "fuzziness", material.fuzziness },
				{ "refraction_index", material.refraction_index },
				{ "material_model", material.material_model },
			};
			*/
		}

		void from_json(const json& j, Material& material)
		{
			/*
			std::array<float, 4> highlight_color;
			j.at("highlight_color").get_to(highlight_color);
			material.highlight_color = ConvertFromArray(highlight_color);
			j.at("highlight").get_to(material.highlight);
			std::array<float, 4> diffuse;
			j.at("diffuse").get_to(diffuse);
			material.albedo = ConvertFromArray(diffuse);
			j.at("fuzziness").get_to(material.fuzziness);
			j.at("refraction_index").get_to(material.refraction_index);
			j.at("material_model").get_to(material.material_model);
			*/
		}

		void to_json(json& j, const MaterialInfo& material_info)
		{
			json material_json;
			to_json(material_json, material_info.material);

			j = json
			{
				{ "hash", material_info.hash },
				{ "name", material_info.name },
				{ "material", std::move(material_json) },
			};
		}

		void from_json(const json& j, MaterialInfo& material_info)
		{
			j.at("hash").get_to(material_info.hash);
			j.at("name").get_to(material_info.name);
			j.at("material").get_to(material_info.material);
		}

		PipelineManager::PipelineManager()
		{
		}

		void SetupImGui(RaysterizerEngine::Context& c, CMShared<RenderPass> rp, GLFWWindow* glfw_window)
		{
			vk::Device device = c.GetDevice();

			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000;
			pool_info.poolSizeCount = std::size(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;

			vk::DescriptorPool imgui_dp = AssignOrPanicVkError(device.createDescriptorPool(pool_info));

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			// Setup Dear ImGui style
			//ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

			ImGui_ImplGlfw_InitForVulkan(reinterpret_cast<GLFWwindow*>(glfw_window->GetUnderlyingWindow()), true);
			auto imgui_init_info = ImGui_ImplVulkan_InitInfo{};
			imgui_init_info.Instance = c.GetInstance();
			imgui_init_info.PhysicalDevice = c.GetPhysicalDevice();
			imgui_init_info.Device = c.GetDevice();
			imgui_init_info.QueueFamily = c.GetGraphicsQueueFamily();
			imgui_init_info.Queue = c.GetGraphicsQueue();
			imgui_init_info.DescriptorPool = imgui_dp;
			imgui_init_info.MinImageCount = c.GetNumFrames();
			imgui_init_info.ImageCount = c.GetNumFrames() * 2;
			imgui_init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			ImGui_ImplVulkan_Init(&imgui_init_info, rp->render_pass);

			PanicIfError(c.ImmediateGraphicsSubmit([&](CommandBuffer& cb)
				{
					ImGui_ImplVulkan_CreateFontsTexture(*cb);
				}));
			device.waitIdle();

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		void RenderImGui(vk::CommandBuffer cb)
		{
			ImDrawData* draw_data = ImGui::GetDrawData();
			const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
			if (!is_minimized)
			{
				ImGui_ImplVulkan_RenderDrawData(draw_data, cb);
			}
		}

		void PipelineManager::Setup()
		{
			RenderFrame& render_frame = c.GetRenderFrame();

			GenerateRaytracingStorageTexture();

			PanicIfError(c.CreateGlobalRenderPass());
			CMShared<RenderPass> rp = c.GetGlobalRenderPass();
			c.SetName(rp, "ImGui renderpass");

			GLFWWindow* glfw_window = (GLFWWindow*)&*vulkan_window;
			SetupImGui(c, rp, glfw_window);
			//imgui_command_buffer = AssignOrPanic(render_frame.GetCommandBuffer(QueueType::Graphics));

			blas_cache.SetContext(&c);

			auto& context = Raysterizer::OpenGL::Context::Get();
			auto& state = context.state;
			auto raysterizer_info = state.GetRaysterizerInfo();
			raysterizer_info_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(raysterizer_info), true));

			const static std::size_t draw_call_binding_index_pool_interval = Config["raysterizer"]["draw_call"]["binding_index_pool_interval"];
			const static std::size_t draw_call_storage_buffer_binding_index_pool_starting_index = Config["raysterizer"]["draw_call"]["storage_buffer_binding_index_pool_starting_index"];
			const static std::size_t draw_call_uniform_buffer_binding_index_pool_starting_index = Config["raysterizer"]["draw_call"]["uniform_buffer_binding_index_pool_starting_index"];
			const static std::size_t draw_call_combined_sampler_binding_index_pool_starting_index = Config["raysterizer"]["draw_call"]["combined_sampler_binding_index_pool_starting_index"];
			
			draw_call_storage_buffer_binding_index_pool = IndexPool{ draw_call_storage_buffer_binding_index_pool_starting_index };
			draw_call_uniform_buffer_binding_index_pool = IndexPool{ draw_call_uniform_buffer_binding_index_pool_starting_index };
			draw_call_combined_sampler_binding_index_pool = IndexPool{ draw_call_combined_sampler_binding_index_pool_starting_index };

			const static std::size_t raysterizer_draw_call_set = Config["raysterizer"]["draw_call"]["set"];

			/*
			{
				const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];

				auto shader_module_create_infos = base_shader_module_create_infos;
				flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};
				variable_set_index_to_count[0] = max_variable_bindings;

				auto plci = PipelineLayoutCreateInfo
				{
					shader_module_create_infos,
					variable_set_index_to_count
				};

				base_pli = AssignOrPanic(c.CreatePipelineLayoutInfo(plci));
				auto descriptor_set_layout_create_infos2 = AssignOrPanic(plci.BuildDescriptorSetLayoutCreateInfos(base_pli->combined_shader_reflection));
				auto& descriptor_set_layout_create_infos = descriptor_set_layout_create_infos2.GetCreateInfos();
				for (auto& descriptor_set_layout_create_info : descriptor_set_layout_create_infos)
				{
					for (auto& [binding, descriptor_layout_binding] : descriptor_set_layout_create_info.GetBindings())
					{
						descriptor_layout_binding.descriptor_set_layout_binding.stageFlags |= vk::ShaderStageFlagBits::eAll;
					}
				}
				//auto base_descriptor_set_layouts = base_pli->descriptor_set_layouts;
				auto base_descriptor_set_layouts = AssignOrPanic(c.Get(descriptor_set_layout_create_infos2));

				// now add the rest of descriptor set layout
				DescriptorSetLayoutCreateInfo dslci;
				for (auto i = 0; i < draw_call_binding_index_pool_interval; i++)
				{
					dslci.AddBinding(i + draw_call_storage_buffer_binding_index_pool_starting_index, max_variable_bindings, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
				}
				for (auto i = 0; i < draw_call_binding_index_pool_interval; i++)
				{
					dslci.AddBinding(i + draw_call_uniform_buffer_binding_index_pool_starting_index, max_variable_bindings, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
				}
				for (auto i = 0; i < draw_call_binding_index_pool_interval; i++)
				{
					dslci.AddBinding(i + draw_call_combined_sampler_binding_index_pool_starting_index, max_variable_bindings, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eClosestHitKHR);
				}

				auto added_dsl = AssignOrPanic(c.CreateDescriptorSetLayout(dslci));
				base_descriptor_set_layouts.descriptor_set_layouts.emplace_back(added_dsl);

				global_descriptor_set_layouts = base_descriptor_set_layouts;
			}

			const auto& vulkan_descriptor_set_layouts = global_descriptor_set_layouts.GetDescriptorSetLayouts();
			{
				auto pipeline_create_info = vk::PipelineLayoutCreateInfo{}
					.setPushConstantRanges({})
					.setSetLayouts(vulkan_descriptor_set_layouts);

				AssignOrPanicVkError(auto vulkan_pipeline_layout, c.GetDevice().createPipelineLayout(pipeline_create_info));
				auto pipeline_layout = std::make_shared<PipelineLayout>();
				pipeline_layout->pipeline_layout = vulkan_pipeline_layout;

				global_pipeline_layout = pipeline_layout;
			}

			{
				global_raytracing_pipeline = std::make_shared<RaytracingPipeline>();
			}
			*/

			// TEMP?
			auto pool_size = uint32_t(100000);
			auto pool_sizes = std::vector<vk::DescriptorPoolSize>
			{
				{ vk::DescriptorType::eSampler, pool_size },
				{ vk::DescriptorType::eCombinedImageSampler, pool_size },
				{ vk::DescriptorType::eSampledImage, pool_size },
				{ vk::DescriptorType::eStorageImage, pool_size },
				{ vk::DescriptorType::eUniformTexelBuffer, pool_size },
				{ vk::DescriptorType::eStorageTexelBuffer, pool_size },
				{ vk::DescriptorType::eUniformBuffer, pool_size },
				{ vk::DescriptorType::eStorageBuffer, pool_size },
				{ vk::DescriptorType::eUniformBufferDynamic, pool_size },
				{ vk::DescriptorType::eStorageBufferDynamic, pool_size },
				{ vk::DescriptorType::eInputAttachment, pool_size },
				{ vk::DescriptorType::eInlineUniformBlock, pool_size },
				{ vk::DescriptorType::eAccelerationStructureKHR, pool_size },
			};

			auto descriptor_pool_create_info = vk::DescriptorPoolCreateInfo{}
				.setMaxSets(10000)
				.setFlags(Constants::DEFAULT_DESCRIPTOR_POOL_CREATE_FLAGS)
				.setPoolSizes(pool_sizes);

			auto num_frames = c.GetNumFrames();
			for (auto i = 0; i < num_frames; i++)
			{
				auto sbt_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, 1, true));
				sbt_buffers.emplace_back(sbt_buffer);

				auto dp = AssignOrPanic(c.CreateDescriptorPool(descriptor_pool_create_info));
				global_descriptor_pools.emplace_back(dp);

				const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];

				/*
				{
					auto variable_bindings = global_descriptor_set_layouts.GetVariableBindings();

					std::vector<std::optional<uint32_t>> descriptor_sets_counts(vulkan_descriptor_set_layouts.size());
					for (const auto& [set, variable_binding] : variable_bindings)
					{
						descriptor_sets_counts[set] = variable_binding.num_bindings;
					}

					std::vector<vk::DescriptorSet> vulkan_descriptor_sets(vulkan_descriptor_set_layouts.size());
					for (auto i = 0; i < vulkan_descriptor_sets.size(); i++)
					{
						bool has_variable_binding = descriptor_sets_counts[i].has_value();

						uint32_t descriptor_set_count{};
						if (has_variable_binding)
						{
							descriptor_set_count = *descriptor_sets_counts[i];
						}

						auto descriptor_set_variable_descriptor_count_allocate_info = vk::DescriptorSetVariableDescriptorCountAllocateInfo{}
							.setDescriptorCounts(descriptor_set_count);

						auto info = vk::DescriptorSetAllocateInfo{}
							.setDescriptorPool(*dp)
							.setSetLayouts(vulkan_descriptor_set_layouts[i]);

						if (has_variable_binding)
						{
							info.setPNext(&descriptor_set_variable_descriptor_count_allocate_info);
						}

						AssignOrPanicVkError(auto vulkan_descriptor_set, c.GetDevice().allocateDescriptorSets(info));
						vulkan_descriptor_sets[i] = vulkan_descriptor_set[0];
					}

					auto descriptor_set = std::make_shared<DescriptorSet>(vulkan_descriptor_sets, DescriptorSetCreateInfo{});
					//PanicIfError(descriptor_set->InitializeWriteDescriptorSets());

					global_descriptor_sets.emplace_back(descriptor_set);
				}
				*/
			}
			sbt_shader_hashes.resize(num_frames);
			tlases.resize(num_frames);

			auto properties = c.GetPhysicalDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR, vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();
			raytracing_properties = properties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

			{
				common_resources = std::make_shared<Pass::CommonResources>();
				common_resources->global_descriptor_pools = global_descriptor_pools;
				common_resources->pipeline_manager = this;
				PanicIfError(common_resources->Setup());

				//common_resources->g_buffer_pass = &g_buffer_pass;

				//PanicIfError(g_buffer_pass.Setup(common_resources));
				PanicIfError(blue_noise.Setup(common_resources));
				PanicIfError(ray_traced_shadows_pass.Setup(common_resources));
				PanicIfError(ray_traced_ao_pass.Setup(common_resources));
				PanicIfError(ddgi_pass.Setup(common_resources));
				PanicIfError(ray_traced_reflections_pass.Setup(common_resources));
				PanicIfError(deferred_shading_pass.Setup(common_resources));
				PanicIfError(temporal_aa_pass.Setup(common_resources));
				PanicIfError(tone_map_pass.Setup(common_resources));
				PanicIfError(sky_environment.Setup(common_resources));

				common_resources->blue_noise = &blue_noise;
				common_resources->ray_traced_shadows_pass = &ray_traced_shadows_pass;
				common_resources->ray_traced_ao_pass = &ray_traced_ao_pass;
				common_resources->ddgi_pass = &ddgi_pass;
				common_resources->ray_traced_reflections_pass = &ray_traced_reflections_pass;
				common_resources->deferred_shading_pass = &deferred_shading_pass;
				common_resources->temporal_aa_pass = &temporal_aa_pass;
				common_resources->tone_map_pass = &tone_map_pass;
				common_resources->sky_environment = &sky_environment;

				PanicIfError(render_frame.ResetCommandBufferPools());
				frame_command_buffer = AssignOrPanic(render_frame.GetCommandBuffer(QueueType::Graphics));

				BeginFrame();
			}

			if (GetModuleHandle("Dolphin.exe") || GetModuleHandle("DolphinD.exe"))
			{
				game_type = GameType::Dolphin;
				common_resources->environment_type = Pass::EnvironmentType::ENVIRONMENT_TYPE_PROCEDURAL_SKY;
				common_resources->current_visualization_type = Pass::VisualizationType::VISUALIZATION_TYPE_FINAL;
				//common_resources->current_visualization_type = Pass::VisualizationType::VISUALIZATION_TYPE_GROUND_TRUTH;


				common_resources->ray_traced_shadows_pass->SetBias(1.0f);

				common_resources->ray_traced_reflections_pass->SetRoughDDGIIntensity(2.0f);


				common_resources->ddgi.min_extents = vec3(5000.0, -1000.0, 5000.0);
				common_resources->ddgi.max_extents = vec3(10000.0, -500.0, 10000.0);
				glm::vec3 scene_length = common_resources->ddgi.max_extents - common_resources->ddgi.min_extents;

				auto longest_length = std::max(scene_length.x, scene_length.z);
				auto probe_distance = longest_length;
				common_resources->ddgi_pass->SetProbeDistance(probe_distance);
				PanicIfError(common_resources->ddgi_pass->Setup(common_resources));

				auto& light = common_resources->light;
				light.light_radius = 0.0f;
				light.light_intensity = 2.0f;
				light.light_direction = vec3(-1.1f, 0.579f, -0.823f);

				common_resources->procedural_sky_direction = vec3(0.214f, 0.586f, -0.782f);

				common_resources->ao.scale = Pass::RayTraceScale::RAY_TRACE_SCALE_HALF_RES;
				common_resources->ray_traced_ao_pass->EnableBilateralBlur(false);
				PanicIfError(common_resources->ray_traced_ao_pass->Setup(common_resources));

				common_resources->temporal_aa_pass->SetFeedbackMin(0.25f);
				common_resources->temporal_aa_pass->SetFeedbackMax(0.95f);
				common_resources->temporal_aa_pass->Enable(false);

				deferred_shading_pass.SetUseRayTracedAO(false);
				deferred_shading_pass.SetUseRayTracedReflections(false);

				auto editor_dolphin_window = FindWindowA("Qt5150QWindowIcon", "Dolphin 5.0-14304-dirty");
				auto dolphin_game_window = FindWindowA("Qt5150QWindowIcon", "Dolphin");
				auto raysterizer_game_window = FindWindowA(nullptr, "Raysterizer");

				RECT editor_dolphin_window_rect;
				RECT dolphin_game_window_rect;
				RECT raysterizer_game_window_rect;

				GetWindowRect(editor_dolphin_window, &editor_dolphin_window_rect);
				GetWindowRect(dolphin_game_window, &dolphin_game_window_rect);
				GetWindowRect(raysterizer_game_window, &raysterizer_game_window_rect);

				SetWindowPos(editor_dolphin_window, nullptr, 1920, 800, editor_dolphin_window_rect.right - editor_dolphin_window_rect.left, editor_dolphin_window_rect.bottom - editor_dolphin_window_rect.top, (UINT)HWND_TOP);
				SetWindowPos(dolphin_game_window, nullptr, 1920, 0, dolphin_game_window_rect.right - dolphin_game_window_rect.left, dolphin_game_window_rect.bottom - dolphin_game_window_rect.top, (UINT)HWND_TOP);
				SetWindowPos(raysterizer_game_window, nullptr, 0, 0, raysterizer_game_window_rect.right - raysterizer_game_window_rect.left, raysterizer_game_window_rect.bottom - raysterizer_game_window_rect.top, (UINT)HWND_TOP);
			}
			if (GetModuleHandle("java.exe"))
			{
				game_type = GameType::OSRS;
				common_resources->environment_type = Pass::EnvironmentType::ENVIRONMENT_TYPE_PROCEDURAL_SKY;
				common_resources->current_visualization_type = Pass::VisualizationType::VISUALIZATION_TYPE_FINAL;


				common_resources->ray_traced_shadows_pass->SetBias(1.0f);

				common_resources->ray_traced_reflections_pass->SetRoughDDGIIntensity(2.0f);


				common_resources->ddgi.min_extents = vec3(5000.0, -1000.0, 5000.0);
				common_resources->ddgi.max_extents = vec3(10000.0, -500.0, 10000.0);
				glm::vec3 scene_length = common_resources->ddgi.max_extents - common_resources->ddgi.min_extents;

				auto longest_length = std::max(scene_length.x, scene_length.z);
				auto probe_distance = longest_length;
				common_resources->ddgi_pass->SetProbeDistance(probe_distance);
				PanicIfError(common_resources->ddgi_pass->Setup(common_resources));

				auto& light = common_resources->light;
				light.light_radius = 0.0f;
				light.light_intensity = 2.0f;
				light.light_direction = vec3(0.376f, 0.579f, -0.723f);

				common_resources->ao.scale = Pass::RayTraceScale::RAY_TRACE_SCALE_HALF_RES;
				common_resources->ray_traced_ao_pass->EnableBilateralBlur(false);
				PanicIfError(common_resources->ray_traced_ao_pass->Setup(common_resources));

				common_resources->temporal_aa_pass->SetFeedbackMin(0.25f);
				common_resources->temporal_aa_pass->SetFeedbackMax(0.95f);
				common_resources->temporal_aa_pass->Enable(false);
			}
			if (GetModuleHandle("RobloxStudioBeta.exe"))
			{
				game_type = GameType::Roblox;
				common_resources->environment_type = Pass::EnvironmentType::ENVIRONMENT_TYPE_NONE;
				common_resources->current_visualization_type = Pass::VisualizationType::VISUALIZATION_TYPE_GROUND_TRUTH;
			}
			if (GetModuleHandle("PPSSPPDebug64.exe"))
			{
				game_type = GameType::PPSSPP;
				common_resources->environment_type = Pass::EnvironmentType::ENVIRONMENT_TYPE_NONE;
				common_resources->current_visualization_type = Pass::VisualizationType::VISUALIZATION_TYPE_GROUND_TRUTH;
			}
		}

		flat_hash_map<GLuint, std::shared_ptr<DrawCalls>>& PipelineManager::GetCurrentProgramToDrawCalls()
		{
			return program_to_draw_calls;
		}

		const flat_hash_map<GLuint, std::shared_ptr<DrawCalls>>& PipelineManager::GetCurrentProgramToDrawCalls() const
		{
			return program_to_draw_calls;
		}

		Error PipelineManager::InsertProgramToDrawCall(GLuint program_id, Raysterizer::OpenGL::VertexShader& vertex_shader, Raysterizer::OpenGL::FragmentShader& fragment_shader)
		{
			auto& program_to_draw_calls = GetCurrentProgramToDrawCalls();

			auto draw_calls_attached_to_shader_source = std::make_shared<DrawCalls>();
			ReturnIfError(draw_calls_attached_to_shader_source->Init(program_id, &vertex_shader, &fragment_shader, this));
			auto [_, success] = program_to_draw_calls.try_emplace(program_id, std::move(draw_calls_attached_to_shader_source));
			if (!success)
			{
				return StringError("Program to draw call already inserted");
			}
			return NoError();
		}

		Error PipelineManager::RemoveProgramToDrawCall(GLuint program_id)
		{
			auto& program_to_draw_calls = GetCurrentProgramToDrawCalls();

			auto success = program_to_draw_calls.erase(program_id);
			if (!success)
			{
				return StringError("Program to draw call [{}] cannot be removed!", program_id);
			}
			program_to_instance_data_offsets.erase(program_id);
			return NoError();
		}

		Expected<DrawCalls&> PipelineManager::GetProgramToDrawCalls(GLuint program)
		{
			auto& program_to_draw_calls = GetCurrentProgramToDrawCalls();

			if (auto found = program_to_draw_calls.find(program); found != std::end(program_to_draw_calls))
			{
				auto& [_, draw_calls] = *found;
				return *draw_calls;
			}
			else
			{
				return StringError("Vulkan pipeline not found");
			}
		}

		void PipelineManager::BeginFrame()
		{
			if (skip_frame)
			{
				skip_frame = false;
				return;
			}

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			{
				ScopedCPUProfileRaysterizer("Beginning of Frame Updates");
				PanicIfError(render_frame.PerformBeginningOfFrameUpdates());
			}

			present_semaphore = AssignOrPanic(render_frame.GetBinarySemaphore());
			c.SetName(present_semaphore, fmt::format("Present semaphore frame {}", current_frame_index));

			{
				ScopedCPUProfileRaysterizer("Prepare Frame");
				if (auto err = c.PrepareFrame(present_semaphore))
				{
					if (err.isA<SwapchainOutOfDateError>())
					{
						ConsumeError(err);
						return;
					}
					else
					{
						PanicError(err);
					}
				}
			}

			PanicIfError(render_frame.ResetCommandBufferPools());

			render_semaphore = AssignOrPanic(render_frame.GetBinarySemaphore());
			render_fence = AssignOrPanic(render_frame.GetFence());
			frame_command_buffer = AssignOrPanic(render_frame.GetCommandBuffer(QueueType::Graphics));
			
			c.SetName(render_semaphore, fmt::format("Render semaphore frame {}", current_frame_index));
			c.SetName(render_fence, fmt::format("Render fence frame {}", current_frame_index));
			c.SetName(frame_command_buffer, fmt::format("Command buffer frame {}", current_frame_index));
		}

		void PipelineManager::BuildResourcesForRaytracing()
		{
			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			{
				ScopedCPUProfileRaysterizer("ImGui Draw calls");
				
				if (ImGui::Begin("TLAS"))
				{
					auto& context = Raysterizer::OpenGL::Context::Get();
					auto& state = context.state;
					auto perform_perspective_correction = state.GetPerformPerspectiveCorrection();
					if (ImGui::Checkbox("Perform Perspective Correction", &perform_perspective_correction))
					{
						context.state.SetPerformPerspectiveCorrection(perform_perspective_correction);
					}
				
					auto& raysterizer_info = state.GetRaysterizerInfo();
					ImGui::InputFloat("Z Near", &raysterizer_info.z_near, -1.0f, 1.0f);
					ImGui::InputFloat("Z Far", &raysterizer_info.z_far, -1.0f, 1000.0f);
					ImGui::InputInt("Num samples", (int*)&raysterizer_info.number_of_samples);
					ImGui::InputInt("Num bounces", (int*)&raysterizer_info.number_of_bounces);
				
					static auto prev_time = std::chrono::high_resolution_clock::now();
					auto time_step_diff = 200;
					auto increment_amount = 0.05f;
					auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - prev_time).count();
				
					if (diff > time_step_diff)
					{
						if (GetAsyncKeyState(VK_LEFT) & 0x8000)
						{
							raysterizer_info.z_near -= increment_amount;
						}
						else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
						{
							raysterizer_info.z_near += increment_amount;
						}
						else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
						{
							raysterizer_info.z_far -= increment_amount;
						}
						else if (GetAsyncKeyState(VK_UP) & 0x8000)
						{
							raysterizer_info.z_far += increment_amount;
						}
						else if (GetAsyncKeyState(VK_SPACE) & 0x8000)
						{
							context.state.SetPerformPerspectiveCorrection(!perform_perspective_correction);
						}
					}
				
					auto ToggleButton = [](const char* str_id, bool* v)
					{
						ImVec4* colors = ImGui::GetStyle().Colors;
						ImVec2 p = ImGui::GetCursorScreenPos();
						ImDrawList* draw_list = ImGui::GetWindowDrawList();
				
						float height = ImGui::GetFrameHeight();
						float width = height * 1.55f;
						float radius = height * 0.50f;
				
						ImGui::InvisibleButton(str_id, ImVec2(width, height));
						bool changed_v = false;
						if (ImGui::IsItemClicked())
						{
							changed_v = true;
							*v = !*v;
						}
						ImGuiContext& imgui_context = *GImGui;
						float ANIM_SPEED = 0.085f;
						if (imgui_context.LastActiveId == imgui_context.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
							float t_anim = ImSaturate(imgui_context.LastActiveIdTimer / ANIM_SPEED);
						if (ImGui::IsItemHovered())
							draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
						else
							draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
						draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
						return changed_v;
					};
				
					static const fs::path material_info_path = Config["raysterizer"]["materials"]["path"];
					static const glm::vec4 default_material_color = ConvertFromArray(Config["raysterizer"]["materials"]["default_color"]);
				
					// Initialize material first
					CallOnce
					{
						if (!fs::exists(material_info_path))
						{
							auto material_info_path_base = material_info_path;
							material_info_path_base.remove_filename();
							fs::create_directories(material_info_path_base);
						}
				
						std::ifstream f(material_info_path);
						if (f)
						{
							json j;
							f >> j;
				
							for (const auto& [k, v] : j.items())
							{
								auto hash = std::strtoumax(k.c_str(), nullptr, 10);
								auto& allocated_material_info = GetMaterialInfoWithIndex(hash);
								allocated_material_info.m = MaterialInfo{ j };
							}
						}
					};
				
					if (ImGui::Button("Save settings to disk"))
					{
						{
							json j;
							for (auto& [hash, material_info_with_index] : material_info_mapping)
							{
								auto hash_string = fmt::format("{}", hash);
								material_info_with_index.m.material = materials_buffer->MapAs<Material*>()[material_info_with_index.index];
								j[hash_string] = material_info_with_index.m;
							}
				
							std::ofstream f(material_info_path, std::ios::trunc);
							if (f)
							{
								f << std::setw(4) << j << std::endl;
							}
						}
						{
							const auto& j = Config.GetJson();
				
							auto config_path = Constants::CONFIG_PATH;
							std::ofstream f(config_path.data());
							if (f)
							{
								f << std::setw(4) << j << std::endl;
							}
						}
					}
				
					phmap::flat_hash_set<std::size_t> already_inserted_hashes;
					for (const auto& [program_id, draw_calls] : program_to_draw_calls)
					{
						auto draw_call_count = draw_calls->GetDrawCount();
						if (draw_call_count == 0)
						{
							continue;
						}
						auto count_to_draw = draw_calls->GetCountToDraw();
				
						const auto& vertex_buffer_pointer_views = draw_calls->GetVertexBufferPointerViews();
						const auto& index_buffer_pointer_views = draw_calls->GetIndexBufferPointerViews();
						const auto& vertex_buffers = draw_calls->GetVertexBuffers();
						const auto& index_buffers = draw_calls->GetIndexBuffers();
				
						for (auto i = 0; i < vertex_buffer_pointer_views.size(); i++)
						{
							const auto& vertex_buffer_pointer_view = vertex_buffer_pointer_views[i];
				
							auto hash = program_id;
				
							if (already_inserted_hashes.contains(hash))
							{
								continue;
							}
							else
							{
								already_inserted_hashes.emplace(hash);
							}
				
							ImGui::PushID(hash);
				
							auto& material_info_with_index = GetMaterialInfoWithIndex(hash);
							auto& material_info = material_info_with_index.m;
							auto& material = materials_buffer->MapAs<Material*>()[material_info_with_index.index];
				
							ToggleButton("Highlight", &material.highlight);
							ImGui::SameLine();
				
							if (ImGui::TreeNode((void*)i, "%s", material_info.name.c_str()))
							{
								//static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
								//ImGui::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnArrow", &base_flags, ImGuiTreeNodeFlags_OpenOnArrow);

								ImGui::Text("Hash 0x%X", hash);
								ImGui::InputText("Name", &material_info.name);
								ImGui::ColorEdit4("Highlight color", glm::value_ptr(material.highlight_color));
				
								ImGui::Text("Material");
				
								ImGui::ColorEdit4("diffuse", glm::value_ptr(material.albedo));
								ImGui::ColorEdit4("emissive", glm::value_ptr(material.emissive));
								ImGui::SliderFloat("roughness", &material.roughness_metallic.r, 0.0f, 1.0f);
								ImGui::SliderFloat("metallic", &material.roughness_metallic.g, 0.0f, 1.0f);
								//dirty_buffer |= ImGui::SliderFloat("material_model", &material.material_model, 0.0f, 1.0f);
				
								/*
								const uint MaterialLambertian = 0;
								const uint MaterialMetallic = 1;
								const uint MaterialDielectric = 2;
								const uint MaterialIsotropic = 3;
								const uint MaterialDiffuseLight = 4;
				
								dirty_buffer |= ImGui::RadioButton("Lambertian", reinterpret_cast<int*>(&material.material_model), 0); ImGui::SameLine();
								dirty_buffer |= ImGui::RadioButton("Metallic", reinterpret_cast<int*>(&material.material_model), 1); ImGui::SameLine();
								dirty_buffer |= ImGui::RadioButton("Dielectric", reinterpret_cast<int*>(&material.material_model), 2); ImGui::SameLine();
								dirty_buffer |= ImGui::RadioButton("Isotropic", reinterpret_cast<int*>(&material.material_model), 3); ImGui::SameLine();
								dirty_buffer |= ImGui::RadioButton("DiffuseLight", reinterpret_cast<int*>(&material.material_model), 4);
								*/
				
								ImGui::TreePop();
							}
				
							ImGui::PopID();
						}
					}
				}
				ImGui::End();
			}

			const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];
			static uint32_t recursion_depth = Config["raysterizer"]["vulkan"]["recursion_depth"];

			const static FrameCounter pipeline_release_entries_past_frame_counter_difference = Config["raysterizer"]["pipeline_release_entries_past_frame_counter_difference"];
			blas_cache.ClearEntriesPastFrameCounterDifference(pipeline_release_entries_past_frame_counter_difference);

			{
				ScopedCPUProfileRaysterizer("Transformation computation");
				
				auto& context = Raysterizer::OpenGL::Context::Get();
				auto& state = context.state;
				auto& raysterizer_info = state.GetRaysterizerInfo();
				
				raysterizer_info.projection_view = glm::mat4(1.0f);
				bool changed = false;
				for (auto& [_, draw_calls] : program_to_draw_calls)
				{
					auto& transformation_results = draw_calls->GetTransformationResults();
				
					static bool run_async = Config["raysterizer"]["mvp_analysis"]["run_async"];
					if (run_async)
					{
						draw_calls->WaitForExecution();
					}
				
					if (!transformation_results.empty())
					{
						const auto& transformation_result = transformation_results[0];
						if (!transformation_result.empty())
						{
							const auto& result = transformation_result[0];
							raysterizer_info.projection_view = result.projection_view;
							changed = true;
							break;
						}
				
						for (const auto& transformation_result : transformation_results)
						{
							for (const auto& result : transformation_result)
							{
								if (raysterizer_info.projection_view != result.projection_view && changed)
								{
									//PANIC("Projection view not matching {} {}", glm::to_string(raysterizer_info.projection_view_matrix), glm::to_string(result.projection_view));
								}
				
								if (result.projection_view[0][0] != 1.0f)
								{
									raysterizer_info.projection_view = result.projection_view;
								}
								changed = true;
							}
						}
					}

					if (0 && game_type == GameType::Dolphin)
					{
						struct VertexShaderConstants
						{
							u32 components;           // .x
							u32 xfmem_dualTexInfo;    // .y
							u32 xfmem_numColorChans;  // .z
							u32 missing_color_hex;    // .w, used for change detection but not directly by shaders
							float4 missing_color_value;

							std::array<float4, 6> posnormalmatrix;
							std::array<float4, 4> projection;
							std::array<int4, 4> materials;
							struct Light
							{
								int4 color;
								float4 cosatt;
								float4 distatt;
								float4 pos;
								float4 dir;
							};
							std::array<Light, 8> lights;
							std::array<float4, 24> texmatrices;
							std::array<float4, 64> transformmatrices;
							std::array<float4, 32> normalmatrices;
							std::array<float4, 64> posttransformmatrices;
							float4 pixelcentercorrection;
							std::array<float, 2> viewport;  // .xy
							std::array<float, 2> pad2;      // .zw

							// .x - texMtxInfo, .y - postMtxInfo, [0..1].z = color, [0..1].w = alpha
							std::array<glm::uvec4, 8> xfmem_pack1;

							float4 cached_tangent;
							float4 cached_binormal;
							// For UberShader vertex loader
							u32 vertex_stride;
							std::array<u32, 3> vertex_offset_normals;
							u32 vertex_offset_position;
							u32 vertex_offset_posmtx;
							std::array<u32, 2> vertex_offset_colors;
							std::array<u32, 8> vertex_offset_texcoords;
						};

						for (const auto& [program_id, draw_calls] : program_to_draw_calls)
						{
							if (auto vs_block_or_err = draw_calls->GetVariableInVulkanRef("VSBlock"))
							{
								auto& vs_block = *vs_block_or_err;

								auto vs_block_mapping = vs_block->MapAs<VertexShaderConstants*>();
								auto& projection = vs_block_mapping->projection;
								raysterizer_info.projection_view = glm::make_mat4(&projection[0][0]);
								break;
							}
							else
							{
								ConsumeError(vs_block_or_err.takeError());
							}
						}
					}
				}
				raysterizer_info.projection_view_inverse = glm::inverse(raysterizer_info.projection_view);
				
				{
					ScopedCPUProfileRaysterizer("Copy raysterizer info buffer");
					PanicIfError(raysterizer_info_buffer->Copy(PointerView(raysterizer_info)));
				}
			}
			
			instance_data_pool.Clear();
			instance_data_pool.SetName("Instance Data Buffer");
			mesh_data_pool.Clear();
			out_color_buffers.clear();

			auto& tlas = tlases[current_frame_index];
			{
				ScopedCPUProfileRaysterizer("Acceleration structure building");
				
				constexpr vk::BuildAccelerationStructureFlagsKHR build_flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
				constexpr vk::GeometryInstanceFlagsKHR geometry_instance_flags = vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable;
				
				TopLevelAccelerationStructureCreateInfo tlas_create_info;
				tlas_create_info.build_flags = build_flags;
				auto& tlases_blases = tlas_create_info.blases;
				auto& acceleration_structure_instances = tlas_create_info.instances;
				acceleration_structure_instances.clear();
				
				if (tlas)
				{
					tlas_create_info.existing_instance_buffer = tlas->instance_buffer;
				}
				
				uint32_t current_instance_custom_index = 0;
				for (const auto& [program_id, draw_calls] : program_to_draw_calls)
				{
					auto& draw_call_states_buffer = draw_calls->GetDrawCallStates();
				
					auto draw_call_count = draw_calls->GetDrawCount();
					if (draw_call_count == 0)
					{
						continue;
					}
					auto count_to_draw = draw_calls->GetCountToDraw();
				
					auto vertex_buffer_stride = static_cast<uint32_t>(draw_calls->GetVertexBufferStride());
					auto index_buffer_stride = static_cast<uint32_t>(draw_calls->GetIndexBufferStride());
					if (vertex_buffer_stride == 0)
					{
						vertex_buffer_stride = 1;
					}
				
					vk::IndexType index_type{};
					if (index_buffer_stride == 0)
					{
						index_type = vk::IndexType::eNoneKHR;
					}
					else if (index_buffer_stride == 1)
					{
						PANIC("Index type cannot be uint8");
						index_type = vk::IndexType::eUint8EXT;
					}
					else if (index_buffer_stride == 2)
					{
						index_type = vk::IndexType::eUint16;
					}
					else if (index_buffer_stride == 4)
					{
						index_type = vk::IndexType::eUint32;
					}
					else
					{
						PANIC("index buffer stride", index_buffer_stride);
					}
				
					const auto& vertex_buffer_pointer_views = draw_calls->GetVertexBufferPointerViews();
					const auto& index_buffer_pointer_views = draw_calls->GetIndexBufferPointerViews();
					const auto& vertex_buffers = draw_calls->GetVertexBuffers();
					const auto& index_buffers = draw_calls->GetIndexBuffers();

					auto& vertex_analyzer = draw_calls->GetVertexAnalyzer();
					const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();

					InstanceDataOffsets instance_data_offsets;
					if (auto found = program_to_instance_data_offsets.find(program_id); found != std::end(program_to_instance_data_offsets))
					{
						instance_data_offsets = found->second;
					}
					else
					{
						for (const auto& [name, pipeline_input] : vertex_pipeline_inputs)
						{
							auto offset = pipeline_input.GetOffset();
							const auto* glsl_type = pipeline_input.GetGLSLType();

							auto ConvertToTypeFromGLSLType = [&]()
							{
								auto basic_type = glsl_type->getBasicType();
								auto num_elements = glsl_type->getVectorSize();
								switch (basic_type)
								{
								case glslang::TBasicType::EbtInt:
								{
									switch (num_elements)
									{
									case 1:
										return RAYSTERIZER_DATA_TYPE_INT;
									case 2:
										return RAYSTERIZER_DATA_TYPE_IVEC2;
									case 3:
										return RAYSTERIZER_DATA_TYPE_IVEC3;
									case 4:
										return RAYSTERIZER_DATA_TYPE_IVEC4;
									}
									break;
								}
								case glslang::TBasicType::EbtUint:
								{
									switch (num_elements)
									{
									case 1:
										return RAYSTERIZER_DATA_TYPE_UINT;
									case 2:
										return RAYSTERIZER_DATA_TYPE_UVEC2;
									case 3:
										return RAYSTERIZER_DATA_TYPE_UVEC3;
									case 4:
										return RAYSTERIZER_DATA_TYPE_UVEC4;
									}
									break;
								}
								case glslang::TBasicType::EbtFloat:
								{
									switch (num_elements)
									{
									case 1:
										return RAYSTERIZER_DATA_TYPE_FLOAT;
									case 2:
										return RAYSTERIZER_DATA_TYPE_VEC2;
									case 3:
										return RAYSTERIZER_DATA_TYPE_VEC3;
									case 4:
										return RAYSTERIZER_DATA_TYPE_VEC4;
									}
									break;
								}
								}
								return RAYSTERIZER_DATA_TYPE_UNKNOWN;
							};

							static const std::regex position_search = Config["shader"]["converter"]["position_search"];
							static const std::regex normal_search = Config["shader"]["converter"]["normal_search"];
							static const std::regex coord_tex_search = Config["shader"]["converter"]["coord_tex_search"];

							if (std::regex_search(name, position_search))
							{
								instance_data_offsets.position_offset = offset;
								instance_data_offsets.position_data_type = ConvertToTypeFromGLSLType();
							}
							else if (std::regex_search(name, normal_search))
							{
								instance_data_offsets.normal_offset = offset;
								instance_data_offsets.normal_data_type = ConvertToTypeFromGLSLType();
							}
							else if (std::regex_search(name, coord_tex_search))
							{
								instance_data_offsets.tex_coord_offset = offset;
								instance_data_offsets.tex_coord_data_type = ConvertToTypeFromGLSLType();
							}
						}
						program_to_instance_data_offsets.try_emplace(program_id, instance_data_offsets);
					}

					uint32_t instance_shader_binding_offset = draw_call_to_set_starting_index;//draw_calls->GetShaderBindingIndex() - draw_call_to_set_starting_index;
				
					const auto& transformation_results = draw_calls->GetTransformationResults();
				
					for (auto i = 0; i < draw_call_count; i++)
					{
						auto num_elements = static_cast<uint32_t>(count_to_draw[i]);
				
						const auto& vertex_buffer_pointer_view = vertex_buffer_pointer_views[i];
						const auto& index_buffer_pointer_view = index_buffer_pointer_views[i];
						const auto& vertex_buffer = vertex_buffers[i];
						const auto& index_buffer = index_buffers[i];
				
						if (vertex_buffer_pointer_view.GetStride() != vertex_buffer_stride)
						{
							//PANIC("Vertex strides not equal {} != {}", vertex_buffer_pointer_view.GetStride(), vertex_buffer_stride);
						}
				
						auto vertex_buffer_hash = vertex_buffer_pointer_view.Hash();
						auto index_buffer_hash = index_buffer_pointer_view.Hash();
						auto vertex_index_buffers_hash = vertex_buffer_hash;
						HashCombine(vertex_index_buffers_hash, index_buffer_hash);

						CMShared<BottomLevelAccelerationStructure> blas{};
						if (auto found_or_err = blas_cache.Get(vertex_buffer_hash))
						{
							ScopedCPUProfileRaysterizer("BLAS acceleration structure cached");
							blas = *found_or_err;
						}
						else
						{
							ConsumeError(found_or_err.takeError());
				
							ScopedCPUProfileRaysterizer("BLAS acceleration structure building");
				
							auto geometry_vertex_buffer = vertex_buffer;
							auto geometry_index_buffer = index_buffer;
				
#define MAYBE_COPY_BUFFER_FOR_BLAS 0
							if (MAYBE_COPY_BUFFER_FOR_BLAS)
							{
								geometry_vertex_buffer = AssignOrPanic(render_frame.CopyBuffer(vertex_buffer));
								geometry_index_buffer = AssignOrPanic(render_frame.CopyBuffer(index_buffer));
							}
				
							auto vertex_format = draw_calls->GetVertexFormat();
							if (vertex_format == vk::Format::eR32G32B32A32Sint)
							{
								//vertex_format = vk::Format::eR32G32B32Sint;
							}
							//auto vertex_format = vk::Format::eR32G32B32Sfloat;
							GeometryDescription geometry_description
							{
								GeometryVertexBuffer{geometry_vertex_buffer, num_elements, vertex_buffer_stride, vertex_format},
								GeometryIndexBuffer{geometry_index_buffer, num_elements, index_buffer_stride, index_type},
								vk::GeometryFlagsKHR{}
							};
							std::vector<GeometryDescription> geometry_descriptions{ geometry_description };
							vk::BuildAccelerationStructureFlagsKHR build_flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction | vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
				
							BottomLevelAccelerationStructureCreateInfo blas_create_info{ geometry_descriptions, build_flags };
							auto blas_transfer_job = AssignOrPanic(c.CreateBottomLevelAccelerationStructure(blas_create_info));
							graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, blas_transfer_job.semaphore);
							blas = blas_transfer_job.blas;
				
							auto blas_compact_transfer_job = AssignOrPanic(c.CompactBottomLevelAccelerationStructure(blas_transfer_job));
							PanicIfError(c.CleanupCompactBottomLevelAccelerationStructure(blas_compact_transfer_job));
							graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, blas_compact_transfer_job.semaphore);
							graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, blas_compact_transfer_job.blas_compact_semaphore);
							blas = blas_compact_transfer_job.blas;
				
							// Remove dependency on vertex/index buffer, so they can be deallocated
							//blas->create_info.geometry_descriptions.clear();
				
							blas_cache.Emplace(vertex_buffer_hash, blas);
						}
				
						c.SetName(blas, fmt::format("blas frame {}", current_frame_index));
				
						frame_command_buffer->AddDependencyTo(blas);
						tlases_blases.emplace_back(blas);
				
						ScopedCPUProfileRaysterizer("BLAS acceleration structure instance creation");
				
						if (!transformation_results.empty())
						{
							const auto& transformation_result = transformation_results[i];
							for (const auto& transformation : transformation_result)
							{
								auto& transform = transformation.model;
								vk::TransformMatrixKHR transform_matrix
								(
									std::array<std::array<float, 4>, 3>
								{
									std::array<float, 4>
									{
										transform[0][0], transform[1][0], transform[2][0], transform[3][0]
									},
										std::array<float, 4>
										{
											transform[0][1], transform[1][1], transform[2][1], transform[3][1]
										},
											std::array<float, 4>
											{
												transform[0][2], transform[1][2], transform[2][2], transform[3][2]
											}
								}
								);

								auto acceleration_structure_instance = vk::AccelerationStructureInstanceKHR{}
									.setTransform(transform_matrix)
									.setInstanceCustomIndex(current_instance_custom_index)
									.setInstanceShaderBindingTableRecordOffset(instance_shader_binding_offset)
									.setFlags(geometry_instance_flags)
									.setMask(0xFF)
									.setAccelerationStructureReference(blas->GetAccelerationStructureAddress());

								acceleration_structure_instances.emplace_back(std::move(acceleration_structure_instance));

								auto hash = program_id;

								auto& material_info_with_index = GetMaterialInfoWithIndex(hash);
								InstanceData& instance_data = instance_data_pool.Get(current_instance_custom_index);
								MeshData& mesh_data = mesh_data_pool.Get(current_instance_custom_index);

								instance_data.material_index = material_info_with_index.index;
								instance_data.vertices = vertex_buffer->GetAddress();
								instance_data.indices = index_buffer->GetAddress();
								instance_data.vertex_stride = vertex_buffer_stride;
								instance_data.index_stride = index_buffer_stride;
								instance_data.position_offset = -1;
								instance_data.normal_offset = -1;
								instance_data.tex_coord_offset = -1;
								instance_data.position_data_type = RAYSTERIZER_DATA_TYPE_UNKNOWN;
								instance_data.normal_data_type = RAYSTERIZER_DATA_TYPE_UNKNOWN;
								instance_data.tex_coord_data_type = RAYSTERIZER_DATA_TYPE_UNKNOWN;

								instance_data.out_color_buffer_index = current_instance_custom_index;
								auto& out_color_buffers_for_draw_call = draw_calls->GetOutColorBuffers();
								out_color_buffers.emplace_back(out_color_buffers_for_draw_call[i]);

								instance_data.position_offset = instance_data_offsets.position_offset;
								instance_data.position_data_type = instance_data_offsets.position_data_type;

								instance_data.normal_offset = instance_data_offsets.normal_offset;
								instance_data.normal_data_type = instance_data_offsets.normal_data_type;

								instance_data.tex_coord_offset = instance_data_offsets.tex_coord_offset;
								instance_data.tex_coord_data_type = instance_data_offsets.tex_coord_data_type;

								current_instance_custom_index++;
							}
						}
					}
				}
				
				{
					ScopedCPUProfileRaysterizer("TLAS acceleration structure building");
				
					auto tlas_transfer_job = AssignOrPanic(c.CreateTopLevelAccelerationStructure(tlas_create_info));
					tlas = tlas_transfer_job.tlas;
					c.SetName(tlas, fmt::format("tlas frame {}", current_frame_index));

					graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, tlas_transfer_job.semaphore);

					frame_command_buffer->AddDependencyTo(tlas);
				}
			}
				
			frame_command_buffer->AddDependencyTo(materials_buffer);
		}

		void PipelineManager::Draw()
		{
			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			transfer_qbm = QueueBatchManager(c);
			compute_qbm = QueueBatchManager(c);
			graphics_qbm = QueueBatchManager(c);

			// remove any g buffer passes
			for (auto iter = std::begin(to_be_deleted_g_buffer_pass); iter != std::begin(to_be_deleted_g_buffer_pass);)
			{
				const auto& g_buffer_pass = *iter;
				if (!g_buffer_pass->lasted_used_command_buffer->IsInUse())
				{
					iter = to_be_deleted_g_buffer_pass.erase(iter);
				}
				else
				{
					iter++;
				}
			}

			for (const auto& [id, pass] : id_to_g_buffer_pass)
			{
				pass->Finialize(frame_command_buffer);
			}

			auto& g_buffer_pass = common_resources->g_buffer_pass;
			if (!render_call_records.empty())
			{
				g_buffer_pass = render_call_records[render_call_records.size() - 1].g_buffer_pass;
			}
			else
			{
				g_buffer_pass = nullptr;
			}
			bool valid_render_call = false;
			for (const auto& render_call_record : render_call_records)
			{
				switch (GetGameType())
				{
				case GameType::Dolphin:
				{
					// Over 200 for smash
					if (render_call_record.count > 3)
					{
						g_buffer_pass = render_call_record.g_buffer_pass;
						valid_render_call = true;
						/*
						for (const auto& r : render_call_records)
						{
							if (r.count == 186)
							{
								g_buffer_pass = r.g_buffer_pass;
								break;
							}
						}
						*/
						//valid_render_call = true;
					}
					break;
				}
				case GameType::OSRS:
				{
					if (render_call_record.count > 4)
					{
						g_buffer_pass = render_call_record.g_buffer_pass;
						valid_render_call = true;
					}
					break;
				}
				default:
				{
					break;
				}
				}
				if (valid_render_call)
				{
					break;
				}
				/*
				if (g_buffer_pass->pipeline_layout)
				{
					if (auto e = std::get_if<ShaderModuleSourceCreateInfo>(&g_buffer_pass->pipeline_layout->pipeline_layout_info->shader_modules[1]->shader_module_create_info.source))
					{
						const auto& fragment_main_code = e->source;
						if (fragment_main_code.find("samp") != std::string::npos && fragment_main_code.find("indtevtrans3") != std::string::npos)
						{
							common_resources->g_buffer_pass = g_buffer_pass;
							g_buffer_found = true;
							break;
						}
					}
				}
				*/
			}

			/*
			for (const auto& [id, pass] : id_to_g_buffer_pass)
			{
				if (pass != g_buffer_pass)
				{
					pass->Finialize(frame_command_buffer);
				}
			}

			if (valid_render_call && g_buffer_pass)
			{
				g_buffer_pass->Finialize(frame_command_buffer, true);
			}
			*/

			if (0 && !valid_render_call)
			{
				frame_command_buffer->Reset();

				for (auto& [_, draw_calls] : program_to_draw_calls)
				{
					auto& transformation_results = draw_calls->GetTransformationResults();

					static bool run_async = Config["raysterizer"]["mvp_analysis"]["run_async"];
					if (run_async)
					{
						draw_calls->WaitForExecution();
					}
				}

				PanicIfError(render_frame.EndFrame());
				PanicIfError(c.EndFrame());

				//PanicIfError(render_frame.PerformBeginningOfFrameUpdates());

				render_call_records.clear();
				skip_frame = true;
				return;
			}

			{
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGuizmo::BeginFrame();
			}

			auto image_subresource_range = vk::ImageSubresourceRange{}
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1);

			const vk::Image& swapchain_image = c.GetSwapchainImages()[current_frame_index];

			auto graphics_queue_family = c.GetGraphicsQueueFamily();
			auto present_queue_family = c.GetPresentQueueFamily();

			if (g_buffer_pass)
			{
				BuildResourcesForRaytracing();

				{
					auto& context = Raysterizer::OpenGL::Context::Get();
					auto& state = context.state;
					auto& raysterizer_info = state.GetRaysterizerInfo();

					double time = glfwGetTime() * 0.5f;

					auto& light = common_resources->light;
					if (light.light_animation)
					{
						if (light.light_type == Pass::LightType::LIGHT_TYPE_SPOT)
						{
							float t = sinf(light.light_animation_time) * 0.5f + 0.5f;

							glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
							glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::mix(glm::vec3(-8.25f, 7.5f, 6.0f), glm::vec3(0.25f, 7.5f, 6.0f), t));

							light.light_transform = T * R;
						}
						else if (light.light_type == Pass::LightType::LIGHT_TYPE_DIRECTIONAL)
						{
							light.light_direction.x = sinf(time);
							light.light_direction.z = cosf(time);
							light.light_direction.y = sinf(time) * 1.0f;
							light.light_direction = glm::normalize(light.light_direction);
						}
						light.light_animation_time += time;
					}

					//light.light_direction = glm::normalize(glm::mat3(light.light_transform) * glm::vec3(0.0f, -1.0f, 0.0f));
					light.light_position = glm::vec3(light.light_transform[3][0], light.light_transform[3][1], light.light_transform[3][2]);

					glm::mat4 current_jitter = glm::translate(glm::mat4(1.0f), glm::vec3(common_resources->temporal_aa_pass->CurrentJitter(), 0.0f));

					Pass::UBO ubo;

					ubo.cam_pos;
					ubo.view_inverse;
					ubo.proj_inverse;

					ubo.cam_pos = raysterizer_info.camera_position;
					ubo.view_proj = raysterizer_info.projection_view;
					ubo.view_proj_inverse = glm::inverse(raysterizer_info.projection_view);
					ubo.prev_view_proj = common_resources->first_frame ? common_resources->view.prev_view_projection : current_jitter * common_resources->view.prev_view_projection;
					ubo.current_prev_jitter = glm::vec4(common_resources->temporal_aa_pass->CurrentJitter(), common_resources->temporal_aa_pass->PrevJitter());

					ubo.near_far = glm::vec4(raysterizer_info.z_near, raysterizer_info.z_far, 0.0, 0.0);

					ubo.light.set_light_radius(light.light_radius);
					ubo.light.set_light_color(light.light_color);
					ubo.light.set_light_intensity(light.light_intensity);
					ubo.light.set_light_type(light.light_type);
					ubo.light.set_light_direction(-light.light_direction);
					ubo.light.set_light_position(light.light_position);
					ubo.light.set_light_cos_theta_inner(glm::cos(glm::radians(light.light_cone_angle_inner)));
					ubo.light.set_light_cos_theta_outer(glm::cos(glm::radians(light.light_cone_angle_outer)));

					common_resources->view.prev_view_projection = raysterizer_info.projection_view;

					PanicIfError(common_resources->GetUBOBuffer()->Copy(PointerView(ubo)));

					float z_buffer_params_x = -1.0 + (raysterizer_info.z_near / raysterizer_info.z_far);
					common_resources->ao.z_buffer_params = glm::vec4(z_buffer_params_x, 1.0f, z_buffer_params_x / raysterizer_info.z_near, 1.0f / raysterizer_info.z_near);
					common_resources->ao.z_buffer_params = glm::vec4(-1.0, 1.0, -1.0, 1.0);

					if (common_resources->environment_type == Pass::ENVIRONMENT_TYPE_PROCEDURAL_SKY)
					{
						common_resources->sky_environment->GetHosekWilkieSkyModel().Update(frame_command_buffer, common_resources->procedural_sky_direction);

						{
							ScopedGPUProfileRaysterizer(frame_command_buffer, "Generate Skybox Mipmap");
							PanicIfError(render_frame.GenerateMipMaps(frame_command_buffer, common_resources->sky_environment->GetHosekWilkieSkyModel().GetImage()));
						}

						common_resources->sky_environment->GetCubemapShProjection().Update(frame_command_buffer);
						common_resources->sky_environment->GetCubemapPrefilter().Update(frame_command_buffer);
					}
				}

				ray_traced_shadows_pass.Render(frame_command_buffer);
				ray_traced_ao_pass.Render(frame_command_buffer);
				ddgi_pass.Render(frame_command_buffer);
				ray_traced_reflections_pass.Render(frame_command_buffer);
				deferred_shading_pass.Render(frame_command_buffer);
				temporal_aa_pass.Render(frame_command_buffer);

				auto present_to_draw_barrier = vk::ImageMemoryBarrier{}
					.setSrcQueueFamilyIndex(present_queue_family)
					.setDstQueueFamilyIndex(present_queue_family)
					.setSrcAccessMask({})
					.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
					.setOldLayout(vk::ImageLayout::eUndefined)
					.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
					.setSubresourceRange(image_subresource_range)
					.setImage(swapchain_image);

				PanicIfError(frame_command_buffer->InsertImageMemoryBarrier(present_to_draw_barrier, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput));

				tone_map_pass.Render(frame_command_buffer);

				auto draw_to_present_barrier = vk::ImageMemoryBarrier{}
					.setSrcQueueFamilyIndex(present_queue_family)
					.setDstQueueFamilyIndex(present_queue_family)
					.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
					.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
					.setOldLayout(vk::ImageLayout::ePresentSrcKHR)
					.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
					.setSubresourceRange(image_subresource_range)
					.setImage(swapchain_image);

				PanicIfError(frame_command_buffer->InsertImageMemoryBarrier(draw_to_present_barrier, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe));
			}
			else
			{
				frame_command_buffer->Begin();

				auto present_to_draw_barrier = vk::ImageMemoryBarrier{}
					.setSrcQueueFamilyIndex(present_queue_family)
					.setDstQueueFamilyIndex(present_queue_family)
					.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
					.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
					.setOldLayout(vk::ImageLayout::eUndefined)
					.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
					.setSubresourceRange(image_subresource_range)
					.setImage(swapchain_image);

				PanicIfError(frame_command_buffer->InsertImageMemoryBarrier(present_to_draw_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer));

				auto draw_to_present_barrier = vk::ImageMemoryBarrier{}
					.setSrcQueueFamilyIndex(present_queue_family)
					.setDstQueueFamilyIndex(present_queue_family)
					.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
					.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
					.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
					.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
					.setSubresourceRange(image_subresource_range)
					.setImage(swapchain_image);

				frame_command_buffer->command_buffer.clearColorImage(swapchain_image, vk::ImageLayout::eTransferDstOptimal, vk::ClearColorValue{}.setFloat32({ 0.0f, 0.0f, 0.0f, 0.0f }), image_subresource_range);

				PanicIfError(frame_command_buffer->InsertImageMemoryBarrier(draw_to_present_barrier, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe));
			}

			{
				{
					ScopedCPUProfileRaysterizer("ImGui Draw calls");

					auto fps = fmt::format("FPS: {}", c.GetLastFramePerSecond());

					struct PushId
					{
						explicit PushId(void* x)
						{
							ImGui::PushID(x);
						}
						~PushId()
						{
							ImGui::PopID();
						}
					};

					{
						auto& light = common_resources->light;

						if(0)
						{
							static ImGuizmo::OPERATION current_gizmo_operation(ImGuizmo::ROTATE);
							static ImGuizmo::MODE current_gizmo_mode(ImGuizmo::WORLD);
							{
								PushId("ImGuizmo config");
								if (ImGui::IsKeyPressed('t'))
								{
									current_gizmo_operation = ImGuizmo::TRANSLATE;
								}
								if (ImGui::IsKeyPressed('r'))
								{
									current_gizmo_operation = ImGuizmo::ROTATE;
								}
								if (ImGui::IsKeyPressed('s'))
								{
									current_gizmo_operation = ImGuizmo::SCALE;
								}
								if (ImGui::RadioButton("Translate", current_gizmo_operation == ImGuizmo::TRANSLATE))
								{
									current_gizmo_operation = ImGuizmo::TRANSLATE;
								}
								ImGui::SameLine();
								if (ImGui::RadioButton("Rotate", current_gizmo_operation == ImGuizmo::ROTATE))
								{
									current_gizmo_operation = ImGuizmo::ROTATE;
								}
								ImGui::SameLine();
								if (ImGui::RadioButton("Scale", current_gizmo_operation == ImGuizmo::SCALE))
								{
									current_gizmo_operation = ImGuizmo::SCALE;
								}

								if (current_gizmo_mode != ImGuizmo::SCALE)
								{
									if (ImGui::RadioButton("Local", current_gizmo_mode == ImGuizmo::LOCAL))
									{
										current_gizmo_mode = ImGuizmo::LOCAL;
									}
									ImGui::SameLine();
									if (ImGui::RadioButton("World", current_gizmo_mode == ImGuizmo::WORLD))
									{
										current_gizmo_mode = ImGuizmo::WORLD;
									}
								}
							}

							ImGuizmo::SetOrthographic(false);
							ImGuiIO& io = ImGui::GetIO();
							ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

							auto& context = Raysterizer::OpenGL::Context::Get();
							auto& state = context.state;
							auto& raysterizer_info = state.GetRaysterizerInfo();

							auto identity = glm::mat4(1.0f);

							if (ImGuizmo::Manipulate(&identity[0][0], &raysterizer_info.projection_view[0][0], light.light_transform_operation, ImGuizmo::WORLD, &light.light_transform[0][0], NULL, NULL))
							{

							}
						}

						//ImGui::ShowDemoWindow();
						if (ImGui::Begin("Raysterizer"))
						{
							ImGui::Text("FPS %s", fps.c_str());

							if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
							{
								const std::vector<std::string>            environment_types = { "None", "Procedural Sky" };
								const std::vector<std::string_view> visualization_types = { "Final", "Shadows", "Ambient Occlusion", "Reflections", "Global Illumination", "G Buffer", "Normals"};

								if (ImGui::BeginCombo("Skybox Environment", environment_types[common_resources->environment_type].c_str()))
								{
									for (uint32_t i = 0; i < environment_types.size(); i++)
									{
										const bool is_selected = (i == common_resources->environment_type);

										if (ImGui::Selectable(environment_types[i].c_str(), is_selected))
										{
											common_resources->environment_type = (Pass::EnvironmentType)i;
										}

										if (is_selected)
											ImGui::SetItemDefaultFocus();
									}
									ImGui::EndCombo();
								}

								if (common_resources->environment_type == Pass::EnvironmentType::ENVIRONMENT_TYPE_PROCEDURAL_SKY)
								{
									PushId("Procedural Sky");
									ImGui::SliderFloat3("Value", &common_resources->procedural_sky_direction[0], 0.0f, 1.0f);
									ImGui::gizmo3D("Direction", common_resources->procedural_sky_direction);
								}

								if (ImGui::BeginCombo("Visualize Buffer", visualization_types[common_resources->current_visualization_type].data()))
								{
									for (uint32_t i = 0; i < visualization_types.size(); i++)
									{
										const bool is_selected = (i == common_resources->current_visualization_type);

										if (ImGui::Selectable(visualization_types[i].data(), is_selected))
										{
											common_resources->current_visualization_type = (Pass::VisualizationType)i;
										}

										if (is_selected)
										{
											ImGui::SetItemDefaultFocus();
										}
									}
									ImGui::EndCombo();
								}

								if (common_resources->current_visualization_type == Pass::VisualizationType::VISUALIZATION_TYPE_REFLECTIONS)
								{
									Pass::RayTracedReflections::OutputType type = ray_traced_reflections_pass.GetOutputType();

									if (ImGui::BeginCombo("Buffers", Pass::RayTracedReflections::kOutputTypeNames[type].c_str()))
									{
										for (uint32_t i = 0; i < Pass::RayTracedReflections::kNumOutputTypes; i++)
										{
											const bool is_selected = (i == type);

											if (ImGui::Selectable(Pass::RayTracedReflections::kOutputTypeNames[i].c_str(), is_selected))
											{
												type = (Pass::RayTracedReflections::OutputType)i;
											}

											if (is_selected)
											{
												ImGui::SetItemDefaultFocus();
											}
										}
										ImGui::EndCombo();
									}

									ray_traced_reflections_pass.SetOutputType(type);
								}
								else if (common_resources->current_visualization_type == Pass::VisualizationType::VISUALIZATION_TYPE_SHADOWS)
								{
									Pass::RayTracedShadows::OutputType type = ray_traced_shadows_pass.GetOutputType();

									if (ImGui::BeginCombo("Buffers", Pass::RayTracedShadows::kOutputTypeNames[type].c_str()))
									{
										for (uint32_t i = 0; i < Pass::RayTracedShadows::kNumOutputTypes; i++)
										{
											const bool is_selected = (i == type);

											if (ImGui::Selectable(Pass::RayTracedShadows::kOutputTypeNames[i].c_str(), is_selected))
											{
												type = (Pass::RayTracedShadows::OutputType)i;
											}

											if (is_selected)
											{
												ImGui::SetItemDefaultFocus();
											}
										}
										ImGui::EndCombo();
									}

									ray_traced_shadows_pass.SetOutputType(type);
								}
								else if (common_resources->current_visualization_type == Pass::VisualizationType::VISUALIZATION_TYPE_AMBIENT_OCCLUSION)
								{
									Pass::RayTracedAO::OutputType type = ray_traced_ao_pass.GetOutputType();

									if (ImGui::BeginCombo("Buffers", Pass::RayTracedAO::kOutputTypeNames[type].c_str()))
									{
										for (uint32_t i = 0; i < Pass::RayTracedAO::kNumOutputTypes; i++)
										{
											const bool is_selected = (i == type);

											if (ImGui::Selectable(Pass::RayTracedAO::kOutputTypeNames[i].c_str(), is_selected))
											{
												type = (Pass::RayTracedAO::OutputType)i;
											}

											if (is_selected)
											{
												ImGui::SetItemDefaultFocus();
											}
										}
										ImGui::EndCombo();
									}

									ray_traced_ao_pass.SetOutputType(type);
								}
							}
							if (ImGui::TreeNode("Light Settings"))
							{
								const std::vector<std::string> light_types = { "Directional", "Point", "Spot" };

								Pass::LightType type = light.light_type;

								if (ImGui::BeginCombo("Type", light_types[type].c_str()))
								{
									for (uint32_t i = 0; i < light_types.size(); i++)
									{
										const bool is_selected = (i == type);

										if (ImGui::Selectable(light_types[i].c_str(), is_selected))
										{
											type = (Pass::LightType)i;

											if (type != Pass::LightType::LIGHT_TYPE_DIRECTIONAL)
											{
												auto middle = (common_resources->ddgi.min_extents + common_resources->ddgi.max_extents) / 2.0f;
												middle += vec3(0.0f, 10.0f, 0.0f);
												light.light_transform = glm::mat4(1.0f);
												glm::translate(light.light_transform, middle);
											}
										}

										if (is_selected)
										{
											ImGui::SetItemDefaultFocus();
										}
									}
									ImGui::EndCombo();
								}

								if (light.light_type != type)
								{
									light.light_type = type;
								}

								if (type == Pass::LightType::LIGHT_TYPE_DIRECTIONAL)
								{
									ImGui::ColorEdit3("Color", &light.light_color.x);
									ImGui::InputFloat("Intensity", &light.light_intensity);
									ImGui::SliderFloat("Radius", &light.light_radius, 0.0f, 10.0f);

									glm::vec3 position;
									glm::vec3 rotation;
									glm::vec3 scale;

									ImGuizmo::DecomposeMatrixToComponents(&light.light_transform[0][0], &position.x, &rotation.x, &scale.x);

									ImGui::InputFloat3("Rotation", &rotation.x);

									ImGuizmo::RecomposeMatrixFromComponents(&position.x, &rotation.x, &scale.x, &light.light_transform[0][0]);

									glm::vec3 out_skew;
									glm::vec4 out_persp;
									glm::vec3 out_scale;
									glm::quat out_orientation;
									glm::vec3 out_position;

									glm::decompose(light.light_transform, out_scale, out_orientation, out_position, out_skew, out_persp);

									ImGui::DragFloat3("Direction Drag", glm::value_ptr(light.light_direction));
									ImGui::gizmo3D("Direction", light.light_direction);

									ImGui::Checkbox("Animation", &light.light_animation);
								}
								else if (type == Pass::LightType::LIGHT_TYPE_POINT)
								{
									ImGui::ColorEdit3("Color", &light.light_color.x);
									ImGui::InputFloat("Intensity", &light.light_intensity);
									ImGui::SliderFloat("Radius", &light.light_radius, 0.0f, 10000.0f);

									if (ImGui::RadioButton("Translate", light.light_transform_operation == ImGuizmo::TRANSLATE))
									{
										light.light_transform_operation = ImGuizmo::TRANSLATE;
									}

									glm::vec3 position;
									glm::vec3 rotation;
									glm::vec3 scale;

									ImGuizmo::DecomposeMatrixToComponents(&light.light_transform[0][0], &position.x, &rotation.x, &scale.x);

									ImGui::DragFloat3("Position", &position.x);
									ImGui::gizmo3D("Direction", rotation);

									ImGuizmo::RecomposeMatrixFromComponents(&position.x, &rotation.x, &scale.x, &light.light_transform[0][0]);
								}
								else if (type == Pass::LightType::LIGHT_TYPE_SPOT)
								{
									ImGui::ColorEdit3("Color", &light.light_color.x);
									ImGui::InputFloat("Intensity", &light.light_intensity);
									ImGui::SliderFloat("Radius", &light.light_radius, 0.0f, 10000.0f);
									ImGui::SliderFloat("Inner Cone Angle", &light.light_cone_angle_inner, 1.0f, 10000.0f);
									ImGui::SliderFloat("Outer Cone Angle", &light.light_cone_angle_outer, 1.0f, 10000.0f);

									if (ImGui::RadioButton("Translate", light.light_transform_operation == ImGuizmo::TRANSLATE))
									{
										light.light_transform_operation = ImGuizmo::TRANSLATE;
									}

									ImGui::SameLine();

									if (ImGui::RadioButton("Rotate", light.light_transform_operation == ImGuizmo::ROTATE))
									{
										light.light_transform_operation = ImGuizmo::ROTATE;
									}

									glm::vec3 position;
									glm::vec3 rotation;
									glm::vec3 scale;

									ImGuizmo::DecomposeMatrixToComponents(&light.light_transform[0][0], &position.x, &rotation.x, &scale.x);

									ImGui::InputFloat3("Position", &position.x);
									ImGui::InputFloat3("Rotation", &rotation.x);
									ImGui::gizmo3D("Direction", rotation);

									ImGuizmo::RecomposeMatrixFromComponents(&position.x, &rotation.x, &scale.x, &light.light_transform[0][0]);

									ImGui::Checkbox("Animation", &light.light_animation);
								}

								auto& context = Raysterizer::OpenGL::Context::Get();
								auto& state = context.state;
								auto& raysterizer_info = state.GetRaysterizerInfo();

								ImGui::DragFloat3("Camera position", &raysterizer_info.camera_position[0]);

								ImGui::TreePop();
								ImGui::Separator();
							}

							const std::vector<std::string> ray_trace_scales = { "Full", "Half", "Quarter" };
							if (ImGui::TreeNode("DDGI"))
							{
								PushId push_id(&ddgi_pass);

								Pass::RayTraceScale scale = common_resources->ddgi.scale;

								if (ImGui::BeginCombo("Scale", ray_trace_scales[scale].c_str()))
								{
									for (uint32_t i = 0; i < ray_trace_scales.size(); i++)
									{
										const bool is_selected = (i == scale);

										if (ImGui::Selectable(ray_trace_scales[i].c_str(), is_selected))
										{
											device.waitIdle();
											common_resources->ddgi.scale = (Pass::RayTraceScale)i;
											PanicIfError(ddgi_pass.Setup(common_resources));
										}

										if (is_selected)
										{
											ImGui::SetItemDefaultFocus();
										}
									}
									ImGui::EndCombo();
								}

								if (ImGui::Button("Auto set min and max extents"))
								{
									common_resources->ddgi.sample_from_vertices = true;
									common_resources->ddgi.min_extents = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
									common_resources->ddgi.max_extents = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
									common_resources->ddgi.id++;
								}

								ddgi_pass.UpdateGui();
								deferred_shading_pass.UpdateGui();

								ImGui::TreePop();
								ImGui::Separator();
							}
							if (ImGui::TreeNode("Shadows"))
							{
								PushId push_id(&ray_traced_shadows_pass);

								Pass::RayTraceScale scale = common_resources->shadows.scale;

								if (ImGui::BeginCombo("Scale", ray_trace_scales[scale].c_str()))
								{
									for (uint32_t i = 0; i < ray_trace_scales.size(); i++)
									{
										const bool is_selected = (i == scale);

										if (ImGui::Selectable(ray_trace_scales[i].c_str(), is_selected))
										{
											device.waitIdle();
											common_resources->shadows.scale = (Pass::RayTraceScale)i;
											PanicIfError(ray_traced_shadows_pass.Setup(common_resources));
										}

										if (is_selected)
										{
											ImGui::SetItemDefaultFocus();
										}
									}
									ImGui::EndCombo();
								}

								ray_traced_shadows_pass.UpdateGui();

								ImGui::TreePop();
								ImGui::Separator();
							}
							if (ImGui::TreeNode("Ambient Occlusion"))
							{
								PushId push_id(&ray_traced_ao_pass);

								Pass::RayTraceScale scale = common_resources->ao.scale;

								if (ImGui::BeginCombo("Scale", ray_trace_scales[scale].c_str()))
								{
									for (uint32_t i = 0; i < ray_trace_scales.size(); i++)
									{
										const bool is_selected = (i == scale);

										if (ImGui::Selectable(ray_trace_scales[i].c_str(), is_selected))
										{
											device.waitIdle();
											common_resources->ao.scale = (Pass::RayTraceScale)i;
											PanicIfError(ray_traced_ao_pass.Setup(common_resources));
										}

										if (is_selected)
										{
											ImGui::SetItemDefaultFocus();
										}
									}
									ImGui::EndCombo();
								}

								ray_traced_ao_pass.UpdateGui();

								ImGui::TreePop();
								ImGui::Separator();
							}
							if (ImGui::TreeNode("Reflections"))
							{
								PushId push_id(&ray_traced_reflections_pass);

								Pass::RayTraceScale scale = common_resources->reflections.scale;

								if (ImGui::BeginCombo("Scale", ray_trace_scales[scale].c_str()))
								{
									for (uint32_t i = 0; i < ray_trace_scales.size(); i++)
									{
										const bool is_selected = (i == scale);

										if (ImGui::Selectable(ray_trace_scales[i].c_str(), is_selected))
										{
											device.waitIdle();
											common_resources->reflections.scale = (Pass::RayTraceScale)i;
											PanicIfError(ray_traced_reflections_pass.Setup(common_resources));
										}

										if (is_selected)
										{
											ImGui::SetItemDefaultFocus();
										}
									}
									ImGui::EndCombo();
								}

								ray_traced_reflections_pass.UpdateGui();

								ImGui::TreePop();
								ImGui::Separator();
							}

							if (ImGui::TreeNode("Shading Settings"))
							{
								PushId push_id(&deferred_shading_pass);

								bool enabled = false;

								enabled = deferred_shading_pass.GetUseRayTracedShadows();
								if (ImGui::Checkbox("Enabled Raytraced Shadows", &enabled))
								{
									deferred_shading_pass.SetUseRayTracedShadows(enabled);
								}

								enabled = deferred_shading_pass.GetUseRayTracedAO();
								if (ImGui::Checkbox("Enabled Raytraced AO", &enabled))
								{
									deferred_shading_pass.SetUseRayTracedAO(enabled);
								}

								enabled = deferred_shading_pass.GetUseRayTracedReflections();
								if (ImGui::Checkbox("Enabled Raytraced Reflections", &enabled))
								{
									deferred_shading_pass.SetUseRayTracedReflections(enabled);
								}

								enabled = deferred_shading_pass.GetUseDDGI();
								if (ImGui::Checkbox("Enabled DDGI", &enabled))
								{
									deferred_shading_pass.SetUseDDGI(enabled);
								}

								deferred_shading_pass.UpdateGui();

								ImGui::TreePop();
								ImGui::Separator();
							}

							if (ImGui::TreeNode("TAA"))
							{
								PushId push_id(&temporal_aa_pass);

								temporal_aa_pass.UpdateGui();

								ImGui::TreePop();
								ImGui::Separator();
							}

							if (ImGui::TreeNode("Tone Mapping"))
							{
								PushId push_id(&tone_map_pass);

								tone_map_pass.UpdateGui();

								ImGui::TreePop();
								ImGui::Separator();
							}
						}
						ImGui::End();
					}
				}

				auto& cb = **frame_command_buffer;
				CMShared<RenderPass> rp = c.GetGlobalRenderPass();

				auto cv = std::vector<vk::ClearValue>
				{
					vk::ClearValue{}
						.setColor(vk::ClearColorValue()),
					vk::ClearValue{}
						.setDepthStencil(
							vk::ClearDepthStencilValue{}
							.setDepth(1.0f)
							.setStencil(0)
						)
				};

				auto render_pass_begin_info = vk::RenderPassBeginInfo{}
					.setRenderPass(rp->render_pass)
					.setRenderArea(vk::Rect2D{}
						.setOffset({ 0, 0 })
						.setExtent(c.GetWindowExtent())
					)
					.setFramebuffer(c.GetFrameCurrentBuffer())
					.setClearValues(cv);

				cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

				ImGui::Render();


				static bool show_gui = true;
				{
					const auto& io = ImGui::GetIO();
					static auto last_time = std::chrono::high_resolution_clock::now();
					if (GetAsyncKeyState(VK_F10))
					{
						auto now = std::chrono::high_resolution_clock::now();
						if ((now - last_time).count() > 0.750)
						{
							show_gui = !show_gui;
						}
					}
				}
				if (show_gui)
				{
					RenderImGui(cb);
				}

				cb.endRenderPass();
			}

			auto wait_dst_stage_mask = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
			graphics_qbm.EnqueueCommandBuffer(frame_command_buffer);
			graphics_qbm.EnqueueWait(wait_dst_stage_mask, present_semaphore);
			graphics_qbm.EnqueueSignal(render_semaphore);
			PanicIfError(graphics_qbm.Submit(QueueType::Graphics, render_fence));

			const static bool wait_idle = Config["raysterizer"]["wait_idle"];
			if (wait_idle)
			{
				device.waitIdle();
			}

			auto image_index = c.GetNextFrameIndex();

			{
				ScopedCPUProfileRaysterizer("Present image");
				if (auto err = c.Present(render_semaphore, image_index))
				{
					if (err.isA<SwapchainOutOfDateError>())
					{
						ConsumeError(err);
						GenerateRaytracingStorageTexture();
					}
					else
					{
						PanicError(err);
					}
				}
			}
	
			PanicIfError(render_frame.EndFrame());
			PanicIfError(c.EndFrame());

			c.AdvanceFrame();

			render_call_records.clear();

			if (common_resources->first_frame)
			{
				common_resources->first_frame = false;
			}
			common_resources->ping_pong = !common_resources->ping_pong;

			return;









//
//			ScopedCPUProfileRaysterizerCurrentFunction();
//
//			QueueBatchManager transfer_qbm = QueueBatchManager(c);
//			QueueBatchManager compute_qbm = QueueBatchManager(c);
//			QueueBatchManager graphics_qbm = QueueBatchManager(c);
//
//			auto& program_to_draw_calls = GetCurrentProgramToDrawCalls();
//
//			vk::Device device = c.GetDevice();
//			RenderFrame& render_frame = c.GetRenderFrame();
//
//			auto current_frame = c.GetFrame();
//			auto current_frame_index = c.GetFrameIndex();
//			auto num_frames = c.GetNumFrames();
//
//			{
//				ScopedCPUProfileRaysterizer("Beginning of Frame Updates");
//				PanicIfError(render_frame.PerformBeginningOfFrameUpdates());
//			}
//
//			CMShared<Semaphore> present_semaphore = AssignOrPanic(render_frame.GetBinarySemaphore());
//			c.SetName(present_semaphore, fmt::format("Present semaphore frame {}", current_frame_index));
//			{
//				ScopedCPUProfileRaysterizer("Prepare Frame");
//				if (auto err = c.PrepareFrame(present_semaphore))
//				{
//					if (err.isA<SwapchainOutOfDateError>())
//					{
//						ConsumeError(err);
//						return;
//					}
//					else
//					{
//						PanicError(err);
//					}
//				}
//			}
//
//			const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];
//			static uint32_t recursion_depth = Config["raysterizer"]["vulkan"]["recursion_depth"];
//			static uint32_t updated_recursion_depth = recursion_depth;
//
//			const static FrameCounter pipeline_release_entries_past_frame_counter_difference = Config["raysterizer"]["pipeline_release_entries_past_frame_counter_difference"];
//			blas_cache.ClearEntriesPastFrameCounterDifference(pipeline_release_entries_past_frame_counter_difference);
//
//			auto shader_module_create_infos = base_shader_module_create_infos;
//			flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};
//			variable_set_index_to_count[0] = max_variable_bindings;
//
//			std::map<uint32_t, std::shared_ptr<DrawCalls>> instance_shader_binding_offset_to_draw_calls;
//			for (auto& [_, draw_calls] : program_to_draw_calls)
//			{
//				uint32_t instance_shader_binding_offset = draw_calls->GetShaderBindingIndex() - draw_call_to_set_starting_index;
//				instance_shader_binding_offset_to_draw_calls[instance_shader_binding_offset] = draw_calls;
//			}
//
//			auto plci = PipelineLayoutCreateInfo
//			{
//				shader_module_create_infos,
//				variable_set_index_to_count
//			};
//
//			CMShared<PipelineLayoutInfo> pli{};
//			{
//				ScopedCPUProfileRaysterizer("PipelineLayoutInfo Creation");
//				pli = AssignOrPanic(c.Get(plci));
//				c.SetName(pli, fmt::format("Pipeline layout info {}", current_frame_index));
//			}
//
//			/*
//			DescriptorSetLayoutCreateInfos dslcis = AssignOrPanic(plci.BuildDescriptorSetLayoutCreateInfos(pli->combined_shader_reflection));
//			if (dslcis.GetCreateInfos().empty())
//			{
//				PANIC("Should not be empty");
//			}
//			CMShared<DescriptorPool> dp = AssignOrPanic(c.Get(DescriptorPoolCreateInfo{ dslcis }));
//			*/
//
//			CMShared<DescriptorPool> dp = global_descriptor_pools[current_frame_index];
//
//			CMShared<DescriptorSet> ds{};
//			CMShared<PipelineLayout> pl{};
//			
//			{
//				ScopedCPUProfileRaysterizer("DescriptorSet Creation");
//				ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
//				c.SetName(ds, fmt::format("Descriptor set frame {}", current_frame_index));
//			}
//			{
//				ScopedCPUProfileRaysterizer("PipelineLayout Creation");
//				pl = AssignOrPanic(c.Get(pli));
//				c.SetName(pl, fmt::format("Pipeline layout frame {}", current_frame_index));
//			}
//
//			RaytracingPipelineCreateInfo raytracing_pipeline_create_info{ pl, recursion_depth };
//			CMShared<RaytracingPipeline> raytracing_pipeline{};
//			{
//				ScopedCPUProfileRaysterizer("RaytracingPipeline Creation");
//				raytracing_pipeline = AssignOrPanic(c.Get(raytracing_pipeline_create_info));
//				c.SetName(raytracing_pipeline, fmt::format("Raytracing pipeline frame {}", current_frame_index));
//			}
//			
//			auto& write_descriptor_sets = ds->GetWriteDescriptorSets();
//
//			PanicIfError(render_frame.ResetCommandBufferPools());
//
//			CMShared<Semaphore> render_semaphore = AssignOrPanic(render_frame.GetBinarySemaphore());
//			CMShared<Fence> render_fence = AssignOrPanic(render_frame.GetFence());
//			CMShared<CommandBuffer> command_buffer = AssignOrPanic(render_frame.GetCommandBuffer(QueueType::Graphics));
//			//CMShared<CommandBuffer> command_buffer = AssignOrPanic(render_frame.GetCommandBufferFromResetPool(QueueType::Graphics));
//			
//			c.SetName(render_semaphore, fmt::format("Render semaphore frame {}", current_frame_index));
//			c.SetName(render_fence, fmt::format("Render fence frame {}", current_frame_index));
//			c.SetName(command_buffer, fmt::format("Command buffer frame {}", current_frame_index));
//
//			render_fence->AddCompletionTo(ds);
//
//#ifndef NDEBUG
//			std::map<GLuint, std::map<uint32_t, std::map<uint32_t, RaysterizerEngine::WriteDescriptorSet>>> program_to_set_binding_descriptor_write_resource;
//#endif
//
//			const static std::size_t draw_call_binding_index_pool_interval = Config["raysterizer"]["draw_call"]["binding_index_pool_interval"];
//			const static std::size_t draw_call_storage_buffer_binding_index_pool_starting_index = Config["raysterizer"]["draw_call"]["storage_buffer_binding_index_pool_starting_index"];
//			const static std::size_t draw_call_uniform_buffer_binding_index_pool_starting_index = Config["raysterizer"]["draw_call"]["uniform_buffer_binding_index_pool_starting_index"];
//			const static std::size_t draw_call_combined_sampler_binding_index_pool_starting_index = Config["raysterizer"]["draw_call"]["combined_sampler_binding_index_pool_starting_index"];
//
//			auto& sbt_buffer = sbt_buffers[current_frame_index];
//			{
//				ScopedCPUProfileRaysterizer("SBT building");
//
//				auto new_sbt_shader_hash = 0;
//
//				ShaderBindingTable new_sbt{};
//				new_sbt.Reset();
//				new_sbt.AddRayGenProgram(0);
//				new_sbt.AddMissProgram(1);
//
//				auto start_index = 2;
//				for (auto& [_, draw_calls] : program_to_draw_calls)
//				{
//					//auto shader = draw_calls->GetReflectShader();
//
//					new_sbt.AddHitGroup(start_index++);
//				}
//
//				auto& sbt_shader_hash = sbt_shader_hashes[current_frame_index];
//				if (new_sbt_shader_hash != sbt_shader_hash)
//				{
//					ScopedCPUProfileRaysterizer("SBT rebuilding");
//					sbt = new_sbt;
//
//					auto sbt_data = sbt.GetHandleBytes(c.GetDevice(), raytracing_properties, **raytracing_pipeline);
//
//					if (sbt_buffer->GetSize() < sbt_data.size())
//					{
//						sbt_buffer = AssignOrPanic(render_frame.ResizeBuffer(sbt_buffer, sbt_data.size()));
//					}
//					PanicIfError(sbt_buffer->Copy(PointerView(sbt_data)));
//					c.SetName(sbt_buffer, fmt::format("SBT {}", current_frame_index));
//
//					sbt_shader_hash = new_sbt_shader_hash;
//				}
//
//				//auto sbt_transfer_job = AssignOrPanic(render_frame.UploadDataToGPUBuffer(PointerView(sbt_data), vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR));
//				//transfer_qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, sbt_transfer_job.semaphore);
//				//sbt_buffer = sbt_transfer_job.gpu_buffer;
//				//sbt_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(raysterizer_info), true));
//			}
//
//
//			{
//				ScopedCPUProfileRaysterizer("ImGui Draw calls");
//
//				auto fps = fmt::format("FPS: {}", c.GetLastFramePerSecond());
//
//				ImGui_ImplVulkan_NewFrame();
//				ImGui_ImplGlfw_NewFrame();
//				ImGui::NewFrame();
//
//				//imgui commands
//				ImGui::ShowDemoWindow();
//				if (ImGui::Begin("Test"))
//				{
//					//ImGui::DragFloat4("push constants data", glm::value_ptr(constants.data));
//					ImGui::Text(fps.c_str());
//
//					auto& context = Raysterizer::OpenGL::Context::Get();
//					auto& state = context.state;
//					auto perform_perspective_correction = state.GetPerformPerspectiveCorrection();
//					if (ImGui::Checkbox("Perform Perspective Correction", &perform_perspective_correction))
//					{
//						context.state.SetPerformPerspectiveCorrection(perform_perspective_correction);
//					}
//
//					auto& raysterizer_info = state.GetRaysterizerInfo();
//					ImGui::InputFloat("Z Near", &raysterizer_info.z_near, -1.0f, 1.0f);
//					ImGui::InputFloat("Z Far", &raysterizer_info.z_far, -1.0f, 1000.0f);
//					ImGui::InputInt("Num samples", (int*)&raysterizer_info.number_of_samples);
//					ImGui::InputInt("Num bounces", (int*)&raysterizer_info.number_of_bounces);
//
//					const auto recursion_min = 1;
//					ImGui::SliderScalar("Ray recursion depth", ImGuiDataType_U32, &updated_recursion_depth, &recursion_min, &raytracing_properties.maxRayRecursionDepth);
//
//					static auto prev_time = std::chrono::high_resolution_clock::now();
//					auto time_step_diff = 200;
//					auto increment_amount = 0.05f;
//					auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - prev_time).count();
//
//					if (diff > time_step_diff)
//					{
//						if (GetAsyncKeyState(VK_LEFT) & 0x8000)
//						{
//							raysterizer_info.z_near -= increment_amount;
//						}
//						else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
//						{
//							raysterizer_info.z_near += increment_amount;
//						}
//						else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
//						{
//							raysterizer_info.z_far -= increment_amount;
//						}
//						else if (GetAsyncKeyState(VK_UP) & 0x8000)
//						{
//							raysterizer_info.z_far += increment_amount;
//						}
//						else if (GetAsyncKeyState(VK_SPACE) & 0x8000)
//						{
//							context.state.SetPerformPerspectiveCorrection(!perform_perspective_correction);
//						}
//					}
//
//					auto ToggleButton = [](const char* str_id, bool* v)
//					{
//						ImVec4* colors = ImGui::GetStyle().Colors;
//						ImVec2 p = ImGui::GetCursorScreenPos();
//						ImDrawList* draw_list = ImGui::GetWindowDrawList();
//
//						float height = ImGui::GetFrameHeight();
//						float width = height * 1.55f;
//						float radius = height * 0.50f;
//
//						ImGui::InvisibleButton(str_id, ImVec2(width, height));
//						bool changed_v = false;
//						if (ImGui::IsItemClicked())
//						{
//							changed_v = true;
//							*v = !*v;
//						}
//						ImGuiContext& imgui_context = *GImGui;
//						float ANIM_SPEED = 0.085f;
//						if (imgui_context.LastActiveId == imgui_context.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
//							float t_anim = ImSaturate(imgui_context.LastActiveIdTimer / ANIM_SPEED);
//						if (ImGui::IsItemHovered())
//							draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
//						else
//							draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
//						draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
//						return changed_v;
//					};
//
//					static const fs::path material_info_path = Config["raysterizer"]["material_info"]["path"];
//					static const glm::vec4 default_material_color = ConvertFromArray(Config["raysterizer"]["material_info"]["default_color"]);
//
//					// Initialize material first
//					CallOnce
//					{
//						if (!fs::exists(material_info_path))
//						{
//							auto material_info_path_base = material_info_path;
//							material_info_path_base.remove_filename();
//							fs::create_directories(material_info_path_base);
//						}
//
//						std::ifstream f(material_info_path);
//						if (f)
//						{
//							json j;
//							f >> j;
//
//							for (const auto& [k, v] : j.items())
//							{
//								auto hash = std::strtoumax(k.c_str(), nullptr, 10);
//
//								CMShared<Buffer> buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(Material), true));
//								PanicIfError(buffer->Copy(PointerView(v)));
//
//								const MaterialInfoWithPointer material_info_with_index{ v, buffer };
//								material_info_mapping[hash] = material_info_with_index;
//							}
//						}
//					};
//
//					if (ImGui::Button("Save settings to disk"))
//					{
//						{
//							json j;
//							for (const auto& [hash, material_info_with_index] : material_info_mapping)
//							{
//								if (material_info_with_index.buffer)
//								{
//									auto hash_string = fmt::format("{}", hash);
//									j[hash_string] = material_info_with_index.m;
//								}
//							}
//
//							std::ofstream f(material_info_path, std::ios::trunc);
//							if (f)
//							{
//								f << std::setw(4) << j << std::endl;
//							}
//						}
//						{
//							const auto& j = Config.GetJson();
//
//							auto config_path = Constants::CONFIG_PATH;
//							std::ofstream f(config_path.data());
//							if (f)
//							{
//								f << std::setw(4) << j << std::endl;
//							}
//						}
//					}
//
//					phmap::flat_hash_set<std::size_t> already_inserted_hashes;
//					for (auto& [_, draw_calls] : program_to_draw_calls)
//					{
//						auto draw_call_count = draw_calls->GetDrawCount();
//						if (draw_call_count == 0)
//						{
//							continue;
//						}
//						auto count_to_draw = draw_calls->GetCountToDraw();
//
//						const auto& vertex_buffer_pointer_views = draw_calls->GetVertexBufferPointerViews();
//						const auto& index_buffer_pointer_views = draw_calls->GetIndexBufferPointerViews();
//						const auto& vertex_buffers = draw_calls->GetVertexBuffers();
//						const auto& index_buffers = draw_calls->GetIndexBuffers();
//
//						for (auto i = 0; i < vertex_buffer_pointer_views.size(); i++)
//						{
//							const auto& vertex_buffer_pointer_view = vertex_buffer_pointer_views[i];
//
//							auto hash = vertex_buffer_pointer_view.Hash();
//
//							if (already_inserted_hashes.contains(hash))
//							{
//								continue;
//							}
//							else
//							{
//								already_inserted_hashes.emplace(hash);
//							}
//
//							ImGui::PushID(hash);
//
//							// write to mapping
//							decltype(material_info_mapping)::iterator material_info_iter{};
//							if (auto found = material_info_mapping.find(hash); found != std::end(material_info_mapping))
//							{
//								material_info_iter = found;
//							}
//							else
//							{
//								const MaterialInfo default_material_info{ hash, fmt::format("{:08X}", hash), Material{default_material_color} };
//								const MaterialInfoWithPointer default_material_info_with_index{ default_material_info, nullptr };
//								auto [pair, inserted] = material_info_mapping.try_emplace(hash, default_material_info_with_index);
//								material_info_iter = pair;
//							}
//							auto& [_, material_info_with_index] = *material_info_iter;
//							auto& material_info = material_info_with_index.m;
//							auto& material = material_info.material;
//
//							bool dirty_buffer = false;
//							dirty_buffer |= ToggleButton("Highlight", &material.highlight);
//							if (dirty_buffer)
//							{
//								// Allocate buffer here if this is selected
//								if (!material_info_with_index.buffer)
//								{
//									material_info_with_index.buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(Material), true));
//									PanicIfError(material_info_with_index.buffer->Copy(PointerView(material_info)));
//								}
//							}
//							ImGui::SameLine();
//
//							if (ImGui::TreeNode((void*)i, "%s", material_info.name.c_str()))
//							{
//								//static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
//								//ImGui::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnArrow", &base_flags, ImGuiTreeNodeFlags_OpenOnArrow);
//
//								// Allocate buffer here if this is selected
//								if (!material_info_with_index.buffer)
//								{
//									material_info_with_index.buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(Material), true));
//									PanicIfError(material_info_with_index.buffer->Copy(PointerView(material_info)));
//								}
//
//								ImGui::InputText("Name", &material_info.name);
//								ImGui::ColorEdit4("Highlight color", glm::value_ptr(material.highlight_color));
//
//								ImGui::Text("Material");
//
//								dirty_buffer |= ImGui::ColorEdit4("diffuse", glm::value_ptr(material.diffuse));
//								dirty_buffer |= ImGui::SliderFloat("fuzziness", &material.fuzziness, 0.0f, 1.0f);
//								dirty_buffer |= ImGui::SliderFloat("refraction_index", &material.refraction_index, 0.0f, 1.0f);
//								//dirty_buffer |= ImGui::SliderFloat("material_model", &material.material_model, 0.0f, 1.0f);
//
//								const uint MaterialLambertian = 0;
//								const uint MaterialMetallic = 1;
//								const uint MaterialDielectric = 2;
//								const uint MaterialIsotropic = 3;
//								const uint MaterialDiffuseLight = 4;
//
//								dirty_buffer |= ImGui::RadioButton("Lambertian", reinterpret_cast<int*>(&material.material_model), 0); ImGui::SameLine();
//								dirty_buffer |= ImGui::RadioButton("Metallic", reinterpret_cast<int*>(&material.material_model), 1); ImGui::SameLine();
//								dirty_buffer |= ImGui::RadioButton("Dielectric", reinterpret_cast<int*>(&material.material_model), 2); ImGui::SameLine();
//								dirty_buffer |= ImGui::RadioButton("Isotropic", reinterpret_cast<int*>(&material.material_model), 3); ImGui::SameLine();
//								dirty_buffer |= ImGui::RadioButton("DiffuseLight", reinterpret_cast<int*>(&material.material_model), 4);
//
//								ImGui::TreePop();
//							}
//
//							//if (dirty_buffer)
//							{
//								if (material_info_with_index.buffer)
//								{
//									PanicIfError(material_info_with_index.buffer->Copy(PointerView(material)));
//								}
//							}
//
//							ImGui::PopID();
//						}
//					}
//					ImGui::ShowDemoWindow();
//				}
//				ImGui::End();
//				ImGui::Render();
//			}
//
//			{
//				ScopedCPUProfileRaysterizer("Transformation computation");
//
//				auto& context = Raysterizer::OpenGL::Context::Get();
//				auto& state = context.state;
//				auto& raysterizer_info = state.GetRaysterizerInfo();
//
//				raysterizer_info.projection_view = glm::mat4(1.0f);
//				bool changed = false;
//				for (auto& [_, draw_calls] : program_to_draw_calls)
//				{
//					auto& transformation_results = draw_calls->GetTransformationResults();
//
//					static bool run_async = Config["raysterizer"]["mvp_analysis"]["run_async"];
//					if (run_async)
//					{
//						draw_calls->WaitForExecution();
//					}
//
//					if (!transformation_results.empty())
//					{
//						const auto& transformation_result = transformation_results[0];
//						if (!transformation_result.empty())
//						{
//							const auto& result = transformation_result[0];
//							raysterizer_info.projection_view = result.projection_view;
//							changed = true;
//							break;
//						}
//
//						for (const auto& transformation_result : transformation_results)
//						{
//							for (const auto& result : transformation_result)
//							{
//								if (raysterizer_info.projection_view != result.projection_view && changed)
//								{
//									//PANIC("Projection view not matching {} {}", glm::to_string(raysterizer_info.projection_view_matrix), glm::to_string(result.projection_view));
//								}
//
//								if (result.projection_view[0][0] != 1.0f)
//								{
//									raysterizer_info.projection_view = result.projection_view;
//								}
//								changed = true;
//							}
//						}
//					}
//				}
//				raysterizer_info.projection_view_inverse = glm::inverse(raysterizer_info.projection_view);
//
//				{
//					ScopedCPUProfileRaysterizer("Copy raysterizer info buffer");
//					PanicIfError(raysterizer_info_buffer->Copy(PointerView(raysterizer_info)));
//				}
//			}
//
//			material_info_buffers.clear();
//
//			auto& tlas = tlases[current_frame_index];
//			{
//				ScopedCPUProfileRaysterizer("Acceleration structure building");
//
//				constexpr vk::BuildAccelerationStructureFlagsKHR build_flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
//				constexpr vk::GeometryInstanceFlagsKHR geometry_instance_flags = vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable;
//
//				TopLevelAccelerationStructureCreateInfo tlas_create_info;
//				tlas_create_info.build_flags = build_flags;
//				auto& tlases_blases = tlas_create_info.blases;
//				auto& acceleration_structure_instances = tlas_create_info.instances;
//				acceleration_structure_instances.clear();
//
//				if (tlas)
//				{
//					tlas_create_info.existing_instance_buffer = tlas->instance_buffer;
//				}
//
//				uint32_t material_index = INSTANCE_CUSTOM_INDEX_MATERIAL_START_INDEX;
//				flat_hash_map<std::size_t, uint32_t> existing_vertex_hash_to_instance_custom_index;
//				for (auto& [_, draw_calls] : program_to_draw_calls)
//				{
//					auto& draw_call_states_buffer = draw_calls->GetDrawCallStates();
//
//					auto draw_call_count = draw_calls->GetDrawCount();
//					if (draw_call_count == 0)
//					{
//						continue;
//					}
//					auto count_to_draw = draw_calls->GetCountToDraw();
//
//					auto vertex_buffer_stride = static_cast<uint32_t>(draw_calls->GetVertexBufferStride());
//					auto index_buffer_stride = static_cast<uint32_t>(draw_calls->GetIndexBufferStride());
//					if (vertex_buffer_stride == 0)
//					{
//						vertex_buffer_stride = 1;
//					}
//
//					vk::IndexType index_type{};
//					if (index_buffer_stride == 0)
//					{
//						index_type = vk::IndexType::eNoneKHR;
//					}
//					else if (index_buffer_stride == 1)
//					{
//						PANIC("Index type cannot be uint8");
//						index_type = vk::IndexType::eUint8EXT;
//					}
//					else if (index_buffer_stride == 2)
//					{
//						index_type = vk::IndexType::eUint16;
//					}
//					else if (index_buffer_stride == 4)
//					{
//						index_type = vk::IndexType::eUint32;
//					}
//					else
//					{
//						PANIC("index buffer stride", index_buffer_stride);
//					}
//
//					const auto& vertex_buffer_pointer_views = draw_calls->GetVertexBufferPointerViews();
//					const auto& index_buffer_pointer_views = draw_calls->GetIndexBufferPointerViews();
//					const auto& vertex_buffers = draw_calls->GetVertexBuffers();
//					const auto& index_buffers = draw_calls->GetIndexBuffers();
//					uint32_t instance_shader_binding_offset = draw_calls->GetShaderBindingIndex() - draw_call_to_set_starting_index;
//
//					const auto& transformation_results = draw_calls->GetTransformationResults();
//
//					for (auto i = 0; i < draw_call_count; i++)
//					{
//						auto num_elements = static_cast<uint32_t>(count_to_draw[i]);
//
//						const auto& vertex_buffer_pointer_view = vertex_buffer_pointer_views[i];
//						const auto& index_buffer_pointer_view = index_buffer_pointer_views[i];
//						const auto& vertex_buffer = vertex_buffers[i];
//						const auto& index_buffer = index_buffers[i];
//
//						if (vertex_buffer_pointer_view.GetStride() != vertex_buffer_stride)
//						{
//							//PANIC("Vertex strides not equal {} != {}", vertex_buffer_pointer_view.GetStride(), vertex_buffer_stride);
//						}
//
//						const auto& transformation_result = transformation_results[i];
//
//						auto vertex_buffer_hash = vertex_buffer_pointer_view.Hash();
//
//						uint32_t current_instance_custom_index = i;
//						if (auto found = material_info_mapping.find(vertex_buffer_hash); found != std::end(material_info_mapping))
//						{
//							auto& [_, material_info_with_index] = *found;
//							if (material_info_with_index.buffer)
//							{
//								material_info_buffers.emplace_back(material_info_with_index.buffer);
//							}
//						}
//
//						CMShared<BottomLevelAccelerationStructure> blas{};
//						if (auto found_or_err = blas_cache.Get(vertex_buffer_hash))
//						{
//							ScopedCPUProfileRaysterizer("BLAS acceleration structure cached");
//							blas = *found_or_err;
//						}
//						else
//						{
//							ConsumeError(found_or_err.takeError());
//
//							ScopedCPUProfileRaysterizer("BLAS acceleration structure building");
//
//							auto geometry_vertex_buffer = vertex_buffer;
//							auto geometry_index_buffer = index_buffer;
//
//#define MAYBE_COPY_BUFFER_FOR_BLAS 0
//							if (MAYBE_COPY_BUFFER_FOR_BLAS)
//							{
//								geometry_vertex_buffer = AssignOrPanic(render_frame.CopyBuffer(vertex_buffer));
//								geometry_index_buffer = AssignOrPanic(render_frame.CopyBuffer(index_buffer));
//							}
//
//							GeometryDescription geometry_description
//							{
//								GeometryVertexBuffer{geometry_vertex_buffer, num_elements, vertex_buffer_stride, vk::Format::eR32G32B32Sfloat},
//								GeometryIndexBuffer{geometry_index_buffer, num_elements, index_buffer_stride, index_type},
//								vk::GeometryFlagsKHR{}
//							};
//							std::vector<GeometryDescription> geometry_descriptions{ geometry_description };
//							vk::BuildAccelerationStructureFlagsKHR build_flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction | vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
//
//							BottomLevelAccelerationStructureCreateInfo blas_create_info{ geometry_descriptions, build_flags };
//							auto blas_transfer_job = AssignOrPanic(c.CreateBottomLevelAccelerationStructure(blas_create_info));
//							graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, blas_transfer_job.semaphore);
//							blas = blas_transfer_job.blas;
//
//							auto blas_compact_transfer_job = AssignOrPanic(c.CompactBottomLevelAccelerationStructure(blas_transfer_job));
//							PanicIfError(c.CleanupCompactBottomLevelAccelerationStructure(blas_compact_transfer_job));
//							graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, blas_compact_transfer_job.semaphore);
//							graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, blas_compact_transfer_job.blas_compact_semaphore);
//							blas = blas_compact_transfer_job.blas;
//
//							// Remove dependency on vertex/index buffer, so they can be deallocated
//							blas->create_info.geometry_descriptions.clear();
//
//							blas_cache.Emplace(vertex_buffer_hash, blas);
//						}
//
//						c.SetName(blas, fmt::format("blas frame {}", current_frame_index));
//
//						command_buffer->AddDependencyTo(blas);
//						tlases_blases.emplace_back(blas);
//
//						ScopedCPUProfileRaysterizer("BLAS acceleration structure instance creation");
//
//						for (const auto& transformation : transformation_result)
//						{
//							auto& transform = transformation.model;
//							vk::TransformMatrixKHR transform_matrix
//							(
//								std::array<std::array<float, 4>, 3>
//								{
//									std::array<float, 4>
//									{
//										transform[0][0], transform[1][0], transform[2][0], transform[3][0]
//									},
//									std::array<float, 4>
//									{
//										transform[0][1], transform[1][1], transform[2][1], transform[3][1]
//									},
//									std::array<float, 4>
//									{
//										transform[0][2], transform[1][2], transform[2][2], transform[3][2]
//									}
//								}
//							);
//
//							auto acceleration_structure_instance = vk::AccelerationStructureInstanceKHR{}
//								.setTransform(transform_matrix)
//								.setInstanceCustomIndex(current_instance_custom_index)
//								.setInstanceShaderBindingTableRecordOffset(instance_shader_binding_offset)
//								.setFlags(geometry_instance_flags)
//								.setMask(0xFF)
//								.setAccelerationStructureReference(blas->GetAccelerationStructureAddress());
//
//							acceleration_structure_instances.emplace_back(std::move(acceleration_structure_instance));
//						}
//					}
//				}
//
//				{
//					ScopedCPUProfileRaysterizer("TLAS acceleration structure building");
//
//					auto tlas_transfer_job = AssignOrPanic(c.CreateTopLevelAccelerationStructure(tlas_create_info));
//					tlas = tlas_transfer_job.tlas;
//					graphics_qbm.EnqueueWait(vk::PipelineStageFlagBits::eComputeShader, tlas_transfer_job.semaphore);
//
//					c.SetName(tlas, fmt::format("tlas frame {}", current_frame_index));
//
//					command_buffer->AddDependencyTo(tlas);
//				}
//			}
//
//			command_buffer->AddDependencyTo(material_info_buffers);
//
//			{
//				ScopedCPUProfileRaysterizer("Flush descriptor set");
//				{
//					PanicIfError(ds->Bind(0, 0, tlas));
//					PanicIfError(ds->Bind(0, 1, raytracing_write_storage_texture));
//					PanicIfError(ds->Bind(0, 2, raysterizer_info_buffer));
//					PanicIfError(ds->Bind(0, 3, material_info_buffers));
//				}
//
//				PanicIfError(render_frame.FlushPendingWrites(ds));
//			}
//
//			auto ComputeAndCopyImagePass = [&](CMShared<CommandBuffer> command_buffer)
//			{
//				ScopedGPUProfile(c.GetProfiler(), *command_buffer, "ComputeAndCopyImagePass");
//				vk::CommandBuffer cb = **command_buffer;
//
//				const vk::Image& swapchain_image = c.GetSwapchainImages()[current_frame_index];
//
//				auto image_subresource_range = vk::ImageSubresourceRange{}
//					.setAspectMask(vk::ImageAspectFlagBits::eColor)
//					.setBaseMipLevel(0)
//					.setLevelCount(1)
//					.setBaseArrayLayer(0)
//					.setLayerCount(1);
//
//				/*
//				if (DO_COMPUTE)
//				{
//					cb.bindPipeline(vk::PipelineBindPoint::eCompute, *compute_pipeline);
//					cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, compute_pl->pipeline_layout, 0, compute_ds->descriptor_sets, {});
//
//					auto compute_image_memory_barrier = vk::ImageMemoryBarrier{}
//						.setSrcQueueFamilyIndex({})
//						.setDstQueueFamilyIndex({})
//						.setSrcAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)
//						.setDstAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)
//						.setOldLayout(vk::ImageLayout::eGeneral)
//						.setNewLayout(vk::ImageLayout::eGeneral)
//						.setSubresourceRange(image_subresource_range);
//
//					compute_image_memory_barrier.setImage(*raytracing_read_storage_texture->image);
//					PanicIfError(command_buffer->InsertImageMemoryBarrier(compute_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
//
//					compute_image_memory_barrier.setImage(*raytracing_storage_texture->image);
//					PanicIfError(command_buffer->InsertImageMemoryBarrier(compute_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
//
//					auto COMPUTE_WG_X_SIZE = 16;
//					auto COMPUTE_WG_Y_SIZE = 16;
//					int dispatch_width = c.GetWindowExtent().width / COMPUTE_WG_X_SIZE + ((c.GetWindowExtent().width % COMPUTE_WG_X_SIZE) > 0 ? 1 : 0);
//					int dispatch_height = c.GetWindowExtent().height / COMPUTE_WG_Y_SIZE + ((c.GetWindowExtent().height % COMPUTE_WG_Y_SIZE) > 0 ? 1 : 0);
//					cb.dispatch(c.GetWindowExtent().width, c.GetWindowExtent().height, 1);
//				}
//				*/
//
//				auto dst_image_memory_barrier = vk::ImageMemoryBarrier{}
//					.setSrcQueueFamilyIndex({})
//					.setDstQueueFamilyIndex({})
//					.setSrcAccessMask({})
//					.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
//					.setOldLayout(vk::ImageLayout::eUndefined)
//					.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
//					.setSubresourceRange(image_subresource_range)
//					.setImage(swapchain_image);
//
//				PanicIfError(command_buffer->InsertImageMemoryBarrier(dst_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
//
//				auto src_image_memory_barrier = vk::ImageMemoryBarrier{}
//					.setSrcQueueFamilyIndex({})
//					.setDstQueueFamilyIndex({})
//					.setSrcAccessMask({})
//					.setDstAccessMask(vk::AccessFlagBits::eTransferRead)
//					.setOldLayout(vk::ImageLayout::eGeneral)
//					.setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
//					.setSubresourceRange(image_subresource_range)
//					.setImage(*raytracing_write_storage_texture->image);
//
//				PanicIfError(command_buffer->InsertImageMemoryBarrier(src_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
//
//				PanicIfError(command_buffer->InsertImageMemoryBarrier(raytracing_read_storage_texture->image, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, image_subresource_range));
//
//				auto image_copy = vk::ImageCopy{}
//					.setSrcSubresource
//					(
//						vk::ImageSubresourceLayers{}
//						.setAspectMask(vk::ImageAspectFlagBits::eColor)
//						.setMipLevel(0)
//						.setBaseArrayLayer(0)
//						.setLayerCount(1)
//					)
//					.setSrcOffset(vk::Offset3D{ 0, 0, 0 })
//					.setDstSubresource
//					(
//						vk::ImageSubresourceLayers{}
//						.setAspectMask(vk::ImageAspectFlagBits::eColor)
//						.setMipLevel(0)
//						.setBaseArrayLayer(0)
//						.setLayerCount(1)
//					)
//					.setDstOffset(vk::Offset3D{ 0, 0, 0 })
//					.setExtent(c.GetWindowExtent3D());
//
//				cb.copyImage(*raytracing_write_storage_texture->image, vk::ImageLayout::eTransferSrcOptimal, swapchain_image, vk::ImageLayout::eTransferDstOptimal, image_copy);
//
//				dst_image_memory_barrier = vk::ImageMemoryBarrier{}
//					.setSrcQueueFamilyIndex({})
//					.setDstQueueFamilyIndex({})
//					.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
//					.setDstAccessMask({})
//					.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
//					.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
//					.setSubresourceRange(image_subresource_range)
//					.setImage(swapchain_image);
//
//				PanicIfError(command_buffer->InsertImageMemoryBarrier(dst_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
//
//				src_image_memory_barrier = vk::ImageMemoryBarrier{}
//					.setSrcQueueFamilyIndex({})
//					.setDstQueueFamilyIndex({})
//					.setSrcAccessMask(vk::AccessFlagBits::eTransferRead)
//					.setDstAccessMask({})
//					.setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
//					.setNewLayout(vk::ImageLayout::eGeneral)
//					.setSubresourceRange(image_subresource_range)
//					.setImage(*raytracing_write_storage_texture->image);
//
//				PanicIfError(command_buffer->InsertImageMemoryBarrier(src_image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
//				PanicIfError(command_buffer->InsertImageMemoryBarrier(raytracing_read_storage_texture->image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral, image_subresource_range));
//			};
//
//			auto UIPass = [&](CMShared<CommandBuffer> command_buffer)
//			{
//				ScopedGPUProfile(c.GetProfiler(), *command_buffer, "UIPass");
//				vk::CommandBuffer cb = **command_buffer;
//
//				auto cv = std::vector<vk::ClearValue>
//				{
//					vk::ClearValue{}
//						.setColor(vk::ClearColorValue()),
//					vk::ClearValue{}
//						.setDepthStencil(
//							vk::ClearDepthStencilValue{}
//							.setDepth(1.0f)
//							.setStencil(0)
//						)
//				};
//
//				CMShared<RenderPass> rp = c.GetGlobalRenderPass();
//
//				auto render_pass_begin_info = vk::RenderPassBeginInfo{}
//					.setRenderPass(rp->render_pass)
//					.setRenderArea(vk::Rect2D{}
//						.setOffset({ 0, 0 })
//						.setExtent(c.GetWindowExtent())
//					)
//					.setFramebuffer(c.GetFrameCurrentBuffer())
//					.setClearValues(cv);
//
//				cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
//
//				RenderImGui(cb);
//
//				cb.endRenderPass();
//			};
//
//			command_buffer->RecordAndEnd([&](CommandBuffer& _)
//				{
//					vk::CommandBuffer cb = **command_buffer;
//					{
//						ScopedGPUProfile(c.GetProfiler(), *command_buffer, "Frame command buffer");
//
//						cb.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, *raytracing_pipeline);
//						cb.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, pl->pipeline_layout, 0, ds->descriptor_sets, {});
//
//						{
//							vk::StridedDeviceAddressRegionKHR raygen_sbt{};
//							raygen_sbt.setDeviceAddress(sbt_buffer->GetAddress() + sbt.GetRaygenOffset());
//							raygen_sbt.setStride(sbt.GetRaygenStride());
//							raygen_sbt.setSize(sbt.GetRaygenSize());
//
//							vk::StridedDeviceAddressRegionKHR raymiss_sbt{};
//							raymiss_sbt.setDeviceAddress(sbt_buffer->GetAddress() + sbt.GetMissOffset());
//							raymiss_sbt.setStride(sbt.GetMissStride());
//							raymiss_sbt.setSize(sbt.GetMissSize());
//
//							vk::StridedDeviceAddressRegionKHR rayhit_sbt{};
//							rayhit_sbt.setDeviceAddress(sbt_buffer->GetAddress() + sbt.GetHitGroupOffset());
//							rayhit_sbt.setStride(sbt.GetHitGroupStride());
//							rayhit_sbt.setSize(sbt.GetHitGroupSize());
//
//							vk::StridedDeviceAddressRegionKHR callable_sbt{};
//							callable_sbt.setDeviceAddress({});
//							callable_sbt.setStride({});
//							callable_sbt.setSize({});
//
//							/*
//							cb.traceRaysKHR(raygen_sbt, raymiss_sbt, rayhit_sbt, callable_sbt,
//								c.GetWindowExtent().width, c.GetWindowExtent().height, 1);
//
//							ComputeAndCopyImagePass(command_buffer);
//							*/
//							//UIPass(command_buffer);
//
//							{
//								auto cv = std::vector<vk::ClearValue>
//								{
//									vk::ClearValue{}
//										.setColor(vk::ClearColorValue()),
//									vk::ClearValue{}
//										.setDepthStencil(
//											vk::ClearDepthStencilValue{}
//											.setDepth(1.0f)
//											.setStencil(0)
//										)
//								};
//
//								CMShared<RenderPass> rp = c.GetGlobalRenderPass();
//
//								auto render_pass_begin_info = vk::RenderPassBeginInfo{}
//									.setRenderPass(rp->render_pass)
//									.setRenderArea(vk::Rect2D{}
//										.setOffset({ 0, 0 })
//										.setExtent(c.GetWindowExtent())
//									)
//									.setFramebuffer(c.GetFrameCurrentBuffer())
//									.setClearValues(cv);
//
//								cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
//								for (auto& [_, draw_calls] : program_to_draw_calls)
//								{
//									auto& command_buffer_callbacks = draw_calls->GetCommandBufferCallbacks();
//									for (const auto& cb : command_buffer_callbacks)
//									{
//										cb(*command_buffer);
//									}
//								}
//								cb.endRenderPass();
//							}
//						}
//					}
//					CollectGPUProfile(c.GetProfiler(), *command_buffer);
//				}, vk::CommandBufferBeginInfo{});
//
//			//PanicIfError(transfer_qbm.Submit(QueueType::Transfer));
//
//			CMShared<Semaphore> compute_semaphore = AssignOrPanic(render_frame.GetBinarySemaphore());
//			CMShared<Semaphore> gui_semaphore = AssignOrPanic(render_frame.GetBinarySemaphore());
//
//			auto wait_dst_stage_mask = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
//			graphics_qbm.EnqueueCommandBuffer(command_buffer);
//			/*
//			for (auto& [_, draw_calls] : program_to_draw_calls)
//			{
//				auto& command_buffers = draw_calls->GetCommandBuffers();
//				for (const auto& cb : command_buffers)
//				{
//					graphics_qbm.EnqueueCommandBuffer(cb);
//					graphics_qbm.EnqueueWait(wait_dst_stage_mask, present_semaphore);
//					graphics_qbm.EnqueueSignal(render_semaphore);
//				}
//			}
//			*/
//
//			graphics_qbm.EnqueueWait(wait_dst_stage_mask, present_semaphore);
//			graphics_qbm.EnqueueSignal(render_semaphore);
//			PanicIfError(graphics_qbm.Submit(QueueType::Graphics, render_fence));
//
//			const static bool wait_idle = Config["raysterizer"]["wait_idle"];
//			if (wait_idle)
//			{
//				device.waitIdle();
//			}
//
//			auto image_index = c.GetNextFrameIndex();
//
//			{
//				ScopedCPUProfileRaysterizer("Present image");
//				if (auto err = c.Present(render_semaphore, image_index))
//				{
//					if (err.isA<SwapchainOutOfDateError>())
//					{
//						ConsumeError(err);
//						GenerateRaytracingStorageTexture();
//					}
//					else
//					{
//						PanicError(err);
//					}
//				}
//			}
//
//			PanicIfError(render_frame.EndFrame());
//			PanicIfError(c.EndFrame());
//
//			c.AdvanceFrame();
//
//			recursion_depth = updated_recursion_depth;
		}

		Error PipelineManager::CreateGBufferPass(std::size_t id)
		{
			if (auto found = id_to_g_buffer_pass.find(id); found != std::end(id_to_g_buffer_pass))
			{
				return StringError("GBufferPass for id {} already exists", id);
			}
			else
			{
				auto g_buffer_pass = std::make_shared<Pass::GBufferPass>();
				PanicIfError(g_buffer_pass->Setup(common_resources));
				id_to_g_buffer_pass[id] = g_buffer_pass;
			}
			return NoError();
		}

		Error PipelineManager::DeleteGBufferPass(std::size_t id)
		{
			if (auto found = id_to_g_buffer_pass.find(id); found != std::end(id_to_g_buffer_pass))
			{
				const auto& g_buffer_pass = found->second;
				to_be_deleted_g_buffer_pass.emplace_back(g_buffer_pass);
				id_to_g_buffer_pass.erase(found);
			}
			else
			{
				return StringError("GBufferPass for id {} did not exist for deletion", id);
			}
			return NoError();
		}

		Error PipelineManager::SetActiveGBufferPass(std::size_t id)
		{
			AssignOrReturnError(auto g_buffer_pass, GetGBufferPass(id));
			active_g_buffer_pass = g_buffer_pass;
			return NoError();
		}

		Expected<std::shared_ptr<Pass::GBufferPass>> PipelineManager::GetGBufferPass(std::size_t id)
		{
			if (auto found = id_to_g_buffer_pass.find(id); found != std::end(id_to_g_buffer_pass))
			{
				return found->second;
			}
			else
			{
				return StringError("GBufferPass for id {} does not exist", id);
			}
		}

		void PipelineManager::RecordRenderCall(RenderCallRecord render_call_record)
		{
			render_call_records.emplace_back(render_call_record);
		}

		MaterialInfoWithIndex& Raysterizer::MiddleWare::PipelineManager::GetMaterialInfoWithIndex(std::size_t hash)
		{
			RenderFrame& render_frame = c.GetRenderFrame();

			if (auto found = material_info_mapping.find(hash); found != std::end(material_info_mapping))
			{
				return found->second;
			}

			auto index = materials_buffer_index_pool.Get();
			auto material_info_with_index = MaterialInfoWithIndex{ index };
			auto [pair, inserted] = material_info_mapping.try_emplace(hash, material_info_with_index);

			if (materials_buffer)
			{
				if ((materials_buffer->GetSize() / sizeof(Material)) < materials_buffer_index_pool.GetLimit())
				{
					materials_buffer = AssignOrPanic(render_frame.ResizeBuffer(materials_buffer, sizeof(Material) * materials_buffer_index_pool.GetLimit()));
				}
			}
			else
			{
				materials_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eStorageBuffer, sizeof(Material), true));
			}
			
			c.SetName(materials_buffer, "Materials Buffer");

			return pair->second;
		}

		Material& Raysterizer::MiddleWare::PipelineManager::GetMaterialWithIndex(std::size_t hash)
		{
			auto& material_info_with_index = GetMaterialInfoWithIndex(hash);
			auto& material_info = material_info_with_index.m;
			auto& material = materials_buffer->MapAs<Material*>()[material_info_with_index.index];

			if (game_type == GameType::Roblox || game_type == GameType::OSRS || game_type == GameType::Dolphin)
			{
				material.roughness_metallic.r = 1.0f;
				auto material_ptr = materials_buffer->MapAs<Material*>();
				for (auto i = 0; i < materials_buffer_index_pool.GetLimit(); i++)
				{
					material_ptr[i].highlight = false;
				}
			}

			return material;
		}

		void Raysterizer::MiddleWare::PipelineManager::RemoveMaterialInfoWithIndex(std::size_t hash)
		{
			if (auto found = material_info_mapping.find(hash); found != std::end(material_info_mapping))
			{
				const auto& material_info = found->second;
				materials_buffer_index_pool.Remove(material_info.index);
				material_info_mapping.erase(found);
			}
		}

		void PipelineManager::GenerateRaytracingStorageTexture()
		{
			auto vk_image_create_info = vk::ImageCreateInfo{}
				.setImageType(vk::ImageType::e2D)
				.setFormat(c.GetSwapchainFormat())
				.setExtent(c.GetWindowExtent3D())
				.setMipLevels(1)
				.setArrayLayers(1)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setTiling(vk::ImageTiling::eOptimal)
				//.setSharingMode(vk::SharingMode::eExclusive)
				.setInitialLayout(vk::ImageLayout::eUndefined)
				.setUsage(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);

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
				.setFormat(c.GetSwapchainFormat())
				.setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
				.setSubresourceRange
				(
					vk::ImageSubresourceRange{}
						.setAspectMask(vk::ImageAspectFlagBits::eColor)
						.setBaseMipLevel(0)
						.setLevelCount(1)
						.setBaseArrayLayer(0)
						.setLayerCount(1)
				)
			};

			TextureCreateInfo texture_create_info
			{
				image_create_info,
				image_view_create_info,
				{}
			};

			raytracing_write_storage_texture = AssignOrPanic(c.CreateTexture(texture_create_info));
			raytracing_write_storage_texture->image->SetImageLayout(vk::ImageLayout::eGeneral);

			texture_create_info.image_create_info.image_create_info.setFormat(vk::Format::eR32G32B32A32Sfloat);
			texture_create_info.image_view_create_info.image_view_create_info.setFormat(vk::Format::eR32G32B32A32Sfloat);
			raytracing_read_storage_texture = AssignOrPanic(c.CreateTexture(texture_create_info));
			raytracing_read_storage_texture->image->SetImageLayout(vk::ImageLayout::eGeneral);

			PanicIfError(c.ImmediateGraphicsSubmit([=](CommandBuffer& command_buffer)
				{
					auto image_subresource_range = vk::ImageSubresourceRange{}
						.setAspectMask(vk::ImageAspectFlagBits::eColor)
						.setBaseMipLevel(0)
						.setLevelCount(1)
						.setBaseArrayLayer(0)
						.setLayerCount(1);

					auto image_memory_barrier = vk::ImageMemoryBarrier{}
						.setSrcQueueFamilyIndex({})
						.setDstQueueFamilyIndex({})
						.setSrcAccessMask({})
						.setDstAccessMask({})
						.setOldLayout(vk::ImageLayout::eUndefined)
						.setNewLayout(vk::ImageLayout::eGeneral)
						.setSubresourceRange(image_subresource_range)
						.setImage(**raytracing_read_storage_texture->image);

					PanicIfError(command_buffer.InsertImageMemoryBarrier(image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));

					image_memory_barrier = vk::ImageMemoryBarrier{}
						.setSrcQueueFamilyIndex({})
						.setDstQueueFamilyIndex({})
						.setSrcAccessMask({})
						.setDstAccessMask({})
						.setOldLayout(vk::ImageLayout::eUndefined)
						.setNewLayout(vk::ImageLayout::eGeneral)
						.setSubresourceRange(image_subresource_range)
						.setImage(**raytracing_write_storage_texture->image);

					PanicIfError(command_buffer.InsertImageMemoryBarrier(image_memory_barrier, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands));
				}));
		};

		void RaysterizerVulkanState::Setup()
		{
			//auto num_frames = c.GetNumFrames();
			auto num_frames = 1;
			for (auto i = 0; i < num_frames; i++)
			{
				pipeline_managers.emplace_back(PipelineManager{});
				pipeline_managers.back().Setup();
			}
		}

		PipelineManager& RaysterizerVulkanState::GetPipelineManager()
		{
			if (pipeline_managers.empty())
			{
				Setup();
			}
			//return pipeline_managers[c.GetFrameIndex()];
			return pipeline_managers[0];
		}
	}
}