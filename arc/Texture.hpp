#pragma once

#include <arc/TypeTraits.hpp>

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>

namespace ArcGraphics {

class Image : IsNotLvalueCopyable
{
public:
    static std::unique_ptr<Image> load_from_path(const std::string& path);
    
    Image(unsigned char* pixels,
          const int width,
          const int height,
          const int channels
          );
    ~Image();
    VkDeviceSize device_size() const;
    unsigned char* pixels() const;
    int width() const;
    int height() const;
    int channels() const;

    unsigned char* m_pixels;
    int m_width;
    int m_height;
    int m_channels;
};
    
class Texture : IsNotLvalueCopyable
{
public:
    Texture(const VkImage image,
            const VkDeviceMemory memory,
            const VkFormat format,
            const std::vector<VkImageView> views,
            const std::vector<VkSampler> samplers,
            const std::vector<VkDescriptorImageInfo> infos
            );

    void destroy(const VkDevice logical_device);
    ~Texture();

    /**
     * @brief Create texture. 
     * @todo Creating textures requires a bunch of optional stuff, and should be
     * fully handled through a builder instead of here.
     */
    static std::unique_ptr<Texture> create_staging(const VkPhysicalDevice& physical_device,
                                                   const VkDevice& logical_device,
                                                   const VkCommandPool& command_pool,
                                                   const VkQueue& graphics_queue,
                                                   const uint32_t frames_in_flight,
                                                   const Image* image);
    
    [[nodiscard]]
    const VkImage& image();

    [[nodiscard]]
    VkFormat format();
   
    [[nodiscard]]
    const std::vector<VkDescriptorImageInfo>& image_infos();

private:
    [[nodiscard]]
    const std::vector<VkImageView>& views();

    [[nodiscard]]
    const std::vector<VkSampler>& samplers();
    

    VkImage m_image;
    VkDeviceMemory m_memory;
    VkFormat m_format;
    // Per-Frame-in-Flight views / samplers and their combined infos
    std::vector<VkImageView> m_views;
    std::vector<VkSampler> m_samplers;
    std::vector<VkDescriptorImageInfo> m_infos;
};

} 
