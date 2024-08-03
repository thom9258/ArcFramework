#pragma once

#include <vulkan/vulkan.h>

#include "TypeTraits.hpp"
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
                           const VkDeviceSize memsize,
                           VkBuffer& out_buffer,
                           VkDeviceMemory& out_memory,
                           void*& out_mapped);


class UniformBuffer : IsNotLvalueCopyable
{
public:
    static std::unique_ptr<UniformBuffer> create(const VkPhysicalDevice& physical_device,
                                                 const VkDevice& logical_device,
                                                 const VkDeviceSize memsize);

    UniformBuffer(VkBuffer buffer,
                  VkDeviceMemory memory,
                  void* mapping,
                  VkDeviceSize size
                  );

    void destroy(const VkDevice& logical_device);
    
    VkDescriptorBufferInfo descriptor_buffer_info();
private:
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    void* m_mapping;
    VkDeviceSize m_size;
};

}
