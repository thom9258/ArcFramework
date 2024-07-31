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

    Renderer(Device* device,
             SDL_Window* window,
             const VkSurfaceKHR window_surface,
             const VkSwapchainKHR swapchain,
             const std::vector<VkImageView> swapchain_image_views,
             const VkSurfaceFormatKHR surface_format,
             const DeviceRenderingCapabilities capabilities,
             const VkQueue graphics_queue
             );

    ~Renderer() = default;
    void destroy();
    
    size_t swapchain_image_view_count() const;
    const VkImageView& swapchain_image_view(const size_t index) const;
    const DeviceRenderingCapabilities& capabilities() const;
    SDL_Window* const& window() const;
    const VkSurfaceKHR& window_surface() const;
    const VkSwapchainKHR& swapchain() const;
    const VkSurfaceFormatKHR& surface_format() const;
    const VkQueue& graphics_queue() const;

private:
    Device* m_device{nullptr};
    SDL_Window* m_window{nullptr};
    VkSurfaceKHR m_window_surface;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImageView> m_swapchain_image_views;
    VkSurfaceFormatKHR m_surface_format;
    DeviceRenderingCapabilities m_capabilities;
    VkQueue m_graphics_queue;
};
    
class Renderer::Builder 
{
 public:
    Builder(Device* device);
    ~Builder() = default;

    [[nodiscard]]
    Renderer produce();

    Builder& with_wanted_window_size(const uint32_t width,
                                     const uint32_t height);
    Builder& with_window_name(const std::string& name);
    Builder& with_window_flags(const uint32_t flags);

 private:
    Device* m_device{nullptr};
    std::string m_window_name{"Unnamed Window"};
    uint32_t m_window_width{1200};
    uint32_t m_window_height{900};
    uint32_t m_window_flags{0};
};
    
}
