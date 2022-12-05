#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
    struct VariableBinding
    {
        uint32_t binding;
        uint32_t num_bindings;

        bool operator==(const VariableBinding& o) const
        {
            return std::tie(binding, num_bindings) == std::tie(o.binding, o.num_bindings);
        }
    };

    struct DescriptorBinding
    {
        vk::DescriptorSetLayoutBinding descriptor_set_layout_binding;

        bool operator==(const DescriptorBinding& o) const
        {
            return descriptor_set_layout_binding == o.descriptor_set_layout_binding;
        }
    };

    class DescriptorSetLayoutCreateInfo
    {
    public:
        /*
        DescriptorSetLayoutCreateInfo() = default;
        DescriptorSetLayoutCreateInfo(const DescriptorSetLayoutCreateInfo& other) = delete;
        DescriptorSetLayoutCreateInfo& operator=(const DescriptorSetLayoutCreateInfo& other) = delete;
        DescriptorSetLayoutCreateInfo(DescriptorSetLayoutCreateInfo&& other) = default;
        DescriptorSetLayoutCreateInfo& operator=(DescriptorSetLayoutCreateInfo&& other) = default;
        */

        void AddBinding(uint32_t binding, uint32_t count, vk::DescriptorType type, vk::ShaderStageFlags stage);
        void EnableVariableBindings(uint32_t binding, uint32_t num_bindings);
        void Reset();
        auto& GetBindings() { return bindings; }
        const auto& GetBindings() const { return bindings; }
        const auto& GetVariableBinding() const { return variable_binding; }
    private:
        flat_hash_map<uint32_t, DescriptorBinding> bindings;
        std::optional<VariableBinding> variable_binding{};

    public:
        bool operator==(const DescriptorSetLayoutCreateInfo& o) const
        {
            return bindings == o.bindings && variable_binding == o.variable_binding;
        }
    };

    class DescriptorSetLayoutCreateInfos
    {
    public:
        void AddBinding(uint32_t set, uint32_t binding, uint32_t count, vk::DescriptorType type, vk::ShaderStageFlags stage);
        void EnableVariableBindings(uint32_t set, uint32_t binding, uint32_t num_bindings);
        auto& GetCreateInfos() { return set_to_bindings; }
        const auto& GetCreateInfos() const { return set_to_bindings; }
    private:
        std::vector<DescriptorSetLayoutCreateInfo> set_to_bindings{};

    public:
        bool operator==(const DescriptorSetLayoutCreateInfos& o) const
        {
            return set_to_bindings == o.set_to_bindings;
        }
    };

    struct DescriptorSetLayout
    {
    public:
        vk::DescriptorSetLayout descriptor_set_layout{};
        DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};

    public:
        vk::DescriptorSetLayout operator*() const
        {
            return descriptor_set_layout;
        }

        operator vk::DescriptorSetLayout() noexcept
        {
            return descriptor_set_layout;
        }

    public:
        bool operator==(const DescriptorSetLayout& o) const noexcept {
            //return descriptor_set_layout == o.descriptor_set_layout && descriptor_set_layout_create_info == o.descriptor_set_layout_create_info;
            return descriptor_set_layout_create_info == o.descriptor_set_layout_create_info;
        }
    };

    struct DescriptorSetLayouts
    {
    public:
        std::vector<CMShared<DescriptorSetLayout>> descriptor_set_layouts{};

        std::vector<vk::DescriptorSetLayout> vulkan_descriptor_set_layouts{};

        const std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayouts();
        flat_hash_map<uint32_t, VariableBinding> GetVariableBindings() const;

    public:
        bool operator==(const DescriptorSetLayouts& o) const noexcept {
            return CheckEquality(descriptor_set_layouts, o.descriptor_set_layouts);
        }
    };
}

namespace std
{
    using namespace RaysterizerEngine;

    template<>
    struct hash<VariableBinding>
    {
        size_t operator()(const VariableBinding& o) const noexcept
        {
            return Hash(o.binding, o.num_bindings);
        }
    };

    template<>
    struct hash<DescriptorBinding>
    {
        size_t operator()(const DescriptorBinding& o) const noexcept
        {
            return Hash(o.descriptor_set_layout_binding);
        }
    };

    template<>
    struct hash<DescriptorSetLayoutCreateInfo>
    {
        size_t operator()(const DescriptorSetLayoutCreateInfo& o) const noexcept
        {
            size_t h{};
            for (const auto& [index, binding] : o.GetBindings())
            {
                HashCombine(h, StdHash(index));
                HashCombine(h, StdHash(binding));
            }
            if (o.GetVariableBinding())
            {
                HashCombine(h, StdHash(*o.GetVariableBinding()));
            }
            return h;
        }
    };

    template<>
    struct hash<DescriptorSetLayoutCreateInfos>
    {
        size_t operator()(const DescriptorSetLayoutCreateInfos& o) const noexcept
        {
            size_t h{};
            for (const auto& set_to_binding : o.GetCreateInfos())
            {
                HashCombine(h, StdHash(set_to_binding));
            }
            return h;
        }
    };

    template<>
    struct hash<DescriptorSetLayout>
    {
        size_t operator()(const DescriptorSetLayout& o) const noexcept
        {
            size_t h{};
            //HashCombine(h, Hash(o.descriptor_set_layout));
            HashCombine(h, StdHash(o.descriptor_set_layout_create_info));
            return h;
        }
    };

    template<>
    struct hash<DescriptorSetLayouts>
    {
        size_t operator()(const DescriptorSetLayouts& o) const noexcept
        {
            size_t h{};
            for (const auto& descriptor_set_layouts : o.descriptor_set_layouts)
            {
                HashCombine(h, StdHash(descriptor_set_layouts));
            }
            return h;
        }
    };
}
