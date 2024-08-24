#include "../arc/Texture.hpp"
#include "../arc/BasicBuffer.hpp"
#include "../arc/Algorithm.hpp"

#include <stb/stb_image.h>

#include <iostream>

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
    const auto channels_size = 4; // TODO: this should be dynamic and based on the formatting of the image
    return m_width * m_height * channels_size;
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
    


Texture::Texture(const VkImage image,
                 const VkDeviceMemory memory,
                 const VkFormat format,
                 const VkImageView view,
                 const VkSampler sampler
                 )
    : m_image(image)
    , m_memory(memory)
    , m_format(format)
    , m_view(view)
    , m_sampler(sampler)
{
}

void Texture::destroy(const VkDevice logical_device)
{
    vkDestroySampler(logical_device, m_sampler, nullptr);
    vkDestroyImageView(logical_device, m_view, nullptr);
    vkDestroyImage(logical_device, m_image, nullptr);
    vkFreeMemory(logical_device, m_memory, nullptr);
}

Texture::~Texture() = default;
    

const VkImage& Texture::image()
{
    return m_image;
}
    
const VkDeviceMemory& Texture::memory()
{
    return m_memory;
}

VkFormat Texture::format()
{
    return m_format;
}

const VkImageView& Texture::view()
{
    return m_view;
}
    
const VkSampler& Texture::sampler()
{
    return m_sampler;
}

   
std::unique_ptr<Texture> Texture::create_staging(const VkPhysicalDevice& physical_device,
                                                 const VkDevice& logical_device,
                                                 const VkCommandPool& command_pool,
                                                 const VkQueue& graphics_queue,
                                                 const VkFormat format,
                                                 const Image* image)
{
    if (!image) return nullptr;

    VkBuffer staging_buffer;
    VkBufferCreateInfo staging_buffer_info;
    VkDeviceMemory staging_buffer_memory;

    VkImage texture{};
    VkDeviceMemory texture_memory;

    #if 0
    create_image(physical_device,
                 logical_device,
                 static_cast<uint32_t>(image->width()),
                 static_cast<uint32_t>(image->height()),
                 format,
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 texture,
                 staging_buffer_memory);
    #else
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
    image_info.format = format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    //VK_IMAGE_TILING_LINEAR:
    //  Texels are laid out in row-major order like our pixels array
    //VK_IMAGE_TILING_OPTIMAL:
    //  Texels are laid out in an implementation defined order for optimal access
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;

    image_info.flags = 0; // Optional
    
    auto status = vkCreateImage(logical_device, &image_info, nullptr, &texture);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");
    
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(logical_device, texture, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    
    const auto memory_properties = get_physical_device_memory_properties(physical_device);
    alloc_info.memoryTypeIndex = find_memory_type(memory_properties,
                                                  mem_requirements.memoryTypeBits,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    status = vkAllocateMemory(logical_device, &alloc_info, nullptr, &texture_memory);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");

    vkBindImageMemory(logical_device, texture, texture_memory, 0);   
    #endif
    
    // First we transition the image into a transfer destination layout.
    //so we can copy the image buffer to the image 
    transition_image_layout(logical_device,
                            command_pool,
                            graphics_queue,
                            texture,
                            format,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    copy_buffer_to_image(logical_device,
                         command_pool,
                         graphics_queue,
                         staging_buffer,
                         texture,
                         image->width(),
                         image->height()); 
 
    // Then we transition the image to shader memory
    transition_image_layout(logical_device,
                            command_pool,
                            graphics_queue,
                            texture,
                            format,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    
    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
    
    const auto view = create_image_view(logical_device,
                                        texture,
                                        format,
                                        VK_IMAGE_ASPECT_COLOR_BIT);
    if (!view)
        return nullptr;
    
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //TODO: Make it possible to change interpolation to bilinear as well as others..
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    //TODO: Make it possible to change address_modes..
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    const auto device_properties = get_physical_device_properties(physical_device);
    // TODO: It should be possible to make this an optional thing, if this change is made
    // Remove the forced selection of physical devices in calculate_device_score()

    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = device_properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    VkSampler sampler{};
    status = vkCreateSampler(logical_device, &sampler_info, nullptr, &sampler);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture sampler!");
    
   
    return std::make_unique<Texture>(texture,
                                     texture_memory,
                                     format,
                                     *view,
                                     sampler);
}
   
}
