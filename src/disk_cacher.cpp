#include "include/disk_cacher.h"

namespace RaysterizerEngine
{
	FossilizeState::FossilizeState(Context* c_) : c(c_)
	{

	}

	bool FossilizeState::enqueue_create_sampler(Fossilize::Hash hash, const VkSamplerCreateInfo* create_info, VkSampler* sampler)
	{
		SamplerCreateInfo sampler_create_info{ *create_info };
		CMShared<Sampler> cm_sampler = AssignOrPanic(c->Get(sampler_create_info));

		*sampler = **cm_sampler;
		return true;
	}

	bool FossilizeState::enqueue_create_descriptor_set_layout(Fossilize::Hash hash, const VkDescriptorSetLayoutCreateInfo* create_info, VkDescriptorSetLayout* layout)
	{
		DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;

		for (auto i = 0; i < create_info->bindingCount; i++)
		{
			vk::DescriptorSetLayoutBinding dslb = create_info->pBindings[i];
			descriptor_set_layout_create_info.AddBinding(dslb.binding, dslb.descriptorCount, dslb.descriptorType, dslb.stageFlags);
		}
		if(create_info->pNext)
		{
			PANIC("Did not expect next");
		}
		
		CMShared<DescriptorSetLayout> cm_descriptor_set_layout = AssignOrPanic(c->CreateDescriptorSetLayout(descriptor_set_layout_create_info));
		*layout = **cm_descriptor_set_layout;

		auto dsl_hash = (Fossilize::Hash)*layout;
		descriptor_set_layout_mapping.emplace(dsl_hash, cm_descriptor_set_layout);

		return true;
	}

	bool FossilizeState::enqueue_create_pipeline_layout(Fossilize::Hash hash, const VkPipelineLayoutCreateInfo* create_info, VkPipelineLayout* layout)
	{
		std::vector<ShaderModuleCreateInfo> shader_module_create_infos{};
		flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};

		/*
		for (auto i = 0; i < create_info->setLayoutCount; i++)
		{
			VkDescriptorSetLayout dsl = create_info->pSetLayouts[i];

			// Descriptor set layout handle references shader module hash actually
			auto shader_module_hash = reinterpret_cast<Fossilize::Hash>(dsl);
			if (auto found = shader_module_create_info_mapping.find(shader_module_hash); found != std::end(shader_module_create_info_mapping))
			{
				const auto& shader_module_create_info = found->second;
				shader_module_create_infos.emplace_back(shader_module_create_info);
			}
			else
			{
				PANIC("Shader not found!");
			}
		}
		*/
		for (auto i = 0; i < create_info->pushConstantRangeCount; i++)
		{
			vk::PushConstantRange pcr = create_info->pPushConstantRanges[i];

			auto stage_flags = pcr.stageFlags;
			if (stage_flags == VARIABLE_BINDING_FLAGS)
			{
				// variable index
				//auto set = (uint32_t)pcr.stageFlags;
				auto set = pcr.offset;
				auto count = pcr.size;

				variable_set_index_to_count[set] = count;
			}
			else if (stage_flags == SHADER_MODULE_FLAGS)
			{
				// Descriptor set layout handle references shader module hash actually
				uint64_t top_shader_module_hash = static_cast<uint64_t>(pcr.offset);
				uint64_t bottom_shader_module_hash = static_cast<uint64_t>(pcr.size);

				uint64_t shader_module_hash_ = (((top_shader_module_hash << 32) & 0xFFFFFFFF00000000) | (bottom_shader_module_hash & 0x00000000FFFFFFFF));
				auto shader_module_hash = static_cast<Fossilize::Hash>(shader_module_hash_);
				if (auto found = shader_module_create_info_mapping.find(shader_module_hash); found != std::end(shader_module_create_info_mapping))
				{
					const auto& shader_module_create_info = found->second;
					shader_module_create_infos.emplace_back(shader_module_create_info);
				}
				else
				{
					PANIC("Shader not found!");
				}
			}
			else
			{
				PANIC("Unexpected stage flag {}", vk::to_string(stage_flags));
			}
		}
		if (create_info->pNext)
		{
			PANIC("Did not expect next");
		}

		PipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.shader_module_create_infos = shader_module_create_infos;
		pipeline_layout_create_info.variable_set_index_to_count = variable_set_index_to_count;

		CMShared<PipelineLayoutInfo> pipeline_layout_info = AssignOrPanic(c->CreatePipelineLayoutInfo(pipeline_layout_create_info));
		CMShared<PipelineLayout> pipeline_layout = AssignOrPanic(c->Get(pipeline_layout_info));
		*layout = **pipeline_layout;

		auto pl_hash = (Fossilize::Hash)*layout;
		pipeline_layout_mapping.emplace(pl_hash, pipeline_layout);

		return true;
	}

	bool FossilizeState::enqueue_create_shader_module(Fossilize::Hash hash, const VkShaderModuleCreateInfo* create_info, VkShaderModule* shader_module)
	{
		auto code = (char*)create_info->pCode;
		auto code_size = create_info->codeSize;
		auto code_str = std::string(code, code_size);
		json json_shader_module_create_info = json::parse(code_str);
		ShaderModuleCreateInfo shader_module_create_info = json_shader_module_create_info;
		
		if (create_info->pNext)
		{
			PANIC("Did not expect next");
		}

		CMShared<ShaderModule> cm_shader_module = AssignOrPanic(c->Get(shader_module_create_info));
		*shader_module = **cm_shader_module;

		auto sm_hash = (Fossilize::Hash)StdHash(shader_module_create_info);
		shader_module_create_info_mapping.emplace(sm_hash, shader_module_create_info);

		return true;
	}

	bool FossilizeState::enqueue_create_render_pass(Fossilize::Hash hash, const VkRenderPassCreateInfo* create_info, VkRenderPass* render_pass)
	{
		RenderPassCreateInfo render_pass_create_info;
		render_pass_create_info.attachment_descriptions = std::vector<vk::AttachmentDescription>(create_info->pAttachments, create_info->pAttachments + create_info->attachmentCount);
		render_pass_create_info.subpass_descriptions = std::vector<vk::SubpassDescription>(create_info->pSubpasses, create_info->pSubpasses + create_info->subpassCount);
		render_pass_create_info.subpass_dependencies = std::vector<vk::SubpassDependency>(create_info->pDependencies, create_info->pDependencies + create_info->dependencyCount);
		if (create_info->pNext)
		{
			PANIC("Did not expect next");
		}

		CMShared<RenderPass> cm_render_pass = AssignOrPanic(c->Get(render_pass_create_info));
		*render_pass = **cm_render_pass;

		auto rp_hash = (Fossilize::Hash)*render_pass;
		render_pass_mapping.emplace(rp_hash, cm_render_pass);

		return true;
	}

	bool FossilizeState::enqueue_create_render_pass2(Fossilize::Hash hash, const VkRenderPassCreateInfo2* create_info, VkRenderPass* render_pass)
	{
		PANIC("Not supported!");
		vk::RenderPassCreateInfo render_pass_create_info{};
		return enqueue_create_render_pass(hash, &static_cast<VkRenderPassCreateInfo>(render_pass_create_info), render_pass);
	}

	bool FossilizeState::enqueue_create_compute_pipeline(Fossilize::Hash hash, const VkComputePipelineCreateInfo* create_info, VkPipeline* pipeline)
	{
		ComputePipelineCreateInfo compute_pipeline_create_info;

		auto pl_hash = (Fossilize::Hash)create_info->layout;
		if (auto found = pipeline_layout_mapping.find(pl_hash); found != std::end(pipeline_layout_mapping))
		{
			const auto& pipeline_layout = found->second;
			compute_pipeline_create_info.pipeline_layout = pipeline_layout;
		}
		else
		{
			PANIC("Could not find descriptor set layout");
		}

		if (create_info->pNext)
		{
			PANIC("Did not expect next");
		}

		CMShared<ComputePipeline> cm_compute_pipeline = AssignOrPanic(c->Get(compute_pipeline_create_info));
		*pipeline = **cm_compute_pipeline;

		return true;
	}

	bool FossilizeState::enqueue_create_graphics_pipeline(Fossilize::Hash hash, const VkGraphicsPipelineCreateInfo* create_info, VkPipeline* pipeline)
	{
		GraphicsPipelineCreateInfo graphics_pipeline_create_info;
		graphics_pipeline_create_info.pipeline_color_blend_state_create_info = *create_info->pColorBlendState;
		graphics_pipeline_create_info.pipeline_depth_stencil_state_create_info = *create_info->pDepthStencilState;
		graphics_pipeline_create_info.pipeline_input_assembly_state_create_info = *create_info->pInputAssemblyState;
		graphics_pipeline_create_info.pipeline_multisample_state_create_info = *create_info->pMultisampleState;
		graphics_pipeline_create_info.pipeline_rasterization_state_create_info = *create_info->pRasterizationState;
		graphics_pipeline_create_info.pipeline_vertex_input_state_create_info = *create_info->pVertexInputState;
		graphics_pipeline_create_info.scissor = *create_info->pViewportState->pScissors;
		graphics_pipeline_create_info.viewport = *create_info->pViewportState->pViewports;

		auto rp_hash = (Fossilize::Hash)create_info->renderPass;
		if (auto found = render_pass_mapping.find(rp_hash); found != std::end(render_pass_mapping))
		{
			const auto& render_pass = found->second;
			graphics_pipeline_create_info.render_pass = render_pass;
		}
		else
		{
			PANIC("Could not find descriptor set layout");
		}

		auto pl_hash = (Fossilize::Hash)create_info->layout;
		if (auto found = pipeline_layout_mapping.find(pl_hash); found != std::end(pipeline_layout_mapping))
		{
			const auto& pipeline_layout = found->second;
			graphics_pipeline_create_info.pipeline_layout = pipeline_layout;
		}
		else
		{
			PANIC("Could not find descriptor set layout");
		}

		if (create_info->pNext)
		{
			PANIC("Did not expect next");
		}

		CMShared<GraphicsPipeline> cm_graphics_pipeline = AssignOrPanic(c->Get(graphics_pipeline_create_info));
		*pipeline = **cm_graphics_pipeline;

		return true;
	}

	bool FossilizeState::enqueue_create_raytracing_pipeline(Fossilize::Hash hash, const VkRayTracingPipelineCreateInfoKHR* create_info, VkPipeline* pipeline)
	{
		RaytracingPipelineCreateInfo raytracing_pipeline_create_info;
		raytracing_pipeline_create_info.recursion_depth = create_info->maxPipelineRayRecursionDepth;

		auto pl_hash = (Fossilize::Hash)create_info->layout;
		if (auto found = pipeline_layout_mapping.find(pl_hash); found != std::end(pipeline_layout_mapping))
		{
			auto& pipeline_layout = found->second;
			raytracing_pipeline_create_info.pipeline_layout = pipeline_layout;
		}
		else
		{
			PANIC("Could not find descriptor set layout");
		}

		if (create_info->pNext)
		{
			PANIC("Did not expect next");
		}

		CMShared<RaytracingPipeline> cm_raytracing_pipeline = AssignOrPanic(c->Get(raytracing_pipeline_create_info));
		*pipeline = **cm_raytracing_pipeline;

		return true;
	}

	void FossilizeState::sync_threads()
	{

	}

	void FossilizeState::sync_shader_modules()
	{

	}

	void FossilizeState::notify_replayed_resources_for_type()
	{

	}

	DiskCacher::DiskCacher()
	{
	}

	DiskCacher::~DiskCacher()
	{
		//PanicIfError(SyncFlush());
		if (db)
		{
			db->flush();
		}
		recorder.tear_down_recording_thread();
	}

	Error DiskCacher::Init(Context* c_)
	{
		c = c_;

		recorder.set_database_enable_compression(true);
		recorder.set_database_enable_checksum(true);

		fs::path db_path = Config["cache"]["fossilize"]["path"];
		if (IsEnabled() && fs::exists(db_path))
		{
			std::vector<uint8_t> db_data = Util::ReadFileAsVec<uint8_t>(db_path);
			
			// make a tmp db to read from
			auto tmp_db_path = db_path;
			tmp_db_path.replace_filename(fmt::format("tmp_db.{}", db_path.extension().string()));
			fs::copy_file(db_path, tmp_db_path, std::filesystem::copy_options::overwrite_existing);

			auto tmp_db = std::unique_ptr<Fossilize::DatabaseInterface>(Fossilize::create_database(tmp_db_path.string().c_str(), Fossilize::DatabaseMode::ReadOnly));
			if (!tmp_db || !tmp_db->prepare())
			{
				return StringError("Database could not be initialized");
			}

			/*
			db = std::unique_ptr<Fossilize::DatabaseInterface>(Fossilize::create_database(db_path.string().c_str(), Fossilize::DatabaseMode::OverWrite));
			if (!db || !db->prepare())
			{
				return StringError("Database could not be initialized");
			}

			recorder.init_recording_thread(db.get());
			*/

			FossilizeState state(c);
			Fossilize::StateReplayer replayer;

			std::vector<uint8_t> state_json;
			static const Fossilize::ResourceTag playback_order[] = {
				Fossilize::RESOURCE_APPLICATION_INFO,
				Fossilize::RESOURCE_SAMPLER,
				Fossilize::RESOURCE_SHADER_MODULE,
				Fossilize::RESOURCE_DESCRIPTOR_SET_LAYOUT,
				Fossilize::RESOURCE_PIPELINE_LAYOUT,
				Fossilize::RESOURCE_RENDER_PASS,
				Fossilize::RESOURCE_GRAPHICS_PIPELINE,
				Fossilize::RESOURCE_COMPUTE_PIPELINE,
				Fossilize::RESOURCE_APPLICATION_BLOB_LINK,
				Fossilize::RESOURCE_RAYTRACING_PIPELINE,
			};

			for (auto& tag : playback_order)
			{
				size_t hash_count = 0;
				if (!tmp_db->get_hash_list_for_resource_tag(tag, &hash_count, nullptr))
				{
					return StringError("Failed to get hashes.\n");
				}

				std::vector<Fossilize::Hash> hashes(hash_count);

				if (!tmp_db->get_hash_list_for_resource_tag(tag, &hash_count, hashes.data()))
				{
					return StringError("Failed to get shader module hashes.\n");
				}

				for (auto hash : hashes)
				{
					size_t state_json_size;
					if (!tmp_db->read_entry(tag, hash, &state_json_size, nullptr, 0))
					{
						return StringError("Failed to load blob from cache.\n");
					}

					state_json.resize(state_json_size);

					if (!tmp_db->read_entry(tag, hash, &state_json_size, state_json.data(), 0))
					{
						return StringError("Failed to load blob from cache.\n");
					}

					if (!replayer.parse(state, tmp_db.get(), state_json.data(), state_json.size()))
					{
						return StringError("Failed to parse blob (tag: %d, hash: 0x%" PRIx64 ").\n", tag, hash);
					}
				}
			}

			db = std::unique_ptr<Fossilize::DatabaseInterface>(Fossilize::create_database(db_path.string().c_str(), Fossilize::DatabaseMode::Append));
			if (!db || !db->prepare())
			{
				return StringError("Database could not be initialized");
			}

			recorder.init_recording_thread(db.get());
		}
		else
		{
			db = std::unique_ptr<Fossilize::DatabaseInterface>(Fossilize::create_database(db_path.string().c_str(), Fossilize::DatabaseMode::OverWrite));
			if (!db || !db->prepare())
			{
				return StringError("Database could not be initialized");
			}

			recorder.init_recording_thread(db.get());
		}

		//db->prepare();
		//ReturnIfError(AsyncFlush());

		return NoError();
	}

	Error DiskCacher::SyncFlush()
	{
		db->flush();
		uint8_t* serialized;
		size_t serialized_size;
		if (!recorder.serialize(&serialized, &serialized_size))
		{
			PANIC("Not able to serialize");
		}

		fs::path db_path = Config["cache"]["fossilize_path"];
		std::ofstream f(db_path);
		if (!f)
		{
			PANIC("Not file opened");
		}

		f.write((const char*)serialized, serialized_size);

		recorder.free_serialized(serialized);
	
		return NoError();
	}

	Error DiskCacher::AsyncFlush()
	{
		std::async(std::launch::async, [&]()
			{
				PanicIfError(SyncFlush());
			});
		return NoError();
	}

	bool DiskCacher::IsEnabled()
	{
		bool db_enabled = Config["cache"]["fossilize"]["enable"];
		return db_enabled;
	}

	Error DiskCacher::Record(CMShared<DescriptorSetLayout> descriptor_set_layout)
	{
		if (!IsEnabled()) { return NoError(); }

		const auto& descriptor_set_layout_create_info = descriptor_set_layout->descriptor_set_layout_create_info;
		const auto& bindings = descriptor_set_layout_create_info.GetBindings();
		const auto& variable_binding = descriptor_set_layout_create_info.GetVariableBinding();

		std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings(bindings.size());
		std::transform(std::begin(bindings), std::end(bindings), std::begin(descriptor_set_layout_bindings), [](const auto& it) { return it.second.descriptor_set_layout_binding; });
		std::sort(std::begin(descriptor_set_layout_bindings), std::end(descriptor_set_layout_bindings), [](const auto& e1, const auto& e2) { return e1.binding < e2.binding; });

		auto vulkan_descriptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo{}
			.setFlags(Constants::DEFAULT_DESCRIPTOR_SET_LAYOUT_CREATE_FLAGS)
			.setBindings(descriptor_set_layout_bindings);

		Fossilize::Hash hash = static_cast<Fossilize::Hash>(StdHash(descriptor_set_layout_create_info));
		if (!recorder.record_descriptor_set_layout(**descriptor_set_layout, static_cast<VkDescriptorSetLayoutCreateInfo>(vulkan_descriptor_set_layout_create_info), hash))
		{
			return StringError("Unable to record descriptor set layout");
		}

		return NoError();
	}

	Error DiskCacher::Record(CMShared<PipelineLayout> pipeline_layout)
	{
		if (!IsEnabled()) { return NoError(); }

		auto create_info = pipeline_layout->pipeline_layout_info->pipeline_create_info;
		Fossilize::Hash hash = StdHash(pipeline_layout->pipeline_layout_info);

		/*
		// Descriptor set layouts references shader module hash instead of descriptor set layout handles
		std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
		for (const auto& shader_module : pipeline_layout->pipeline_layout_info.shader_modules)
		{
			auto shader_module_hash = StdHash(shader_module->shader_module_create_info);
			VkDescriptorSetLayout descriptor_set_layout = reinterpret_cast<VkDescriptorSetLayout>(shader_module_hash);
			descriptor_set_layouts.emplace_back(descriptor_set_layout);
		}
		//create_info.setSetLayouts(descriptor_set_layouts);
		*/

		// Descriptor set layouts references shader module hash instead of descriptor set layout handles
		std::vector<vk::PushConstantRange> push_constant_ranges;
		for (const auto& shader_module : pipeline_layout->pipeline_layout_info->shader_modules)
		{
			auto shader_module_hash = StdHash(shader_module->shader_module_create_info);
			static_assert(sizeof(shader_module_hash) == sizeof(uint64_t), "Shader module hash is same as uint64_t");
			uint32_t top_shader_module_hash = ((shader_module_hash & 0xFFFFFFFF00000000) >> 32);
			uint32_t bottom_shader_module_hash = (shader_module_hash & 0x00000000FFFFFFFF);
			auto push_constant_range = vk::PushConstantRange{}
				.setStageFlags(FossilizeState::SHADER_MODULE_FLAGS)
				.setOffset(top_shader_module_hash)
				.setSize(bottom_shader_module_hash);
			push_constant_ranges.emplace_back(push_constant_range);
		}

		// Variable bindings
		for (const auto& [set, variable_binding] : pipeline_layout->pipeline_layout_info->descriptor_set_layouts.GetVariableBindings())
		{
			auto binding = variable_binding.binding;
			auto num_bindings = variable_binding.num_bindings;

			auto push_constant_range = vk::PushConstantRange{}
				.setStageFlags(FossilizeState::VARIABLE_BINDING_FLAGS)
				.setOffset(set)
				.setSize(num_bindings);

			push_constant_ranges.emplace_back(push_constant_range);
		}
		create_info.setPushConstantRanges(push_constant_ranges);

		if (!recorder.record_pipeline_layout(**pipeline_layout, static_cast<VkPipelineLayoutCreateInfo>(create_info), hash))
		{
			return StringError("Unable to record pipeline layout");
		}

		return NoError();
	}

	Error DiskCacher::Record(CMShared<ShaderModule> shader_module)
	{
		if (!IsEnabled()) { return NoError(); }

		const ShaderModuleCreateInfo& shader_module_create_info = shader_module->shader_module_create_info;
		json j = shader_module_create_info;
		std::string json_dump = j.dump(4);
		if (json_dump.length() % sizeof(uint32_t) != 0)
		{
			auto padding = sizeof(uint32_t) - (json_dump.length() % sizeof(uint32_t));
			json_dump += std::string(padding, '\x00');
		}

		auto vk_shader_module_create_info = vk::ShaderModuleCreateInfo{}
			.setPCode((uint32_t*)json_dump.c_str())
			.setCodeSize(json_dump.length());

		Fossilize::Hash hash = static_cast<Fossilize::Hash>(StdHash(shader_module_create_info));
		if (!recorder.record_shader_module(**shader_module, static_cast<VkShaderModuleCreateInfo>(vk_shader_module_create_info), hash))
		{
			return StringError("Unable to record shader module");
		}

		return NoError();
	}

	Error DiskCacher::Record(CMShared<GraphicsPipeline> graphics_pipeline)
	{
		if (!IsEnabled()) { return NoError(); }

		const auto& graphics_pipeline_create_info = graphics_pipeline->graphics_pipeline_create_info;
		const auto& pipeline_layout_info = graphics_pipeline_create_info.pipeline_layout->pipeline_layout_info;
		const auto& combined_shader_reflection = pipeline_layout_info->combined_shader_reflection;
		const auto& shader_modules = pipeline_layout_info->shader_modules;
		const auto& stages = pipeline_layout_info->pipeline_shader_stages;

		auto viewports = std::vector<vk::Viewport>{ graphics_pipeline_create_info.viewport };
		auto scissors = std::vector<vk::Rect2D>{ graphics_pipeline_create_info.scissor };
		auto pipeline_viewport_state_create_info = vk::PipelineViewportStateCreateInfo{}
			.setViewports(viewports)
			.setScissors(scissors);

		auto vulkan_graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo{}
			.setStages(stages)
			.setPVertexInputState(&graphics_pipeline_create_info.pipeline_vertex_input_state_create_info)
			.setPInputAssemblyState(&graphics_pipeline_create_info.pipeline_input_assembly_state_create_info)
			.setPViewportState(&pipeline_viewport_state_create_info)
			.setPRasterizationState(&graphics_pipeline_create_info.pipeline_rasterization_state_create_info)
			.setPMultisampleState(&graphics_pipeline_create_info.pipeline_multisample_state_create_info)
			.setPColorBlendState(&graphics_pipeline_create_info.pipeline_color_blend_state_create_info)
			.setPDepthStencilState(&graphics_pipeline_create_info.pipeline_depth_stencil_state_create_info)
			.setLayout(graphics_pipeline_create_info.pipeline_layout->pipeline_layout)
			.setRenderPass(graphics_pipeline_create_info.render_pass->render_pass)
			.setBasePipelineHandle(VK_NULL_HANDLE);

		Fossilize::Hash hash = static_cast<Fossilize::Hash>(StdHash(graphics_pipeline_create_info));
		if (!recorder.record_graphics_pipeline(**graphics_pipeline, static_cast<VkGraphicsPipelineCreateInfo>(vulkan_graphics_pipeline_create_info), nullptr, 0, hash))
		{
			return StringError("Unable to record graphics pipeline");
		}

		return NoError();
	}

	Error DiskCacher::Record(CMShared<ComputePipeline> compute_pipeline)
	{
		if (!IsEnabled()) { return NoError(); }

		const auto& compute_pipeline_create_info = compute_pipeline->compute_pipeline_create_info;
		const auto& pipeline_layout_info = compute_pipeline_create_info.pipeline_layout->pipeline_layout_info;
		const auto& stages = pipeline_layout_info->pipeline_shader_stages;

		if (stages.size() != 1)
		{
			return StringError("Compute pipeline creation expected stages to be 1 when it is actually {}", stages.size());
		}

		auto vulkan_compute_pipeline_create_info = vk::ComputePipelineCreateInfo{}
			.setStage(stages[0])
			.setLayout(compute_pipeline_create_info.pipeline_layout->pipeline_layout);

		Fossilize::Hash hash = static_cast<Fossilize::Hash>(StdHash(compute_pipeline_create_info));
		if (!recorder.record_compute_pipeline(**compute_pipeline, static_cast<VkComputePipelineCreateInfo>(vulkan_compute_pipeline_create_info), nullptr, 0, hash))
		{
			return StringError("Unable to record compute pipeline");
		}

		return NoError();
	}

	Error DiskCacher::Record(CMShared<RaytracingPipeline> raytracing_pipeline)
	{
		if (!IsEnabled()) { return NoError(); }

		const auto& raytracing_pipeline_create_info = raytracing_pipeline->raytracing_pipeline_create_info;
		const auto& pipeline_layout_info = raytracing_pipeline->raytracing_pipeline_create_info.pipeline_layout->pipeline_layout_info;
		const auto& combined_shader_reflection = pipeline_layout_info->combined_shader_reflection;
		const auto& shader_modules = pipeline_layout_info->shader_modules;
		const auto& stages = pipeline_layout_info->pipeline_shader_stages;
		AssignOrReturnError(auto raytracing_shader_group_create_infos, pipeline_layout_info->GetRaytracingShaderGroupCreateInfos());

		auto vulkan_raytracing_pipeline_create_info = vk::RayTracingPipelineCreateInfoKHR{}
			.setStages(stages)
			.setGroups(raytracing_shader_group_create_infos)
			.setMaxPipelineRayRecursionDepth(raytracing_pipeline_create_info.recursion_depth)
			.setLayout(raytracing_pipeline_create_info.pipeline_layout->pipeline_layout);

		Fossilize::Hash hash = static_cast<Fossilize::Hash>(StdHash(raytracing_pipeline_create_info));
		if (!recorder.record_raytracing_pipeline(**raytracing_pipeline, static_cast<VkRayTracingPipelineCreateInfoKHR>(vulkan_raytracing_pipeline_create_info), nullptr, 0, hash))
		{
			return StringError("Unable to record raytracing pipeline");
		}

		return NoError();
	}

	Error DiskCacher::Record(CMShared<RenderPass> render_pass)
	{
		if (!IsEnabled()) { return NoError(); }

		const auto& render_pass_create_info = render_pass->render_pass_create_info;
		auto vulkan_render_pass_create_info = render_pass_create_info.CreateVulkanRenderPassCreateInfo();

		Fossilize::Hash hash = static_cast<Fossilize::Hash>(StdHash(render_pass_create_info));
		if (!recorder.record_render_pass(**render_pass, static_cast<VkRenderPassCreateInfo>(vulkan_render_pass_create_info), hash))
		{
			return StringError("Unable to record render pass");
		}

		return NoError();
	}

	Error DiskCacher::Record(CMShared<Sampler> sampler)
	{
		if (!IsEnabled()) { return NoError(); }

		const auto& sampler_create_info = sampler->sampler_create_info;
		auto vulkan_sampler_create_info = sampler_create_info.sampler_create_info;

		Fossilize::Hash hash = static_cast<Fossilize::Hash>(StdHash(sampler_create_info));
		if (!recorder.record_sampler(**sampler, static_cast<VkSamplerCreateInfo>(vulkan_sampler_create_info), hash))
		{
			return StringError("Unable to record sampler");
		}

		return NoError();
	}
}