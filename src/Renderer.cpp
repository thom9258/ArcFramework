#include "../arc/Renderer.hpp"
#include "../arc/UniformBuffer.hpp"

namespace ArcGraphics {
    

Renderer::Renderer(std::shared_ptr<Device> device,
                   SDL_Window* window,
                   const VkSurfaceKHR window_surface,
                   const VkSwapchainKHR swapchain,
                   const std::vector<VkImageView> swapchain_image_views,
                   const VkSurfaceFormatKHR surface_format,
                   const DeviceRenderingCapabilities capabilities
                   )
    : m_device(device)
    , m_window(window)
    , m_window_surface(window_surface)
    , m_swapchain(swapchain)
    , m_swapchain_image_views(swapchain_image_views)
    , m_surface_format(surface_format)
    , m_capabilities(capabilities)
{
}

Renderer::~Renderer()
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

const VkSurfaceKHR& Renderer::window_surface() const
{
    return m_window_surface;
}
    
const VkSwapchainKHR& Renderer::swapchain() const
{
    return m_swapchain;
}

const VkSurfaceFormatKHR& Renderer::surface_format() const
{
    return m_surface_format;
}

Renderer::Builder::Builder(std::shared_ptr<Device> device)
{
    reset_builder();
    m_device = device;
}
    
void Renderer::Builder::reset_builder()
{
    m_window_name = "Unknown Window";
    m_window_width = 800;
    m_window_height = 600;
    m_window_flags = 0;
    m_device = nullptr;
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

Renderer::Builder::~Builder()
{
    reset_builder();
}

std::shared_ptr<Renderer> Renderer::Builder::produce()
{
    
    VkExtent2D window_size;
    window_size.width = m_window_width;
    window_size.height = m_window_height;

    const auto capabilities = m_device->capabilities();

    auto window = SDL_CreateWindow(m_window_name.c_str(),
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   window_size.width,
                                   window_size.height,
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
   
    reset_builder();

    return std::make_shared<Renderer>(m_device,
                                      window,
                                      window_surface,
                                      swap_chain.swap_chain,
                                      swap_chain.image_views,
                                      swap_chain.surface_format,
                                      capabilities
                                      );
}
   

}
