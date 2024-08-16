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
                 const std::vector<VkImageView> views,
                 const std::vector<VkSampler> samplers,
                 const std::vector<VkDescriptorImageInfo> infos
                 )
    : m_image(image)
    , m_memory(memory)
    , m_format(format)
    , m_views(views)
    , m_samplers(samplers)
    , m_infos(infos)
{
}

void Texture::destroy(const VkDevice logical_device)
{
    for (const auto& sampler: m_samplers)
        vkDestroySampler(logical_device, sampler, nullptr);

    for (const auto& view: m_views)
        vkDestroyImageView(logical_device, view, nullptr);

    vkDestroyImage(logical_device, m_image, nullptr);
    vkFreeMemory(logical_device, m_memory, nullptr);
}

Texture::~Texture() = default;
    

const VkImage& Texture::image()
{
    return m_image;
}

VkFormat Texture::format()
{
    return m_format;
}

const std::vector<VkImageView>& Texture::views()
{
    return m_views;
}
    
const std::vector<VkSampler>& Texture::samplers()
{
    return m_samplers;
}
    
const std::vector<VkDescriptorImageInfo>& Texture::image_infos()
{
    return m_infos;
}

std::unique_ptr<Texture> Texture::create_staging(const VkPhysicalDevice& physical_device,
                                                 const VkDevice& logical_device,
                                                 const VkCommandPool& command_pool,
                                                 const VkQueue& graphics_queue,
                                                 const uint32_t frames_in_flight,
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
    const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    
    // NOTE: For some reason we need to do 2 transitions to get to SHADER_READ_ONLY_OPTIMAL
    transition_image_layout(logical_device,
                            command_pool,
                            graphics_queue,
                            texture,
                            format,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
 
    transition_image_layout(logical_device,
                            command_pool,
                            graphics_queue,
                            texture,
                            format,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    
    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
    
    std::vector<VkImageView> views{};
    for (uint32_t i = 0; i < frames_in_flight; i++) {
        const auto view = create_image_view(logical_device, texture, format);
        if (!view)
            return nullptr;
        views.push_back(*view);
    }
    
    std::vector<VkSampler> samplers{};
    for (uint32_t i = 0; i < frames_in_flight; i++) {
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
        samplers.push_back(sampler);
    }
    
    std::vector<VkDescriptorImageInfo> descriptor_image_infos;
    for (uint32_t i = 0; i < frames_in_flight; i++) {
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = views[i];
        image_info.sampler = samplers[i];
        descriptor_image_infos.push_back(image_info);
    }
    std::cout << "created descriptor image infos for texture" << std::endl;
    
    return std::make_unique<Texture>(texture,
                                     texture_memory,
                                     format,
                                     views,
                                     samplers,
                                     descriptor_image_infos);
 
    //TODO: this should happen for each cmd draw command in the main renderpipeline.
    //for (size_t i = 0; i < frames_in_flight; i++) {
    //    // Setup Descriptor buffer for ViewPort
    //    VkDescriptorBufferInfo buffer_info =
    //        uniform_viewports[i]->descriptor_buffer_info();
    //    
    //    VkWriteDescriptorSet descriptor_write{};
    //    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //    descriptor_write.dstSet = descriptor_sets[i];
    //    descriptor_write.dstBinding = 0;
    //    descriptor_write.dstArrayElement = 0;
    //    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //    descriptor_write.descriptorCount = 1;
    //    descriptor_write.pBufferInfo = &buffer_info;
    //    descriptor_write.pImageInfo = &descriptor_image_infos[i]; // Optional
    //    descriptor_write.pTexelBufferView = nullptr; // Optional
    //
    //    uint32_t count = 1;
    //    vkUpdateDescriptorSets(m_device->logical_device(),
    //                           count,
    //                           &descriptor_write,
    //                           0,
    //                           nullptr);
    //}
}
   
}
