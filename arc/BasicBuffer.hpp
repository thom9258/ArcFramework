#pragma once

#include <vulkan/vulkan.h>

#include "TypeTraits.hpp"
#include "Algorithm.hpp"

#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <string.h>

namespace ArcGraphics {

  

void create_buffer(const VkPhysicalDevice& physical_device,
                   const VkDevice& logical_device,
                   const VkDeviceSize size,
                   const VkBufferUsageFlags usage,
                   const VkMemoryPropertyFlags properties,
                   VkBufferCreateInfo& out_info,
                   VkBuffer& out_buffer,
                   VkDeviceMemory& out_memory);
    
void with_single_use_command_buffer(const VkDevice& logical_device,
                                    const VkCommandPool& command_pool,
                                    const VkQueue graphics_queue,
                                    std::function<void(VkCommandBuffer&)>&& f);

void copy_buffer(const VkDevice& logical_device,
                 const VkCommandPool& command_pool,
                 const VkQueue& graphics_queue,
                 const VkDeviceSize& size,
                 const VkBuffer& src,
                 VkBuffer& dst);
   
 void transition_image_layout(const VkDevice& logical_device,
                             const VkCommandPool& command_pool,
                             const VkQueue& graphics_queue,
                             VkImage image,
                             VkFormat /*format*/,
                             VkImageLayout oldLayout,
                             VkImageLayout newLayout);

void copy_buffer_to_image(const VkDevice& logical_device,
                          const VkCommandPool& command_pool,
                          const VkQueue& graphics_queue,
                          VkBuffer buffer,
                          VkImage image,
                          uint32_t width,
                          uint32_t height);

void memcopy_to_buffer(const VkDevice& logical_device,
                       const void* src,
                       const VkDeviceSize& memsize,
                       VkDeviceMemory& dst);
    
void with_memory_mapping(const VkDevice& logical_device,
                         const VkDeviceSize size,
                         const VkDeviceMemory& target_memory,
                         const std::function<void(void*)> lambda);
   

template<typename T>
concept BasicBufferPolicy = requires
{
    { T::buffer_type_bit };
    { std::same_as<decltype(T::buffer_type_bit), uint32_t> };
    //{ T::value_type }; // TODO: is it possible to check for using directives in concepts?
};
   
template <BasicBufferPolicy Policy>
class BasicBuffer : public IsNotLvalueCopyable 
{
public:
    using value_type = typename Policy::value_type;
    using vector_type = std::vector<value_type>;

    [[nodiscard]]
    static std::unique_ptr<BasicBuffer> create(const VkPhysicalDevice& physical_device,
                                               const VkDevice& logical_device,
                                               const vector_type& values);

    [[nodiscard]]
    static std::unique_ptr<BasicBuffer> create_staging(const VkPhysicalDevice& physical_device,
                                                       const VkDevice& logical_device,
                                                       const VkCommandPool& command_pool,
                                                       const VkQueue& graphics_queue,
                                                       const vector_type& values);
 
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
    
template <BasicBufferPolicy Policy>
std::unique_ptr<BasicBuffer<Policy>> BasicBuffer<Policy>::create(
    const VkPhysicalDevice& physical_device,
    const VkDevice& logical_device,
    const vector_type& values)
{
    auto buffer = std::make_unique<BasicBuffer<Policy>>(logical_device);
    buffer->m_count = values.size();
    create_buffer(physical_device,
                  logical_device,
                  sizeof(values[0]) * values.size(),
                  Policy::buffer_type_bit,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  buffer->m_info,
                  buffer->m_buffer,
                  buffer->m_memory);

    memcopy_to_buffer(logical_device,
                      static_cast<const void*>(values.data()),
                      sizeof(values[0]) * values.size(),
                      buffer->m_memory);
    return buffer;
}
    
template <BasicBufferPolicy Policy>
std::unique_ptr<BasicBuffer<Policy>>
BasicBuffer<Policy>::create_staging(const VkPhysicalDevice& physical_device,
                                    const VkDevice& logical_device,
                                    const VkCommandPool& command_pool,
                                    const VkQueue& graphics_queue,
                                    const vector_type& values)
{
    const auto size = sizeof(values[0]) * values.size();
    auto staging = std::make_unique<BasicBuffer<Policy>>(logical_device);
    staging->m_count = values.size();
    create_buffer(physical_device,
                  logical_device,
                  size,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  staging->m_info,
                  staging->m_buffer,
                  staging->m_memory);

    memcopy_to_buffer(logical_device,
                      static_cast<const void*>(values.data()),
                      size,
                      staging->m_memory);

    auto buffer = std::make_unique<BasicBuffer<Policy>>(logical_device);
    buffer->m_count = values.size();
    create_buffer(physical_device,
                  logical_device,
                  size,
                  Policy::buffer_type_bit | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  buffer->m_info,
                  buffer->m_buffer,
                  buffer->m_memory);

    copy_buffer(logical_device,
                command_pool,
                graphics_queue,
                size,
                staging->m_buffer,
                buffer->m_buffer);
   
    return buffer;
}
    
template <BasicBufferPolicy Policy>
BasicBuffer<Policy>::BasicBuffer(const VkDevice& logical_device)
    : m_logical_device(logical_device)
{
}
 
template <BasicBufferPolicy Policy>
BasicBuffer<Policy>::~BasicBuffer()
{
    vkDestroyBuffer(m_logical_device, m_buffer, nullptr);
    vkFreeMemory(m_logical_device, m_memory, nullptr);
}
   
template <BasicBufferPolicy Policy>
VkBuffer BasicBuffer<Policy>::get_buffer()
{
    return m_buffer;
}

template <BasicBufferPolicy Policy>
size_t BasicBuffer<Policy>::get_memsize()
{
    return m_info.size;
}

template <BasicBufferPolicy Policy>
size_t BasicBuffer<Policy>::get_count()
{
    return m_count;
}
 
} 
