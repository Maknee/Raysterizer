#include "include/descriptor_set_layout.h"

namespace RaysterizerEngine
{
    void DescriptorSetLayoutCreateInfo::AddBinding(uint32_t binding, uint32_t count, vk::DescriptorType type, vk::ShaderStageFlags stage)
    {
        if (variable_binding)
        {
            count = variable_binding->num_bindings;
        }

        vk::DescriptorSetLayoutBinding b{};
        b
            .setBinding(binding)
            .setDescriptorCount(count)
            .setDescriptorType(type)
            .setStageFlags(stage);

        bindings[binding] = DescriptorBinding{ b };
    }

    void DescriptorSetLayoutCreateInfo::EnableVariableBindings(uint32_t binding, uint32_t num_bindings)
    {
        VariableBinding vb{ binding, num_bindings };
        variable_binding = vb;
        bindings[binding].descriptor_set_layout_binding.setDescriptorCount(num_bindings);
    }

    void DescriptorSetLayoutCreateInfo::Reset()
    {
        bindings.clear();
    }

    void DescriptorSetLayoutCreateInfos::AddBinding(uint32_t set, uint32_t binding, uint32_t count, vk::DescriptorType type, vk::ShaderStageFlags stage)
    {
        if (set_to_bindings.size() <= set)
        {
            set_to_bindings.resize(set + 1);
        }

        set_to_bindings[set].AddBinding(binding, count, type, stage);
    }

    void DescriptorSetLayoutCreateInfos::EnableVariableBindings(uint32_t set, uint32_t binding, uint32_t num_bindings)
    {
        if (set_to_bindings.size() <= set)
        {
            set_to_bindings.resize(set + 1);
        }

        set_to_bindings[set].EnableVariableBindings(binding, num_bindings);
    }

    const std::vector<vk::DescriptorSetLayout>& DescriptorSetLayouts::GetDescriptorSetLayouts()
    {
        if (vulkan_descriptor_set_layouts.size() != descriptor_set_layouts.size())
        {
            vulkan_descriptor_set_layouts.resize(descriptor_set_layouts.size());

            std::transform(std::begin(descriptor_set_layouts), std::end(descriptor_set_layouts), std::begin(vulkan_descriptor_set_layouts),
                [](const auto& e) { return e->descriptor_set_layout; }
            );
        }

        return vulkan_descriptor_set_layouts;
    }

    flat_hash_map<uint32_t, VariableBinding> DescriptorSetLayouts::GetVariableBindings() const
    {
        flat_hash_map<uint32_t, VariableBinding> variable_bindings;

        for (auto i = 0; i < descriptor_set_layouts.size(); i++)
        {
            if (auto variable_binding = descriptor_set_layouts[i]->descriptor_set_layout_create_info.GetVariableBinding(); variable_binding)
            {
                variable_bindings[i] = *variable_binding;
            }
        }

        return variable_bindings;
    }

}