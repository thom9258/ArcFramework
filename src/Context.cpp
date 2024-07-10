#include "../arc/Context.hpp"

#include <iostream>
#include <algorithm>
#include <map>
#include <set>
#include <optional>
#include <sstream>
#include <string>
#include <cstdint>
#include <limits>

#ifndef UNREFERENCED
#define UNREFERENCED(param) void(param)
#endif

// https://github.com/AndreVallestero/sdl-vulkan-tutorial/blob/master/hello-triangle/main.cpp
// https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Instance

// TODO: Get message callbacks with vulkan errors:
// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers


namespace arc {
    
namespace global {

std::atomic_bool GraphicsContext_created{false};

}
    
using Score = uint32_t;
using ScoredDevice = std::pair<Score, VkPhysicalDevice>;


struct PhysicalDevicePropertyFeatureSet {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};
    
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    bool is_complete() {
        return graphics && present;
    }
    std::string stringify() {
        std::stringstream ss;
        if (graphics)
            ss << "[Graphics: " << *graphics << ",  ";
        else
            ss << "[Graphics: 'nil', ";
        if (present)
            ss << "Present: " << *present << "]";
        else
            ss << "Present: 'nil']";
        return ss.str();
    }
};
    
struct SwapChainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};


VkApplicationInfo create_app_info(const char* appname) 
{
    VkApplicationInfo info{};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = appname;
    info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    info.pEngineName = "No Engine";
    info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    info.apiVersion = VK_API_VERSION_1_0;
    return info;
}
    
std::vector<const char*> get_available_extensions(SDL_Window* window)
{
    uint32_t count{0};
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> available(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, available.data());
    return available;
}
   
VkInstanceCreateInfo create_instance_info(const std::vector<const char*>& extensions,
                                          const VkApplicationInfo* app_info) 
{
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = app_info;
    info.enabledExtensionCount = extensions.size();
    info.ppEnabledExtensionNames = extensions.data();

    return info;
}
    
std::vector<VkExtensionProperties> get_available_extension_properties()
{
    uint32_t count{0};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());
    return available;
}
    
std::vector<VkLayerProperties> get_available_validation_layers()
{
    uint32_t count{0};
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());
    return available;
}
    
    
bool is_validation_layers_supported(const GraphicsContext::ValidationLayers layers)
{
    const auto available_layers = get_available_validation_layers();

    const auto layer_eq = [] (const auto validation_layer, const char* layer_name) {
        return strcmp(layer_name, validation_layer.layerName) == 0;
    };

    const auto layer_in_available = [&layer_eq] (const auto& available_layers,
                                                 const char* layer_name) {
        for (const auto& properties : available_layers) {
            if (layer_eq(properties, layer_name))
                return true;
        }
        return false;
    };
        
    for (const auto& layer_name : layers) {
        if (!layer_in_available(available_layers, layer_name)) {
            throw std::runtime_error("Provided validation layer [" + 
                                     std::string(layer_name) +
                                     "] is not supported");
        }
    }
    return true;
}
    
std::vector<VkPhysicalDevice> get_available_physical_devices(const VkInstance& instance)
{
    uint32_t count{0};
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> available(count);
    vkEnumeratePhysicalDevices(instance, &count, available.data());
    return available;
}
    
VkPhysicalDeviceProperties get_physical_device_properties(const VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    return properties;
}
VkPhysicalDeviceFeatures get_physical_device_features(const VkPhysicalDevice device)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);   
    return features;
}
    
PhysicalDevicePropertyFeatureSet get_physical_device_properties_features(const VkPhysicalDevice device)
{
    return {get_physical_device_properties(device), get_physical_device_features(device)};
}

/*
bool queue_family_has_graphics(std::vector<VkQueueFamilyProperties> families, )
{
    return family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
}
    
bool queue_familty_has_present_support(const VkPhysicalDevice& device,
                                       const uint32_t queue_family_index,
                                       const VkSurfaceKHR& surface)
{
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_family_index, surface, &present_support);
    return present_support;
}
*/

QueueFamilyIndices
find_graphics_present_indices(std::vector<VkQueueFamilyProperties> families,
                              const VkPhysicalDevice& device,
                              const VkSurfaceKHR& surface)
{
    QueueFamilyIndices indices;
    uint32_t i = 0;
    for (const auto& family : families) {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphics = i;

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support)
            indices.present = i;

        if (indices.is_complete())
            break;
        i++;
    }
    return indices;
}
    
std::vector<VkQueueFamilyProperties> get_queue_families(const VkPhysicalDevice& device)
{
    uint32_t count{0};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> available(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, available.data());
    return available;
}
    
std::vector<VkExtensionProperties> get_device_extensions(const VkPhysicalDevice& device)
{
    uint32_t count{0};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());
    return available;
}

bool is_device_extensions_supported(VkPhysicalDevice device,
                                    const std::vector<const char*> needed_extensions) 
{
    auto actual_extensions = get_device_extensions(device);

    std::set<std::string> missing;
    for (const auto& extension: needed_extensions)
        missing.insert(std::string(extension));

    for (const auto& actual_extension : actual_extensions) {
        missing.erase(actual_extension.extensionName);
    }
    return missing.empty();
}

std::vector<VkSurfaceFormatKHR> get_swap_chain_formats(const VkPhysicalDevice& device,
                                                       const VkSurfaceKHR& surface)
{
    uint32_t count{0};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> available(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, available.data());
    return available;
}

std::vector<VkPresentModeKHR> get_swap_chain_present_modes(const VkPhysicalDevice& device,
                                                           const VkSurfaceKHR& surface)
{
    uint32_t count{0};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> available(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, available.data());
    return available;
}

SwapChainSupportInfo get_swap_chain_support_info(const VkPhysicalDevice& device, 
                                                 const VkSurfaceKHR& surface)
{
    SwapChainSupportInfo details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    details.formats = get_swap_chain_formats(device, surface);
    details.present_modes = get_swap_chain_present_modes(device, surface);
    return details;
}

std::optional<VkSurfaceFormatKHR>
find_swap_chain_surface_format(const std::vector<VkSurfaceFormatKHR>& surfaceformats,
                               const VkFormat format,
                               const VkColorSpaceKHR colorspace)
{
    for (const auto& surfaceformat: surfaceformats)
        if (surfaceformat.format == format && surfaceformat.colorSpace == colorspace)
            return surfaceformat;
    return {};
}


std::optional<VkSurfaceFormatKHR>
find_ideal_swap_chain_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
{
    return find_swap_chain_surface_format(formats,
                                          VK_FORMAT_B8G8R8A8_SRGB,
                                          VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
}

std::optional<VkPresentModeKHR>
find_swap_chain_present_mode(const std::vector<VkPresentModeKHR>& presentmodes,
                             const VkPresentModeKHR mode)
{
    for (const auto& presentmode: presentmodes)
        if (presentmode == mode)
            return presentmode;
    return {};

}

VkPresentModeKHR get_default_swap_chain_present_mode()
{
    return VK_PRESENT_MODE_FIFO_KHR;
}

std::optional<VkPresentModeKHR>
find_ideal_swap_chain_present_mode(const std::vector<VkPresentModeKHR>& presentmodes)
{
    return find_swap_chain_present_mode(presentmodes, VK_PRESENT_MODE_MAILBOX_KHR);
}
    

VkExtent2D get_swap_chain_extent(const VkSurfaceCapabilitiesKHR& capabilities, 
                                 SDL_Window* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    
    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    
    actualExtent.width = std::clamp(actualExtent.width,
                                    capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
                                     capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);
    return actualExtent;
}

uint32_t get_minimum_swap_chain_image_count(const VkSurfaceCapabilitiesKHR& capabilities)
{
    auto count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && count > capabilities.maxImageCount) 
        count = capabilities.maxImageCount;
    return count;
}


uint32_t calculate_device_score(VkPhysicalDevice device,
                                const VkSurfaceKHR& surface,
                                const std::vector<const char*> needed_extensions)
{
    uint32_t score{0};
    if (device == VK_NULL_HANDLE)
        return 0;
    
    if (!is_device_extensions_supported(device, needed_extensions))
        return 0;
    
    const auto queue_families = get_queue_families(device);
    auto queue_families_indices = find_graphics_present_indices(queue_families, device, surface);
    if (!queue_families_indices.is_complete())
        return 0;
    
    const auto swap_chain_info = get_swap_chain_support_info(device, surface);

    const bool is_swap_chain_ok = !swap_chain_info.formats.empty() 
                               && !swap_chain_info.present_modes.empty();
    if (!is_swap_chain_ok)
        return 0;

    const auto ideal_surface_format =
        find_ideal_swap_chain_surface_format(swap_chain_info.formats);
    if (ideal_surface_format)
        score += 200;
    
    const auto ideal_present_mode =
        find_ideal_swap_chain_present_mode(swap_chain_info.present_modes);
    if (ideal_present_mode)
        score += 200;
    
    const auto info = get_physical_device_properties_features(device);
    if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;
    if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        score += 500;

    //TODO get more score paramters..?
    return score;
}

std::vector<ScoredDevice> sort_devices_by_score(const std::vector<VkPhysicalDevice>& devices,
                                                const VkSurfaceKHR& surface,
                                                const std::vector<const char*> extensions) 
{
    std::multimap<Score, VkPhysicalDevice> reverse_sorted;
    for (const auto& dpf : devices)
        reverse_sorted.insert({calculate_device_score(dpf, surface, extensions), dpf});
    
    std::vector<ScoredDevice> sorted;
    for (auto it = reverse_sorted.rbegin(); it != reverse_sorted.rend(); it++)
        sorted.push_back({it->first, it->second});

    return sorted;
}

std::vector<ScoredDevice>
remove_zero_score_devices(const std::vector<ScoredDevice>& score_devices)
{
    std::vector<ScoredDevice> nonzero;
    std::copy_if(score_devices.begin(), score_devices.end(), std::back_inserter(nonzero),
                 [] (const auto& score_device) -> bool {
                     return score_device.first > 0;
                 });
    return nonzero;
}
    
std::optional<VkImageView> create_image_view(const VkDevice& device,
                                             const VkImage image,
                                             const VkFormat format)
{
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    
    VkImageView view;
    const auto status = vkCreateImageView(device, &create_info, nullptr, &view);
    if (status != VK_SUCCESS)
        return {};

    return view;
}

std::vector<VkImage> get_swap_chain_images(const VkDevice& device,
                                           const VkSwapchainKHR& swap_chain)
{
    uint32_t count{0};
    vkGetSwapchainImagesKHR(device, swap_chain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(device, swap_chain, &count, images.data());
    return images;
}


std::vector<VkImageView> get_swap_chain_image_views(const VkDevice& device,
                                                    const VkSwapchainKHR& swap_chain,
                                                    const VkFormat format)
{
    std::vector<VkImageView> views;
    const auto images = get_swap_chain_images(device, swap_chain);
    for (size_t i = 0; i < images.size(); i++) {
        const auto view = create_image_view(device, images[i], format);
        if (!view)
            throw std::runtime_error("Could not create swap chain image views!");
        views.push_back(*view);
    }
    return views;
}

std::unique_ptr<GraphicsContext> GraphicsContext::create(uint32_t width,
                                                         uint32_t height,
                                                         const ValidationLayers validation_layers,
                                                         const DeviceExtensions device_extensions
                                                         )
{
    if (global::GraphicsContext_created)
        throw std::runtime_error("GraphicsContext::create() can only be called once");

    global::GraphicsContext_created = true;
    auto graphics_context = std::make_unique<GraphicsContext>();
    

    /* ===================================================================
     * Create Instance
     */

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Vulkan_LoadLibrary(nullptr);
   
    graphics_context->m_window = SDL_CreateWindow("Unnamed Window",
                                                  SDL_WINDOWPOS_UNDEFINED,
                                                  SDL_WINDOWPOS_UNDEFINED,
                                                  width,
                                                  height, 
                                                  SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    
    const auto extension_properties = get_available_extension_properties();
    std::cout << "Supported Extensions:\n";
    for (const auto& property: extension_properties) {
        std::cout << "\t" << property.extensionName << "\n";
    }

    const auto app_info = create_app_info("noname");
    const auto extensions = get_available_extensions(graphics_context->m_window);
    auto instance_info = create_instance_info(extensions, &app_info);
    
    std::cout << "Provided Validation Layers:\n";
    for (const auto& layer: validation_layers) {
        std::cout << "\t" << layer << "\n";
    }
    
    if (!validation_layers.empty()) {
        is_validation_layers_supported(validation_layers);
        instance_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        instance_info.ppEnabledLayerNames = validation_layers.data();
    } else {
        instance_info.enabledLayerCount = 0;
    }
    
    VkResult err{VK_ERROR_UNKNOWN};
    err = vkCreateInstance(&instance_info, nullptr, &graphics_context->m_instance);
    if (err)
        throw std::runtime_error("vkCreateInstance() returned non-ok");
    
    /* ===================================================================
     * Create Window Surface as the Main Rendertarget
     */

    SDL_Vulkan_CreateSurface(graphics_context->m_window,
                             graphics_context->m_instance, 
                             &graphics_context->m_window_surface);

    /* ===================================================================
     * Find Best Physical Device for our Instance & Window
     */

    const auto devices = get_available_physical_devices(graphics_context->m_instance);
    if (devices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    const auto sorted_score_devices = sort_devices_by_score(devices,
                                                      graphics_context->m_window_surface,
                                                      device_extensions);
    if (sorted_score_devices.empty())
        throw std::runtime_error("Failed to find suitable Device!");

    std::cout << "Devices in Machine:\n";
    for (const auto& score_device: sorted_score_devices) {
        auto info = get_physical_device_properties_features(score_device.second);
        const auto queue_families = get_queue_families(score_device.second);
        auto render_present_indices = find_graphics_present_indices(queue_families,
                                    score_device.second,
                                    graphics_context->m_window_surface);

        std::cout << "\t[Score: " << score_device.first << "]  " 
                  << info.properties.deviceName
                  << "  Family Indices: " << render_present_indices.stringify() << std::endl;
    }
    
    const auto nonzero_sorted_score_devices = remove_zero_score_devices(sorted_score_devices);
    if (nonzero_sorted_score_devices.empty())
        throw std::runtime_error("All Devices were invalid!");

    const auto best_device = sorted_score_devices[0].second;
    const auto best_properties = get_physical_device_properties(best_device);
 
    std::cout << "Best Device: " << best_properties.deviceName << "\n";
    graphics_context->m_physical_device = best_device;

    /* ===================================================================
     * Create Logical Device from our best device
     */

    const auto queue_families = get_queue_families(graphics_context->m_physical_device);
    auto render_present_indices = find_graphics_present_indices(queue_families,
                                    graphics_context->m_physical_device,
                                    graphics_context->m_window_surface);

    if (!render_present_indices.is_complete())
        throw std::runtime_error("Failed to find complete queue family in device!");
 
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = render_present_indices.graphics.value();
    queue_create_info.queueCount = 1;
    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features{};
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = &device_features;
    // Enable extensions
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();

    err = vkCreateDevice(graphics_context->m_physical_device,
                         &device_create_info,
                         nullptr,
                         &graphics_context->m_logical_device);
    if (err) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(graphics_context->m_logical_device, 
                     render_present_indices.graphics.value(),
                     0,
                     &graphics_context->m_graphics_queue);
    
    /* ===================================================================
     * Create Swap Chain
     */
    const auto swap_chain_info =
        get_swap_chain_support_info(graphics_context->m_physical_device,
                                    graphics_context->m_window_surface); 
    
    auto surface_format = find_ideal_swap_chain_surface_format(swap_chain_info.formats);
    if (!surface_format) {
        if (swap_chain_info.formats.empty())
            throw std::runtime_error("No surface formats exist on physical device!");
        surface_format = swap_chain_info.formats[0];
    }
    auto present_mode = find_ideal_swap_chain_present_mode(swap_chain_info.present_modes);
    if (!present_mode) {
        present_mode = get_default_swap_chain_present_mode();
    }
    
    const auto extent = get_swap_chain_extent(swap_chain_info.capabilities,
                                              graphics_context->m_window);
    
    const auto minimum_swap_chain_image_count =
        get_minimum_swap_chain_image_count(swap_chain_info.capabilities);
    std::cout << "Wanted image count in swap chain: " << minimum_swap_chain_image_count << std::endl;
    
    // TODO extract create swap chain to function to avoid name redefinitions
    // info on what this stuff means:
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain

    VkSwapchainCreateInfoKHR swap_chain_create_info{};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = graphics_context->m_window_surface;
    swap_chain_create_info.minImageCount = minimum_swap_chain_image_count;
    swap_chain_create_info.imageFormat = surface_format.value().format;
    swap_chain_create_info.imageColorSpace = surface_format.value().colorSpace;
    swap_chain_create_info.imageExtent = extent;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const auto sw_queue_families = get_queue_families(graphics_context->m_physical_device);
    const auto sw_indices = find_graphics_present_indices(queue_families,
                                                          graphics_context->m_physical_device,
                                                          graphics_context->m_window_surface);

    uint32_t queueFamilyIndices[] = {sw_indices.graphics.value(),
                                     sw_indices.present.value()};
    if (sw_indices.graphics != sw_indices.present) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        swap_chain_create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_create_info.queueFamilyIndexCount = 0; // Optional
        swap_chain_create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    swap_chain_create_info.preTransform = swap_chain_info.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.presentMode = present_mode.value();
    swap_chain_create_info.clipped = VK_TRUE;
    swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;
    
    err = vkCreateSwapchainKHR(graphics_context->m_logical_device,
                               &swap_chain_create_info, nullptr, 
                               &graphics_context->m_swap_chain);
    if (err)
        throw std::runtime_error("Failed to create swap chain!");
    

    /* ===================================================================
     * Get Swap Chain Image Views
     */
    graphics_context->m_swap_chain_image_views = 
        get_swap_chain_image_views(graphics_context->m_logical_device,
                                   graphics_context->m_swap_chain,
                                   surface_format.value().format);
    if (graphics_context->m_swap_chain_image_views.size() < minimum_swap_chain_image_count)
        throw std::runtime_error("Failed to create enough swap chain image views!");
    
    /* ===================================================================
     * Create Graphics Pipeline
     */



    std::cout << "GraphicsContext::create() complete!" << std::endl;
    return graphics_context;
}
    

GraphicsContext::~GraphicsContext()
{
    for (auto view : m_swap_chain_image_views)
        vkDestroyImageView(m_logical_device, view, nullptr);

    vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
    vkDestroySurfaceKHR(m_instance, m_window_surface, nullptr);
    vkDestroyDevice(m_logical_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    SDL_DestroyWindow(m_window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

}
