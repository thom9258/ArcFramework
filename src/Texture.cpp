#include "../arc/Texture.hpp"
#include "../arc/BasicBuffer.hpp"

#include <stb/stb_image.h>

namespace ArcGraphics {
    

Image::Image(unsigned char* pixels,
             const int width,
             const int height,
             const int channels
             )
    : m_pixels(pixels)
    , m_width(width)
    , m_height(height)
    , m_channels(channels)
{
}

Image::~Image()
{
    stbi_image_free(m_pixels);
}

std::unique_ptr<Image> Image::load_from_path(const std::string& path)
{
    int width;
    int height;
    int channels;
    const auto pixels = stbi_load(path.c_str(),
                                  &width,
                                  &height,
                                  &channels,
                                  STBI_rgb_alpha);
    if (!pixels)
        return nullptr;
    
    return std::make_unique<Image>(pixels, width, height, channels);
}
    

VkDeviceSize Image::device_size() const
{
    return m_width * m_height * 4 /*TODO: Should this be m_channels*/;
}

int Image::width() const { return m_width; }
int Image::height() const { return m_height; }
int Image::channels() const { return m_channels; }
unsigned char* Image::pixels() const { return m_pixels; }
    

VkCommandBuffer begin_single_time_commands(const VkDevice& logical_device,
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

void end_single_time_commands(VkCommandBuffer commandBuffer,
                              VkQueue graphics_queue,
                              const VkDevice& logical_device,
                              const VkCommandPool& command_pool) 
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
    


Texture::Texture(const VkImage texture,
                 const VkDeviceMemory texture_memory)
    : m_texture(texture)
    , m_texture_memory(texture_memory)
{
}

void Texture::destroy(const VkDevice logical_device)
{
    vkDestroyImage(logical_device, m_texture, nullptr);
    vkFreeMemory(logical_device, m_texture_memory, nullptr);
}

Texture::~Texture() = default;

std::unique_ptr<Texture> Texture::create_staging(const VkPhysicalDevice& physical_device,
                                                 const VkDevice& logical_device,
                                                 const VkCommandPool& command_pool,
                                                 const VkQueue& graphics_queue,
                                                 const Image* image)
{
    if (!image) return nullptr;

    VkBuffer staging_buffer;
    VkBufferCreateInfo staging_buffer_info;
    VkDeviceMemory staging_buffer_memory;
    
    create_buffer(physical_device,
                  logical_device,
                  image->device_size(),
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  staging_buffer_info,
                  staging_buffer,
                  staging_buffer_memory);

    memcopy_to_buffer(logical_device,
                      image->pixels(),
                      image->device_size(),
                      staging_buffer_memory);

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(image->width());
    image_info.extent.height = static_cast<uint32_t>(image->height());
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    //VK_IMAGE_TILING_LINEAR:
    //  Texels are laid out in row-major order like our pixels array
    //VK_IMAGE_TILING_OPTIMAL:
    //  Texels are laid out in an implementation defined order for optimal access

    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0; // Optional
    
    VkImage texture;
    auto status = vkCreateImage(logical_device, &image_info, nullptr, &texture);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");
    
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(logical_device, texture, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    

    //https://vulkan-tutorial.com/Texture_mapping/Images
    const auto memory_properties = get_physical_device_memory_properties(physical_device);

    alloc_info.memoryTypeIndex = find_memory_type(memory_properties,
                                                  mem_requirements.memoryTypeBits,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory texture_memory;
    status = vkAllocateMemory(logical_device, &alloc_info, nullptr, &texture_memory);
    if (status != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(logical_device, texture, texture_memory, 0);   
    transition_image_layout(logical_device,
                            command_pool,
                            graphics_queue,
                            texture,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    
    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
    
    return std::make_unique<Texture>(texture, texture_memory);
}
    
}
