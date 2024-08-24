#include "../arc/BasicBuffer.hpp"

namespace ArcGraphics {


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
        throw std::runtime_error("failed to allocate buffer memory!"
                                 " now you have a leak because we did not clean up anyhting..");
    
    vkBindBufferMemory(logical_device,
                       out_buffer,
                       out_memory, 0);
}
    
[[nodiscard]]
VkCommandBuffer begin_single_use_command_buffer(const VkDevice& logical_device,
                                                const VkCommandPool& command_pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void end_single_use_command_buffer(const VkDevice& logical_device,
                                   const VkCommandPool& command_pool,
                                   const VkCommandBuffer commandBuffer,
                                   const VkQueue graphics_queue)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &commandBuffer);
}
    
void with_single_use_command_buffer(const VkDevice& logical_device,
                                    const VkCommandPool& command_pool,
                                    const VkQueue graphics_queue,
                                    std::function<void(VkCommandBuffer&)>&& f)
{
    auto command_buffer = begin_single_use_command_buffer(logical_device, command_pool);
    f(command_buffer);
    end_single_use_command_buffer(logical_device, command_pool, command_buffer, graphics_queue);
}

void copy_buffer(const VkDevice& logical_device,
                 const VkCommandPool& command_pool,
                 const VkQueue& graphics_queue,
                 const VkDeviceSize& size,
                 const VkBuffer& src,
                 VkBuffer& dst)
{
    const auto copy_entire_region = [&](VkCommandBuffer& command_buffer){
        VkBufferCopy command{};
        command.srcOffset = 0;
        command.dstOffset = 0;
        command.size = size;
        vkCmdCopyBuffer(command_buffer, src, dst, 1, &command);
    };

    with_single_use_command_buffer(logical_device,
                                   command_pool,
                                   graphics_queue,
                                   copy_entire_region);
}
    
void transition_image_layout(const VkDevice& logical_device,
                             const VkCommandPool& command_pool,
                             const VkQueue& graphics_queue,
                             VkImage image,
                             VkFormat /*format*/,
                             VkImageLayout oldLayout,
                             VkImageLayout newLayout) 
{
    /* TODO: I am getting an error:
UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout(ERROR / SPEC): msgNum: 1303270965 - Validation Error: [ UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout ] Object 0: handle = 0x5a8e27c45480, type = VK_OBJECT_TYPE_COMMAND_BUFFER; Object 1: handle = 0x4295ab0000000035, type = VK_OBJECT_TYPE_IMAGE; | MessageID = 0x4dae5635 | vkQueueSubmit(): pSubmits[0].pCommandBuffers[0] command buffer VkCommandBuffer 0x5a8e27c45480[] expects VkImage 0x4295ab0000000035[] (subresource: aspectMask 0x1 array layer 0, mip level 0) to be in layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL--instead, current layout is VK_IMAGE_LAYOUT_UNDEFINED.


    The problem seems to be that the layout is somehow not handled correctly, and reset to
    VK_IMAGE_LAYOUT_UNDEFINED
     */

    const auto transition = [&] (VkCommandBuffer& command_buffer) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        
        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED 
         && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
                && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(command_buffer,
                             source_stage,
                             destination_stage,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
    };

    with_single_use_command_buffer(logical_device,
                                   command_pool,
                                   graphics_queue,
                                   transition);
}
    
void copy_buffer_to_image(const VkDevice& logical_device,
                          const VkCommandPool& command_pool,
                          const VkQueue& graphics_queue,
                          VkBuffer buffer,
                          VkImage image,
                          uint32_t width,
                          uint32_t height) 
{
    const auto copy = [&] (VkCommandBuffer& command_buffer) {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };
        vkCmdCopyBufferToImage(command_buffer,
                               buffer,
                               image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);
    };

    with_single_use_command_buffer(logical_device,
                                   command_pool,
                                   graphics_queue,
                                   copy);
}
 
 
void copy_buffer_old(const VkDevice& logical_device,
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
    
void with_memory_mapping(const VkDevice& logical_device,
                         const VkDeviceSize size,
                         const VkDeviceMemory& target_memory,
                         const std::function<void(void*)> f)
{
    void* dst_mapping{nullptr};
    vkMapMemory(logical_device, target_memory, 0, size, 0, &dst_mapping);
    f(dst_mapping);
    vkUnmapMemory(logical_device, target_memory); 
}
    
void memcopy_to_buffer(const VkDevice& logical_device,
                       const void* src,
                       const VkDeviceSize& memsize,
                       VkDeviceMemory& dst)
{
    const auto copy_to_mapping = [&] (void* dst_mapping) {
        memcpy(dst_mapping, src, static_cast<size_t>(memsize));
    };

    with_memory_mapping(logical_device,
                        memsize,
                        dst,
                        copy_to_mapping);
}
 
    

}
