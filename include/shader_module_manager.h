#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	class ShaderModuleManager
	{
	public:
		explicit ShaderModuleManager() {}
		explicit ShaderModuleManager(Context* c_);
		Expected<CMShared<ShaderModule>> LoadShader(const fs::path& path);
		Expected<CMShared<ShaderModule>> LoadShader(const ShaderModuleCreateInfo& shader_module_create_info);
		Error CacheShader(const ShaderModuleCreateInfo& shader_module_create_info, CMShared<ShaderModule> shader_module);
		Error Flush();
		static Expected<fs::path> FindFileInSearchPath(const fs::path& p);
		Error CacheShaderToDisk(std::size_t shader_hash, CMShared<ShaderModule> shader_module);

	private:
		Expected<Spirv> LoadCachedSpirvFromDisk(const fs::path& path);
		Expected<CMShared<ShaderModule>> LoadShaderFromDisk(const fs::path& path);

	private:
		Context* c;
		CacheMappingWithFrameCounter<ShaderModuleCreateInfo, Spirv> source_to_spirv_cache;
		flat_hash_map<std::size_t, fs::path> shader_hash_to_cached_path;

		inline static constexpr std::string_view SPV_EXTENSION = ".spv";
		inline static tf::Executor executor{};
	};
}