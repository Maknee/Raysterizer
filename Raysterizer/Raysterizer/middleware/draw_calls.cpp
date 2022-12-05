#include "draw_calls.h"

//#define DISABLE_SPIRV_VM

enum DolphinGame
{
	Melee,
	SuperMonkeyBall,
	MetriodPrime,
	PokemonColosseum,
	BeyondGoodAndEvil,
	SimsponsHitAndRun,
	EternalDarkness,
	OOT,
	MetalGearSolid,
	Spongebob,
	Starfox,
	Pikman,
	SoulCaliber,
	Windwaker,
	ResidentEvil4,
	PaperMario,
	MarioParty,
	FireEmblem,
	None
};

DolphinGame dolphin_game = DolphinGame::None;

namespace Raysterizer
{
	namespace MiddleWare
	{
		DrawCalls::~DrawCalls()
		{
			auto& draw_call_to_set = pipeline_manager->GetDrawCallToBindingIndex();
			draw_call_to_set.Remove(shader_binding_index);

			// Sync remaining variables if possible
			
			if (spirv_vm_variable_setup_run_info_sync_dirty)
			{
				for (const auto& [var, _] : spirv_vm_variable_setup_run_info_sync_dirty_variables)
				{
					PanicIfError(SyncVariableInVM(var));
				}
			}

			// unsync the variables...
			for (auto i = 1; i < setup_run_infos.size(); i++)
			{
				auto& spirv_vm_state = setup_run_infos[i].spirv_vm_state;
				for (const auto& [var, _] : spirv_vm_variable_setup_run_info_sync_dirty_variables)
				{
					PanicIfError(spirv_vm->DeassociateUnderlyingVMType(spirv_vm_state, var));
				}
			}

			for (const auto& i : storage_buffer_binding_indices)
			{
				pipeline_manager->GetDrawCallStorageBufferBindingIndexPool().Remove(i);
			}
			for (const auto& i : uniform_buffer_binding_indices)
			{
				pipeline_manager->GetDrawCallUniformBufferBindingIndexPool().Remove(i);
			}
			for (const auto& i : combined_sampler_binding_indices)
			{
				pipeline_manager->GetDrawCallCombinedSamplerBufferBindingIndexPool().Remove(i);
			}

		}

		Error DrawCalls::Init(GLuint program_id_, Raysterizer::OpenGL::VertexShader* vertex_shader_, Raysterizer::OpenGL::FragmentShader* fragment_shader_, PipelineManager* pipeline_manager_)
		{
			vertex_shader = vertex_shader_;
			fragment_shader = fragment_shader_;

			program_id = program_id_;
			pipeline_manager = pipeline_manager_;

			auto& vertex_analyzer = vertex_shader->GetAnalyzer();
			auto& fragment_analyzer = fragment_shader->GetAnalyzer();

			const static bool raysterizer_cache_with_opengl_source = Config["shader"]["cache_with_opengl_source"];
			opengl_glsl_hash = RaysterizerEngine::StdHash(vertex_analyzer.GetSource() + fragment_analyzer.GetSource());
			opengl_glsl_hash_cached_path = fmt::format("{}.spv", opengl_glsl_hash);

			auto vertex_opengl_glsl_hash = RaysterizerEngine::StdHash(vertex_analyzer.GetSource());
			auto vertex_opengl_glsl_hash_cached_path = fmt::format("{}.spv", vertex_opengl_glsl_hash);

			auto fragment_opengl_glsl_hash = RaysterizerEngine::StdHash(fragment_analyzer.GetSource());
			auto fragment_opengl_glsl_hash_cached_path = fmt::format("{}.spv", fragment_opengl_glsl_hash);

			auto& draw_call_to_binding_index = pipeline_manager->GetDrawCallToBindingIndex();
			shader_binding_index = draw_call_to_binding_index.Get();

			const static std::size_t raysterizer_draw_call_set = Config["raysterizer"]["draw_call"]["set"];
			auto current_binding_index = 0;

			shader_converter.uniform_buffers_start_set = raysterizer_draw_call_set;
			shader_converter.uniform_buffers_start_binding = current_binding_index++;

			auto opengl_450_source = vertex_analyzer.GetSource();
			auto opengl_450_analyzer = Raysterizer::Analysis::GLSLAnalyzer();
			PanicIfError(opengl_450_analyzer.Init(opengl_450_source));

			if (!shader_converter.initialized)
			{
				shader_converter.ConvertToVulkanRasterizerShaderSetup(vertex_analyzer, fragment_analyzer);
			}
			if (raysterizer_cache_with_opengl_source)
			{
				if (auto vulkan_vertex_shader_shader_or_err = c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{ {}, vertex_opengl_glsl_hash_cached_path, {} } }))
				{
					vulkan_vertex_shader = *vulkan_vertex_shader_shader_or_err;
					if (auto vulkan_fragment_shader_shader_or_err = c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{ {}, fragment_opengl_glsl_hash_cached_path, {} } }))
					{
						vulkan_fragment_shader = *vulkan_fragment_shader_shader_or_err;
					}
					else
					{
						ConsumeError(vulkan_fragment_shader_shader_or_err.takeError());

						auto [vulkan_vertex_shader_source, fragment_vertex_shader_source, vulkan_tesc_shader_source, vulkan_tese_shader_source] = shader_converter.ConvertToVulkanRasterizerShader(vertex_analyzer, fragment_analyzer);

						AssignOrReturnError(vulkan_vertex_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_vertex_shader_source, {}, ShaderKind::VERT } }));
						AssignOrReturnError(vulkan_fragment_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{fragment_vertex_shader_source, {}, ShaderKind::FRAG } }));
						//AssignOrReturnError(vulkan_tesc_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_tesc_shader_source, {}, ShaderKind::TESC } }));
						//AssignOrReturnError(vulkan_tese_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_tese_shader_source, {}, ShaderKind::TESE } }));
						ReturnIfError(c.GetShaderModuleManager().CacheShaderToDisk(vertex_opengl_glsl_hash, vulkan_vertex_shader));
						ReturnIfError(c.GetShaderModuleManager().CacheShaderToDisk(fragment_opengl_glsl_hash, vulkan_fragment_shader));
					}
				}
				else
				{
					ConsumeError(vulkan_vertex_shader_shader_or_err.takeError());

					auto [vulkan_vertex_shader_source, fragment_vertex_shader_source, vulkan_tesc_shader_source, vulkan_tese_shader_source] = shader_converter.ConvertToVulkanRasterizerShader(vertex_analyzer, fragment_analyzer);

					AssignOrReturnError(vulkan_vertex_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_vertex_shader_source, {}, ShaderKind::VERT } }));
					AssignOrReturnError(vulkan_fragment_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{fragment_vertex_shader_source, {}, ShaderKind::FRAG } }));
					//AssignOrReturnError(vulkan_tesc_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_tesc_shader_source, {}, ShaderKind::TESC } }));
					//AssignOrReturnError(vulkan_tese_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_tese_shader_source, {}, ShaderKind::TESE } }));
					ReturnIfError(c.GetShaderModuleManager().CacheShaderToDisk(vertex_opengl_glsl_hash, vulkan_vertex_shader));
					ReturnIfError(c.GetShaderModuleManager().CacheShaderToDisk(fragment_opengl_glsl_hash, vulkan_fragment_shader));
				}
			}
			else
			{
				auto [vulkan_vertex_shader_source, fragment_vertex_shader_source, vulkan_tesc_shader_source, vulkan_tese_shader_source] = shader_converter.ConvertToVulkanRasterizerShader(vertex_analyzer, fragment_analyzer);
				AssignOrReturnError(vulkan_vertex_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_vertex_shader_source, {}, ShaderKind::VERT } }));
				AssignOrReturnError(vulkan_fragment_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{fragment_vertex_shader_source, {}, ShaderKind::FRAG } }));
				//AssignOrReturnError(vulkan_tesc_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_tesc_shader_source, {}, ShaderKind::TESC } }));
				//AssignOrReturnError(vulkan_tese_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{vulkan_tese_shader_source, {}, ShaderKind::TESE } }));
			}

			auto& render_frame = c.GetRenderFrame();

			dp = pipeline_manager->GetGlobalDescriptorPool();

			std::vector<ShaderModuleCreateInfo> shader_module_create_infos{ 
				vulkan_vertex_shader->shader_module_create_info,
				vulkan_fragment_shader->shader_module_create_info,
				//vulkan_tesc_shader->shader_module_create_info,
				//vulkan_tese_shader->shader_module_create_info,
			};

			const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];
			flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};
			for (const auto& [set, _] : vulkan_vertex_shader->shader_reflection.GetDescriptorResourceMapping())
			{
				variable_set_index_to_count[set] = max_variable_bindings;
			}
			for (const auto& [set, _] : vulkan_fragment_shader->shader_reflection.GetDescriptorResourceMapping())
			{
				variable_set_index_to_count[set] = max_variable_bindings;
			}

			auto plci = PipelineLayoutCreateInfo
			{
				shader_module_create_infos,
				variable_set_index_to_count,
			};
			
			{
				ScopedCPUProfileRaysterizer("PipelineLayoutInfo Creation");
				pli = AssignOrPanic(c.Get(plci));
				c.SetName(pli, fmt::format("Pipeline layout info {}", opengl_glsl_hash));
			}

			CMShared<DescriptorSet> ds{};
			{
				ScopedCPUProfileRaysterizer("DescriptorSet Creation");
				ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
				c.SetName(ds, fmt::format("Descriptor set frame {}", opengl_glsl_hash));
			}

			/*
			PanicIfError(shader_converter.SetupRaytracingGLSLParsing(vertex_shader.GetAnalyzer(), fragment_shader.GetAnalyzer(),
				[&]()
				{
					auto storage_binding = pipeline_manager->GetDrawCallStorageBufferBindingIndexPool().Get();
					storage_buffer_binding_indices.emplace(storage_binding);
					return storage_binding;
				},
				[&]()
				{
					auto uniform_binding = pipeline_manager->GetDrawCallUniformBufferBindingIndexPool().Get();
					uniform_buffer_binding_indices.emplace(uniform_binding);
					return uniform_binding;
				},
				[&]()
				{
					auto combined_sampler = pipeline_manager->GetDrawCallCombinedSamplerBufferBindingIndexPool().Get();
					combined_sampler_binding_indices.emplace(combined_sampler);
					return combined_sampler;
				}
				));
			if (raysterizer_cache_with_opengl_source)
			{
				if (auto reflect_shader_or_err = c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{ {}, opengl_glsl_hash_cached_path, {} } }))
				{
					reflect_shader = *reflect_shader_or_err;
					reflection_has_edited = true;
				}
				else
				{
					ConsumeError(reflect_shader_or_err.takeError());

					raytracing_glsl = shader_converter.RemapOpenGLToRaytracingGLSLUpdated2(vertex_analyzer, fragment_analyzer);
					AssignOrReturnError(reflect_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{raytracing_glsl, {}, ShaderKind::RCHIT } }));
					ReturnIfError(c.GetShaderModuleManager().CacheShaderToDisk(opengl_glsl_hash, reflect_shader));
				}
			}
			else
			{
				raytracing_glsl = shader_converter.RemapOpenGLToRaytracingGLSLUpdated2(vertex_analyzer, fragment_analyzer);
				AssignOrReturnError(reflect_shader, c.Get(ShaderModuleCreateInfo{ ShaderModuleSourceCreateInfo{raytracing_glsl, {}, ShaderKind::RCHIT } }));
			}
			*/

			const auto& vertex_spirv = opengl_450_analyzer.GetSpirv();
			spirv_vm = std::make_unique<Raysterizer::Analysis::SPIRVVirtualMachine>(vertex_spirv);

			auto num_frames = c.GetNumFrames();
			vertex_buffer_cache.SetContext(&c);
			index_buffer_cache.SetContext(&c);
			vbo_cache.SetContext(&c);
			texture_cache.SetContext(&c);

			per_frame_vertex_buffers.resize(num_frames);
			per_frame_index_buffers.resize(num_frames);

			per_frame_out_colors_buffers.resize(num_frames);

			return NoError();
		}

		void DrawCalls::CreateVulkanRasterizerShaders()
		{
			
		}

		void DrawCalls::LoadVertexBufferObject(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
			std::optional<GLuint> first,
			std::optional<GLuint> count)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto& context = Raysterizer::OpenGL::Context::Get();
			auto& state = context.state;
			auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
			auto& vbo = AssignOrPanic(state.GetNonDivisorVertexBufferObjectBoundToVAO());
			const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();

			auto vbo_view = vbo.GetPointerView().AsHashedPointerView();

			auto start_index = first ? *first : 0;
			auto num_elements = count ? *count : vbo_view.GetNumElements();

			auto& vertex_buffers = GetVertexBuffers();
			auto& index_buffers = GetIndexBuffers();

			// check if all vbos are in vap (apply optimization), meaning that there is a single vbo for all vap
			// meaning we can directly use the vbo and map it
			if (vertex_attrib_pointers.empty())
			{
				PANIC("No vertex attribute pointers");
			}
			const auto& first_vap = vertex_attrib_pointers[0];
			auto first_vap_vbo = first_vap.associated_vbo;
			auto first_vap_total_size = first_vap.GetStride();
			auto VboPass = [&](const Raysterizer::OpenGL::VertexAttribPointer& vap)
			{
				return !vap.enabled || (vap.associated_vbo == first_vap_vbo && vap.GetStride() == first_vap_total_size);
			};
			auto all_vbos_match = std::all_of(std::begin(vertex_attrib_pointers), std::end(vertex_attrib_pointers), [&](const auto& e)
				{
					return VboPass(e);
				});
			if (all_vbos_match)
			{
				const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
				const auto& vertex_index_to_pipeline_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

				std::vector<Raysterizer::OpenGL::VertexAttribPointer> sorted_vaps(vertex_attrib_pointers);
				std::sort(std::begin(sorted_vaps), std::end(sorted_vaps),
					[](const auto& e1, const auto& e2)
					{
						return e1.offset < e2.offset;
					});

				flat_hash_map<Raysterizer::Analysis::PipelineInput*, uint32_t> vertex_pipeline_input_to_index;
				for (auto& [index, pipeline_input] : vertex_index_to_pipeline_input)
				{
					vertex_pipeline_input_to_index[pipeline_input] = index;
				}

				flat_hash_map<uint32_t, Raysterizer::Analysis::PipelineInput*> fixed_vertex_pipeline_input_to_index;

				// trace vap that have been already set
				phmap::flat_hash_set<std::size_t> already_set_vap_indices;
				for (auto& [vertex_pipeline_input, i] : vertex_pipeline_input_to_index)
				{
					auto member_index = i;
					if (member_index >= vertex_attrib_pointers.size())
					{
						continue;
					}

					auto vap = vertex_attrib_pointers[member_index];
					while (true)
					{
						while (!vap.enabled)
						{
							member_index++;
							if (member_index >= vertex_attrib_pointers.size())
							{
								PANIC("Out of bounds");
							}
							vap = vertex_attrib_pointers[member_index];
							//continue;
						}
						if (auto found = already_set_vap_indices.find(member_index); found != std::end(already_set_vap_indices))
						{
							member_index++;
							vap = vertex_attrib_pointers[member_index];
							continue;
						}
						break;
					}

					already_set_vap_indices.insert(member_index);
					auto [_, success] = fixed_vertex_pipeline_input_to_index.try_emplace(member_index, vertex_pipeline_input);
					if (!success)
					{
						PANIC("Already inserted");
					}
				}

				std::stringstream modified_vertex_members;
				auto padding_index = 0;

				HashedPointerView vbo_view{};
				auto& buffer = AssignOrPanic(context.buffer_manager.GetBuffer(first_vap_vbo));
				if (auto vbo = std::get_if<Raysterizer::OpenGL::VertexBufferObject>(&buffer))
				{
					vbo_view = vbo->GetPointerView().AsHashedPointerView();
				}
				else
				{
					PANIC("Not VBO");
				}

				vertex_buffer_stride = first_vap.GetStride();
				if (vertex_buffer_stride == 0)
				{
					vertex_buffer_stride = vbo_view.GetStride();
				}

				auto& render_frame = c.GetRenderFrame();

				auto vertex_size = vertex_buffer_stride * num_elements;
				if (pipeline_manager->GetGameType() == GameType::Roblox)
				{
					vertex_size = std::max(4096ull, vertex_size);
				}

				if (vertex_buffers.size() != current_draw_call_index)
				{
					PANIC("Expected vertex buffer to have current draw call index {} != {}", vertex_buffers.size(), current_draw_call_index);
				}

				vertex_buffer_pointer_views.emplace_back(vbo_view);
				
				SizedHashedPointerView sized_hashed_pointer_view{ vbo_view, vertex_size };

				auto vbo_view_hash = vbo_view.Hash();
				BufferWithHash vertex_buffer_with_hash{};
				if (auto found_or_err = vertex_buffer_cache.Get(sized_hashed_pointer_view))
				{
					const auto& e = *found_or_err;
					vertex_buffer_with_hash = e;
					vertex_buffers.emplace_back(e.buffer);
				}
				else
				{
					ConsumeError(found_or_err.takeError());

					auto vertex_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, vertex_size, true));
					c.SetName(vertex_buffer, fmt::format("Vertex buffer {}", vbo_view.Hash()));

					vertex_buffer_with_hash = BufferWithHash{ vbo_view.Hash(), vertex_buffer };
					vertex_buffer_cache.Emplace(sized_hashed_pointer_view, vertex_buffer_with_hash);
					vertex_buffers.emplace_back(vertex_buffer);

					uint8_t* vertex_buffer_data = vertex_buffer->Map();
					if (pipeline_manager->GetGameType() == GameType::Roblox)
					{
						auto vbo_total_size = vbo_view.GetTotalSize();
						if (vertex_size > vbo_total_size)
						{
							vertex_size = vbo_total_size;
						}
					}
					vbo_view.CopyBytesInto(vertex_buffer_data, vertex_size);
				}

				auto& vertex_buffer = vertex_buffer_with_hash.buffer;

				if (vbo_view_hash != vertex_buffer_with_hash.hash)
				{
					// Check if GPU buffer
					if (vertex_buffer->buffer_create_info.vma_allocation_create_info.usage == static_cast<VmaMemoryUsage>(MemoryUsage::GpuOnly))
					{
						vertex_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, vertex_size, true));

						auto& e = AssignOrPanic(vertex_buffer_cache.GetWithCounters(sized_hashed_pointer_view));
						e.t = vertex_buffer_with_hash;
						e.usage_counter = 0;
					}

					uint8_t* vertex_buffer_data = vertex_buffer->Map();

					// extract vbo into buffer
					vbo_view.CopyBytesInto(vertex_buffer_data, vertex_size);
				}

				// Find a vertex input to input into spirv-vm
				{
					const auto& vap = vertex_attrib_pointers[0];

					auto attrib_index = vap.index;

					auto attrib_offset = static_cast<std::size_t>(vap.offset);
					auto attrib_stride = static_cast<std::size_t>(vap.GetTotalSize());

					vertex_position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

					if (vap.type != GL_FLOAT)
					{
						PANIC("Other types not support for vertex {}", vap.type);
					}

					auto index_captured = -1;
					auto* dd = vbo_view.GetDataAs<uint8_t*>();
					auto dd_stride = vap.GetStride();

					static thread_local std::mt19937 rg{ std::random_device{}() };
					auto sample_from_vertices = pipeline_manager->common_resources->ddgi.sample_from_vertices;
					auto sample_from_vertices_amount = pipeline_manager->common_resources->ddgi.sample_from_vertices_amount;
					auto& min_extents = pipeline_manager->common_resources->ddgi.min_extents;
					auto& max_extents = pipeline_manager->common_resources->ddgi.max_extents;

					if (attrib_stride == sizeof(glm::vec2))
					{
						if (sample_from_vertices)
						{
							auto* dd_begin = (glm::vec2*)(dd + attrib_offset);
							auto* dd_end = (glm::vec2*)(dd + (vbo_view.GetNumElements() * dd_stride) + attrib_offset);

							std::vector<glm::vec2> dd_samples(sample_from_vertices_amount);
							std::sample(dd_begin, dd_end, std::begin(dd_samples), sample_from_vertices_amount, rg);

							for (const auto& s : dd_samples)
							{
								for (auto i = 0; i < s.length() && i < min_extents.length(); i++)
								{
									const auto& v = s[i];
									if (!glm::isnan(v))
									{
										min_extents[i] = std::min(min_extents[i], v);
										max_extents[i] = std::max(max_extents[i], v);
									}
								}
							}
						}
						for (auto i = 0; i < vbo_view.GetNumElements(); i++)
						{
							auto data = *(glm::vec2*)(dd + (i * dd_stride) + attrib_offset);
							// skip vertex with 0s in them cause it affects how mvp is calculated
							if (data[0] == 0.0f || glm::any(glm::isnan(data)))
							{
								continue;
							}
							for (auto i = 0; i < data.length(); i++)
							{
								vertex_position[i] = data[i];
							}
							//PanicIfError(spirv_vm->SetVariable(vertex_name, data));
							index_captured = i;
							break;
						}
					}
					else if (attrib_stride == sizeof(glm::vec3))
					{
						if (sample_from_vertices)
						{
							auto* dd_begin = (glm::vec3*)(dd + attrib_offset);
							auto* dd_end = (glm::vec3*)(dd + (vbo_view.GetNumElements() * dd_stride) + attrib_offset);

							std::vector<glm::vec3> dd_samples(sample_from_vertices_amount);
							std::sample(dd_begin, dd_end, std::begin(dd_samples), sample_from_vertices_amount, rg);

							for (const auto& s : dd_samples)
							{
								for (auto i = 0; i < s.length() && i < min_extents.length(); i++)
								{
									const auto& v = s[i];
									if (!glm::isnan(v))
									{
										min_extents[i] = std::min(min_extents[i], v);
										max_extents[i] = std::max(max_extents[i], v);
									}
								}
							}
						}
						for (auto i = 0; i < vbo_view.GetNumElements(); i++)
						{
							auto data = *(glm::vec3*)(dd + (i * dd_stride) + attrib_offset);
							// skip vertex with 0s in them cause it affects how mvp is calculated
							if (data[0] == 0.0f || glm::any(glm::isnan(data)))
							{
								continue;
							}
							for (auto i = 0; i < data.length(); i++)
							{
								vertex_position[i] = data[i];
							}
							//PanicIfError(spirv_vm->SetVariable(vertex_name, data));
							index_captured = i;
							break;
						}
					}
					else if (attrib_stride == sizeof(glm::vec4))
					{
						if (sample_from_vertices)
						{
							auto* dd_begin = (glm::vec4*)(dd + attrib_offset);
							auto* dd_end = (glm::vec4*)(dd + (vbo_view.GetNumElements() * dd_stride) + attrib_offset);

							std::vector<glm::vec4> dd_samples(sample_from_vertices_amount);
							std::sample(dd_begin, dd_end, std::begin(dd_samples), sample_from_vertices_amount, rg);

							for (const auto& s : dd_samples)
							{
								for (auto i = 0; i < s.length() && i < min_extents.length(); i++)
								{
									const auto& v = s[i];
									if (!glm::isnan(v))
									{
										min_extents[i] = std::min(min_extents[i], v);
										max_extents[i] = std::max(max_extents[i], v);
									}
								}
							}
						}
						for (auto i = 0; i < vbo_view.GetNumElements(); i++)
						{
							auto data = *(glm::vec4*)(dd + (i * dd_stride) + attrib_offset);
							if (data[0] == 0.0f || glm::any(glm::isnan(data)))
							{
								continue;
							}
							for (auto i = 0; i < data.length(); i++)
							{
								vertex_position[i] = data[i];
							}
							//PanicIfError(spirv_vm->SetVariable(vertex_name, data));
							index_captured = i;
							break;
						}
					}
					if (sample_from_vertices)
					{
						min_extents *= 3.0;
						max_extents *= 3.0;
					}

					if (std::isnan(vertex_position[0]))
					{
						vertex_position = glm::vec4(1.0f);
						PANIC("Vertex [Size: {}] [captured {}]", glm::to_string(vertex_position), index_captured);
					}
					//vertex_position /= vertex_position.w;

					if (index_captured == -1)
					{
						PANIC("No index for vertex data");
					}
				}
			}
			else
			{
				if (first_vap.divisor != 0)
				{
					PANIC("Expected divisor to be 0");
				}
				vertex_buffer_stride = first_vap.GetStride();

				auto& render_frame = c.GetRenderFrame();
				auto vertex_size = vertex_buffer_stride * num_elements;
				if (pipeline_manager->GetGameType() == GameType::Roblox)
				{
					vertex_size = std::max(4096ull, vertex_size);
				}

				if (vertex_buffers.size() != current_draw_call_index)
				{
					PANIC("Expected vertex buffer to have current draw call index {} != {}", vertex_buffers.size(), current_draw_call_index);
				}

				vertex_buffer_pointer_views.emplace_back(vbo_view);

				SizedHashedPointerView sized_hashed_pointer_view{ vbo_view, vertex_size };
				auto vbo_view_hash = vbo_view.Hash();
				BufferWithHash vertex_buffer_with_hash{};
				if (auto found_or_err = vertex_buffer_cache.Get(sized_hashed_pointer_view))
				{
					const auto& e = *found_or_err;
					vertex_buffer_with_hash = e;
					vertex_buffers.emplace_back(e.buffer);
				}
				else
				{
					ConsumeError(found_or_err.takeError());

					auto vertex_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, vertex_size, true));
					c.SetName(vertex_buffer, fmt::format("Vertex buffer {}", vbo_view.Hash()));

					vertex_buffer_with_hash = BufferWithHash{ vbo_view.Hash(), vertex_buffer };
					vertex_buffer_cache.Emplace(sized_hashed_pointer_view, vertex_buffer_with_hash);
					vertex_buffers.emplace_back(vertex_buffer);

					uint8_t* vertex_buffer_data = vertex_buffer->Map();
					vbo_view.CopyBytesInto(vertex_buffer_data, vertex_size);
				}

				auto& vertex_buffer = vertex_buffer_with_hash.buffer;

				auto GetVertexBufferData = [&]()
				{
					// Check if GPU buffer
					if (vertex_buffer->buffer_create_info.vma_allocation_create_info.usage == static_cast<VmaMemoryUsage>(MemoryUsage::GpuOnly))
					{
						vertex_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, vertex_size, true));

						auto& e = AssignOrPanic(vertex_buffer_cache.GetWithCounters(sized_hashed_pointer_view));
						e.t = vertex_buffer_with_hash;
						e.usage_counter = 0;
					}
					uint8_t* vertex_buffer_data = vertex_buffer->Map();
					return vertex_buffer_data;
				};

				if (vbo_view_hash != vertex_buffer_with_hash.hash)
				{
					uint8_t* vertex_buffer_data = GetVertexBufferData();

					// extract vbo into buffer
					vbo_view.CopyBytesInto(vertex_buffer_data, vertex_size);
				}

				const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
				const auto& vertex_index_to_pipeline_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

				flat_hash_map<const Raysterizer::Analysis::PipelineInput*, uint32_t> vertex_pipeline_input_to_index;
				for (const auto& [index, pipeline_input] : vertex_index_to_pipeline_input)
				{
					vertex_pipeline_input_to_index[pipeline_input] = index;
				}

				// trace vap that have been already set
				phmap::flat_hash_set<std::size_t> already_set_vap_indices;

				// vertex_attrib_pointers could be better than glsl offset...
				for (auto& [i, pipeline_input] : vertex_index_to_pipeline_input)
				{
					auto member_index = i;

					auto offset = pipeline_input->GetOffset();
					auto member_size = pipeline_input->GetElementSize();

					if (member_index >= vertex_attrib_pointers.size())
					{
						continue;
					}

					auto vap = vertex_attrib_pointers[member_index];
					while (true)
					{
						while (!vap.enabled)
						{
							member_index++;
							vap = vertex_attrib_pointers[member_index];
							if (member_index >= vertex_attrib_pointers.size())
							{
								PANIC("Out of bounds");
							}
							//continue;
						}
						if (auto found = already_set_vap_indices.find(member_index); found != std::end(already_set_vap_indices))
						{
							member_index++;
							vap = vertex_attrib_pointers[member_index];
							continue;
						}
						break;
					}
					already_set_vap_indices.insert(member_index);

					auto attrib_index = vap.index;

					auto attrib_offset = static_cast<std::size_t>(vap.offset);
					auto attrib_stride = static_cast<std::size_t>(vap.GetTotalSize());

					// mat4
					if (member_size == sizeof(glm::mat4) && attrib_stride <= sizeof(glm::vec4))
					{
						// combine the size of the next four
						auto result_attribute_stride = 0;
						for (auto i = 0; i < 4; i++)
						{
							result_attribute_stride += attrib_stride;

							if (!vap.enabled)
							{
								PANIC("Vertex attribute should be enabled");
							}
						}
						attrib_stride = result_attribute_stride;
					}

					HashedPointerView vbo_view{};
					auto& buffer = AssignOrPanic(context.buffer_manager.GetBuffer(vap.associated_vbo));
					if (auto vbo = std::get_if<Raysterizer::OpenGL::VertexBufferObject>(&buffer))
					{
						vbo_view = vbo->GetPointerView().AsHashedPointerView();
					}
					else
					{
						PANIC("Not VBO");
					}

					if (vap.divisor > 0)
					{
						// TODO
						// What we want to do is copy this into the vertex buffer
						// but problem is that this is only for ONE vertex instance
						// we want to keep each vertex instance *seperate*
						// One solution: transform the shader to accomadate another buffer for attributes with divisors?
						// Vertex (aPos, textureCoord, aInstanceMatrix) -> Vertex (aPos, textureCoord), InstanceInfo (aInstanceMatrix)?
						// I think this only occurs for attributes that have divisors
						// not for just cases using gl_InstanceId in code
					}
					else
					{
						/*
						uint8_t* vertex_buffer_data = GetVertexBufferData();
						vbo_view.ExtractSubdataInto(vertex_buffer_data, offset, vertex_buffer_stride, attrib_offset, attrib_stride, start_index, num_elements);
						auto vbo_view_stride = vbo_view.GetStride();

						auto* data = vbo_view.GetDataAs<uint8_t*>();
						auto* data2 = (uint8_t*)vertex_buffer_data;
#ifndef NDEBUG
						for (auto i = 0; i <= 25 && i < num_elements; i++)
						{
							auto dd = data + (i * vbo_view_stride) + attrib_offset;
							auto dd2 = data2 + (i * vertex_buffer_stride) + offset;

							if (memcmp(dd, dd2, attrib_stride))
							{
								PANIC("UP {} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X}", i, (uintptr_t)data + attrib_offset, (uintptr_t)data2 + offset, (uintptr_t)dd, (uintptr_t)dd2, vbo_view_stride, vertex_buffer_stride);
							}
						}
#endif
						*/
					}
				}

				// Find a vertex input to input into spirv-vm
				{
					const auto& vap = vertex_attrib_pointers[0];

					auto attrib_index = vap.index;

					auto attrib_offset = static_cast<std::size_t>(vap.offset);
					auto attrib_stride = static_cast<std::size_t>(vap.GetTotalSize());

					vertex_position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

					auto* dd = vbo_view.GetDataAs<uint8_t*>();
					auto dd_stride = vap.GetStride();
					auto index_captured = 0;

					static thread_local std::mt19937 rg{ std::random_device{}() };
					auto sample_from_vertices = pipeline_manager->common_resources->ddgi.sample_from_vertices;
					auto sample_from_vertices_amount = pipeline_manager->common_resources->ddgi.sample_from_vertices_amount;
					auto& min_extents = pipeline_manager->common_resources->ddgi.min_extents;
					auto& max_extents = pipeline_manager->common_resources->ddgi.max_extents;

					if (attrib_stride == sizeof(glm::vec2))
					{
						if (sample_from_vertices)
						{
							auto* dd_begin = (glm::vec2*)(dd + attrib_offset);
							auto* dd_end = (glm::vec2*)(dd + (vbo_view.GetNumElements() * dd_stride) + attrib_offset);

							std::vector<glm::vec2> dd_samples(sample_from_vertices_amount);
							std::sample(dd_begin, dd_end, std::begin(dd_samples), sample_from_vertices_amount, rg);

							for (const auto& s : dd_samples)
							{
								for (auto i = 0; i < s.length() && i < min_extents.length(); i++)
								{
									min_extents[i] = std::min(min_extents[i], s[i]);
									max_extents[i] = std::max(max_extents[i], s[i]);
								}
							}
						}
						for (auto i = 0; i < vbo_view.GetNumElements(); i++)
						{
							auto data = *(glm::vec2*)(dd + (i * vbo_view.GetStride()) + attrib_offset);
							// skip vertex with 0s in them cause it affects how mvp is calculated
							if (data[0] == 0.0f || glm::any(glm::isnan(data)))
							{
								continue;
							}
							for (auto i = 0; i < attrib_stride / sizeof(float); i++)
							{
								vertex_position[i] = data[i];
							}
							//PanicIfError(spirv_vm->SetVariable(vertex_name, data));
							index_captured = i;
							break;
						}
					}
					else if (attrib_stride == sizeof(glm::vec3))
					{
						if (sample_from_vertices)
						{
							auto* dd_begin = (glm::vec3*)(dd + attrib_offset);
							auto* dd_end = (glm::vec3*)(dd + (vbo_view.GetNumElements() * dd_stride) + attrib_offset);

							std::vector<glm::vec3> dd_samples(sample_from_vertices_amount);
							std::sample(dd_begin, dd_end, std::begin(dd_samples), sample_from_vertices_amount, rg);

							for (const auto& s : dd_samples)
							{
								for (auto i = 0; i < s.length() && i < min_extents.length(); i++)
								{
									min_extents[i] = std::min(min_extents[i], s[i]);
									max_extents[i] = std::max(max_extents[i], s[i]);
								}
							}
						}
						for (auto i = 0; i < vbo_view.GetNumElements(); i++)
						{
							auto data = *(glm::vec3*)(dd + (i * vbo_view.GetStride()) + attrib_offset);
							// skip vertex with 0s in them cause it affects how mvp is calculated
							if (data[0] == 0.0f || glm::any(glm::isnan(data)))
							{
								continue;
							}
							for (auto i = 0; i < data.length(); i++)
							{
								vertex_position[i] = data[i];
							}
							//PanicIfError(spirv_vm->SetVariable(vertex_name, data));
							index_captured = i;
							break;
						}
					}
					else if (attrib_stride == sizeof(glm::vec4))
					{
						if (sample_from_vertices)
						{
							auto* dd_begin = (glm::vec4*)(dd + attrib_offset);
							auto* dd_end = (glm::vec4*)(dd + (vbo_view.GetNumElements() * dd_stride) + attrib_offset);

							std::vector<glm::vec4> dd_samples(sample_from_vertices_amount);
							std::sample(dd_begin, dd_end, std::begin(dd_samples), sample_from_vertices_amount, rg);

							for (const auto& s : dd_samples)
							{
								for (auto i = 0; i < s.length() && i < min_extents.length(); i++)
								{
									min_extents[i] = std::min(min_extents[i], s[i]);
									max_extents[i] = std::max(max_extents[i], s[i]);
								}
							}
						}
						for (auto i = 0; i < vbo_view.GetNumElements(); i++)
						{
							auto data = *(glm::vec4*)(dd + (i * vbo_view.GetStride()) + attrib_offset);
							if (data[0] == 0.0f || glm::any(glm::isnan(data)))
							{
								continue;
							}
							for (auto i = 0; i < data.length(); i++)
							{
								vertex_position[i] = data[i];
							}
							//PanicIfError(spirv_vm->SetVariable(vertex_name, data));
							index_captured = i;
							break;
						}
					}
					if (sample_from_vertices)
					{
						min_extents *= 3.0;
						max_extents *= 3.0;
					}

					if (std::isnan(vertex_position[0]))
					{
						vertex_position = glm::vec4(1.0f);

						uintptr_t data = vbo_view.GetDataAs<uintptr_t>();
						uint8_t* vertex_buffer_data = GetVertexBufferData();
						uintptr_t data2 = (uintptr_t)vertex_buffer_data;
						PANIC("Vertex [Size: {}] [captured {}] {} {}", glm::to_string(vertex_position), index_captured, data, data2);
					}
					//vertex_position /= vertex_position.w;
				}
			}
		}

//#define CONVERT_16_TO_32_EBO
#ifdef CONVERT_16_TO_32_EBO

#endif

		void DrawCalls::LoadElementBufferObject(Raysterizer::OpenGL::ElementBufferObject& ebo,
										 	    std::optional<GLuint> first,
									 		    std::optional<GLuint> count)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto ebo_view = ebo.GetPointerView().AsHashedPointerView();

			auto start_index = first ? *first : 0;
			auto num_elements = count ? *count : (ebo_view.GetNumElements());
			
			auto stride = ebo_view.GetStride();

#ifdef CONVERT_16_TO_32_EBO
			if (stride == 2)
			{
				static CacheMappingWithFrameCounter<PointerView, std::vector<uint32_t>> ebo_16_to_32;
				ebo_16_to_32.SetContext(&c);
				if (auto ebo_32_or_err = ebo_16_to_32.Get(ebo_view))
				{
					auto& ebo_32 = *ebo_32_or_err;
					ebo_view = HashedPointerView(ebo_32);
				}
				else
				{
					ConsumeError(ebo_32_or_err.takeError());

					auto ebo_ptr = ebo_view.GetDataAs<uint16_t*>();
					std::vector<GLuint> ebo_32;
					for (auto i = 0; i < ebo_view.GetNumElements(); i++)
					{
						ebo_32.emplace_back(ebo_ptr[i]);
					}
					ebo_16_to_32.Emplace(ebo_view, ebo_32);
					auto& ebo_322 = AssignOrPanic(ebo_16_to_32.Get(ebo_view));
					ebo_view = HashedPointerView(ebo_322);
				}
				ebo_view.ChangeStride(sizeof(GLuint));
				num_elements = count ? *count : (ebo_view.GetNumElements());
				stride = sizeof(GLuint);
			}
#endif

			auto& vertex_buffers = GetVertexBuffers();
			auto& index_buffers = GetIndexBuffers();

			if (count_to_draw.size() <= current_draw_call_index)
			{
				count_to_draw.resize(current_draw_call_index + 1);
			}
			count_to_draw[current_draw_call_index] = num_elements;
			index_buffer_stride = stride;
			/*
			if (!index_buffer_stride)
			{
				index_buffer_stride = stride;
			}
			else
			{
				// check if stride has been changed
				if (index_buffer_stride != stride)
				{
					PANIC("Changed index stride twice!");
				}
			}
			*/

			auto index_size = stride * num_elements;
			if (pipeline_manager->GetGameType() == GameType::Roblox)
			{
				index_size = std::max(4096ull, index_size);
			}

			auto& render_frame = c.GetRenderFrame();
			if (index_buffers.size() != current_draw_call_index)
			{
				PANIC("Expected index buffer to have current draw call index {} != {}", index_buffers.size(), current_draw_call_index);
			}
			
			// Resize view
			//ebo_view = HashedPointerView(ebo_view.GetDataAs<uint8_t*>() + (start_index * stride), stride, num_elements);

			index_buffer_pointer_views.emplace_back(ebo_view);

			SizedHashedPointerView sized_hashed_pointer_view{ ebo_view, index_size };
			auto ebo_view_hash = ebo_view.Hash();
			BufferWithHash index_buffer_with_hash{};
			if (auto found_or_err = index_buffer_cache.Get(sized_hashed_pointer_view))
			{
				const auto& e = *found_or_err;
				index_buffer_with_hash = e;
				index_buffers.emplace_back(e.buffer);
			}
			else
			{
				ConsumeError(found_or_err.takeError());

				auto index_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, index_size, true));
				c.SetName(index_buffer, fmt::format("Index buffer {}", ebo_view.Hash()));

				index_buffer_with_hash = BufferWithHash{ ebo_view.Hash(), index_buffer };
				index_buffer_cache.Emplace(sized_hashed_pointer_view, index_buffer_with_hash);
				index_buffers.emplace_back(index_buffer);

				uint8_t* index_buffer_data = index_buffer->Map();
				ebo_view.CopyBytesInto(index_buffer_data, index_size);
			}

			auto& index_buffer = index_buffer_with_hash.buffer;
			if (ebo_view_hash != index_buffer_with_hash.hash)
			{
				// Check if GPU buffer
				if (index_buffer->buffer_create_info.vma_allocation_create_info.usage == static_cast<VmaMemoryUsage>(MemoryUsage::GpuOnly))
				{
					index_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, index_size, true));

					auto& e = AssignOrPanic(index_buffer_cache.GetWithCounters(sized_hashed_pointer_view));
					e.t = index_buffer_with_hash;
					e.usage_counter = 0;
				}

				uint8_t* index_buffer_data = index_buffer->Map();

				// extract ebo into buffer
				ebo_view.CopyBytesInto(index_buffer_data, index_size);
			}
		}

		void DrawCalls::LoadVertexArrayObject(Raysterizer::OpenGL::VertexArrayObject& vao)
		{
			PANIC("TODO");
		}

		void DrawCalls::SyncCurrentBoundTextures()
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto& context = Raysterizer::OpenGL::Context::Get();
			auto& state = context.state;
			auto& sampler_manager = context.sampler_manager;
			auto& texture_manager = context.texture_manager;
			auto& shader_manager = context.shader_manager;

			auto& render_frame = c.GetRenderFrame();

			auto ds = GetDescriptorSet();
			auto& write_descriptor_sets = ds->GetWriteDescriptorSets();

			auto& active_program = AssignOrPanic(state.GetActiveProgram());
			const auto& active_texture_unit_to_name = active_program.GetActiveTextureUnitToName();
			flat_hash_map<std::string, std::vector<CMShared<Texture>>> name_to_textures;
			for (auto& [active_texture_unit, name] : active_texture_unit_to_name)
			{
				// Set variable inside the vulkan buffer
				const auto& uniform_name_to_info = shader_converter.uniform_name_to_info;
				if (auto found = uniform_name_to_info.find(name); found != std::end(uniform_name_to_info))
				{
					const auto& [_, uniform_info] = *found;
					const auto& transformed_name = uniform_info.transformed_name;

					std::shared_ptr<ShaderReflection::DescriptorResource> descriptor_resource_ptr{};
					if (auto descriptor_resource_or_err = vulkan_vertex_shader->shader_reflection.GetDescriptorResource(transformed_name))
					{
						descriptor_resource_ptr = *descriptor_resource_or_err;
					}
					else
					{
						ConsumeError(descriptor_resource_or_err.takeError());
						if (auto descriptor_resource_or_err = vulkan_fragment_shader->shader_reflection.GetDescriptorResource(transformed_name))
						{
							descriptor_resource_ptr = *descriptor_resource_or_err;
						}
						else
						{
							LogError(descriptor_resource_or_err.takeError());
						}
					}

					auto GetTextureFromTextureId = [&](GLuint texture_unit, GLuint texture_id)
					{
						CMShared<Texture> texture{};

						auto DumpTexture = [&](const HashedPointerView& data_view)
						{
							const static bool dump_all_textures_enable = Config["shader"]["dump_all_textures"]["enable"];
							if (dump_all_textures_enable)
							{
								const static fs::path dump_textures_path = Config["shader"]["dump_all_textures"]["path"];
								const auto file_name = fmt::format("{:016X}.png", data_view.Hash());
								//const auto file_name = fmt::format("{}.png", transformed_name);
								if (!fs::exists(dump_textures_path))
								{
									fs::create_directories(dump_textures_path);
								}
								const auto file_full_path = dump_textures_path / file_name;
								if (!fs::exists(file_full_path))
								{
									if (auto f = std::ofstream(file_full_path, std::ios::binary))
									{
										auto data = data_view.ExtractData();
										f.write(reinterpret_cast<const char*>(data.data()), data.size());
									}
								}
							}
						};

						if (auto opengl_texture_or_err = texture_manager.GetTexture(texture_id))
						{
							auto& opengl_texture = *opengl_texture_or_err;
							if (auto texture_1d = std::get_if<Raysterizer::OpenGL::Texture1D>(&opengl_texture))
							{
								PANIC("TODO");
							}
							else if (auto texture_2d = std::get_if<Raysterizer::OpenGL::Texture2D>(&opengl_texture))
							{
								auto data_view = texture_2d->GetDataView();

								auto width = texture_2d->GetWidth();
								auto height = texture_2d->GetHeight();
								auto depth = texture_2d->GetDepth();

								auto color_format = texture_2d->GetColorFormat();
								auto pixel_data_type = texture_2d->GetPixelDataType();

								// vulkan only can read in stride of 16
								if (data_view.GetStride() == 3 && color_format == GL_RGB)
								{
									constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
									auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
									texture_2d->SetColorFormat(GL_RGBA);
									texture_2d->SetPixelFormat(GL_RGBA);
									PanicIfError(texture_2d->InitializeBuffer());
									PanicIfError(texture_2d->SetData(new_data.data(), width, height, depth));
									data_view = texture_2d->GetDataView();
									color_format = texture_2d->GetColorFormat();
									pixel_data_type = texture_2d->GetPixelDataType();
								}

								vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(color_format, pixel_data_type));

								DumpTexture(data_view);

								if (auto found_or_err = texture_cache.Get(data_view))
								{
									texture = *found_or_err;
								}
								else
								{
									ConsumeError(found_or_err.takeError());

									auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture(format, data_view, width, height, depth));
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
									texture = texture_transfer_job.gpu_texture;
									c.SetName(texture, fmt::format("Texture {} {:016X}", name, data_view.Hash()));

									texture_cache.Emplace(data_view, texture);
								}
							}
							else if (auto texture_2d_array = std::get_if<Raysterizer::OpenGL::Texture2DArray>(&opengl_texture))
							{
								auto data_view = texture_2d_array->GetDataView();

								auto width = texture_2d_array->GetWidth();
								auto height = texture_2d_array->GetHeight();
								auto depth = texture_2d_array->GetDepth();

								auto color_format = texture_2d_array->GetColorFormat();
								auto pixel_data_type = texture_2d_array->GetPixelDataType();

								// vulkan only can read in stride of 16
								if (data_view.GetStride() == 3 && color_format == GL_RGB)
								{
									constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
									auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
									texture_2d_array->SetColorFormat(GL_RGBA);
									texture_2d_array->SetPixelFormat(GL_RGBA);
									PanicIfError(texture_2d_array->InitializeBuffer());
									PanicIfError(texture_2d_array->SetData(new_data.data(), width, height, depth));
									data_view = texture_2d_array->GetDataView();
									color_format = texture_2d_array->GetColorFormat();
									pixel_data_type = texture_2d_array->GetPixelDataType();
								}

								vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(color_format, pixel_data_type));

								DumpTexture(data_view);

								if (auto found_or_err = texture_cache.Get(data_view))
								{
									texture = *found_or_err;
								}
								else
								{
									ConsumeError(found_or_err.takeError());

									auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture2DArray(format, data_view, width, height, depth));
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
									texture = texture_transfer_job.gpu_texture;
									c.SetName(texture, fmt::format("Texture {} {:016X}", name, data_view.Hash()));

									texture_cache.Emplace(data_view, texture);
								}
							}
							else if (auto texture_3d = std::get_if<Raysterizer::OpenGL::Texture3D>(&opengl_texture))
							{
								auto data_view = texture_3d->GetDataView();

								auto width = texture_3d->GetWidth();
								auto height = texture_3d->GetHeight();
								auto depth = texture_3d->GetDepth();

								auto color_format = texture_3d->GetColorFormat();
								auto pixel_data_type = texture_3d->GetPixelDataType();

								// vulkan only can read in stride of 16
								if (data_view.GetStride() == 3 && color_format == GL_RGB)
								{
									constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
									auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
									texture_3d->SetColorFormat(GL_RGBA);
									texture_3d->SetPixelFormat(GL_RGBA);
									PanicIfError(texture_3d->InitializeBuffer());
									PanicIfError(texture_3d->SetData(new_data.data(), width, height, depth));
									data_view = texture_3d->GetDataView();
									color_format = texture_3d->GetColorFormat();
									pixel_data_type = texture_3d->GetPixelDataType();
								}

								vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(color_format, pixel_data_type));

								DumpTexture(data_view);

								if (auto found_or_err = texture_cache.Get(data_view))
								{
									texture = *found_or_err;
								}
								else
								{
									ConsumeError(found_or_err.takeError());

									auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture3D(format, data_view, width, height, depth));
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
									texture = texture_transfer_job.gpu_texture;
									c.SetName(texture, fmt::format("Texture {} {:016X}", name, data_view.Hash()));

									texture_cache.Emplace(data_view, texture);
								}
							}
							else if (auto texture_cube_map = std::get_if<Raysterizer::OpenGL::TextureCubeMap>(&opengl_texture))
							{
								auto data_view = texture_cube_map->GetDataView();

								auto width = texture_cube_map->GetWidth();
								auto height = texture_cube_map->GetHeight();
								auto depth = texture_cube_map->GetDepth();

								auto color_format = texture_cube_map->GetColorFormat();
								auto pixel_data_type = texture_cube_map->GetPixelDataType();

								// vulkan only can read in stride of 16
								if (data_view.GetStride() == 3 && color_format == GL_RGB)
								{
									constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
									auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
									texture_cube_map->SetColorFormat(GL_RGBA);
									texture_cube_map->SetPixelFormat(GL_RGBA);
									PanicIfError(texture_cube_map->InitializeBuffer());
									PanicIfError(texture_cube_map->SetData(new_data.data(), width, height, depth));
									data_view = texture_cube_map->GetDataView();
									color_format = texture_cube_map->GetColorFormat();
									pixel_data_type = texture_cube_map->GetPixelDataType();
								}

								vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(color_format, pixel_data_type));

								DumpTexture(data_view);

								if (auto found_or_err = texture_cache.Get(data_view))
								{
									texture = *found_or_err;
								}
								else
								{
									ConsumeError(found_or_err.takeError());

									auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTextureCubeMap(format, data_view, width, height));
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
									//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
									texture = texture_transfer_job.gpu_texture;
									c.SetName(texture, fmt::format("Texture {} {:016X}", name, data_view.Hash()));

									texture_cache.Emplace(data_view, texture);
								}
							}
							else
							{
								// TODO: implement the rest of the textures, ignore for now
								PANIC("Not supported");
							}
						}
						else
						{
							ConsumeError(opengl_texture_or_err.takeError());
						}

						if (texture)
						{
							if (auto sampler_or_err = sampler_manager.GetSamplerFromTextureUnit(texture_unit - GL_TEXTURE0))
							{
								const auto& sampler = *sampler_or_err;
								auto sampler_create_info = vk::SamplerCreateInfo{}
									.setAddressModeU(OpenGL::Util::GlTexAddressToVkTexAddress(sampler.GetWrapS()))
									.setAddressModeV(OpenGL::Util::GlTexAddressToVkTexAddress(sampler.GetWrapT()))
									.setAddressModeW(OpenGL::Util::GlTexAddressToVkTexAddress(sampler.GetWrapR()))
									.setMinFilter(OpenGL::Util::GlTexFilterToVkTexFilter(sampler.GetMinFilter()))
									.setMagFilter(OpenGL::Util::GlTexFilterToVkTexFilter(sampler.GetMagFilter()))
									.setMipmapMode(OpenGL::Util::GlTexMipMapModeToVkMipMapMode(sampler.GetMipFilter()))
									.setMinLod(sampler.GetMinLod())
									.setMaxLod(sampler.GetMaxLod())
									.setMipLodBias(sampler.GetLodBias())
									.setAnisotropyEnable(true)
									.setMaxAnisotropy(12.0f);

								auto vk_sampler = AssignOrPanic(c.Get(SamplerCreateInfo{ sampler_create_info }));
								texture->sampler = vk_sampler;
							}
							else
							{
								ConsumeError(sampler_or_err.takeError());
							}
						}
						return texture;
					};

					std::vector<CMShared<Texture>> textures;

					if (textures.empty())
					{
						if (auto texture_id_or_err = texture_manager.GetTextureIdFromTextureUnit(active_texture_unit))
						{
							auto texture_id = *texture_id_or_err;
							auto texture = GetTextureFromTextureId(active_texture_unit, texture_id);
							if (texture)
							{
								textures.emplace_back(texture);
							}
						}
						else
						{
							ConsumeError(texture_id_or_err.takeError());
						}
					}

					// TODO: fix hack
					// Check if next active textures are part of the program (ie, it's an array)
					auto start_next_texture_id = active_texture_unit + 1;
					while (true)
					{
						if (auto found = active_texture_unit_to_name.find(start_next_texture_id); found != std::end(active_texture_unit_to_name))
						{
							break;
						}
						if (auto texture_id_or_err = texture_manager.GetTextureIdFromTextureUnit(start_next_texture_id))
						{
							auto texture_id = *texture_id_or_err;
							auto texture = GetTextureFromTextureId(start_next_texture_id, texture_id);
							if (texture)
							{
								textures.emplace_back(texture);
								if (pipeline_manager->GetGameType() == GameType::Dolphin)
								{
									switch (dolphin_game)
									{
									case DolphinGame::Melee:
									case DolphinGame::PokemonColosseum:
									{
										if (textures.size() >= 3)
										{
											static TransferJob white_texture_transfer_job;
											CallOnce
											{
												const auto white_texture_format = vk::Format::eR8G8B8A8Unorm;
												const auto white_texture_width = 256;
												const auto white_texture_height = 256;
												const static std::vector<uint32_t> white_texture_data(white_texture_width * white_texture_height, 0xFFFFFFFF);
												const static PointerView white_texture_pointer_view(white_texture_data);
												white_texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture2DArray(white_texture_format, white_texture_pointer_view, white_texture_width, white_texture_height, 1));
												c.SetName(white_texture_transfer_job.gpu_texture, "White texture");
											};
											textures[1] = white_texture_transfer_job.gpu_texture;
											textures[2] = white_texture_transfer_job.gpu_texture;
										}
										break;
									}
									default:
									{
										break;
									}
									}
								}
							}
						}
						else
						{
							ConsumeError(texture_id_or_err.takeError());
							break;
						}
						start_next_texture_id++;
					}

					if (auto found = name_to_textures.find(name); found != std::end(name_to_textures))
					{
						auto& textures2 = found->second;
						textures2.insert(std::end(textures2), std::make_move_iterator(std::begin(textures)), std::make_move_iterator(std::end(textures)));
					}
					else
					{
						name_to_textures.emplace(name, textures);
					}
				}
			}

			for (const auto& [name, textures] : name_to_textures)
			{
				pipeline_manager->frame_command_buffer->AddDependencyTo(textures);

				// Set variable inside the vulkan buffer
				const auto& uniform_name_to_info = shader_converter.uniform_name_to_info;
				if (auto found = uniform_name_to_info.find(name); found != std::end(uniform_name_to_info))
				{
					const auto& [_, uniform_info] = *found;
					const auto& transformed_name = uniform_info.transformed_name;

					std::shared_ptr<ShaderReflection::DescriptorResource> descriptor_resource_ptr{};
					if (auto descriptor_resource_or_err = vulkan_vertex_shader->shader_reflection.GetDescriptorResource(transformed_name))
					{
						descriptor_resource_ptr = *descriptor_resource_or_err;
					}
					else
					{
						ConsumeError(descriptor_resource_or_err.takeError());
						if (auto descriptor_resource_or_err = vulkan_fragment_shader->shader_reflection.GetDescriptorResource(transformed_name))
						{
							descriptor_resource_ptr = *descriptor_resource_or_err;
						}
						else
						{
							LogError(descriptor_resource_or_err.takeError());
						}
					}

					auto& descriptor_resource = *descriptor_resource_ptr;
					if (auto sampler = std::get_if<ShaderReflection::Sampler>(&descriptor_resource))
					{
						auto& write_descriptor_set_resource = write_descriptor_sets[sampler->set][sampler->binding];
						if (sampler->array_size > 0)
						{
							PanicIfError(ds->Bind(sampler->set, sampler->binding, textures));
							if (textures.size() > sampler->array_size)
							{
								PANIC("Array size is greater");
							}
						}
						else
						{
							if (textures.size() == 1)
							{
								const auto& texture = textures[0];
								PanicIfError(ds->Bind(sampler->set, sampler->binding, texture));
							}
						}
					}
				}
			}


			//for (auto& [active_texture_unit, name] : active_program.GetActiveTextureUnitToName())
			//{
			//	// Set variable inside the vulkan buffer
			//	const auto& uniform_name_to_info = shader_converter.uniform_name_to_info;
			//	if (auto found = uniform_name_to_info.find(name); found != std::end(uniform_name_to_info))
			//	{
			//		const auto& [_, uniform_info] = *found;
			//		const auto& transformed_name = uniform_info.transformed_name;

			//		std::shared_ptr<ShaderReflection::DescriptorResource> descriptor_resource_ptr{};
			//		if (auto descriptor_resource_or_err = vulkan_vertex_shader->shader_reflection.GetDescriptorResource(transformed_name))
			//		{
			//			descriptor_resource_ptr = *descriptor_resource_or_err;
			//		}
			//		else
			//		{
			//			ConsumeError(descriptor_resource_or_err.takeError());
			//			if (auto descriptor_resource_or_err = vulkan_fragment_shader->shader_reflection.GetDescriptorResource(transformed_name))
			//			{
			//				descriptor_resource_ptr = *descriptor_resource_or_err;
			//			}
			//			else
			//			{
			//				LogError(descriptor_resource_or_err.takeError());
			//			}
			//		}

			//		auto GetTextureFromTextureId = [&](GLuint texture_id)
			//		{
			//			CMShared<Texture> texture{};
			//			/*
			//			{
			//				// check to see if the program has the texture bounded
			//				// we do this check because the name itself isn't good enough to identify uniqueness
			//				// two shaders can have the same sampler2D name and thus, the wrong image may be chosen
			//				auto& program = AssignOrPanic(shader_manager.GetProgram(program_id));
			//				const flat_hash_map<GLuint, GLuint>& program_active_textures = program.GetLocationToTextureId();
			//				bool okay = false;
			//				for (const auto& [location, id] : program_active_textures)
			//				{
			//					if (active_texture_unit - GL_TEXTURE0 == id)
			//					{
			//						okay = true;
			//						break;
			//					}
			//				}
			//				if (!okay)
			//				{
			//					continue;
			//				}
			//			}
			//			*/

			//			if (auto opengl_texture_or_err = texture_manager.GetTexture(texture_id))
			//			{
			//				auto& opengl_texture = *opengl_texture_or_err;
			//				if (auto texture_1d = std::get_if<Raysterizer::OpenGL::Texture1D>(&opengl_texture))
			//				{
			//					PANIC("TODO");
			//				}
			//				else if (auto texture_2d = std::get_if<Raysterizer::OpenGL::Texture2D>(&opengl_texture))
			//				{
			//					auto data_view = texture_2d->GetDataView();

			//					// vulkan only can read in stride of 16
			//					if (data_view.GetStride() == 3)
			//					{
			//						constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
			//						auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
			//						texture_2d->SetColorFormat(GL_RGBA);
			//						texture_2d->SetPixelFormat(GL_RGBA);
			//						PanicIfError(texture_2d->InitializeBuffer());
			//						PanicIfError(texture_2d->SetData(new_data.data(), texture_2d->GetWidth(), texture_2d->GetHeight()));
			//						data_view = texture_2d->GetDataView();
			//					}

			//					vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(texture_2d->GetColorFormat(), texture_2d->GetPixelDataType()));

			//					if (auto found_or_err = texture_cache.Get(data_view))
			//					{
			//						texture = *found_or_err;
			//					}
			//					else
			//					{
			//						ConsumeError(found_or_err.takeError());

			//						auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture(format, data_view, texture_2d->GetWidth(), texture_2d->GetHeight(), texture_2d->GetDepth()));
			//						//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
			//						//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
			//						texture = texture_transfer_job.gpu_texture;
			//						c.SetName(texture, fmt::format("Texture {} {:016X}", name, data_view.Hash()));

			//						texture_cache.Emplace(data_view, texture);
			//					}
			//				}
			//				else if (auto texture_2d_array = std::get_if<Raysterizer::OpenGL::Texture2DArray>(&opengl_texture))
			//				{
			//					auto data_view = texture_2d_array->GetDataView();

			//					// vulkan only can read in stride of 16
			//					if (data_view.GetStride() == 3)
			//					{
			//						constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
			//						auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
			//						texture_2d_array->SetColorFormat(GL_RGBA);
			//						texture_2d_array->SetPixelFormat(GL_RGBA);
			//						PanicIfError(texture_2d_array->InitializeBuffer());
			//						PanicIfError(texture_2d_array->SetData(new_data.data(), texture_2d_array->GetWidth(), texture_2d_array->GetHeight(), texture_2d_array->GetDepth()));
			//						data_view = texture_2d_array->GetDataView();
			//					}

			//					vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(texture_2d_array->GetColorFormat(), texture_2d_array->GetPixelDataType()));

			//					if (auto found_or_err = texture_cache.Get(data_view))
			//					{
			//						texture = *found_or_err;
			//					}
			//					else
			//					{
			//						ConsumeError(found_or_err.takeError());

			//						auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture2DArray(format, data_view, texture_2d_array->GetWidth(), texture_2d_array->GetHeight(), texture_2d_array->GetDepth()));
			//						//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
			//						//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
			//						texture = texture_transfer_job.gpu_texture;
			//						c.SetName(texture, fmt::format("Texture {} {:016X}", name, data_view.Hash()));

			//						texture_cache.Emplace(data_view, texture);
			//					}
			//				}
			//				else
			//				{
			//					// TODO: implement the rest of the textures, ignore for now
			//					//PANIC("Not supported");
			//				}
			//			}
			//			else
			//			{
			//				ConsumeError(opengl_texture_or_err.takeError());
			//				return texture;
			//			}
			//			return texture;
			//		};


			//		std::vector<CMShared<Texture>> textures;
			//		/*
			//		if (auto sampler_id_or_err = active_program.GetSamplerFromTextureUnit(active_texture_unit - GL_TEXTURE0))
			//		{
			//			const auto& sampler_id = *sampler_id_or_err;
			//			if (auto sampler_or_err = sampler_manager.GetSampler(sampler_id))
			//			{
			//				auto& sampler = *sampler_or_err;
			//				for (const auto& [texture_unit, texture_id] : sampler.GetTextureUnitToTextureId())
			//				{
			//					auto texture = GetTextureFromTextureId(texture_id);
			//					textures.emplace_back(texture);
			//				}
			//			}
			//			else
			//			{
			//				ConsumeError(sampler_or_err.takeError());
			//			}
			//		}
			//		else
			//		{
			//			ConsumeError(sampler_id_or_err.takeError());
			//		}
			//		*/
			//		if (auto sampler_or_err = sampler_manager.GetSamplerFromTextureUnit(active_texture_unit - GL_TEXTURE0))
			//		{
			//			const auto& sampler = *sampler_or_err;
			//			for (const auto& [texture_unit, texture_id] : sampler.GetTextureUnitToTextureId())
			//			{
			//				auto texture = GetTextureFromTextureId(texture_id);
			//				if (texture)
			//				{
			//					textures.emplace_back(texture);
			//				}
			//			}
			//		}
			//		else
			//		{
			//			ConsumeError(sampler_or_err.takeError());
			//		}

			//		if (textures.empty())
			//		{
			//			if (auto texture_id_or_err = texture_manager.GetTextureIdFromTextureUnit(active_texture_unit))
			//			{
			//				auto texture_id = *texture_id_or_err;
			//				auto texture = GetTextureFromTextureId(texture_id);
			//				if (texture)
			//				{
			//					textures.emplace_back(texture);
			//				}
			//			}
			//			else
			//			{
			//				ConsumeError(texture_id_or_err.takeError());
			//			}
			//		}

			//		pipeline_manager->frame_command_buffer->AddDependencyTo(textures);
			//		auto& descriptor_resource = *descriptor_resource_ptr;
			//		if (auto sampler = std::get_if<ShaderReflection::Sampler>(&descriptor_resource))
			//		{
			//			auto& write_descriptor_set_resource = write_descriptor_sets[sampler->set][sampler->binding];
			//			if (sampler->array_size > 0)
			//			{
			//				PanicIfError(ds->Bind(sampler->set, sampler->binding, textures));
			//				if (textures.size() > sampler->array_size)
			//				{
			//					PANIC("Array size is greater");
			//				}
			//			}
			//			else
			//			{
			//				if (textures.size() == 1)
			//				{
			//					const auto& texture = textures[0];
			//					PanicIfError(ds->Bind(sampler->set, sampler->binding, texture));
			//				}
			//			}
			//		}
			//	}
			//}

			/*
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto& context = Raysterizer::OpenGL::Context::Get();
			auto& texture_manager = context.texture_manager;
			auto& shader_manager = context.shader_manager;

			for (auto& [active_texture_unit, name] : texture_manager.GetActiveTextureUnitToName())
			{
				if (auto texture_id_or_err = texture_manager.GetTextureIdFromTextureUnit(active_texture_unit))
				{
					auto texture_id = *texture_id_or_err;
					{
						// check to see if the program has the texture bounded
						// we do this check because the name itself isn't good enough to identify uniqueness
						// two shaders can have the same sampler2D name and thus, the wrong image may be chosen
						auto& program = AssignOrPanic(shader_manager.GetProgram(program_id));
						const flat_hash_map<GLuint, GLuint>& program_active_textures = program.GetLocationToTextureId();
						bool okay = false;
						for (const auto& [location, id] : program_active_textures)
						{
							if (active_texture_unit - GL_TEXTURE0 == id)
							{
								okay = true;
								break;
							}
						}
						if (!okay)
						{
							continue;
						}
					}

					auto& opengl_texture = AssignOrPanic(texture_manager.GetTexture(texture_id));
					if (auto texture_1d = std::get_if<Raysterizer::OpenGL::Texture1D>(&opengl_texture))
					{
						PANIC("TODO");
					}
					else if (auto texture_2d = std::get_if<Raysterizer::OpenGL::Texture2D>(&opengl_texture))
					{
						auto data_view = texture_2d->GetDataView();

						// vulkan only can read in stride of 16
						if (data_view.GetStride() == 3)
						{
							constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
							auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
							texture_2d->SetColorFormat(GL_RGBA);
							texture_2d->SetPixelFormat(GL_RGBA);
							PanicIfError(texture_2d->InitializeBuffer());
							PanicIfError(texture_2d->SetData(new_data.data(), texture_2d->GetWidth(), texture_2d->GetHeight()));
							data_view = texture_2d->GetDataView();
						}

						vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(texture_2d->GetColorFormat(), texture_2d->GetPixelDataType()));

						if (auto found_or_err = texture_cache.Get(data_view))
						{
							const auto& texture = *found_or_err;
							auto texture_name = GetShaderConverter().AppendHash(name);
							if (auto found = textures.find(texture_name); found != std::end(textures))
							{
								auto& textures_ref = found->second;
								textures_ref.emplace_back(texture);
							}
							else
							{
								textures[texture_name].emplace_back(texture);
							}
						}
						else
						{
							ConsumeError(found_or_err.takeError());

							auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture(format, data_view, texture_2d->GetWidth(), texture_2d->GetHeight()));
							//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
							//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
							auto texture = texture_transfer_job.gpu_texture;
							c.SetName(texture, fmt::format("Texture {}", data_view.Hash()));

							texture_cache.Emplace(data_view, texture);
							auto texture_name = GetShaderConverter().AppendHash(name);
							if (auto found = textures.find(texture_name); found != std::end(textures))
							{
								auto& textures_ref = found->second;
								textures_ref.emplace_back(texture);
							}
							else
							{
								textures[texture_name].emplace_back(texture);
							}
						}
					}
					else if (auto texture_2d_array = std::get_if<Raysterizer::OpenGL::Texture2DArray>(&opengl_texture))
					{
						auto data_view = texture_2d_array->GetDataView();

						// vulkan only can read in stride of 16
						if (data_view.GetStride() == 3)
						{
							constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
							auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
							texture_2d_array->SetColorFormat(GL_RGBA);
							texture_2d_array->SetPixelFormat(GL_RGBA);
							PanicIfError(texture_2d_array->InitializeBuffer());
							PanicIfError(texture_2d_array->SetData(new_data.data(), texture_2d_array->GetWidth(), texture_2d_array->GetHeight(), texture_2d_array->GetDepth()));
							data_view = texture_2d_array->GetDataView();
						}

						vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(texture_2d_array->GetColorFormat(), texture_2d_array->GetPixelDataType()));

						if (auto found_or_err = texture_cache.Get(data_view))
						{
							const auto& texture = *found_or_err;
							auto texture_name = GetShaderConverter().AppendHash(name);
							if (auto found = textures.find(texture_name); found != std::end(textures))
							{
								auto& textures_ref = found->second;
								textures_ref.emplace_back(texture);
							}
							else
							{
								textures[texture_name].emplace_back(texture);
							}
						}
						else
						{
							ConsumeError(found_or_err.takeError());

							auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture(format, data_view, texture_2d_array->GetWidth(), texture_2d_array->GetHeight() * texture_2d_array->GetDepth()));
							//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
							//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
							auto texture = texture_transfer_job.gpu_texture;
							c.SetName(texture, fmt::format("Texture {}", data_view.Hash()));

							texture_cache.Emplace(data_view, texture);
							auto texture_name = GetShaderConverter().AppendHash(name);
							if (auto found = textures.find(texture_name); found != std::end(textures))
							{
								auto& textures_ref = found->second;
								textures_ref.emplace_back(texture);
							}
							else
							{
								textures[texture_name].emplace_back(texture);
							}
						}
					}
					else
					{
						// TODO: implement the rest of the textures, ignore for now
						//PANIC("Not supported");
					}
				}
				else
				{
					llvm::consumeError(texture_id_or_err.takeError());
				}
			}
			*/
		}

		Error DrawCalls::BindBufferInVulkan(std::string var)
		{
			return NoError();
		}

		Error DrawCalls::CopyToBufferInVulkan(std::string var, std::size_t offset, PointerView pointer_view)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();
			auto& render_frame = c.GetRenderFrame();

			offset = 0;

			auto ds = GetDescriptorSet();
			auto& write_descriptor_sets = ds->GetWriteDescriptorSets();

			// Set variable inside the vulkan buffer
			const auto& uniform_name_to_info = shader_converter.uniform_name_to_info;
			if (auto found = uniform_name_to_info.find(var); found != std::end(uniform_name_to_info))
			{
				const auto& [_, uniform_info] = *found;
				auto transformed_name = uniform_info.transformed_name;
				if (transformed_name.empty())
				{
					transformed_name = var;
				}

				std::shared_ptr<ShaderReflection::DescriptorResource> descriptor_resource_ptr{};
				if (auto descriptor_resource_or_err = vulkan_vertex_shader->shader_reflection.GetDescriptorResource(transformed_name))
				{
					descriptor_resource_ptr = *descriptor_resource_or_err;
				}
				else
				{
					ConsumeError(descriptor_resource_or_err.takeError());
					if (auto descriptor_resource_or_err = vulkan_fragment_shader->shader_reflection.GetDescriptorResource(transformed_name))
					{
						descriptor_resource_ptr = *descriptor_resource_or_err;
					}
					else
					{
						return descriptor_resource_or_err.takeError();
					}
				}

				auto& descriptor_resource = *descriptor_resource_ptr;
				if (auto uniform_buffer = std::get_if<ShaderReflection::UniformBuffer>(&descriptor_resource))
				{
					auto& write_descriptor_set_resource = write_descriptor_sets[uniform_buffer->set][uniform_buffer->binding];
					if (auto cm_buffer = std::get_if<CMShared<Buffer>>(&write_descriptor_set_resource.binded_resource))
					{
						auto buffer = *cm_buffer;
						auto size = uniform_buffer->size;
						if (pipeline_manager->GetGameType() == GameType::Roblox)
						{
							size = std::max(4096ull, size);
						}

						if (!buffer)
						{
							buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, size, true));
							c.SetName(buffer, uniform_buffer->name);
						}

						if (auto found = name_to_uniform_buffer.find(var); found != std::end(name_to_uniform_buffer))
						{
							auto& buffers = found->second;
							while (buffers.size() <= current_draw_call_index)
							{
								auto copied_buffer = AssignOrPanic(render_frame.CopyBuffer(buffer));
								c.SetName(copied_buffer, fmt::format("{} index {}", uniform_buffer->name, current_draw_call_index));
								buffers.emplace_back(copied_buffer);
							}
							buffer = buffers[current_draw_call_index];
						}
						else
						{
							std::vector<CMShared<Buffer>> buffers{ buffer };
							name_to_uniform_buffer.try_emplace(var, buffers);
						}

						PanicIfError(ds->Bind(uniform_buffer->set, uniform_buffer->binding, buffer, offset, pointer_view.GetTotalSize()));

						if (pipeline_manager->GetGameType() == GameType::Dolphin)
						{
							PanicIfError(ds->SetPendingWrite(uniform_buffer->set, uniform_buffer->binding, true));
						}

						uint8_t* mapped_buffer = buffer->Map();
						auto offset_mapped_buffer = mapped_buffer + offset;
						memcpy(offset_mapped_buffer, pointer_view.GetData(), pointer_view.GetTotalSize());
					}
				}
				else if (auto storage_buffer = std::get_if<ShaderReflection::StorageBuffer>(&descriptor_resource))
				{
					auto& write_descriptor_set_resource = write_descriptor_sets[uniform_buffer->set][uniform_buffer->binding];
					if (auto cm_buffer = std::get_if<CMShared<Buffer>>(&write_descriptor_set_resource.binded_resource))
					{
						auto buffer = *cm_buffer;
						auto size = uniform_buffer->size;

						if (!buffer)
						{
							buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eStorageBuffer, size, true));
							c.SetName(buffer, storage_buffer->name);
						}

						if (auto found = name_to_uniform_buffer.find(var); found != std::end(name_to_uniform_buffer))
						{
							auto& buffers = found->second;
							while (buffers.size() <= current_draw_call_index)
							{
								auto copied_buffer = AssignOrPanic(render_frame.CopyBuffer(buffer));
								c.SetName(copied_buffer, fmt::format("{} index {}", storage_buffer->name, current_draw_call_index));
								buffers.emplace_back(copied_buffer);
							}
							buffer = buffers[current_draw_call_index];
						}
						else
						{
							std::vector<CMShared<Buffer>> buffers{ buffer };
							name_to_uniform_buffer.try_emplace(var, buffers);
						}

						PanicIfError(ds->Bind(storage_buffer->set, storage_buffer->binding, buffer, offset, pointer_view.GetTotalSize()));

						uint8_t* mapped_buffer = buffer->Map();
						auto offset_mapped_buffer = mapped_buffer + offset;
						memcpy(offset_mapped_buffer, pointer_view.GetData(), pointer_view.GetTotalSize());
					}
				}
				else
				{
					return StringError("Unknown resource");
				}

				descriptor_resource_to_draw_call_index[descriptor_resource_ptr] = current_draw_call_index;

				return NoError();
			}
			else
			{
				return StringError("Setting variable that does not exists: {}", var);
			}
			return NoError();
		}

		Error DrawCalls::SetSampler(std::string_view var, GLuint location, GLuint active_texture_unit)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			auto& context = Raysterizer::OpenGL::Context::Get();
			auto& texture_manager = context.texture_manager;
			auto& shader_manager = context.shader_manager;

			auto& render_frame = c.GetRenderFrame();

			auto ds = GetDescriptorSet();
			auto& write_descriptor_sets = ds->GetWriteDescriptorSets();

			// Set variable inside the vulkan buffer
			const auto& uniform_name_to_info = shader_converter.uniform_name_to_info;
			if (auto found = uniform_name_to_info.find(var); found != std::end(uniform_name_to_info))
			{
				const auto& [_, uniform_info] = *found;
				const auto& transformed_name = uniform_info.transformed_name;

				std::shared_ptr<ShaderReflection::DescriptorResource> descriptor_resource_ptr{};
				if (auto descriptor_resource_or_err = vulkan_vertex_shader->shader_reflection.GetDescriptorResource(transformed_name))
				{
					descriptor_resource_ptr = *descriptor_resource_or_err;
				}
				else
				{
					ConsumeError(descriptor_resource_or_err.takeError());
					if (auto descriptor_resource_or_err = vulkan_fragment_shader->shader_reflection.GetDescriptorResource(transformed_name))
					{
						descriptor_resource_ptr = *descriptor_resource_or_err;
					}
					else
					{
						return descriptor_resource_or_err.takeError();
					}
				}

				auto& descriptor_resource = *descriptor_resource_ptr;
				if (auto sampler = std::get_if<ShaderReflection::Sampler>(&descriptor_resource))
				{
					auto& write_descriptor_set_resource = write_descriptor_sets[sampler->set][sampler->binding];
					if (auto cm_texture = std::get_if<CMShared<Texture>>(&write_descriptor_set_resource.binded_resource))
					{
						auto texture = *cm_texture;

						auto texture_id = active_texture_unit;// AssignOrPanic(texture_manager.GetTextureIdFromTextureUnit(GL_TEXTURE0 + active_texture_unit));
						auto& opengl_texture = AssignOrPanic(texture_manager.GetTexture(texture_id));
						if (auto texture_1d = std::get_if<Raysterizer::OpenGL::Texture1D>(&opengl_texture))
						{
							PANIC("TODO");
						}
						else if (auto texture_2d = std::get_if<Raysterizer::OpenGL::Texture2D>(&opengl_texture))
						{
							auto data_view = texture_2d->GetDataView();

							// vulkan only can read in stride of 16
							if (data_view.GetStride() == 3)
							{
								constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
								auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
								texture_2d->SetColorFormat(GL_RGBA);
								texture_2d->SetPixelFormat(GL_RGBA);
								PanicIfError(texture_2d->InitializeBuffer());
								PanicIfError(texture_2d->SetData(new_data.data(), texture_2d->GetWidth(), texture_2d->GetHeight()));
								data_view = texture_2d->GetDataView();
							}

							vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(texture_2d->GetColorFormat(), texture_2d->GetPixelDataType()));

							if (auto found_or_err = texture_cache.Get(data_view))
							{
								texture = *found_or_err;
							}
							else
							{
								ConsumeError(found_or_err.takeError());

								auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture(format, data_view, texture_2d->GetWidth(), texture_2d->GetHeight()));
								//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
								//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
								texture = texture_transfer_job.gpu_texture;
								c.SetName(texture, fmt::format("Texture {}", data_view.Hash()));

								texture_cache.Emplace(data_view, texture);
							}
						}
						else if (auto texture_2d_array = std::get_if<Raysterizer::OpenGL::Texture2DArray>(&opengl_texture))
						{
							auto data_view = texture_2d_array->GetDataView();

							// vulkan only can read in stride of 16
							if (data_view.GetStride() == 3)
							{
								constexpr auto alpha_value = std::numeric_limits<uint8_t>::max();
								auto new_data = data_view.ExtractDataWithNewStride(4, alpha_value);
								texture_2d_array->SetColorFormat(GL_RGBA);
								texture_2d_array->SetPixelFormat(GL_RGBA);
								PanicIfError(texture_2d_array->InitializeBuffer());
								PanicIfError(texture_2d_array->SetData(new_data.data(), texture_2d_array->GetWidth(), texture_2d_array->GetHeight(), texture_2d_array->GetDepth()));
								data_view = texture_2d_array->GetDataView();
							}

							vk::Format format = vk::Format(Raysterizer::MiddleWare::GlColorFormatToVkColorFormat(texture_2d_array->GetColorFormat(), texture_2d_array->GetPixelDataType()));

							if (auto found_or_err = texture_cache.Get(data_view))
							{
								texture = *found_or_err;
							}
							else
							{
								ConsumeError(found_or_err.takeError());

								auto texture_transfer_job = AssignOrPanic(c.GetRenderFrame().UploadDataToGPUTexture(format, data_view, texture_2d_array->GetWidth(), texture_2d_array->GetHeight() * texture_2d_array->GetDepth()));
								//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.semaphore);
								//qbm.EnqueueWait(vk::PipelineStageFlagBits::eTransfer, texture_transfer_job.graphics_sync_semaphore);
								texture = texture_transfer_job.gpu_texture;
								c.SetName(texture, fmt::format("Texture {}", data_view.Hash()));

								texture_cache.Emplace(data_view, texture);
							}
						}
						else
						{
							// TODO: implement the rest of the textures, ignore for now
							//PANIC("Not supported");
						}

						PanicIfError(ds->Bind(sampler->set, sampler->binding, texture));
					}
				}
				else
				{
					return StringError("Unknown resource");
				}

				descriptor_resource_to_draw_call_index[descriptor_resource_ptr] = current_draw_call_index;

				return NoError();
			}
			else
			{
				return StringError("Setting variable that does not exists: {}", var);
			}
		}

		void DrawCalls::PerformRasterization(vk::PrimitiveTopology primitive_topology, std::optional<GLuint> first,
			std::optional<GLuint> count, std::optional<GLuint> instance_count)
		{
			auto& context = Raysterizer::OpenGL::Context::Get();
			auto& state = context.state;
			auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
			auto& vbo = AssignOrPanic(state.GetNonDivisorVertexBufferObjectBoundToVAO());
			const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			auto& active_program = AssignOrPanic(state.GetActiveProgram());

			auto vbo_view = vbo.GetPointerView().AsHashedPointerView();

			auto start_index = first ? *first : 0;
			auto num_elements = count ? *count : vbo_view.GetNumElements();
			auto num_instances = instance_count ? *instance_count : 1;

			auto& render_frame = c.GetRenderFrame();
			//auto command_buffer = AssignOrPanic(render_frame.GetCommandBuffer(QueueType::Graphics));

			auto& vertex_buffers = GetVertexBuffers();
			auto& index_buffers = GetIndexBuffers();
			auto& index_buffer_pointer_views = GetIndexBufferPointerViews();

			auto& vertex_buffer = vertex_buffers[current_draw_call_index];
			auto& index_buffer = index_buffers[current_draw_call_index];
			auto& index_buffer_pointer_view = index_buffer_pointer_views[current_draw_call_index];

			// This is mapping from vbo -> description (being instanced is based on vbo contents)
			// Ex,
			// VAP index 0 VBO -> vertex buffer
			// ..........4
			// VAP index 5 VBO -> model buffer
			// ..........8
			flat_hash_map<GLuint, vk::VertexInputBindingDescription> vbo_to_vertex_input_binding_descriptions{};
			auto vertex_input_binding_description_index = 0;

			std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions;
			for (auto i = 0; i < vertex_attrib_pointers.size(); i++)
			{
				const auto& vap = vertex_attrib_pointers[i];
				if (!vap.enabled)
				{
					continue;
				}
				auto vap_total_size = vap.GetStride();

				auto vbo = vap.associated_vbo;
				if (auto found = vbo_to_vertex_input_binding_descriptions.find(vbo); found != std::end(vbo_to_vertex_input_binding_descriptions))
				{

				}
				else
				{
					auto vertex_input_rate = vk::VertexInputRate::eInstance;
					if (vap_total_size == vertex_buffer_stride)
					{
						vertex_input_rate = vk::VertexInputRate::eVertex;
					}

					auto new_vertex_input_binding_description = vk::VertexInputBindingDescription{}
						//.setBinding(vertex_input_binding_description_index++)
						.setBinding(vap.index)
						.setStride(vap_total_size)
						.setInputRate(vertex_input_rate);
					vbo_to_vertex_input_binding_descriptions.emplace(vbo, new_vertex_input_binding_description);
				}
			}

			flat_hash_map<GLuint, CMShared<Buffer>> vbo_to_buffer;
			for (const auto& [vbo_id, vertex_input_binding_description] : vbo_to_vertex_input_binding_descriptions)
			{
				auto& vbo = AssignOrPanic(buffer_manager.GetVBO(vbo_id));
				auto vbo_view = vbo.GetPointerView().AsHashedPointerView();
				auto vbo_view_hash = vbo_view.Hash();
				BufferWithHash vertex_buffer_with_hash{};
				SizedHashedPointerView sized_hashed_pointer_view{ vbo_view, vbo_view.GetTotalSize() };

				if (vertex_input_binding_description.binding == 0)
				{
					vbo_to_buffer.emplace(vbo_id, vertex_buffer);
					continue;
				}

				/*
				if (auto found_or_err = vertex_buffer_cache.Get(sized_hashed_pointer_view))
				{
					const auto& e = *found_or_err;
					vbo_to_buffer.emplace(vbo_id, e.buffer);
					continue;
				}
				else
				{
					ConsumeError(found_or_err.takeError());
				}
				*/

				if (auto found_or_err = vbo_cache.Get(vbo_view))
				{
					const auto& e = *found_or_err;
					vbo_to_buffer.emplace(vbo_id, e.buffer);
				}
				else
				{
					ConsumeError(found_or_err.takeError());

					auto total_size = vbo_view.GetTotalSize();
					if (pipeline_manager->GetGameType() == GameType::Roblox)
					{
						total_size = std::max(4096ull, total_size);
					}

					auto buffer_usage_flags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
					auto buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, buffer_usage_flags, total_size, true));
					c.SetName(buffer, fmt::format("VBO {} {}", vbo_id, vbo_view.Hash()));

					uint8_t* buffer_data = buffer->Map();
					vbo_view.CopyBytesInto(buffer_data, vbo_view.GetTotalSize());

					auto buffer_with_hash = BufferWithHash{ vbo_view.Hash(), buffer };
					vbo_cache.Emplace(vbo_view, buffer_with_hash);
					vbo_to_buffer.emplace(vbo_id, buffer);
				}
			}

			// Render
			{
				auto& render_frame = c.GetRenderFrame();
				auto ds = GetDescriptorSet();
				//PanicIfError(render_frame.FlushPendingWrites(ds));

				CMShared<PipelineLayout> pl{};

				{
					ScopedCPUProfileRaysterizer("PipelineLayout Creation");
					pl = AssignOrPanic(c.Get(pli));
					c.SetName(pl, fmt::format("Pipeline layout frame {}", opengl_glsl_hash));
				}
				
				auto& vertex_analyzer = vertex_shader->GetAnalyzer();
				const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
				std::map<std::size_t, const Raysterizer::Analysis::PipelineInput*> index_to_vertex_pipeline_inputs;
				for (const auto& [name, pipeline_input] : vertex_pipeline_inputs)
				{
					const auto& object_reflection = pipeline_input.GetObjectReflection();
					const auto& glsl_type = pipeline_input.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();
					auto reflection_attrib_location = pipeline_input.GetReflectionAttribLocation();
					if (reflection_attrib_location)
					{
						index_to_vertex_pipeline_inputs[*reflection_attrib_location] = &pipeline_input;
					}
					else
					{
						index_to_vertex_pipeline_inputs[pipeline_input.GetIndex()] = &pipeline_input;
					}
				}

				std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions;
				flat_hash_map<GLuint, GLuint> vbo_to_location;
				// this is the index of enabled vaps
				auto enabled_vap_index = 0;
				for (auto i = 0; i < vertex_attrib_pointers.size(); i++)
				{
					const auto& vap = vertex_attrib_pointers[i];
					if (!vap.enabled)
					{
						continue;
					}
					auto vap_total_size = vap.GetStride();
					auto vbo = vap.associated_vbo;

					auto& vertex_input_binding_description = vbo_to_vertex_input_binding_descriptions.find(vbo)->second;

					auto location = vap.index;
					auto offset = vap.offset;
					auto normalized = vap.normalized;

					vk::Format format = vk::Format(Raysterizer::OpenGL::Util::GlAttribPointerToVkFormat(vap.GetNumElements(), vap.type, normalized));

					if (pipeline_manager->GetGameType() == GameType::Dolphin)
					{
						if (format == vk::Format::eR8G8B8A8Uscaled)
						{
							format = vk::Format::eR8G8B8A8Uint;
						}
					}

					if (vertex_input_binding_description.binding == 0)
					{
						vertex_format = format;
					}
					auto matrix_offset = sizeof(float);
					auto matrix_repeat = 1;
					if (auto found = index_to_vertex_pipeline_inputs.find(location); found != std::end(index_to_vertex_pipeline_inputs))
					{
						const auto& vertex_pipeline_input = found->second;
						const auto& object_reflection = vertex_pipeline_input->GetObjectReflection();
						const auto& glsl_type = vertex_pipeline_input->GetGLSLType();
						if (glsl_type->isMatrix())
						{
							matrix_repeat = glsl_type->getMatrixRows();
							matrix_offset = glsl_type->getMatrixCols() * sizeof(float);
						}

						auto maybe_location = vertex_pipeline_input->GetLocation();
						if (maybe_location)
						{
							location = *maybe_location;
						}
						else
						{
							PANIC("Expected location");
						}
					}
					else
					{
						// not actually part of the source that is used
						continue;
						if (index_to_vertex_pipeline_inputs.size() == 1)
						{

						}
						else
						{
							if (0)
							{
								for (const auto& [name, pipeline_input] : vertex_pipeline_inputs)
								{
									fmt::print("{}", 1);
								}
								for (const auto& [a, b] : index_to_vertex_pipeline_inputs)
								{
									const auto& pipeline_input = *b;
									const auto& object_reflection = pipeline_input.GetObjectReflection();
									const auto& glsl_type = pipeline_input.GetGLSLType();
									const auto& qualifier = glsl_type->getQualifier();
									fmt::print("{}", 1);
								}
								for (const auto& [a, b] : active_program.GetAttribMapping())
								{
									fmt::print("{}", a);
								}
							}
							//PANIC("Expected vertex pipeline input for index {}", location);
							location = enabled_vap_index;
						}
					}

					vbo_to_location.try_emplace(vbo, location);
				
					for (auto i = 0; i < matrix_repeat; i++)
					{
						auto desc = vk::VertexInputAttributeDescription{}
							.setBinding(vertex_input_binding_description.binding)
							.setLocation(location)
							.setFormat(format)
							.setOffset(offset);

						location++;
						offset += matrix_offset;

						vertex_input_attribute_descriptions.emplace_back(desc);
						enabled_vap_index++;
					}
				}

				std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions(vbo_to_vertex_input_binding_descriptions.size());
				std::transform(std::begin(vbo_to_vertex_input_binding_descriptions), std::end(vbo_to_vertex_input_binding_descriptions), std::begin(vertex_input_binding_descriptions), [](const auto& e)
					{
						return e.second;
					});
				std::sort(std::begin(vertex_input_binding_descriptions), std::end(vertex_input_binding_descriptions), [](const auto& e1, const auto& e2)
					{
						return e1.binding < e2.binding;
					});

				flat_hash_map<GLuint, CMShared<Buffer>> instance_binding_to_buffers{};
				for (const auto& [vbo, location] : vbo_to_location)
				{
					if (auto found = vbo_to_buffer.find(vbo); found != std::end(vbo_to_buffer))
					{
						auto& buffer = found->second;
						instance_binding_to_buffers[location] = buffer;
					}
					else
					{
						PANIC("Not found for vbo {}", vbo);
					}
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

				auto index_count = num_elements;

				auto& out_color_buffers = GetOutColorBuffers();
				auto color_buffer_size = sizeof(glm::vec4) * num_elements;
				while (out_color_buffers.size() <= current_draw_call_index)
				{
					auto vertex_num_elements = vertex_buffer_pointer_views[current_draw_call_index].GetNumElements();
					if (num_elements != vertex_num_elements)
					{
						//PANIC("{} != {}", num_elements, vertex_num_elements);
					}
					auto out_color_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, color_buffer_size, true));
					out_color_buffers.emplace_back(out_color_buffer);
				}

				auto g_buffer_pass = pipeline_manager->active_g_buffer_pass;

				{
					auto& out_color_buffer = out_color_buffers[current_draw_call_index];
					if (out_color_buffer->GetSize() < color_buffer_size)
					{
						out_color_buffer = AssignOrPanic(render_frame.ResizeBuffer(out_color_buffer, color_buffer_size));
					}
					PanicIfError(ds->Bind(0, 0, out_color_buffer));

					PanicIfError(ds->Bind(0, 1, g_buffer_pass->GetPrevGBuffer2Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));

					struct PushConstants
					{
						vec4 highlight_color;
						float roughness;
						float metallic;
						float unused1;
						float unused2;
					} push_constants;

					auto hash = program_id;

					auto& material = pipeline_manager->GetMaterialWithIndex(hash);
					auto roughness = material.roughness_metallic.r;
					auto metallic = material.roughness_metallic.g;

					push_constants.highlight_color = glm::vec4(material.highlight_color.r, material.highlight_color.g, material.highlight_color.b, static_cast<float>(material.highlight));
					push_constants.roughness = roughness;
					push_constants.metallic = metallic;

					pipeline_manager->frame_command_buffer->Begin();
					PanicIfError(render_frame.BindPushConstant(pipeline_manager->frame_command_buffer, pl, PointerView(push_constants)));
				}

				if (pipeline_manager->GetGameType() == GameType::Roblox)
				{
					CallOnce
					{
						auto raise_exception_address = (uint8_t*)GetModuleHandle(NULL) + 0x2317CC0;
						DWORD unused;
						VirtualProtect(raise_exception_address, 4096, PAGE_EXECUTE_READWRITE, &unused);
						raise_exception_address[0] = 0xC3;
					};

					static bool finish_rendering = false;
					static auto current_frame = c.GetFrame();
					static auto render_count = 0;
					static auto render_until = std::optional<int>();
					auto new_frame = false;
					if (current_frame != c.GetFrame())
					{
						finish_rendering = false;
						render_count = 0;
						current_frame = c.GetFrame();
						new_frame = true;
					}
					render_count++;

					if (render_count > 10)
					{
						//return;
					}

					if (count <= 4)
					{
						//finish_rendering = true;
						//return;
					}

					if (render_until)
					{
						if (*render_until != count)
						{
							return;
						}
						else
						{
							render_until = std::nullopt;
						}
					}
					if (finish_rendering)
					{
						return;
					}
				}
				else if (pipeline_manager->GetGameType() == GameType::Dolphin)
				{
					CallOnce
					{
						const static std::string dolphin_game_name = Config["game"]["dolphin"];
						if (dolphin_game_name == "Melee")
						{
							dolphin_game = DolphinGame::Melee;
						}
						else if (dolphin_game_name == "SuperMonkeyBall")
						{
							dolphin_game = DolphinGame::SuperMonkeyBall;
						}
						else if (dolphin_game_name == "MetriodPrime")
						{
							dolphin_game = DolphinGame::MetriodPrime;
						}
						else if (dolphin_game_name == "PokemonColosseum")
						{
							dolphin_game = DolphinGame::PokemonColosseum;
						}
						else if (dolphin_game_name == "BeyondGoodAndEvil")
						{
							dolphin_game = DolphinGame::BeyondGoodAndEvil;
						}
						else if (dolphin_game_name == "SimsponsHitAndRun")
						{
							dolphin_game = DolphinGame::SimsponsHitAndRun;
						}
						else if (dolphin_game_name == "EternalDarkness")
						{
							dolphin_game = DolphinGame::EternalDarkness;
						}
						else if (dolphin_game_name == "OOT")
						{
							dolphin_game = DolphinGame::OOT;
						}
						else if (dolphin_game_name == "MetalGearSolid")
						{
							dolphin_game = DolphinGame::MetalGearSolid;
						}
						else if (dolphin_game_name == "Spongebob")
						{
							dolphin_game = DolphinGame::Spongebob;
						}
						else if (dolphin_game_name == "Starfox")
						{
							dolphin_game = DolphinGame::Starfox;
						}
						else if (dolphin_game_name == "Pikman")
						{
							dolphin_game = DolphinGame::Pikman;
						}
						else if (dolphin_game_name == "SoulCaliber")
						{
							dolphin_game = DolphinGame::SoulCaliber;
						}
						else if (dolphin_game_name == "Windwaker")
						{
							dolphin_game = DolphinGame::Windwaker;
						}
						else if (dolphin_game_name == "ResidentEvil4")
						{
							dolphin_game = DolphinGame::ResidentEvil4;
						}
						else if (dolphin_game_name == "PaperMario")
						{
							dolphin_game = DolphinGame::PaperMario;
						}
						else if (dolphin_game_name == "MarioParty")
						{
							dolphin_game = DolphinGame::MarioParty;
						}
						else if (dolphin_game_name == "FireEmblem")
						{
							dolphin_game = DolphinGame::FireEmblem;
						}
						else
						{
							dolphin_game = DolphinGame::None;
						}
					};

					static bool finish_rendering = false;
					static auto current_frame = c.GetFrame();
					static auto render_count = 0;
					static auto render_until = std::optional<int>();
					auto new_frame = false;
					if (current_frame != c.GetFrame())
					{
						finish_rendering = false;
						render_count = 0;
						current_frame = c.GetFrame();
						new_frame = true;
					}
					render_count++;
					switch (dolphin_game)
					{
					case DolphinGame::Melee:
					{
						if ((count == 3 || count == 6) && vertex_input_attribute_descriptions.size() == 1)
						{
							return;
						}
						else if ((count == 21) && vertex_input_attribute_descriptions.size() == 1)
						{
							finish_rendering = true;
							return;
						}
						else if ((count == 90) && vertex_input_attribute_descriptions.size() == 3)
						{
							return;
						}
						break;
					}
					case DolphinGame::SuperMonkeyBall:
					{
						if ((count == 702))// && vertex_input_attribute_descriptions.size() == 2)
						{
							finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::MetriodPrime:
					{
						//if ((count == 5934 || count == 456 || count == 498 || count == 228 || count == 594) && vertex_input_attribute_descriptions.size() == 5)
						if (vertex_input_attribute_descriptions.size() == 5)
						{
							return;
						}
						break;
					}
					case DolphinGame::PokemonColosseum:
					{
						if ((count == 3 || count == 6) && vertex_input_attribute_descriptions.size() <= 1)
						{
							return;
						}
						if ((count == 288) && vertex_input_attribute_descriptions.size() == 2)
						{
							finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::BeyondGoodAndEvil:
					{
						if ((count == 168))
						{
							finish_rendering = true;
							return;
						}
						if ((count == 3) && vertex_input_attribute_descriptions.size() <= 1)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::SimsponsHitAndRun:
					{
						if ((count == 744))
						{
							finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::OOT:
					{
						if ((count == 30) && vertex_input_attribute_descriptions.size() <= 2)
						{
							finish_rendering = true;
							return;
						}
						if ((count == 3) && vertex_input_attribute_descriptions.size() <= 1)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::MetalGearSolid:
					{
						if ((count == 3297))
						{
							finish_rendering = true;
							return;
						}
						if ((count == 6144 || count == 2430))
						{
							return;
						}
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::Spongebob:
					{
						if ((count == 3297))
						{
							finish_rendering = true;
							return;
						}
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::Starfox:
					{
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::Pikman:
					{
						if ((count == 24))
						{
							finish_rendering = true;
							return;
						}
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::SoulCaliber:
					{
						if ((count == 6 || count == 12 || count == 24 || count == 138))
						{
							return;
						}
						if ((count == 3) && vertex_input_attribute_descriptions.size() <= 1)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::Windwaker:
					{
						if (new_frame)
						{
							render_until = 114;
						}

						if (render_count < 50 || render_count > 370)
						{
							return;
						}
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 1)
						{
							//finish_rendering = true;
							return;
						}
						if ((count == 36 || count == 42 || count == 48) && vertex_input_attribute_descriptions.size() <= 1)
						{
							//finish_rendering = true;
							return;
						}
						if ((count == 408))// && vertex_input_attribute_descriptions.size() == 2)
						{
							finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::ResidentEvil4:
					{
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
						{
							return;
						}
						if ((count == 864))// && vertex_input_attribute_descriptions.size() == 2)
						{
							finish_rendering = true;
							return;
						}
						break;
					}
					case DolphinGame::PaperMario:
					{
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
						{
							return;
						}
						break;
					}
					case DolphinGame::MarioParty:
					{
						static int index_before_126 = 0;
						static int has_encountered_126 = false;
						static bool has_encountered_36 = false;
						if (new_frame)
						{
							has_encountered_126 = 0;
							has_encountered_36 = false;
						}
						if (count == 126)
						{
							if (has_encountered_126 == 1)
							{
								index_before_126 = render_count - 1;
							}
							has_encountered_126++;
						}
						if (render_count == index_before_126)
						{
							return;
						}
						if (has_encountered_126 && count == 36)
						{
							has_encountered_36 = true;
						}
						if (has_encountered_36)
						{
							if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
							{
								finish_rendering = true;
								return;
							}
						}
						break;
					}
					case DolphinGame::FireEmblem:
					{
						if (count == 432)
						{
							finish_rendering = true;
							return;
						}
						if ((count <= 6) && vertex_input_attribute_descriptions.size() <= 2)
						{
							//finish_rendering = true;
							return;
						}
						break;
					}
					default:
					{
						break;
					}
					}
					if (render_until)
					{
						if (*render_until != count)
						{
							return;
						}
						else
						{
							render_until = std::nullopt;
						}
					}
					if (finish_rendering)
					{
						return;
					}

					if (g_buffer_pass->GetBeginRenderPassExecutionCount() > 3)
					{
						//return;
					}
				}

				std::sort(std::begin(vertex_input_attribute_descriptions), std::end(vertex_input_attribute_descriptions), [](const auto& e1, const auto& e2) { return e1.location < e2.location; });

				// Check if input is really weird for vertex_input_attrib_descriptions
				// this occurs if application is description with offsets into the buffer instead (ppsspp)
				auto description_regular = vertex_input_attribute_descriptions.empty();
				auto vertex_offset = 0;
				if (!vertex_input_attribute_descriptions.empty())
				{
					for (const auto& description : vertex_input_attribute_descriptions)
					{
						if (description.offset == 0)
						{
							description_regular = true;
							break;
						}
					}
				}
				if (!description_regular)
				{
					// have to recalculate every description
					const auto& first_vertex_input_attribute_description = vertex_input_attribute_descriptions[0];
					const auto first_offset = first_vertex_input_attribute_description.offset;

					// change the vertex offset
					vertex_offset = first_offset / vertex_buffer_stride;

					for (auto& description : vertex_input_attribute_descriptions)
					{
						description.offset -= first_offset;
					}
				}
				const auto& raysterizer_vulkan_state = Raysterizer::MiddleWare::RaysterizerVulkanState::Get();
				const auto& render_state = raysterizer_vulkan_state.GetRenderState();

				vk::CompareOp depth_compare_op = render_state.GetDepthFunc();
				auto viewport = vk::Viewport{}
					.setX(0.0f)
					.setY(0.0f)
					.setWidth(c.GetWindowExtent().width)
					.setHeight(c.GetWindowExtent().height)
					.setMinDepth(render_state.GetDepthNear())
					.setMaxDepth(render_state.GetDepthFar());

				g_buffer_pass->Render(pipeline_manager->frame_command_buffer, pl, ds,
					vertex_input_attribute_descriptions, vertex_input_binding_descriptions,
					instance_binding_to_buffers, index_buffer, index_type, primitive_topology, depth_compare_op, viewport, 
					start_index, index_count, num_instances, vertex_offset);

				RenderCallRecord render_call_record;
				render_call_record.g_buffer_pass = g_buffer_pass;
				render_call_record.first = start_index;
				render_call_record.count = index_count;
				render_call_record.instance_count = num_instances;

				pipeline_manager->RecordRenderCall(render_call_record);
			}

			/*
			command_buffer_callbacks.emplace_back(([this, start_index, num_elements, num_instances, vertex_attrib_pointers, vertex_buffer, index_buffer, index_buffer_pointer_view, vbo_to_vertex_input_binding_descriptions, vbo_to_buffer](CommandBuffer& command_buffer)
			//command_buffer->RecordAndEnd(([this, start_index, num_elements, vertex_attrib_pointers, vertex_buffer, index_buffer, index_buffer_pointer_view](CommandBuffer& command_buffer)
				{
					auto& render_frame = c.GetRenderFrame();
					auto& cb = *command_buffer;

					CMShared<DescriptorSet> ds = GetDescriptorSet();
					for (const auto& [set, ss] : ds->GetWriteDescriptorSets())
					{
						for (const auto& [binding, res] : ss)
						{
							DEBUG("{} {}", set, binding);
						}
					}
					PanicIfError(render_frame.FlushPendingWrites(ds));

					CMShared<PipelineLayout> pl{};

					{
						ScopedCPUProfileRaysterizer("PipelineLayout Creation");
						pl = AssignOrPanic(c.Get(pli));
						c.SetName(pl, fmt::format("Pipeline layout frame {}", opengl_glsl_hash));
					}

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

					CMShared<RenderPass> rp = c.GetGlobalRenderPass();

					std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions;
					for (auto i = 0; i < vertex_attrib_pointers.size(); i++)
					{
						const auto& vap = vertex_attrib_pointers[i];
						if (!vap.enabled)
						{
							continue;
						}
						auto vap_total_size = vap.total_size;
						auto vbo = vap.associated_vbo;

						auto& vertex_input_binding_description = vbo_to_vertex_input_binding_descriptions.find(vbo)->second;

						auto location = vap.index;
						auto offset = vap.offset;
						vk::Format format = vk::Format(Raysterizer::OpenGL::Util::GlAttribPointerToVkFormat(vap.GetNumElements(), vap.type, vap.normalized));

						auto desc = vk::VertexInputAttributeDescription{}
							.setBinding(vertex_input_binding_description.binding)
							.setLocation(location)
							.setFormat(format)
							.setOffset(offset);

						vertex_input_attribute_descriptions.emplace_back(desc);
					}

					std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions(vbo_to_vertex_input_binding_descriptions.size());
					std::transform(std::begin(vbo_to_vertex_input_binding_descriptions), std::end(vbo_to_vertex_input_binding_descriptions), std::begin(vertex_input_binding_descriptions), [](const auto& e)
						{
							return e.second;
						});

					auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{}
						.setVertexAttributeDescriptions(vertex_input_attribute_descriptions)
						.setVertexBindingDescriptions(vertex_input_binding_descriptions);

					auto pcbas = vk::PipelineColorBlendAttachmentState{}
						.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
						.setBlendEnable(VK_FALSE);

					auto gpci = GraphicsPipelineCreateInfo
					{
						rp,
						pl,
						vertex_input_state_create_info,
						vk::PipelineInputAssemblyStateCreateInfo{}
							.setTopology(vk::PrimitiveTopology::eTriangleList)
							.setPrimitiveRestartEnable(VK_FALSE),
						vk::Viewport{}
							.setX(0.0f)
							.setY(0.0f)
							.setWidth(c.GetWindowExtent().width)
							.setHeight(c.GetWindowExtent().height)
							.setMinDepth(0.0f)
							.setMaxDepth(1.0f)
						,
						vk::Rect2D{}
							.setOffset({ 0, 0 })
							.setExtent(c.GetWindowExtent()),
						vk::PipelineRasterizationStateCreateInfo{}
							.setDepthClampEnable(VK_FALSE)
							.setRasterizerDiscardEnable(VK_FALSE)
							.setPolygonMode(vk::PolygonMode::eFill)
							.setLineWidth(1.0f)
							.setCullMode(vk::CullModeFlagBits::eBack)
							.setFrontFace(vk::FrontFace::eClockwise)
							.setDepthBiasEnable(VK_FALSE)
							.setDepthBiasConstantFactor(0.0f)
							.setDepthBiasClamp(0.0f)
							.setDepthBiasSlopeFactor(0.0f)
						,
						vk::PipelineColorBlendStateCreateInfo{}
							.setLogicOpEnable(VK_FALSE)
							.setLogicOp(vk::LogicOp::eCopy)
							.setAttachments(pcbas),
						vk::PipelineMultisampleStateCreateInfo{}
							.setRasterizationSamples(vk::SampleCountFlagBits::e1)
							.setSampleShadingEnable(VK_FALSE)
							.setMinSampleShading(1.0f)
							.setPSampleMask(nullptr)
							.setAlphaToCoverageEnable(VK_FALSE)
							.setAlphaToOneEnable(VK_FALSE)
						,
						vk::PipelineDepthStencilStateCreateInfo{}
							.setDepthTestEnable(VK_TRUE)
							.setDepthWriteEnable(VK_TRUE)
							.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
							.setBack(vk::StencilOpState{}.setCompareOp(vk::CompareOp::eAlways))
							.setDepthBoundsTestEnable(VK_FALSE)
							.setMinDepthBounds(0.0f)
							.setMaxDepthBounds(1.0f)
							.setStencilTestEnable(VK_FALSE)
						,
						vk::PipelineCache{}
					};

					gpci.viewport
						.setY(gpci.viewport.height - gpci.viewport.y)
						.setHeight(-gpci.viewport.height);

					gpci.pipeline_rasterization_state_create_info
						.setCullMode(vk::CullModeFlagBits::eNone)
						.setFrontFace(vk::FrontFace::eClockwise);

					CMShared<GraphicsPipeline> gp{};

					{
						ScopedCPUProfileRaysterizer("GraphicsPipeline Creation");
						gp = AssignOrPanic(c.Get(gpci));
						c.SetName(gp, fmt::format("GraphicsPipeline {}", opengl_glsl_hash));
					}

					auto render_pass_begin_info = vk::RenderPassBeginInfo{}
						.setRenderPass(rp->render_pass)
						.setRenderArea(vk::Rect2D{}
							.setOffset({ 0, 0 })
							.setExtent(c.GetWindowExtent())
						)
						.setFramebuffer(c.GetFrameCurrentBuffer())
						.setClearValues(cv);

					auto window_extent = c.GetWindowExtent();

					cb.setViewport(0, vk::Viewport{0, 0, static_cast<float>(window_extent.width), static_cast<float>(window_extent.height), 0.0f, 1.0f});
					cb.setScissor(0, vk::Rect2D(window_extent.width, window_extent.height));

					//cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

					cb.bindPipeline(vk::PipelineBindPoint::eGraphics, gp->pipeline);
					cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pl->pipeline_layout, 0, ds->descriptor_sets, {});
					
					//for (auto i = 0; i < vertex_buffers.size(); i++)
					{
						//auto& vertex_buffer = vertex_buffers[current_draw_call_index];
						//auto& index_buffer = index_buffers[current_draw_call_index];
						//auto& index_buffer_pointer_view = index_buffer_pointer_views[current_draw_call_index];

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

						auto vertex_offset = 0;
						//cb.bindVertexBuffers(0, **vertex_buffer, vertex_offset);
						for (const auto& [vbo, vertex_input_binding_description] : vbo_to_vertex_input_binding_descriptions)
						{
							if (auto found = vbo_to_buffer.find(vbo); found != std::end(vbo_to_buffer))
							{
								auto& buffer = found->second;
								cb.bindVertexBuffers(vertex_input_binding_description.binding, **buffer, vertex_offset);
							}
							else
							{
								PANIC("Not found for vbo {}", vbo);
							}
						}

						cb.bindIndexBuffer(**index_buffer, vertex_offset, index_type);

						cb.drawIndexed(index_buffer_pointer_view.GetNumElements(), num_instances, start_index, vertex_offset, start_index);
					}

					//cb.endRenderPass();
				}));

			//c.GetDevice().waitIdle();

			//command_buffers.emplace_back(command_buffer);
			*/
		}

		void DrawCalls::IncrementDrawCount()
		{
			current_draw_call_index++;
		}

		void DrawCalls::ResetDrawCount()
		{
			current_draw_call_index = 0;
			transformation_results.clear();

			for (auto& [_, draw_call_index] : descriptor_resource_to_draw_call_index)
			{
				draw_call_index = 0;
			}

			auto& vertex_buffers = GetVertexBuffers();
			auto& index_buffers = GetIndexBuffers();

			vertex_buffer_pointer_views.clear();
			index_buffer_pointer_views.clear();
			vertex_buffers.clear();
			index_buffers.clear();
			textures.clear();

			const static FrameCounter pipeline_release_entries_past_frame_counter_difference = Config["raysterizer"]["pipeline_release_entries_past_frame_counter_difference"];
			vertex_buffer_cache.ClearEntriesPastFrameCounterDifference(pipeline_release_entries_past_frame_counter_difference);
			index_buffer_cache.ClearEntriesPastFrameCounterDifference(pipeline_release_entries_past_frame_counter_difference);
			vbo_cache.ClearEntriesPastFrameCounterDifference(pipeline_release_entries_past_frame_counter_difference);
			texture_cache.ClearEntriesPastFrameCounterDifference(pipeline_release_entries_past_frame_counter_difference);

			command_buffers.clear();
			command_buffer_callbacks.clear();
			draw_call_index_ds.clear();
			current_ds = nullptr;
		}

		void DrawCalls::CalculateModelViewProjectionFromOpenGLSourceSetup()
		{
			const auto& uniform_buffer_prefix = shader_converter.uniform_buffer_prefix;
			const auto& uniform_struct_prefix = shader_converter.uniform_struct_prefix;
			const auto& uniform_member_names = shader_converter.typical_uniform_names;

			auto& vertex_buffers = GetVertexBuffers();
			auto& index_buffers = GetIndexBuffers();

			for (const auto& member : uniform_member_names)
			{
				PANIC("UNexpected");

				auto vertex_struct_name = fmt::format("{buffer}[{index}].{member}",
					"buffer"_a = uniform_buffer_prefix,
					"index"_a = current_draw_call_index,
					"member"_a = member);

				auto& vertex_buffer_desciptor_resource = AssignOrPanic(reflect_shader->shader_reflection.GetDescriptorResourceAs<ShaderReflection::StorageBuffer>(vertex_struct_name));
				auto& vertex_buffer = vertex_buffers[current_draw_call_index];
				auto found_member = AssignOrPanic(vertex_buffer_desciptor_resource.FindMember(member));
				auto offset = found_member.offset;

				uint8_t* vertex_buffer_data = vertex_buffer->Map();

				//auto datas = AssignOrPanic(spirv_vm->GetVariableInBytes(member));
				auto datas = AssignOrPanic(spirv_vm->GetVariableMat4(setup_run_infos[0].spirv_vm_state, member));
				memcpy(vertex_buffer_data + offset, &datas, sizeof(datas));
			}
		}

		// https://www.scratchapixel.com/code.php?id=4&origin=/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix
		void PerformPerspectiveCorrect(glm::mat4& mvp, const glm::vec4& position)
		{
			auto& context = Raysterizer::OpenGL::Context::Get();
			if (!context.state.GetPerformPerspectiveCorrection())
			{
				return;
			}

			for (auto i = 0; i < 4; i++)
			{
				mvp[i][2] += mvp[i][3];
			}

			return;
			auto w = 0.0f;
			for (auto i = 0; i < 4; i++)
			{
				w += position[i] * mvp[i][3];
			}

			if (w != 1.0f)
			{
				mvp /= w;
			}
		}

		void DrawCalls::CheckForOrtho(std::vector<Raysterizer::Analysis::RunResult>& mvps)
		{
			// check for ortho
			const auto& mvp = mvps[0].model;
			if (0 && mvp[0][1] == 0.0f && mvp[0][2] == 0.0f && mvp[0][3] == 0.0f && mvp[1][0] == 0.0f &&
				mvp[1][2] == 0.0f && mvp[1][3] == 0.0f && mvp[2][0] == 0.0f && mvp[2][1] == 0.0f &&
				mvp[2][3] == 0.0f && mvp[3][3] == 1.0f)
			{
				if (mvp[2][2] != 0.0f && mvp[3][0] != 0.0f && mvp[3][1] != 0.0f)
				{
					//matrix[2][2] = -2 / (far - near)
					//(far - near) = -2 / matrix[2][2]
					//

					/*
					matrix[3][2] = -(far + near) / (far - near)
					matrix[3][2] * (far - near) = -(far + near)
					-(matrix[3][2] * (far - near)) = far + near //sub in
					-(matrix[3][2] * (-2 / matrix[2][2])) = far + near
					-(matrix[3][2] * (-2 / matrix[2][2])) - near = far
					far = -(matrix[3][2] * (-2 / matrix[2][2])) - near

					// sub back
					(far - near) = -2 / matrix[2][2]
					(-(matrix[3][2] * (-2 / matrix[2][2])) - near - near)  = -2 / matrix[2][2]
					-(matrix[3][2] * (-2 / matrix[2][2])) - 2 * near  = -2 / matrix[2][2]
					-(matrix[3][2] * (-2 / matrix[2][2])) - 2 * near  = -2 / matrix[2][2]
					matrix[3][2] * 2 / matrix[2][2]) - 2 * near  = -2 / matrix[2][2]
					-2 * near  = (-2 / matrix[2][2]) - (matrix[3][2] * 2 / matrix[2][2])
					-2 * near  = -((2 + matrix[3][2] * 2) / matrix[2][2])
					near = -((2 + matrix[3][2] * 2) / matrix[2][2]) / -2
					near = ((1 + matrix[3][2]) / matrix[2][2])

					(far - near) = -2 / matrix[2][2]
					far = (-2 / matrix[2][2]) + near;
					*/
					auto z_near = ((1.0f + mvp[3][2]) / mvp[2][2]);
					auto z_far = ((-2.0f / mvp[2][2]) + z_near);

					auto& context = Raysterizer::OpenGL::Context::Get();
					auto& state = context.state;
					if (!state.GetPerformPerspectiveCorrection())
					{
						state.SetZNear(z_near);
						state.SetZFar(z_far);
					}
				}
			}
		};

		void DrawCalls::ExecuteVM
		(
			const Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
			const std::vector<Raysterizer::OpenGL::VertexAttribPointer>& vertex_attrib_pointers,
			std::vector<Raysterizer::Analysis::RunResult>& mvps,
			std::size_t i
		)
		{
			const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
			const auto& vertex_index_to_pipeline_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

			auto& setup_run_info = setup_run_infos[i];
			const auto& vertex_name = shader_converter.opengl_vertex_name;
			const auto& cached_vertex_data = AssignOrPanic(spirv_vm->GetVariableInBytesRef(setup_run_info.spirv_vm_state, vertex_name));
			for (auto i = 0; i < cached_vertex_data.size(); i++)
			{
				*(cached_vertex_data[i]) = vertex_position[i];
			}

			const auto& gl_InstanceID_ptr = AssignOrPanic(spirv_vm->GetVariableInBytesRef(setup_run_info.spirv_vm_state, "gl_InstanceID"));
			*(int*)(gl_InstanceID_ptr[0]) = 0;

			//auto mvp = AssignOrPanic(spirv_vm->RunAfterSetup(setup_run_info));
			auto mvp = AssignOrPanic(spirv_vm->CacheSetupRunInfo(setup_run_info));
			PerformPerspectiveCorrect(mvp.model, setup_run_info.common_info->original_position);
			mvps[i] = mvp;
			CheckForOrtho(mvps);
		};

		void DrawCalls::ExecuteVMSyncVertexAttribPointers
		(
			const Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
			const std::vector<Raysterizer::OpenGL::VertexAttribPointer>& vertex_attrib_pointers,
			std::vector<Raysterizer::Analysis::RunResult>& mvps,
			std::size_t i
		)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();

			const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
			const auto& vertex_index_to_pipeline_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

			auto& context = Raysterizer::OpenGL::Context::Get();

			auto& setup_run_info = setup_run_infos[i];
			const auto& vertex_name = shader_converter.opengl_vertex_name;
			const auto& cached_vertex_data = AssignOrPanic(spirv_vm->GetVariableInBytesRef(setup_run_info.spirv_vm_state, vertex_name));
			for (auto i = 0; i < cached_vertex_data.size(); i++)
			{
				*(cached_vertex_data[i]) = vertex_position[i];
			}

			const auto& gl_InstanceID_ptr = AssignOrPanic(spirv_vm->GetVariableInBytesRef(setup_run_info.spirv_vm_state, "gl_InstanceID"));
			*(int*)(gl_InstanceID_ptr[0]) = 0;

			for (const auto& [index, pipeline_input] : vertex_index_to_pipeline_input)
			{
				const auto& vap = vertex_attrib_pointers[index];
				if (vap.divisor > 0)
				{
					auto attrib_offset = static_cast<std::size_t>(vap.offset);
					auto attrib_stride = static_cast<std::size_t>(vap.GetTotalSize());

					auto& buffer = AssignOrPanic(context.buffer_manager.GetBuffer(vap.associated_vbo));
					if (auto vbo = std::get_if<Raysterizer::OpenGL::VertexBufferObject>(&buffer))
					{
						auto vbo_view = vbo->GetPointerView();
						auto* start_data = vbo_view.GetDataAs<uint8_t*>();
						auto* data = (start_data + attrib_offset) + (vbo_view.GetStride() * i);

						const auto& name = pipeline_input->GetName();
						const auto& vap_ptr = AssignOrPanic(spirv_vm->GetVariableInBytesRef(setup_run_info.spirv_vm_state, name));

						float* data_as_float = (float*)data;
						for (auto i = 0; i < vap_ptr.size(); i++)
						{
							auto* vap_data = vap_ptr[i];
							*vap_data = data_as_float[i];
						}
					}
					else
					{
						PANIC("Not VBO");
					}
				}
			}

			//auto mvp = AssignOrPanic(spirv_vm->RunAfterSetup(setup_run_info));
			auto mvp = AssignOrPanic(spirv_vm->CacheSetupRunInfo(setup_run_info));
			PerformPerspectiveCorrect(mvp.model, setup_run_info.common_info->original_position);
			mvps[i] = mvp;
			if (i == 0)
			{
				CheckForOrtho(mvps);
			}
		}

		Error DrawCalls::SyncVariableInVM(std::string_view var)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();
#ifdef DISABLE_SPIRV_VM
			return NoError();
#endif

			if (!setup_run_infos.empty())
			{
				if (spirv_vm_variable_setup_run_info_sync_dirty)
				{
					spirv_vm_variable_setup_run_info_sync_dirty = false;

					// Set variable inside the vm
					auto& first_spirv_vm_state = setup_run_infos[0].spirv_vm_state;
					for (auto i = 1; i < setup_run_infos.size(); i++)
					{
						auto& spirv_vm_state = setup_run_infos[i].spirv_vm_state;
						ReturnIfError(spirv_vm->AssociateUnderlyingVMType(spirv_vm_state, first_spirv_vm_state, var));
					}

					spirv_vm_variable_setup_run_info_sync_dirty_variables[var] = true;
				}
			}

			return NoError();
		}

		void DrawCalls::SaveModelViewProjectionMatrixForCurrentDrawCall(
			Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
			std::optional<GLsizei> instance_count)
		{
			ScopedCPUProfileRaysterizerCurrentFunction();
#ifdef DISABLE_SPIRV_VM
			return;
#endif

			auto& context = Raysterizer::OpenGL::Context::Get();
			auto& state = context.state;
			auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
			const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();

			if (transformation_results.size() < current_draw_call_index + 1)
			{
				transformation_results.resize(current_draw_call_index + 1);
			}

			while (draw_call_tasks.size() < current_draw_call_index + 1)
			{
				spirv_vm_variable_setup_run_info_sync_dirty = true;
				draw_call_tasks.emplace_back(std::make_unique<DrawCallTask>());
			}

			//auto mvp = CalculateModelViewProjectionFromOpenGLSource();
			//return;
			CalculateModelViewProjectionFromOpenGLSourceSetup();

			// auto saved_state = spirv_vm->SaveState(run_setup_info.saved_state.get());
			
			if (instance_count == std::nullopt)
			{
				instance_count = 1;
			}
			if (instance_count > setup_run_infos.size())
			{
				auto previous_setup_run_size = setup_run_infos.size();
				setup_run_infos.resize(*instance_count);
				for (auto i = previous_setup_run_size; i < instance_count; i++)
				{
					setup_run_infos[i] = spirv_vm->SetupRun();
				}
			}

			static bool run_async = Config["raysterizer"]["mvp_analysis"]["run_async"];

			if (0 && run_async)
			{
				executor.wait_for_all();
			}

			auto& mvps = transformation_results[current_draw_call_index];
			mvps.resize(*instance_count);

			auto& draw_call_task = draw_call_tasks[current_draw_call_index];
			tf::Taskflow& taskflow = draw_call_task->taskflow;
			tf::Task& task = draw_call_task->task;
			tf::Future<void>& execution = draw_call_task->execution;
			
			taskflow.clear();
			
			if (instance_count == 1)
			{
				ScopedCPUProfileRaysterizer("Compute MVP with single instance");

				auto i = 0;

				if (run_async)
				{
					std::size_t draw_call_index = current_draw_call_index;
					task = taskflow.emplace([&, i, draw_call_index]()
					{
						auto& mvp = transformation_results[draw_call_index];
						ExecuteVM(vertex_analyzer, vertex_attrib_pointers, mvps, i);
					});
				}
				else
				{
					ExecuteVM(vertex_analyzer, vertex_attrib_pointers, mvps, i);
				}
			}
			else
			{
				ScopedCPUProfileRaysterizer("Compute MVP with multiple instances");

				if (run_async)
				{
					//executor.silent_async(ExecuteVM, vertex_pipeline_inputs, vertex_index_to_pipeline_input, vertex_attrib_pointers, mvps, i);
					int start = 0;
					int end = *instance_count;
					//auto chunk_size = std::ceil(static_cast<float>(end) / static_cast<float>(executor.num_workers()));
					auto chunk_size = 0;
					task = taskflow.for_each_index_guided(start, end, 1, [&](int i)
					{
						ExecuteVMSyncVertexAttribPointers(vertex_analyzer, vertex_attrib_pointers, mvps, i);
					}, chunk_size);
					/*
					for (auto i = start; i < end; i++)
					{
						executor.silent_async([i]()
							{
								ExecuteVMSyncVertexAttribPointers(vertex_analyzer, vertex_attrib_pointers, mvps, i);
							});
					}
					*/
				}
				else
				{
					for (auto i = 0; i < instance_count; i++)
					{
						ExecuteVMSyncVertexAttribPointers(vertex_analyzer, vertex_attrib_pointers, mvps, i);
					}
				}
			}

			if (run_async)
			{
				execution = executor.run(taskflow);
			}
		}
	}
}