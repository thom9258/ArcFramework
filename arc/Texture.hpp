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
            const VkImageView view,
            const VkSampler sampler
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
                                                   const VkFormat format,
                                                   const Image* image);
    
    [[nodiscard]]
    const VkImage& image();

    [[nodiscard]]
    const VkDeviceMemory& memory();

    [[nodiscard]]
    VkFormat format();
   
    [[nodiscard]]
    const VkImageView& view();

    [[nodiscard]]
    const VkSampler& sampler();
    
private:
    VkImage m_image;
    VkDeviceMemory m_memory;
    VkFormat m_format;
    VkImageView m_view;
    VkSampler m_sampler;
};

} 
