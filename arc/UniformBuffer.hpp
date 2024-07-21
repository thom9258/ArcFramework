#pragma once

#include <vulkan/vulkan.h>

#include "BasicBuffer.hpp"

namespace ArcGraphics {

[[nodiscard]]
VkDescriptorSetLayoutBinding create_descriptor_set_layout_binding(const uint32_t binding_index,
                                                                  const uint32_t count,
                                                                  const VkShaderStageFlags flags);

[[nodiscard]]
VkDescriptorSetLayout create_uniform_descriptor_set_layout(const VkDevice& logical_device,
                                                           const uint32_t binding_index,
                                                           const uint32_t count,
                                                           const VkShaderStageFlags flags);
   
[[nodiscard]]
VkDescriptorSetLayout create_uniform_vertex_descriptorset_layout(const VkDevice& logical_device,
                                                                 const uint32_t binding_index,
                                                                 const uint32_t count);
    
void create_uniform_buffer(const VkPhysicalDevice& physical_device,
                           const VkDevice& logical_device,
                           const VkDeviceSize& memsize,
                           VkBuffer& out_buffer,
                           VkDeviceMemory& out_memory,
                           void*& out_mapped);

}
