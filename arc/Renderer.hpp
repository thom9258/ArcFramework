#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "Device.hpp"

#include <vector>

namespace ArcGraphics {

class Renderer
{
public:
    class Builder;

    Renderer(std::shared_ptr<Device>,
             SDL_Window* window,
             const VkSurfaceKHR window_surface,
             const VkSwapchainKHR swapchain,
             const std::vector<VkImageView> swapchain_image_views,
             const VkSurfaceFormatKHR surface_format,
             const DeviceRenderingCapabilities capabilities
             );
    
    size_t swapchain_image_view_count() const;
    const VkImageView& swapchain_image_view(const size_t index) const;
    const DeviceRenderingCapabilities& capabilities() const;
    SDL_Window* const& window() const;
    const VkSurfaceKHR& window_surface() const;
    const VkSwapchainKHR& swapchain() const;
    const VkSurfaceFormatKHR& surface_format() const;
    ~Renderer();

private:
    std::shared_ptr<Device> m_device;
    SDL_Window* m_window;
    VkSurfaceKHR m_window_surface;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImageView> m_swapchain_image_views;
    VkSurfaceFormatKHR m_surface_format;
    DeviceRenderingCapabilities m_capabilities;
};
    
class Renderer::Builder 
{
 public:
    Builder(std::shared_ptr<Device> device);
    ~Builder();

    [[nodiscard]]
    std::shared_ptr<Renderer> produce();
    void reset_builder();

    Builder& with_wanted_window_size(const uint32_t width,
                                     const uint32_t height);
    Builder& with_window_name(const std::string& name);
    Builder& with_window_flags(const uint32_t flags);

 private:
    std::shared_ptr<Device> m_device;
    std::string m_window_name;
    uint32_t m_window_width;
    uint32_t m_window_height;
    uint32_t m_window_flags;
};
    
}
