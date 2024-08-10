#pragma once

#include <arc/TypeTraits.hpp>

#include <vulkan/vulkan.h>

#include <string>
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
    static std::unique_ptr<Texture> create(const VkPhysicalDevice& physical_device,
                                           const VkDevice& logical_device,
                                           const Image* image);
};

} 
