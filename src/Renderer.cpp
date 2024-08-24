#include "../arc/Renderer.hpp"
#include "../arc/UniformBuffer.hpp"
#include "../arc/Texture.hpp"

#include <iostream>

namespace ArcGraphics {
    
[[nodiscard]]
std::optional<VkFormat> find_supported_texture_format(const VkPhysicalDevice& physical_device,
                                                      const std::vector<VkFormat>& candidates,
                                                      const VkImageTiling tiling,
                                                      const VkFormatFeatureFlags features) 
{
    for (const VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR 
        && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL 
            && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    return std::nullopt;
}
    
[[nodiscard]]
std::optional<VkFormat> find_depthbuffer_format(const VkPhysicalDevice& physical_device) 
{
    const auto depthbuffer_formats = {VK_FORMAT_D32_SFLOAT,
                                      VK_FORMAT_D32_SFLOAT_S8_UINT,
                                      VK_FORMAT_D24_UNORM_S8_UINT};

    return find_supported_texture_format(physical_device,
                                         depthbuffer_formats,
                                         VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
    
[[nodiscard]]
bool format_has_stencil_component(const VkFormat format) 
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT 
        || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

Renderer::Renderer(Device* device,
                   SDL_Window* window,
                   const VkSurfaceKHR window_surface,
                   const uint32_t window_width,
                   const uint32_t window_height,
                   const VkSwapchainKHR swapchain,
                   const std::vector<VkImageView> swapchain_image_views,
                   const VkSurfaceFormatKHR surface_format,
                   const DeviceRenderingCapabilities capabilities,
                   const VkQueue graphics_queue,

                   const VkImage depthbuffer_image,
                   const VkDeviceMemory depthbuffer_memory,
                   const VkImageView depthbuffer_view,
                   const VkFormat depthbuffer_format
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

    , m_depthbuffer_image(depthbuffer_image)
    , m_depthbuffer_memory(depthbuffer_memory)
    , m_depthbuffer_view(depthbuffer_view)
    , m_depthbuffer_format(depthbuffer_format)
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

    vkDestroyImageView(logical_device, m_depthbuffer_view, nullptr);
    vkDestroyImage(logical_device, m_depthbuffer_image, nullptr);
    vkFreeMemory(logical_device, m_depthbuffer_memory, nullptr);

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
    

const VkFormat& Renderer::depthbuffer_format() const
{
    return m_depthbuffer_format;
}
    

const VkImageView& Renderer::depthbuffer_image_view() const
{
    return m_depthbuffer_view;
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
    
    /* ===========================================================================
     * Create Depth Buffer
     */
    const auto depthbuffer_format = find_depthbuffer_format(m_device->physical_device());
    if (!depthbuffer_format)
        throw std::runtime_error("No suitable format could be found for depth buffering!");
   
    VkImage depthbuffer_image{};
    VkDeviceMemory depthbuffer_memory{};
    
    create_image(m_device->physical_device(),
                 m_device->logical_device(),
                 m_window_width,
                 m_window_height,
                 *depthbuffer_format,
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 depthbuffer_image,
                 depthbuffer_memory);
    
    auto depthbuffer_view = create_image_view(m_device->logical_device(),
                                              depthbuffer_image,
                                              *depthbuffer_format,
                                              VK_IMAGE_ASPECT_DEPTH_BIT);
    if (!depthbuffer_view)
        throw std::runtime_error("Could not create depth buffer view!");

    return Renderer(m_device,
                    window,
                    window_surface,
                    m_window_width,
                    m_window_height,
                    swap_chain.swap_chain,
                    swap_chain.image_views,
                    swap_chain.surface_format,
                    capabilities,
                    graphics_queue,

                    depthbuffer_image,
                    depthbuffer_memory,
                    *depthbuffer_view,
                    *depthbuffer_format
                    );
}
   

}
