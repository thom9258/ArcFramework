#include "../arc/UniformBuffer.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>

namespace ArcGraphics {

  /*
*/
VkDescriptorSetLayoutBinding create_descriptor_set_layout_binding(const uint32_t binding_index,
                                                                  const uint32_t count,
                                                                  const VkShaderStageFlags flags)
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = binding_index;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = count;
    binding.stageFlags = flags;
    return binding;
}
    
VkDescriptorSetLayout create_uniform_descriptorset_layout(
    const VkDevice& logical_device,
    const uint32_t binding_index,
    const uint32_t count,
    const VkShaderStageFlags flags)
{
    const auto binding =
        create_descriptor_set_layout_binding(binding_index, count, flags);
    
        VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &binding;
    
    VkDescriptorSetLayout layout{};
    auto status = vkCreateDescriptorSetLayout(logical_device,
                                              &layout_info,
                                              nullptr,
                                              &layout);

    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
    return layout;
}

VkDescriptorSetLayout create_uniform_vertex_descriptorset_layout(const VkDevice& logical_device,
                                                                 const uint32_t binding_index,
                                                                 const uint32_t count)
{
    return create_uniform_descriptorset_layout(logical_device,
                                               binding_index,
                                               count,
                                               VK_SHADER_STAGE_VERTEX_BIT);
}
    
void create_uniform_buffer(const VkPhysicalDevice& physical_device,
                           const VkDevice& logical_device,
                           const VkDeviceSize& memsize,
                           VkBuffer& out_buffer,
                           VkDeviceMemory& out_memory,
                           void*& out_mapped)
{
    VkBufferCreateInfo info;
    create_buffer(physical_device,
                  logical_device,
                  memsize,
                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  info,
                  out_buffer,
                  out_memory);

    vkMapMemory(logical_device,
                out_memory,
                0,
                memsize,
                0,
                &out_mapped);
}

}
