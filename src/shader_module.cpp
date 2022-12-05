#include "include/shader_module.h"

#undef ENABLE_SPV_REFLECT

namespace RaysterizerEngine
{
    namespace
    {
        const TBuiltInResource DefaultTBuiltInResource = {
            /* .MaxLights = */ 32,
            /* .MaxClipPlanes = */ 6,
            /* .MaxTextureUnits = */ 32,
            /* .MaxTextureCoords = */ 32,
            /* .MaxVertexAttribs = */ 64,
            /* .MaxVertexUniformComponents = */ 4096,
            /* .MaxVaryingFloats = */ 64,
            /* .MaxVertexTextureImageUnits = */ 32,
            /* .MaxCombinedTextureImageUnits = */ 80,
            /* .MaxTextureImageUnits = */ 32,
            /* .MaxFragmentUniformComponents = */ 4096,
            /* .MaxDrawBuffers = */ 32,
            /* .MaxVertexUniformVectors = */ 128,
            /* .MaxVaryingVectors = */ 8,
            /* .MaxFragmentUniformVectors = */ 16,
            /* .MaxVertexOutputVectors = */ 16,
            /* .MaxFragmentInputVectors = */ 15,
            /* .MinprogramTexelOffset = */ -8,
            /* .MaxprogramTexelOffset = */ 7,
            /* .MaxClipDistances = */ 8,
            /* .MaxComputeWorkGroupCountX = */ 65535,
            /* .MaxComputeWorkGroupCountY = */ 65535,
            /* .MaxComputeWorkGroupCountZ = */ 65535,
            /* .MaxComputeWorkGroupSizeX = */ 1024,
            /* .MaxComputeWorkGroupSizeY = */ 1024,
            /* .MaxComputeWorkGroupSizeZ = */ 64,
            /* .MaxComputeUniformComponents = */ 1024,
            /* .MaxComputeTextureImageUnits = */ 16,
            /* .MaxComputeImageUniforms = */ 8,
            /* .MaxComputeAtomicCounters = */ 8,
            /* .MaxComputeAtomicCounterBuffers = */ 1,
            /* .MaxVaryingComponents = */ 60,
            /* .MaxVertexOutputComponents = */ 64,
            /* .MaxGeometryInputComponents = */ 64,
            /* .MaxGeometryOutputComponents = */ 128,
            /* .MaxFragmentInputComponents = */ 128,
            /* .MaxImageUnits = */ 8,
            /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
            /* .MaxCombinedShaderOutputResources = */ 8,
            /* .MaxImageSamples = */ 0,
            /* .MaxVertexImageUniforms = */ 0,
            /* .MaxTessControlImageUniforms = */ 0,
            /* .MaxTessEvaluationImageUniforms = */ 0,
            /* .MaxGeometryImageUniforms = */ 0,
            /* .MaxFragmentImageUniforms = */ 8,
            /* .MaxCombinedImageUniforms = */ 8,
            /* .MaxGeometryTextureImageUnits = */ 16,
            /* .MaxGeometryOutputVertices = */ 256,
            /* .MaxGeometryTotalOutputComponents = */ 1024,
            /* .MaxGeometryUniformComponents = */ 1024,
            /* .MaxGeometryVaryingComponents = */ 64,
            /* .MaxTessControlInputComponents = */ 128,
            /* .MaxTessControlOutputComponents = */ 128,
            /* .MaxTessControlTextureImageUnits = */ 16,
            /* .MaxTessControlUniformComponents = */ 1024,
            /* .MaxTessControlTotalOutputComponents = */ 4096,
            /* .MaxTessEvaluationInputComponents = */ 128,
            /* .MaxTessEvaluationOutputComponents = */ 128,
            /* .MaxTessEvaluationTextureImageUnits = */ 16,
            /* .MaxTessEvaluationUniformComponents = */ 1024,
            /* .MaxTessPatchComponents = */ 120,
            /* .MaxPatchVertices = */ 32,
            /* .MaxTessGenLevel = */ 64,
            /* .MaxViewports = */ 16,
            /* .MaxVertexAtomicCounters = */ 0,
            /* .MaxTessControlAtomicCounters = */ 0,
            /* .MaxTessEvaluationAtomicCounters = */ 0,
            /* .MaxGeometryAtomicCounters = */ 0,
            /* .MaxFragmentAtomicCounters = */ 8,
            /* .MaxCombinedAtomicCounters = */ 8,
            /* .MaxAtomicCounterBindings = */ 1,
            /* .MaxVertexAtomicCounterBuffers = */ 0,
            /* .MaxTessControlAtomicCounterBuffers = */ 0,
            /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
            /* .MaxGeometryAtomicCounterBuffers = */ 0,
            /* .MaxFragmentAtomicCounterBuffers = */ 1,
            /* .MaxCombinedAtomicCounterBuffers = */ 1,
            /* .MaxAtomicCounterBufferSize = */ 16384,
            /* .MaxTransformFeedbackBuffers = */ 4,
            /* .MaxTransformFeedbackInterleavedComponents = */ 64,
            /* .MaxCullDistances = */ 8,
            /* .MaxCombinedClipAndCullDistances = */ 8,
            /* .MaxSamples = */ 4,
            /* .maxMeshOutputVerticesNV = */ 256,
            /* .maxMeshOutputPrimitivesNV = */ 512,
            /* .maxMeshWorkGroupSizeX_NV = */ 32,
            /* .maxMeshWorkGroupSizeY_NV = */ 1,
            /* .maxMeshWorkGroupSizeZ_NV = */ 1,
            /* .maxTaskWorkGroupSizeX_NV = */ 32,
            /* .maxTaskWorkGroupSizeY_NV = */ 1,
            /* .maxTaskWorkGroupSizeZ_NV = */ 1,
            /* .maxMeshViewCountNV = */ 4,
            /* .maxDualSourceDrawBuffersEXT = */ 1,

            /* .limits = */ {
                /* .nonInductiveForLoops = */ 1,
                /* .whileLoops = */ 1,
                /* .doWhileLoops = */ 1,
                /* .generalUniformIndexing = */ 1,
                /* .generalAttributeMatrixVectorIndexing = */ 1,
                /* .generalVaryingIndexing = */ 1,
                /* .generalSamplerIndexing = */ 1,
                /* .generalVariableIndexing = */ 1,
                /* .generalConstantMatrixVectorIndexing = */ 1,
            }
        };
    }

    vk::ShaderStageFlagBits TranslateShaderKindToShaderStageFlagBits(ShaderKind shader_kind)
    {
        switch (shader_kind)
        {
        case ShaderKind::VERT:
            return vk::ShaderStageFlagBits::eVertex;
        case ShaderKind::TESC:
            return vk::ShaderStageFlagBits::eTessellationControl;
        case ShaderKind::TESE:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
        case ShaderKind::GEOM:
            return vk::ShaderStageFlagBits::eGeometry;
        case ShaderKind::FRAG:
            return vk::ShaderStageFlagBits::eFragment;
        case ShaderKind::COMP:
            return vk::ShaderStageFlagBits::eCompute;
        case ShaderKind::RGEN:
            return vk::ShaderStageFlagBits::eRaygenKHR;
        case ShaderKind::RCHIT:
            return vk::ShaderStageFlagBits::eClosestHitKHR;
        case ShaderKind::RMISS:
            return vk::ShaderStageFlagBits::eMissKHR;
        case ShaderKind::RAHIT:
            return vk::ShaderStageFlagBits::eAnyHitKHR;
        case ShaderKind::RINT:
            return vk::ShaderStageFlagBits::eIntersectionKHR;
        default:
            PANIC("Unable to convert shader kind");
        }
        return vk::ShaderStageFlagBits::eVertex;
    }

    EShLanguage TranslateShaderKindToInternal(ShaderKind shader_kind)
    {
        switch (shader_kind)
        {
        case ShaderKind::VERT:
            return EShLanguage::EShLangVertex;
        case ShaderKind::TESC:
            return EShLanguage::EShLangTessControl;
        case ShaderKind::TESE:
            return EShLanguage::EShLangTessEvaluation;
        case ShaderKind::GEOM:
            return EShLanguage::EShLangGeometry;
        case ShaderKind::FRAG:
            return EShLanguage::EShLangFragment;
        case ShaderKind::COMP:
            return EShLanguage::EShLangCompute;
        case ShaderKind::RGEN:
            return EShLanguage::EShLangRayGen;
        case ShaderKind::RCHIT:
            return EShLanguage::EShLangClosestHit;
        case ShaderKind::RMISS:
            return EShLanguage::EShLangMiss;
        case ShaderKind::RAHIT:
            return EShLanguage::EShLangAnyHit;
        case ShaderKind::RINT:
            return EShLanguage::EShLangIntersect;
        default:
            //return EShLanguage::EShLangClosestHit;
            PANIC("Unknown");
        }
        return EShLanguage::EShLangVertex;
    }

    ShaderModuleSourceCreateInfo ShaderModuleSourceCreateInfo::File(fs::path path)
    {
        auto full_path = path.string();
        auto extension = path.extension().string();
        auto shader_kind = GetShaderKindFromExtension(extension);
        return ShaderModuleSourceCreateInfo{ "", full_path, shader_kind };
    }

    void to_json(json& j, const ShaderModuleCreateInfo& shader_module_create_info)
    {
        if (auto shader_module_source_create_info = std::get_if<ShaderModuleSourceCreateInfo>(&shader_module_create_info.source))
        {
            j = json
            {
                {"source", shader_module_source_create_info->source },
                {"filename", shader_module_source_create_info->filename },
                {"shader_kind", shader_module_source_create_info->shader_kind }
            };
        }
        else if (auto spirv = std::get_if<Spirv>(&shader_module_create_info.source))
        {
            j = json
            {
                {"spirv", *spirv }
            };
        }
        else
        {
            PANIC("Not supported variant");
        }
    }

    void from_json(const json& j, ShaderModuleCreateInfo& shader_module_create_info)
    {
        if (j.contains("source"))
        {
            ShaderModuleSourceCreateInfo shader_module_source_create_info{};
            j.at("source").get_to(shader_module_source_create_info.source);
            j.at("filename").get_to(shader_module_source_create_info.filename);
            j.at("shader_kind").get_to(shader_module_source_create_info.shader_kind);
            shader_module_create_info.source = shader_module_source_create_info;
        }
        else if (j.contains("spirv"))
        {
            Spirv spirv;
            j.at("spirv").get_to(spirv);
            shader_module_create_info.source = spirv;
        }
        else
        {
            PANIC("Not supported variant");
        }
    }

    Error ShaderModule::PerformReflection()
    {
        ReturnIfError(shader_reflection.Reflect(spirv));

#ifdef ENABLE_SPV_REFLECT
        SpvReflectResult result = spvReflectCreateShaderModule(spirv.size() * sizeof(spirv[0]), spirv.data(), reflect_module.get());
        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            PANIC("Failed to parse shader module {}", result);
        }

        uint32_t descriptor_binding_count = 0;
        spvReflectEnumerateDescriptorBindings(reflect_module.get(), &descriptor_binding_count, nullptr);
        reflect_descriptor_bindings = std::vector<SpvReflectDescriptorBinding*>(descriptor_binding_count);
        spvReflectEnumerateDescriptorBindings(reflect_module.get(), &descriptor_binding_count, reflect_descriptor_bindings.data());
#endif

        return NoError();
    }

    Error ShaderModule::PopulateMetadata(ShaderModuleCreateInfo shader_module_create_info_)
    {
        shader_module_create_info = std::move(shader_module_create_info_);

        spirv = std::vector<uint32_t>{};

        if (auto shader_module_source_create_info = std::get_if<ShaderModuleSourceCreateInfo>(&shader_module_create_info.source))
        {
            const auto& source = shader_module_source_create_info->source;
            const auto& filename = shader_module_source_create_info->filename;

            std::size_t shader_hash = StdHash(shader_module_create_info);
            fs::path cache_path = Config["shader"]["compiler"]["cache_path"];
            if (!fs::exists(cache_path))
            {
                fs::create_directories(cache_path);
            }

            auto shader_cache_path = cache_path / fmt::format("{}.spv", shader_hash);

            // Check in cache if shader exists
            if (fs::exists(shader_cache_path))
            {
                spirv = Util::ReadFileAsVec<uint32_t>(shader_cache_path);
                if (spirv.empty())
                {
                    return StringError("Expected spirv to have content at path: {}", shader_cache_path.string());
                }
            }
            else if (!filename.empty() && filename.find(".spv") != std::string::npos)
            {
                // Load this from disk...
                auto file_shader_path = cache_path / filename;
                if (fs::exists(file_shader_path))
                {
                    // load this spirv
                    PANIC("Not supported yet");
                }
                else
                {
                    return StringError("Shader file does not exist");
                }
            }
            else
            {
                const auto& shader_kind = shader_module_source_create_info->shader_kind;
                auto shader_stage = TranslateShaderKindToInternal(shader_kind);

                const static bool dump_all_shaders_enabled = Config["shader"]["dump_all_shaders"]["enable"];
                if (dump_all_shaders_enabled)
                {
                    const static fs::path dump_shader_path = Config["shader"]["dump_all_shaders"]["dump_shader_path"];
                    CallOnce
                    {
                        if (!fs::exists(dump_shader_path))
                        {
                            fs::create_directories(dump_shader_path);
                        }
                    };

                    auto dump_path = dump_shader_path / fmt::format("{}.glsl", shader_hash);;
                    auto f = std::ofstream(dump_path, std::ios::ate | std::ios::trunc);
                    if (f)
                    {
                        f << source;
                        f << "\n";
                    }
                }

                const static std::vector<std::string> search_paths = Config["shader"]["compiler"]["search_paths"];

                DirStackFileIncluder includer;
                for (const auto& search_path : search_paths)
                {
                    includer.pushExternalLocalDirectory(search_path);
                }

                auto DumpAndReturnError = [&](std::string_view error)
                {
                    const static bool enable_dump = Config["shader"]["compiler"]["failure"]["enable"];
                    if (enable_dump)
                    {
                        std::string dump_path = Config["shader"]["compiler"]["failure"]["dump_shader_path"];
                        auto f = std::ofstream(dump_path, std::ios::ate);
                        f << source;
                        f << "\n";
                        f << error;
                    }
                    PANIC(fmt::format("{}\n{}\n", error, source));
                    return StringError(error);
                };

                glslang::TShader shader(shader_stage);
                glslang::TProgram program;

                auto source_c = source.c_str();
                auto filename_c = filename.c_str();

                shader.setEntryPoint("main");
                shader.setEnvInput(glslang::EShSource::EShSourceGlsl, shader_stage, glslang::EShClient::EShClientVulkan, 120);
                shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_2);
                shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_5);
                shader.setStringsWithLengthsAndNames(&source_c, nullptr, &filename_c, 1);

                auto version = 460;
                auto resources = &DefaultTBuiltInResource;
                EShMessages messages = static_cast<EShMessages>(EShMessages::EShMsgVulkanRules | EShMessages::EShMsgDebugInfo);

                std::string preprocess_shader_string;
                if (!shader.preprocess(resources, version, EProfile::ENoProfile, false, false, messages, &preprocess_shader_string, includer))
                {
                    //return DumpAndReturnError(fmt::format("Shader preprocess error \n{}\n\n\n\n {}", shader.getInfoLog(), shader.getInfoDebugLog()));
                }

                if (!shader.parse(resources, version, false, messages, includer))
                {
                    return DumpAndReturnError(fmt::format("Shader parsing error \n{}\n\n\n\n {}", shader.getInfoLog(), shader.getInfoDebugLog()));
                }

                program.addShader(&shader);

                if (!program.link(messages))
                {
                    return DumpAndReturnError(fmt::format("Shader link error \n{}\n\n\n\n {}", program.getInfoLog(), program.getInfoDebugLog()));
                }
                if (!program.mapIO())
                {
                    return DumpAndReturnError(fmt::format("Shader mapIO error \n{}\n\n\n\n {}", program.getInfoLog(), program.getInfoDebugLog()));
                }
                if (!program.getIntermediate(shader_stage))
                {
                    return DumpAndReturnError(fmt::format("Shader getIntermediate error \n{}\n\n\n\n {}", program.getInfoLog(), program.getInfoDebugLog()));
                }

                auto* intermediate = program.getIntermediate(shader_stage);

                spv::SpvBuildLogger logger;
                glslang::SpvOptions spv_options;

                spv_options.generateDebugInfo = true;
                //spv_options.stripDebugInfo = true;
                spv_options.disableOptimizer = true;
                spv_options.optimizeSize = false;
                spv_options.disassemble = true;
                spv_options.validate = true;

                std::string optimization = Config["shader"]["compiler"]["optimization"];
                if (optimization == "none")
                {
                    spv_options.generateDebugInfo = true;
                    spv_options.stripDebugInfo = false;
                }
                else if (optimization == "performance")
                {
                    spv_options.generateDebugInfo = false;
                    spv_options.stripDebugInfo = true;
                }
                else if (optimization == "size")
                {
                    spv_options.generateDebugInfo = false;
                    spv_options.stripDebugInfo = true;
                }
                else
                {
                    return StringError("Optimization level not avaliable {}", optimization);
                }

                glslang::GlslangToSpv(*intermediate, spirv, &logger, &spv_options);

                auto logger_messages = logger.getAllMessages();
                if (!logger_messages.empty())
                {
                    auto error = logger_messages;
                    return DumpAndReturnError(error);
                }

                // optimization
                {
                    spvtools::Optimizer opt(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
                    if (optimization == "none")
                    {
                    }
                    else if (optimization == "performance")
                    {
                        opt.RegisterPerformancePasses();
                    }
                    else if (optimization == "size")
                    {
                        opt.RegisterSizePasses();
                    }
                    else
                    {
                        return StringError("Optimization level not avaliable {}", optimization);
                    }

                    opt.SetMessageConsumer([&](spv_message_level_t level, const char* source,
                        const spv_position_t& position, const char* message)
                        {
                            if (level == SPV_MSG_ERROR || level == SPV_MSG_FATAL)
                            {
                                PANIC("Encountered error with optimization {}", message);
                            }
                        }
                    );

                    std::vector<uint32_t> spirv_optimized;
                    spvtools::OptimizerOptions opt_options{};
                    opt_options.set_run_validator(false);
                    if (!opt.Run(spirv.data(), spirv.size(), &spirv_optimized, opt_options))
                    {
                        return StringError("Failed to run optimizer");
                    }
                    spirv = spirv_optimized;
                }
            }
        }
        else if (auto spirv_val = std::get_if<Spirv>(&shader_module_create_info.source))
        {
            spirv = *spirv_val;
        }
        else
        {
            return StringError("Shader module undefined shader source");
        }

        ReturnIfError(PerformReflection());

        return NoError();
    }

    Error ShaderModule::CreateVulkanShaderModule(vk::Device device)
    {
        auto spirv = GetSpirv();

        auto info = vk::ShaderModuleCreateInfo{}
            .setCode(spirv);

        if (shader_module)
        {
            device.destroyShaderModule(shader_module);
            shader_module = vk::ShaderModule{};
        }

        AssignOrReturnVkError(shader_module, device.createShaderModule(info));

        return NoError();
    }

    Error ShaderModule::Create(ShaderModuleCreateInfo shader_module_create_info_, vk::Device device)
	{
        ReturnIfError(PopulateMetadata(shader_module_create_info_));
        if (shader_module_create_info_.create_shader_module)
        {
            ReturnIfError(CreateVulkanShaderModule(device));
        }
        return NoError();
	}

    vk::ShaderStageFlagBits ShaderModule::GetShaderStageFlagBits() const
    {
        //return TranslateShaderKindToShaderStageFlagBits(shader_module_create_info.shader_kind);
        return static_cast<vk::ShaderStageFlagBits>(static_cast<VkShaderStageFlags>(shader_reflection.stages));
    }

    vk::PipelineShaderStageCreateInfo ShaderModule::GetPipelineShaderStageCreateInfo() const
    {
        auto pipeline_shader_stage_create_info = vk::PipelineShaderStageCreateInfo{}
            .setStage(GetShaderStageFlagBits())
            .setModule(shader_module)
            .setPName(Constants::DEFAULT_SHADER_MAIN.data());

        return pipeline_shader_stage_create_info;
    }

    Spirv ShaderModule::GetSpirv() const
    {
#ifdef ENABLE_SPV_REFLECT
        const auto* spirv_code = spvReflectGetCode(reflect_module.get());
        auto spirv_size = spvReflectGetCodeSize(reflect_module.get());
        auto spirv = std::vector<uint32_t>(spirv_code, spirv_code + (spirv_size / sizeof(uint32_t)));
#else
#endif
        return spirv;
    }

#define SPIRV_REFLECT_OUT_OF_BOUNDS_VALUE (std::numeric_limits<uint32_t>::max())

    Error ShaderModule::ChangeSet(std::string_view data_name, uint32_t new_set)
    {
#ifdef ENABLE_SPV_REFLECT
        for (auto& v_ : reflect_descriptor_bindings)
        {
            auto& v = *v_;

            auto spirv_id = v.spirv_id;
            std::string_view name = v.name;
            auto set = v.set;
            auto binding = v.binding;

            if (v.set == SPIRV_REFLECT_OUT_OF_BOUNDS_VALUE)
            {
                return StringError("Not suppose to happen");
            }
            if (v.binding == SPIRV_REFLECT_OUT_OF_BOUNDS_VALUE)
            {
                return StringError("Not suppose to happen");
            }

            SpvReflectTypeDescription& type_description = *v.type_description;

            if (data_name == name)
            {
                //fmt::print("{}\n", v.name);
                SpvReflectResult result{};
                result = spvReflectChangeDescriptorBindingNumbers(reflect_module.get(), &v, SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE, new_set);
                assert(result == SPV_SUCCESS);
            }
        }
#else
        return StringError("Not supported");
#endif
        return NoError();
    }

    Error ShaderModule::ChangeBinding(std::string_view data_name, uint32_t new_binding)
    {
#ifdef ENABLE_SPV_REFLECT
        for (auto& v_ : reflect_descriptor_bindings)
        {
            auto& v = *v_;

            auto spirv_id = v.spirv_id;
            std::string_view name = v.name;
            auto set = v.set;
            auto binding = v.binding;

            if (v.set == SPIRV_REFLECT_OUT_OF_BOUNDS_VALUE)
            {
                return StringError("Not suppose to happen");
            }
            if (v.binding == SPIRV_REFLECT_OUT_OF_BOUNDS_VALUE)
            {
                return StringError("Not suppose to happen");
            }

            SpvReflectTypeDescription& type_description = *v.type_description;

            if (data_name == name)
            {
                //fmt::print("{}\n", v.name);
                SpvReflectResult result{};
                //const SpvReflectDescriptorBinding* descriptor_binding = spvReflectGetDescriptorBinding(reflect_module.get(), binding, set, &result);
                //assert(result == SPV_SUCCESS);
                result = spvReflectChangeDescriptorBindingNumbers(reflect_module.get(), &v, new_binding, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
                assert(result == SPV_SUCCESS);
            }
        }

#else
        return StringError("Not supported");
#endif

        return NoError();
    }
}
