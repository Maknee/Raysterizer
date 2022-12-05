#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
    struct ShaderModuleSourceCreateInfo
    {
        std::string source;
        std::string filename;
        ShaderKind shader_kind = ShaderKind::UNKNOWN;

        static ShaderModuleSourceCreateInfo File(fs::path path);

    public:
        bool operator==(const ShaderModuleSourceCreateInfo& o) const
        {
            return source == o.source && filename == o.filename && shader_kind == o.shader_kind;
        }
    };

    struct ShaderModuleCreateInfo
    {
        //explicit ShaderModuleCreateInfo() {}
        //explicit ShaderModuleCreateInfo(Spirv spirv) : source(std::move(spirv)) {}
        //explicit ShaderModuleCreateInfo(ShaderModuleSourceCreateInfo smsci) : source(std::move(smsci)) {}

        std::variant<Spirv, ShaderModuleSourceCreateInfo> source;
        bool create_shader_module = true;

    public:
        bool operator==(const ShaderModuleCreateInfo& o) const
        {
            return source == o.source;
        }
    };

    void to_json(json& j, const ShaderModuleCreateInfo& shader_module_create_info);
    void from_json(const json& j, ShaderModuleCreateInfo& shader_module_create_info);

    class ShaderModule
    {
    public:
        explicit ShaderModule() {};
        ~ShaderModule() {};

#ifdef ENABLE_SPV_REFLECT
    private:
        void swap(ShaderModule& lhs, ShaderModule& rhs)
        {
            std::swap(lhs.shader_module, rhs.shader_module);
            std::swap(lhs.shader_module_create_info, rhs.shader_module_create_info);
            std::swap(lhs.spirv, rhs.spirv);
            std::swap(lhs.shader_reflection, rhs.shader_reflection);
            std::swap(lhs.reflect_module, rhs.reflect_module);
            std::swap(lhs.reflect_descriptor_bindings, rhs.reflect_descriptor_bindings);
        }

    public:
        ShaderModule(const ShaderModule& other) :
            shader_module(other.shader_module),
            shader_module_create_info(other.shader_module_create_info),
            spirv(other.spirv),
            shader_reflection(other.shader_reflection)
            //reflect_module(SpvReflectModule(new SpvReflectShaderModule(*other.reflect_module), &spvReflectDestroyShaderModule)),
            //reflect_descriptor_bindings(other.reflect_descriptor_bindings)
        {
            PanicIfError(PerformReflection());
        }

        ShaderModule(ShaderModule&& other) :
            shader_module(std::move(other.shader_module)),
            shader_module_create_info(std::move(other.shader_module_create_info)),
            spirv(std::move(other.spirv)),
            shader_reflection(std::move(other.shader_reflection)),
            reflect_module(std::move(other.reflect_module)),
            reflect_descriptor_bindings(std::move(other.reflect_descriptor_bindings))
        {
        }

        ShaderModule& operator=(const ShaderModule& other)
        {
            if (&other != this)
            {
                ShaderModule temp(other);
                swap(*this, temp);
            }
            return *this;
        }

        ShaderModule& operator=(ShaderModule&& other)
        {
            if (&other != this)
            {
                swap(*this, other);
            }
            return *this;
        }
#endif
        Error PerformReflection();
        Error PopulateMetadata(ShaderModuleCreateInfo shader_module_create_info_);
        Error CreateVulkanShaderModule(vk::Device device);
        Error Create(ShaderModuleCreateInfo shader_module_create_info_, vk::Device device);
        vk::ShaderStageFlagBits GetShaderStageFlagBits() const;
        vk::PipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo() const;
        Spirv GetSpirv() const;
        Error ChangeSet(std::string_view data_name, uint32_t new_set);
        Error ChangeBinding(std::string_view data_name, uint32_t new_binding);

    public:
        vk::ShaderModule shader_module{};
        ShaderModuleCreateInfo shader_module_create_info{};
        Spirv spirv{};
        ShaderReflection shader_reflection{};

    public:
        vk::ShaderModule operator*() const
        {
            return shader_module;
        }

        operator vk::ShaderModule() noexcept
        {
            return shader_module;
        }

    public:
        bool operator==(const ShaderModule& o) const
        {
            return spirv == o.spirv;
        }

#ifdef ENABLE_SPV_REFLECT
    private:
        using SpvReflectModule = std::unique_ptr<SpvReflectShaderModule, std::function<void(SpvReflectShaderModule*)>>;
        SpvReflectModule reflect_module = SpvReflectModule(new SpvReflectShaderModule{}, &spvReflectDestroyShaderModule);
        std::vector<SpvReflectDescriptorBinding*> reflect_descriptor_bindings;
#endif
    };
}

namespace std
{
    using namespace RaysterizerEngine;

    template<>
    struct hash<ShaderModuleSourceCreateInfo>
    {
        size_t operator()(const ShaderModuleSourceCreateInfo& o) const noexcept
        {
            return Hash(o.source, o.filename);
        }
    };

    template<>
    struct hash<ShaderModuleCreateInfo>
    {
        size_t operator()(const ShaderModuleCreateInfo& o) const noexcept
        {
            size_t h{};
            if (auto spirv = std::get_if<Spirv>(&o.source))
            {
                HashCombine(h, Hash(*spirv));
            }
            else if (auto shader_module_source_create_info = std::get_if<ShaderModuleSourceCreateInfo>(&o.source))
            {
                HashCombine(h, Hash(shader_module_source_create_info->source));
                HashCombine(h, Hash(shader_module_source_create_info->filename));
                HashCombine(h, Hash(shader_module_source_create_info->shader_kind));
            }
            return h;
        }
    };

    template<>
    struct hash<ShaderModule>
    {
        size_t operator()(const ShaderModule& o) const noexcept
        {
            size_t h{};
            //HashCombine(h, Hash(o.shader_module));
            //HashCombine(h, StdHash(o.shader_module_create_info));
            HashCombine(h, Hash(o.spirv));
            return h;
        }
    };
}
