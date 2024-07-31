#include "../arc/Device.hpp"

#include <iostream>
#include <algorithm>

namespace ArcGraphics {
    
Device::Builder::Builder()
{
    GlobalContext::Initialize();
}

Device::Builder& Device::Builder::add_validation_layers(const ValidationLayers layers)
{
    std::copy(layers.begin(), layers.end(), std::back_inserter(m_validation_layers));
    return *this;
}

Device::Builder& Device::Builder::add_khronos_validation_layer()
{
    m_validation_layers.push_back("VK_LAYER_KHRONOS_validation");
    return *this;
}

Device Device::Builder::produce()
{

    std::cout << "==================================================\n"
              << " Producing Device\n"
              << "=================================================="
              << std::endl;

    const DeviceExtensions device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

   auto tmp_window = SDL_CreateWindow("Unnamed Window",
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       50,
                                       50,
                                       SDL_WINDOW_MINIMIZED
                                       | SDL_WINDOW_VULKAN);
   
   if (!tmp_window)
       throw std::runtime_error("could not create temporary window");
    
    const auto extension_properties = get_available_extension_properties();
    std::cout << "Supported Extensions:\n";
    for (const auto& property: extension_properties) {
        std::cout << "\t" << property.extensionName << "\n";
    }

    auto instance = create_instance(tmp_window, m_validation_layers);
    
    VkSurfaceKHR tmp_window_surface;
    SDL_Vulkan_CreateSurface(tmp_window,
                             instance, 
                             &tmp_window_surface);
    
    auto physical_device = get_best_physical_device(instance,
                                                    tmp_window_surface,
                                                    device_extensions);
    
    auto logical_device = get_logical_device(physical_device,
                                             tmp_window_surface,
                                             device_extensions);
    
    auto capabilities = get_rendering_capabilities(physical_device,
                                                   tmp_window_surface);

    /* =============================================================
     * Cleanup Temporary Window and Window-Surface
     */
    vkDestroySurfaceKHR(instance, tmp_window_surface, nullptr);
    SDL_DestroyWindow(tmp_window);
    
    return Device(instance, physical_device, logical_device, capabilities);
}

const VkInstance& Device::instance() const noexcept
{
    return m_instance;
}

const VkPhysicalDevice& Device::physical_device() const noexcept
{
    return m_physical_device;
}
    

const VkDevice& Device::logical_device() const noexcept
{
    return m_logical_device;
}

const DeviceRenderingCapabilities& Device::capabilities() const noexcept
{
    return m_capabilities;
}
    
Device::Device(const VkInstance instance,
               const VkPhysicalDevice physical_device,
               const VkDevice logical_device,
               const DeviceRenderingCapabilities capabilities)
    : m_instance(instance)
    , m_physical_device(physical_device)
    , m_logical_device(logical_device)
    , m_capabilities(capabilities)
{
}

void Device::destroy()
{
    vkDestroyDevice(m_logical_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

}
