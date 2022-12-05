#include "include/shader_module_manager.h"

namespace RaysterizerEngine
{
    static inline std::string ReadFile(const fs::path& path)
    {
        std::ifstream file(path);

        if (file)
        {
            std::string data((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());
            return data;
        }

        PANIC("NO DATA");

        return "";
    }

    static inline std::vector<uint32_t> ReadFileAsU32s(const fs::path& path)
    {
        std::ifstream file(path, std::ios::binary);

        if (file)
        {
            file.seekg(0, std::ios::end);
            auto file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<uint32_t> data(file_size / sizeof(uint32_t));

            file.read(reinterpret_cast<char*>(data.data()), file_size);

            return data;
        }

        PANIC("NO DATA");

        return {};
    }

    ShaderModuleManager::ShaderModuleManager(Context* c_) : c(c_)
    {
        source_to_spirv_cache.SetContext(c);

        fs::path cache_path = Config["shader"]["compiler"]["cache_path"];
        if (!fs::exists(cache_path))
        {
            fs::create_directories(cache_path);
        }

        /*
        for (const auto& dir_entry : fs::directory_iterator(cache_path))
        {
            if (dir_entry.is_regular_file())
            {
                const auto& p = dir_entry.path();
                if (p.extension() == SPV_EXTENSION)
                {
                    auto shader_hash = std::stoi(p.stem().string());
                    shader_hash_to_cached_path.emplace(shader_hash, p);
                }
            }
        }
        */
    }

    Expected<CMShared<ShaderModule>> ShaderModuleManager::LoadShader(const fs::path& path)
	{
        if (auto result = LoadShaderFromDisk(path))
        {
            return *result;
        }
        else
        {
            return StringError("Shader path does not exist {}", path.string());
        }
	}

    Expected<CMShared<ShaderModule>> ShaderModuleManager::LoadShader(const ShaderModuleCreateInfo& shader_module_create_info)
    {
        // Hit source -> spirv cache
        if (auto result = source_to_spirv_cache.Get(shader_module_create_info))
        {
            const auto& spirv = *result;

            auto spirv_shader_module_create_info = ShaderModuleCreateInfo{ spirv };
            CMShared<ShaderModule> shader_module = AssignOrPanic(c->CreateShaderModule(spirv_shader_module_create_info));
            return shader_module;
        }
        else
        {
            ConsumeError(result.takeError());

            // Load source and cache it as spirv
            if (auto shader_module_source_create_info = std::get_if<ShaderModuleSourceCreateInfo>(&shader_module_create_info.source))
            {
                auto hash = StdHash(*shader_module_source_create_info);
                auto hash_string = fmt::format("{}{}", hash, SPV_EXTENSION);

                if (auto result = LoadCachedSpirvFromDisk(hash_string))
                {
                    auto spirv = *result;
                    auto spirv_shader_module_create_info = ShaderModuleCreateInfo{ spirv };
                    CMShared<ShaderModule> shader_module = AssignOrPanic(c->CreateShaderModule(spirv_shader_module_create_info));
                    return shader_module;
                }
                else
                {
                    ConsumeError(result.takeError());
                }

                if (!shader_module_source_create_info->filename.empty())
                {
                    if (auto shader_module = LoadShaderFromDisk(shader_module_source_create_info->filename))
                    {
                        PanicIfError(CacheShader(shader_module_create_info, *shader_module));
                        return *shader_module;
                    }
                    else
                    {
                        // Must be spirv at this point...
                        ConsumeError(shader_module.takeError());
                    }
                }
                else
                {
                    // only source
                    //
                }
            }
        }

        // Spirv
        AssignOrReturnError(CMShared<ShaderModule> shader_module, c->CreateShaderModule(shader_module_create_info));
        PanicIfError(CacheShader(shader_module_create_info, shader_module));

        return shader_module;
    }

    Expected<Spirv> ShaderModuleManager::LoadCachedSpirvFromDisk(const fs::path& path)
    {
        // check cache
        static fs::path cache_path = Config["shader"]["compiler"]["cache_path"];

        auto full_path = cache_path / path;

        if (!fs::exists(full_path))
        {
            return StringError("Cached spirv path does not exist {}", full_path.string());
        }

        auto spirv = ReadFileAsU32s(full_path);

        return spirv;
    }

    Expected<CMShared<ShaderModule>> ShaderModuleManager::LoadShaderFromDisk(const fs::path& path)
    {
        AssignOrReturnError(auto full_path, FindFileInSearchPath(path));
        auto extension = full_path.extension();
        auto p = full_path.string();

        ShaderModuleCreateInfo shader_module_create_info{};
        if (extension == SPV_EXTENSION)
        {
            Spirv spirv;
            AssignOrReturnError(spirv, LoadCachedSpirvFromDisk(path));
            shader_module_create_info = ShaderModuleCreateInfo{ spirv };
        }
        else
        {
            auto source = ReadFile(full_path);
            auto filename = full_path.string();
            auto shader_kind = GetShaderKindFromExtension(extension.string());
            shader_module_create_info = ShaderModuleCreateInfo
            {
                ShaderModuleSourceCreateInfo{ source, filename, shader_kind }
            };
        }
        CMShared<ShaderModule> shader_module = AssignOrPanic(c->CreateShaderModule(shader_module_create_info));
        return shader_module;
    }

    Error ShaderModuleManager::CacheShader(const ShaderModuleCreateInfo& shader_module_create_info, CMShared<ShaderModule> shader_module)
    {
        source_to_spirv_cache.Emplace(shader_module_create_info, shader_module->spirv);

        fs::path cache_path = Config["shader"]["compiler"]["cache_path"];
        if (!fs::exists(cache_path))
        {
            fs::create_directories(cache_path);
        }
        std::size_t shader_hash = StdHash(shader_module_create_info);

        if (auto found = shader_hash_to_cached_path.find(shader_hash); found != std::end(shader_hash_to_cached_path))
        {

        }
        else
        {
            ReturnIfError(CacheShaderToDisk(shader_hash, shader_module));
        }
        return NoError();
    }

    Error ShaderModuleManager::Flush()
    {
        source_to_spirv_cache.Clear();
        return NoError();
    }

    Expected<fs::path> ShaderModuleManager::FindFileInSearchPath(const fs::path& p)
    {
        fs::path full_path;

        std::vector<std::string> search_paths = Config["shader"]["compiler"]["search_paths"];
        for (const auto& search_path : search_paths)
        {
            fs::path exact_path = fs::path(search_path) / p;
            if (fs::exists(exact_path))
            {
                full_path = exact_path;
            }
        }

        if (full_path.empty())
        {
            return StringError("Path does not exist {}", p.string());
        }

        return full_path;
    }

    Error ShaderModuleManager::CacheShaderToDisk(std::size_t shader_hash, CMShared<ShaderModule> shader_module)
    {
        fs::path cache_path = Config["shader"]["compiler"]["cache_path"];
        auto shader_cache_path = cache_path / fmt::format("{}{}", shader_hash, SPV_EXTENSION);
        shader_hash_to_cached_path.emplace(shader_hash, shader_cache_path.string());

        // Write to disk
        if (!fs::exists(shader_cache_path))
        {
            const auto& spirv = shader_module->spirv;
            executor.silent_async([spirv, shader_cache_path]()
                {
                    std::ofstream file(shader_cache_path, std::ios::binary | std::ios::trunc);
                    if (!file)
                    {
                        PANIC("Could not open shader cache path for writing: {}", shader_cache_path.string());
                    }
                    file.write((char*)spirv.data(), spirv.size() * sizeof(spirv[0]));
                });
        }

        return NoError();
    }

}