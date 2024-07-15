#include "../arc/BasicBuffer.hpp"

namespace arc {

[[nodiscard]]
VkPhysicalDeviceMemoryProperties
get_physical_device_memory_properties(const VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(device, &properties);
    return properties;
}   

uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties mem_properties,
                          const uint32_t type_filter,
                          const VkMemoryPropertyFlags property_flags)
{
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i)
        && (mem_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
            return i;
    }
    throw std::runtime_error("failed to find suitable memory type!");
}
 
void create_buffer(const VkPhysicalDevice& physical_device,
                   const VkDevice& logical_device,
                   const VkDeviceSize size,
                   const VkBufferUsageFlags usage,
                   const VkMemoryPropertyFlags properties,
                   VkBufferCreateInfo& out_info,
                   VkBuffer& out_buffer,
                   VkDeviceMemory& out_memory)
{
    out_info = VkBufferCreateInfo{};
    out_buffer = VkBuffer{};
    out_memory = VkDeviceMemory{};

    out_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    out_info.size = size;
    out_info.usage = usage;
    out_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    auto status = vkCreateBuffer(logical_device, &out_info, nullptr, &out_buffer);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create buffer!");

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(logical_device,
                                  out_buffer,
                                  &mem_requirements);
    
    const auto physical_memory_properties =
        get_physical_device_memory_properties(physical_device);
    
    VkMemoryAllocateInfo memalloc_info{};
    memalloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memalloc_info.allocationSize = mem_requirements.size;
    memalloc_info.memoryTypeIndex = find_memory_type(physical_memory_properties, 
                                                     mem_requirements.memoryTypeBits,
                                                     properties);
    status = vkAllocateMemory(logical_device,
                              &memalloc_info,
                              nullptr,
                              &out_memory);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!"
                                 " now you have a leak because we did not clean up anyhting..");
    
    vkBindBufferMemory(logical_device,
                       out_buffer,
                       out_memory, 0);
}

void copy_buffer(const VkDevice& logical_device,
                 const VkCommandPool& command_pool,
                 const VkQueue& graphics_queue,
                 const VkDeviceSize& size,
                 const VkBuffer& src,
                 VkBuffer& dst)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);   
    
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);

    vkEndCommandBuffer(command_buffer);
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
}
    
void memcopy_to_buffer(const VkDevice& logical_device,
                       const void* src,
                       const VkDeviceSize& memsize,
                       VkDeviceMemory& dst)
{
    void* data_ptr;
    vkMapMemory(logical_device, dst, 0, memsize, 0, &data_ptr);
    memcpy(data_ptr, src, static_cast<size_t>(memsize));
    vkUnmapMemory(logical_device, dst); 
}
 
}
