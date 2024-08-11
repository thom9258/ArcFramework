#include "../arc/Renderer.hpp"
#include "../arc/UniformBuffer.hpp"

#include <iostream>

namespace ArcGraphics {
    

Renderer::Renderer(Device* device,
                   SDL_Window* window,
                   const VkSurfaceKHR window_surface,
                   const uint32_t window_width,
                   const uint32_t window_height,
                   const VkSwapchainKHR swapchain,
                   const std::vector<VkImageView> swapchain_image_views,
                   const VkSurfaceFormatKHR surface_format,
                   const DeviceRenderingCapabilities capabilities,
                   const VkQueue graphics_queue
                   )
    : m_device(device)
    , m_window(window)
    , m_window_surface(window_surface)
    , m_window_width(window_width)
    , m_window_height(window_height)
    , m_swapchain(swapchain)
    , m_swapchain_image_views(swapchain_image_views)
    , m_surface_format(surface_format)
    , m_capabilities(capabilities)
    , m_graphics_queue(graphics_queue)
{
    if (!m_device)
        throw std::runtime_error("Renderer() device was nullptr!");
    if (!m_window)
        throw std::runtime_error("Renderer() window was nullptr!");
}

void Renderer::destroy()
{
    //TODO segfault here, because renderer cannot get logical device as Device is freed it seems..
    const auto logical_device = m_device->logical_device();
    for (auto& view: m_swapchain_image_views)
        vkDestroyImageView(logical_device, view, nullptr);

    vkDestroySwapchainKHR(logical_device, m_swapchain, nullptr);
}
    

size_t Renderer::swapchain_image_view_count() const
{
    return m_swapchain_image_views.size();
}

const VkImageView& Renderer::swapchain_image_view(const size_t index) const
{
    return m_swapchain_image_views.at(index);
}

const DeviceRenderingCapabilities& Renderer::capabilities() const
{
    return m_capabilities;
}
    

SDL_Window* const& Renderer::window() const
{
    return m_window;
}

const VkQueue& Renderer::graphics_queue() const
{
    return m_graphics_queue;
}
 
const VkSurfaceKHR& Renderer::window_surface() const
{
    return m_window_surface;
}
    

VkExtent2D Renderer::window_size() const
{
    VkExtent2D size;
    size.width = m_window_width;
    size.height = m_window_height;
    return size;
}
    
const VkSwapchainKHR& Renderer::swapchain() const
{
    return m_swapchain;
}

const VkSurfaceFormatKHR& Renderer::surface_format() const
{
    return m_surface_format;
}

Renderer::Builder::Builder(Device* device)
    : m_device(device)
{
    if (!m_device)
        throw std::runtime_error("Renderer() device was nullptr!");
}
    
Renderer::Builder& Renderer::Builder::with_wanted_window_size(const uint32_t width,
                                                              const uint32_t height)
{
    m_window_width = width;
    m_window_height = height;
    return *this;
}

Renderer::Builder& Renderer::Builder::with_window_name(const std::string& name)
{
    m_window_name = name;
    return *this;
}
    
Renderer::Builder& Renderer::Builder::with_window_flags(const uint32_t flags)
{
    m_window_flags |= flags;
    return *this;
}

Renderer Renderer::Builder::produce()
{
    std::cout << "==================================================\n"
              << " Producing Renderer\n"
              << "=================================================="
              << std::endl;

    const auto capabilities = m_device->capabilities();

    auto window = SDL_CreateWindow(m_window_name.c_str(),
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   m_window_width,
                                   m_window_height,
                                   m_window_flags | SDL_WINDOW_VULKAN);
   
    VkSurfaceKHR window_surface;
    SDL_Vulkan_CreateSurface(window,
                             m_device->instance(), 
                             &window_surface);
    
    const auto swap_chain = create_swap_chain(m_device->physical_device(),
                                              m_device->logical_device(),
                                              window_surface,
                                              m_window_width,
                                              m_window_height);
    
    const auto queue_families = get_queue_families(m_device->physical_device());
    const auto indices = find_graphics_present_indices(queue_families,
                                                       m_device->physical_device(),
                                                       window_surface);

    VkQueue graphics_queue{};
    vkGetDeviceQueue(m_device->logical_device(), 
                     indices.graphics.value(),
                     0,
                     &graphics_queue);
   
    return Renderer(m_device,
                    window,
                    window_surface,
                    m_window_width,
                    m_window_height,
                    swap_chain.swap_chain,
                    swap_chain.image_views,
                    swap_chain.surface_format,
                    capabilities,
                    graphics_queue
                    );
}
   

}
