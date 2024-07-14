#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "DeclareNotCopyable.hpp"

#include <memory>
#include <vector>
#include <array>

namespace arc {
    

[[nodiscard]]
VkPhysicalDeviceMemoryProperties
get_physical_device_memory_properties(const VkPhysicalDevice device);
   
uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties mem_properties,
                          const uint32_t type_filter,
                          const VkMemoryPropertyFlags property_flags);
 
void create_buffer(const VkPhysicalDevice& physical_device,
                   const VkDevice& logical_device,
                   const VkDeviceSize size,
                   const VkBufferUsageFlags usage,
                   const VkMemoryPropertyFlags properties,
                   VkBufferCreateInfo& out_info,
                   VkBuffer& out_buffer,
                   VkDeviceMemory& out_memory);

void copy_buffer(const VkDevice& logical_device,
                 const VkCommandPool& command_pool,
                 const VkQueue& graphics_queue,
                 const VkDeviceSize& size,
                 const VkBuffer& src,
                 VkBuffer& dst);

   
template <class Type, class TypeTag>
class BasicBuffer : public DeclareNotCopyable {
public:
    using value_type = Type;

    [[nodiscard]]
    static std::unique_ptr<BasicBuffer> create(const VkPhysicalDevice& physical_device,
                                               const VkDevice& logical_device,
                                               const std::vector<Type>& values);

    [[nodiscard]]
    static std::unique_ptr<BasicBuffer> create_staging(const VkPhysicalDevice& physical_device,
                                                       const VkDevice& logical_device,
                                                       const VkCommandPool& command_pool,
                                                       const VkQueue& graphics_queue,
                                                       const std::vector<Type>& values);
 
    BasicBuffer() = delete;
    BasicBuffer(const VkDevice& logical_device);

    [[nodiscard]]
    size_t get_count();

    [[nodiscard]]
    size_t get_memsize();

    [[nodiscard]]
    VkBuffer get_buffer();

    ~BasicBuffer();

private:
    const VkDevice& m_logical_device;
    VkBufferCreateInfo m_info;
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    size_t m_count;
};
    
template <class Type, class TypeTag>
std::unique_ptr<BasicBuffer<Type, TypeTag>> BasicBuffer<Type, TypeTag>::create(
    const VkPhysicalDevice& physical_device,
    const VkDevice& logical_device,
    const std::vector<Type>& values)
{
    auto buffer = std::make_unique<BasicBuffer<Type, TypeTag>>(logical_device);
    buffer->m_count = values.size();
    create_buffer(physical_device,
                  logical_device,
                  sizeof(values[0]) * values.size(),
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  buffer->m_info,
                  buffer->m_buffer,
                  buffer->m_memory);

    void* data_ptr;
    vkMapMemory(logical_device,
                buffer->m_memory,
                0,
                buffer->get_memsize(),
                0,
                &data_ptr);

    memcpy(data_ptr, values.data(), static_cast<size_t>(buffer->get_memsize()));
    vkUnmapMemory(logical_device, buffer->m_memory);
    return buffer;
}
    
template <class Type, class TypeTag>
std::unique_ptr<BasicBuffer<Type, TypeTag>>
BasicBuffer<Type, TypeTag>::create_staging(const VkPhysicalDevice& physical_device,
                                             const VkDevice& logical_device,
                                             const VkCommandPool& command_pool,
                                             const VkQueue& graphics_queue,
                                             const std::vector<Type>& values)
{
    const auto size = sizeof(values[0]) * values.size();

    auto staging = std::make_unique<BasicBuffer<Type, TypeTag>>(logical_device);
    staging->m_count = values.size();
    create_buffer(physical_device,
                  logical_device,
                  size,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  staging->m_vertice_info,
                  staging->m_vertex_buffer,
                  staging->m_vertex_buffer_memory);

    void* data_ptr;
    vkMapMemory(logical_device,
                staging->m_vertex_buffer_memory,
                0,
                staging->get_memsize(),
                0,
                &data_ptr);
    
    memcpy(data_ptr, values.data(), staging->get_memsize());
    vkUnmapMemory(logical_device, staging->m_vertex_buffer_memory);

    auto buffer = std::make_unique<BasicBuffer<Type, TypeTag>>(logical_device);
    buffer->m_count = values.size();
    create_buffer(physical_device,
                  logical_device,
                  size,
                  TypeTag::bit | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  buffer->m_vertice_info,
                  buffer->m_vertex_buffer,
                  buffer->m_vertex_buffer_memory);

    copy_buffer(logical_device,
                command_pool,
                graphics_queue,
                size,
                staging->m_vertex_buffer,
                buffer->m_vertex_buffer);
   
    return buffer;
}
    
template <class Type, class TypeTag>
VkBuffer BasicBuffer<Type, TypeTag>::get_buffer()
{
    return m_buffer;
}

template <class Type, class TypeTag>
size_t BasicBuffer<Type, TypeTag>::get_memsize()
{
    return m_info.size;
}

template <class Type, class TypeTag>
size_t BasicBuffer<Type, TypeTag>::get_count()
{
    return m_count;
}

template <class Type, class TypeTag>
BasicBuffer<Type, TypeTag>::~BasicBuffer()
{
    vkDestroyBuffer(m_logical_device, m_buffer, nullptr);
    vkFreeMemory(m_logical_device, m_memory, nullptr);
}
 
} 
