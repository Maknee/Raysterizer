#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace MiddleWare
	{
		struct BufferWithHash
		{
			XXH64_hash_t hash{};
			CMShared<Buffer> buffer{};
		};

		struct SizedHashedPointerView
		{
			HashedPointerView ptr_view;
			std::size_t size;

			bool operator==(const SizedHashedPointerView& other) const
			{
				return ptr_view == other.ptr_view &&
					size == other.size;
			}
			bool operator!=(const SizedHashedPointerView& other) const
			{
				return !(*this == other);
			}
		};
	}
}

namespace std
{
	using namespace Raysterizer::MiddleWare;

	template <>
	struct hash<SizedHashedPointerView>
	{
		std::size_t operator()(const SizedHashedPointerView& o) const
		{
			auto hash = 0;
			HashCombine(hash, o.ptr_view.Hash());
			HashCombine(hash, o.size);
			return hash;
		}
	};
}

namespace Raysterizer
{
	namespace MiddleWare
	{
		class PipelineManager;
		class DrawCalls
		{
		public:
			explicit DrawCalls() = default;
			~DrawCalls();

			Error Init(GLuint program_id_, Raysterizer::OpenGL::VertexShader* vertex_shader_, Raysterizer::OpenGL::FragmentShader* fragment_shader_, PipelineManager* pipeline_manager_);
			void CreateVulkanRasterizerShaders();

			// At this point, we are at draw call...
			void LoadVertexBufferObject(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
										std::optional<GLuint> first = std::nullopt,
										std::optional<GLuint> count = std::nullopt);
			void LoadElementBufferObject(Raysterizer::OpenGL::ElementBufferObject& ebo,
										 std::optional<GLuint> first = std::nullopt,
										 std::optional<GLuint> count = std::nullopt);
			void LoadVertexArrayObject(Raysterizer::OpenGL::VertexArrayObject& vao);
			void SyncCurrentBoundTextures();

			void IncrementDrawCount();
			void ResetDrawCount();
			std::size_t GetDrawCount() { return current_draw_call_index; }

			template<typename T>
			Error SetVariable(std::string_view var, T&& data)
			{
				ScopedCPUProfileRaysterizerCurrentFunction();
				if (auto err = SetVariableInVM(var, std::forward<T>(data)))
				{
					//return std::move(err);

					// VM only has vertex shader, not fragment
					llvm::consumeError(std::move(err));
				}
				if (auto err = SetVariableInVulkan(var, std::forward<T>(data)))
				{
					return std::move(err);
				}
				return NoError();
			}

			template<typename T>
			Error SetVariableInVM(std::string_view var, T&& data)
			{
				ScopedCPUProfileRaysterizerCurrentFunction();
				ReturnIfError(SyncVariableInVM(var));

				// Set variable inside the vm
				for (auto i = 0; i < setup_run_infos.size(); i++)
				{
					if (i > 0)
					{
						break;
					}
					auto& setup_run_info = setup_run_infos[i];
					if (auto err = spirv_vm->SetVariable(setup_run_info.spirv_vm_state, var, std::forward<T>(data)))
					{
						return std::move(err);
					}
				}
				return NoError();
			}

			template<typename T>
			Error SetVariableInVulkan(std::string_view var, T&& data)
			{
				ScopedCPUProfileRaysterizerCurrentFunction();
				auto& render_frame = c.GetRenderFrame();

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
						AssignOrReturnError(ShaderReflection::FoundMember member, uniform_buffer->FindMember(var));
						auto& write_descriptor_set_resource = write_descriptor_sets[uniform_buffer->set][uniform_buffer->binding];
						if (auto cm_buffer = std::get_if<CMShared<Buffer>>(&write_descriptor_set_resource.binded_resource))
						{
							auto buffer = *cm_buffer;
							auto size = uniform_buffer->size;

							if (!buffer)
							{
								buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, size, true));
							}

							if (auto found = name_to_uniform_buffer.find(var); found != std::end(name_to_uniform_buffer))
							{
								auto& buffers = found->second;
								while (buffers.size() <= current_draw_call_index)
								{
									auto copied_buffer = AssignOrPanic(render_frame.CopyBuffer(buffer));
									buffers.emplace_back(copied_buffer);
								}
								buffer = buffers[current_draw_call_index];
							}
							else
							{
								std::vector<CMShared<Buffer>> buffers{ buffer };
								name_to_uniform_buffer.try_emplace(var, buffers);
							}

							PanicIfError(ds->Bind(uniform_buffer->set, uniform_buffer->binding, buffer));

							uint8_t* mapped_buffer = buffer->Map();
							auto data_size = sizeof(data);
							memcpy(mapped_buffer + member.member->offset, &data, data_size);
						}
					}
					else if (auto storage_buffer = std::get_if<ShaderReflection::StorageBuffer>(&descriptor_resource))
					{
						AssignOrReturnError(ShaderReflection::FoundMember member, uniform_buffer->FindMember(var));
						auto& write_descriptor_set_resource = write_descriptor_sets[uniform_buffer->set][uniform_buffer->binding];
						if (auto cm_buffer = std::get_if<CMShared<Buffer>>(&write_descriptor_set_resource.binded_resource))
						{
							auto buffer = *cm_buffer;
							auto size = storage_buffer->min_size;

							if (!buffer)
							{
								buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eStorageBuffer, size, true));
							}

							if (auto found = name_to_uniform_buffer.find(var); found != std::end(name_to_uniform_buffer))
							{
								auto& buffers = found->second;
								while (buffers.size() <= current_draw_call_index)
								{
									auto copied_buffer = AssignOrPanic(render_frame.CopyBuffer(buffer));
									buffers.emplace_back(copied_buffer);
								}
								buffer = buffers[current_draw_call_index];
							}
							else
							{
								std::vector<CMShared<Buffer>> buffers{ buffer };
								name_to_uniform_buffer.try_emplace(var, buffers);
							}

							PanicIfError(ds->Bind(storage_buffer->set, storage_buffer->binding, buffer));

							uint8_t* mapped_buffer = buffer->Map();
							memcpy(mapped_buffer + member.member->offset, &data, sizeof(data));
						}
					}
					else if (auto storage_image = std::get_if<ShaderReflection::StorageImage>(&descriptor_resource))
					{
					}
					else if (auto texel_buffer = std::get_if<ShaderReflection::TexelBuffer>(&descriptor_resource))
					{
					}
					else if (auto sampler = std::get_if<ShaderReflection::Sampler>(&descriptor_resource))
					{
					}
					else if (auto acceleration_structure = std::get_if<ShaderReflection::AccelerationStructure>(&descriptor_resource))
					{
					}
					else if (auto subpass_input = std::get_if<ShaderReflection::SubpassInput>(&descriptor_resource))
					{
						return StringError("Subpass input cannot be not part of descriptor set");
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

			Expected<CMShared<Buffer>&> GetVariableInVulkanRef(std::string_view var)
			{
				auto& render_frame = c.GetRenderFrame();

				auto ds = GetDescriptorSet();
				auto& write_descriptor_sets = ds->GetWriteDescriptorSets();
				ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));

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
					if (auto uniform_buffer = std::get_if<ShaderReflection::UniformBuffer>(&descriptor_resource))
					{
						auto& write_descriptor_set_resource = write_descriptor_sets[uniform_buffer->set][uniform_buffer->binding];
						if (auto cm_buffer = std::get_if<CMShared<Buffer>>(&write_descriptor_set_resource.binded_resource))
						{
							auto buffer = *cm_buffer;
							auto size = uniform_buffer->size;
							if (!buffer)
							{
								buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, size, true));
							}

							if (auto found = name_to_uniform_buffer.find(var); found != std::end(name_to_uniform_buffer))
							{
								auto& buffers = found->second;
								while (buffers.size() <= current_draw_call_index)
								{
									auto copied_buffer = AssignOrPanic(render_frame.CopyBuffer(buffer));
									buffers.emplace_back(copied_buffer);
								}
								buffer = buffers[current_draw_call_index];
							}
							else
							{
								std::vector<CMShared<Buffer>> buffers{ buffer };
								name_to_uniform_buffer.try_emplace(var, buffers);
							}

							//PanicIfError(ds->Bind(uniform_buffer->set, uniform_buffer->binding, buffer));

							return buffer;
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
							}

							if (auto found = name_to_uniform_buffer.find(var); found != std::end(name_to_uniform_buffer))
							{
								auto& buffers = found->second;
								while (buffers.size() <= current_draw_call_index)
								{
									auto copied_buffer = AssignOrPanic(render_frame.CopyBuffer(buffer));
									buffers.emplace_back(copied_buffer);
								}
								buffer = buffers[current_draw_call_index];
							}
							else
							{
								std::vector<CMShared<Buffer>> buffers{ buffer };
								name_to_uniform_buffer.try_emplace(var, buffers);
							}

							//PanicIfError(ds->Bind(storage_buffer->set, storage_buffer->binding, buffer));

							return buffer;
						}
					}
					else if (auto storage_image = std::get_if<ShaderReflection::StorageImage>(&descriptor_resource))
					{
					}
					else if (auto texel_buffer = std::get_if<ShaderReflection::TexelBuffer>(&descriptor_resource))
					{
					}
					else if (auto sampler = std::get_if<ShaderReflection::Sampler>(&descriptor_resource))
					{
					}
					else if (auto acceleration_structure = std::get_if<ShaderReflection::AccelerationStructure>(&descriptor_resource))
					{
					}
					else if (auto subpass_input = std::get_if<ShaderReflection::SubpassInput>(&descriptor_resource))
					{
						return StringError("Subpass input cannot be not part of descriptor set");
					}
					else
					{
						return StringError("Unknown resource");
					}
				}
				return StringError("Setting variable that does not exists: {}", var);
			}
			
			Error BindBufferInVulkan(std::string var);
			Error CopyToBufferInVulkan(std::string var, std::size_t offset, PointerView pointer_view);
			Error SetSampler(std::string_view var, GLuint location, GLuint active_texture_unit);

			void PerformRasterization(vk::PrimitiveTopology primitive_topology, std::optional<GLuint> first,
				std::optional<GLuint> count, std::optional<GLuint> instance_count = std::nullopt);

			void CalculateModelViewProjectionFromOpenGLSourceSetup();
			void CheckForOrtho(std::vector<Raysterizer::Analysis::RunResult>& mvps);
			void ExecuteVM(const Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
				const std::vector<Raysterizer::OpenGL::VertexAttribPointer>& vertex_attrib_pointers,
				std::vector<Raysterizer::Analysis::RunResult>& mvps,
				std::size_t i
			);
			void ExecuteVMSyncVertexAttribPointers(const Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
				const std::vector<Raysterizer::OpenGL::VertexAttribPointer>& vertex_attrib_pointers,
				std::vector<Raysterizer::Analysis::RunResult>& mvps,
				std::size_t i
			);
			Error SyncVariableInVM(std::string_view var);
			void SaveModelViewProjectionMatrixForCurrentDrawCall(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer, std::optional<GLsizei> instance_count = std::nullopt);

			std::vector<std::vector<Raysterizer::Analysis::RunResult>>& GetTransformationResults() { return transformation_results; }
			const std::vector<std::vector<Raysterizer::Analysis::RunResult>>& GetTransformationResults() const { return transformation_results; }
			auto& GetExecutor() { return executor; }
			void WaitForExecution()
			{
				ScopedCPUProfile("Compute MVP wait for execution");
				for (auto i = 0; i < current_draw_call_index; i++)
				{
					if (draw_call_tasks.size() > i)
					{
						auto& draw_call_task = draw_call_tasks[i];
						if (draw_call_task->task.has_work())
						{
							draw_call_task->execution.wait();
						}
					}
				}
			}

			CMShared<ShaderModule> GetReflectShader() { return reflect_shader; }

			std::size_t GetVertexBufferStride() const
			{
				return vertex_buffer_stride;
			}

			const std::vector<std::size_t>& GetCountToDraw() const
			{
				return count_to_draw;
			}

			std::size_t GetIndexBufferStride() const
			{
				return index_buffer_stride;
			}

			const std::string& GetRaytracingGLSL() const
			{
				return raytracing_glsl;
			}

			const auto& GetShaderConverter() const { return shader_converter; }

			auto& GetWriteDescriptorResources() { return write_descriptor_resources; }
			const auto& GetWriteDescriptorResources() const { return write_descriptor_resources; }

			auto GetShaderBindingIndex() const { return shader_binding_index; }
			const auto& GetVertexBufferPointerViews() const { return vertex_buffer_pointer_views; }
			const auto& GetIndexBufferPointerViews() const { return index_buffer_pointer_views; }
			const auto& GetPerFrameVertexBuffers() const { return per_frame_vertex_buffers; }
			const auto& GetPerFrameIndexBuffers() const { return per_frame_index_buffers; }
			auto& GetVertexBuffers() { return per_frame_vertex_buffers[c.GetFrameIndex()]; }
			auto& GetIndexBuffers() { return per_frame_index_buffers[c.GetFrameIndex()]; }
			const auto& GetVertexBuffers() const { return per_frame_vertex_buffers[c.GetFrameIndex()]; }
			const auto& GetIndexBuffers() const { return per_frame_index_buffers[c.GetFrameIndex()]; }
			const auto& GetTextures() const { return textures; }
			auto GetVertexFormat() const { return vertex_format; }
			auto& GetOutColorBuffers() { return per_frame_out_colors_buffers[c.GetFrameIndex()]; }

			auto& GetVertexBufferCache() { return vertex_buffer_cache; }
			auto& GetIndexBufferCache() { return index_buffer_cache; }

			auto& GetStorageBufferBindingIndices() { return storage_buffer_binding_indices; }
			auto& GetUniformBufferBindingIndices() { return uniform_buffer_binding_indices; }
			auto& GetCombinedSamplerBindingIndices() { return combined_sampler_binding_indices; }

			struct DrawCallVertexBufferInfo
			{
				PointerView pointer_view{};
				std::size_t count{};
				std::size_t stride;
			};

			struct DrawCallIndexBufferInfo
			{
				PointerView pointer_view{};
				std::size_t count{};
				std::size_t stride;
			};

			Raysterizer::Analysis::GLSLAnalyzer& GetVertexAnalyzer()
			{
				return vertex_shader->GetAnalyzer();
			}

			Raysterizer::Analysis::GLSLAnalyzer& GetFragmentAnalyzer()
			{
				return fragment_shader->GetAnalyzer();
			}

			CMShared<Buffer>& GetDrawCallStates()
			{
				return draw_call_states;
			}

			CMShared<DescriptorSet> GetDescriptorSet()
			{
				auto& render_frame = c.GetRenderFrame();

				// Should only occur on first frame this draw call is active!
				if (!current_ds)
				{
					ScopedCPUProfileRaysterizer("DescriptorSet Creation");
					current_ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
					c.SetName(current_ds, fmt::format("Descriptor set frame {} draw call index {}", c.GetFrameIndex(), current_draw_call_index));
				}

				if (draw_call_index_ds.empty())
				{
					draw_call_index_ds.emplace_back(current_ds);
				}

				while (draw_call_index_ds.size() <= current_draw_call_index)
				{
					// Copy from previous active ds
					auto copied_ds = AssignOrPanic(render_frame.Copy(current_ds));
					draw_call_index_ds.emplace_back(copied_ds);
				}

				current_ds = draw_call_index_ds[current_draw_call_index];
				return current_ds;
			}

			auto& GetCommandBuffers() { return command_buffers; }
			auto& GetCommandBufferCallbacks() { return command_buffer_callbacks; }

		private:
			GLuint program_id{};
			Raysterizer::OpenGL::VertexShader* vertex_shader{};
			Raysterizer::OpenGL::FragmentShader* fragment_shader{};
			CMShared<Buffer> draw_call_states;

			PipelineManager* pipeline_manager{};

			std::size_t current_draw_call_index = 0;
			flat_hash_map<std::shared_ptr<ShaderReflection::DescriptorResource>, std::size_t> descriptor_resource_to_draw_call_index;

			std::string raytracing_glsl;

			// related to shader reflection
			std::size_t vertex_buffer_stride{};
			std::vector<std::size_t> count_to_draw{};
			std::size_t index_buffer_stride{};
			bool reflection_has_edited = false;

			Raysterizer::MiddleWare::ShaderConverter shader_converter{};

			CMShared<ShaderModule> vulkan_vertex_shader{};
			CMShared<ShaderModule> vulkan_fragment_shader{};
			CMShared<ShaderModule> vulkan_tesc_shader{};
			CMShared<ShaderModule> vulkan_tese_shader{};

			flat_hash_map<std::string, WriteDescriptorSetBindedResource> write_descriptor_resources;

			std::size_t shader_binding_index = 0;
			CMShared<ShaderModule> reflect_shader{};
			std::unique_ptr<Raysterizer::Analysis::SPIRVVirtualMachine> spirv_vm{};
			std::mutex spirv_vm_m;
			flat_hash_map<std::string, bool> spirv_vm_variable_setup_run_info_sync_dirty_variables;
			bool spirv_vm_variable_setup_run_info_sync_dirty = false;

			tf::Executor executor = tf::Executor(std::thread::hardware_concurrency() - 1);
			struct DrawCallTask
			{
				tf::Taskflow taskflow{};
				tf::Task task{};
				tf::Future<void> execution{};
			};
			std::vector<std::unique_ptr<DrawCallTask>> draw_call_tasks;

			glm::vec4 vertex_position;

			std::vector<Raysterizer::Analysis::SetupRunInfo> setup_run_infos;
			std::vector<std::vector<Raysterizer::Analysis::RunResult>> transformation_results{};

			CacheMappingWithFrameAndUsageCounter<SizedHashedPointerView, BufferWithHash> vertex_buffer_cache;
			CacheMappingWithFrameAndUsageCounter<SizedHashedPointerView, BufferWithHash> index_buffer_cache;
			CacheMappingWithFrameAndUsageCounter<HashedPointerView, BufferWithHash> vbo_cache;
			CacheMappingWithFrameAndUsageCounter<HashedPointerView, CMShared<Texture>> texture_cache;
			
			std::vector<HashedPointerView> vertex_buffer_pointer_views;
			std::vector<HashedPointerView> index_buffer_pointer_views;
			std::vector<std::vector<CMShared<Buffer>>> per_frame_vertex_buffers;
			std::vector<std::vector<CMShared<Buffer>>> per_frame_index_buffers;
			flat_hash_map<std::string, std::vector<CMShared<Texture>>> textures;

			vk::Format vertex_format;

			std::vector<std::vector<CMShared<Buffer>>> per_frame_out_colors_buffers;

			phmap::flat_hash_set<std::size_t> storage_buffer_binding_indices;
			phmap::flat_hash_set<std::size_t> uniform_buffer_binding_indices;
			phmap::flat_hash_set<std::size_t> combined_sampler_binding_indices;

			std::size_t opengl_glsl_hash;
			std::string opengl_glsl_hash_cached_path;

			CMShared<DescriptorPool> dp{};
			CMShared<PipelineLayoutInfo> pli{};
			std::vector<CMShared<CommandBuffer>> command_buffers;
			std::vector<std::function<void(CommandBuffer&)>> command_buffer_callbacks;

			std::vector<CMShared<DescriptorSet>> draw_call_index_ds;
			CMShared<DescriptorSet> current_ds{};
			flat_hash_map<std::string, std::vector<CMShared<Buffer>>> name_to_uniform_buffer;
		};
	}
}
