#include "../arc/Algorithm.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
#include <limits>

namespace ArcGraphics {

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
    
VkDescriptorPool create_uniform_descriptor_pool(const VkDevice& logical_device,
                                                const uint32_t frames_in_flight
)
{
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = frames_in_flight;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = frames_in_flight;

    VkDescriptorPool pool{};
    const auto status = vkCreateDescriptorPool(logical_device, &pool_info, nullptr, &pool);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool for uniform buffer!");

    return pool;
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
    
[[nodiscard]]
std::vector<VkExtensionProperties> get_available_extension_properties()
{
    uint32_t count{0};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());
    return available;
}
    
[[nodiscard]]
std::vector<VkLayerProperties> get_available_validation_layers()
{
    uint32_t count{0};
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());
    return available;
}
    
    
[[nodiscard]]
bool is_validation_layers_supported(const ValidationLayers layers)
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
    
   
[[nodiscard]]
std::vector<VkPhysicalDevice> get_available_physical_devices(const VkInstance instance)
{
    uint32_t count{0};
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> available(count);
    vkEnumeratePhysicalDevices(instance, &count, available.data());
    return available;
}
    
   
[[nodiscard]]
VkPhysicalDeviceProperties get_physical_device_properties(const VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    return properties;
}

[[nodiscard]]
VkPhysicalDeviceFeatures get_physical_device_features(const VkPhysicalDevice device)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);   
    return features;
}
    
[[nodiscard]]
PhysicalDevicePropertyFeatureSet get_physical_device_properties_features(const VkPhysicalDevice device)
{
    return {get_physical_device_properties(device), get_physical_device_features(device)};
}

[[nodiscard]]
QueueFamilyIndices
find_graphics_present_indices(const std::vector<VkQueueFamilyProperties> families,
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
    
[[nodiscard]]
std::vector<VkQueueFamilyProperties> get_queue_families(const VkPhysicalDevice& device)
{
    uint32_t count{0};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> available(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, available.data());
    return available;
}
    
[[nodiscard]]
std::vector<VkExtensionProperties> get_device_extensions(const VkPhysicalDevice& device)
{
    uint32_t count{0};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());
    return available;
}

[[nodiscard]]
bool is_device_extensions_supported(const VkPhysicalDevice device,
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

[[nodiscard]]
std::vector<VkSurfaceFormatKHR> get_swap_chain_formats(const VkPhysicalDevice& device,
                                                       const VkSurfaceKHR& surface)
{
    uint32_t count{0};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> available(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, available.data());
    return available;
}

[[nodiscard]]
std::vector<VkPresentModeKHR> get_swap_chain_present_modes(const VkPhysicalDevice& device,
                                                           const VkSurfaceKHR& surface)
{
    uint32_t count{0};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> available(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, available.data());
    return available;
}

[[nodiscard]]
DeviceRenderingCapabilities get_rendering_capabilities(const VkPhysicalDevice& device, 
                                                       const VkSurfaceKHR& surface)
{
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,
                                              surface,
                                              &surface_capabilities);

    DeviceRenderingCapabilities capabilities{};
    capabilities.min_image_count = surface_capabilities.minImageCount;
    capabilities.max_image_count = surface_capabilities.maxImageCount;
    capabilities.max_image_array_layers = surface_capabilities.maxImageArrayLayers;
    capabilities.current_transform = surface_capabilities.currentTransform;
    capabilities.supported_transforms = surface_capabilities.supportedTransforms;
    capabilities.supported_composite_alpha = surface_capabilities.supportedCompositeAlpha;
    capabilities.supported_usage_flags = surface_capabilities.supportedUsageFlags;

    capabilities.formats = get_swap_chain_formats(device, surface);
    capabilities.present_modes = get_swap_chain_present_modes(device, surface);
    return capabilities;
}

[[nodiscard]]
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

[[nodiscard]]
std::optional<VkSurfaceFormatKHR>
find_ideal_swap_chain_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
{
    return find_swap_chain_surface_format(formats,
                                          VK_FORMAT_B8G8R8A8_SRGB,
                                          VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
}

[[nodiscard]]
std::optional<VkPresentModeKHR>
find_swap_chain_present_mode(const std::vector<VkPresentModeKHR>& presentmodes,
                             const VkPresentModeKHR mode)
{
    for (const auto& presentmode: presentmodes)
        if (presentmode == mode)
            return presentmode;
    return {};

}

[[nodiscard]]
VkPresentModeKHR get_default_swap_chain_present_mode()
{
    return VK_PRESENT_MODE_FIFO_KHR;
}

[[nodiscard]]
std::optional<VkPresentModeKHR>
find_ideal_swap_chain_present_mode(const std::vector<VkPresentModeKHR>& presentmodes)
{
    return find_swap_chain_present_mode(presentmodes, VK_PRESENT_MODE_MAILBOX_KHR);
}
    
[[nodiscard]]
uint32_t get_minimum_swap_chain_image_count(const DeviceRenderingCapabilities& capabilities)
{
    auto count = capabilities.min_image_count + 1;
    if (capabilities.max_image_count > 0 && count > capabilities.max_image_count) 
        count = capabilities.max_image_count;
    return count;
}


[[nodiscard]]
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
    
    const auto swap_chain_info = get_rendering_capabilities(device, surface);

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
    if (!info.features.samplerAnisotropy)
        return 0; // This is apparently a required thing, so we need it here?

    // Ensure that discrete gpu's should always be picked over integrated,
    if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 5000;

    if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        score += 500;

    //TODO get more score paramters..?
    return score;
}

[[nodiscard]]
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

[[nodiscard]]
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

VkExtent2D get_window_size(const VkDevice logical_device, SDL_Window* window)
{
    int width, height;
    //SDL_GetWindowSize(window, &width, &height);
    vkDeviceWaitIdle(logical_device);
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    
    const VkExtent2D size = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
   return size;
}
    
[[nodiscard]]
std::optional<VkImageView> create_image_view(const VkDevice& device,
                                             const VkImage image,
                                             const VkFormat format)
{
    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    // Note that VK_COMPONENT_SWIZZLE_IDENTITY is specified as 0, and it is therefore
    // not fully nessecary to set the components of ImageViewCreateInfo
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

[[nodiscard]]
std::vector<VkImage> get_swap_chain_images(const VkDevice& device,
                                           const VkSwapchainKHR& swap_chain)
{
    uint32_t count{0};
    vkGetSwapchainImagesKHR(device, swap_chain, &count, nullptr);
    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(device, swap_chain, &count, images.data());
    return images;
}

[[nodiscard]]
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

[[nodiscard]]
VkPipelineColorBlendAttachmentState
create_color_blend_attachement_state(const bool use_alpha_blending)
{
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
                                          | VK_COLOR_COMPONENT_G_BIT 
                                          | VK_COLOR_COMPONENT_B_BIT 
                                          | VK_COLOR_COMPONENT_A_BIT;
    
    if (!use_alpha_blending) {
        color_blend_attachment.blendEnable = VK_FALSE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    } else {
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
 ;       color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    return color_blend_attachment;
}

[[nodiscard]]
VkPipelineColorBlendStateCreateInfo
create_color_blend_state_info(const VkPipelineColorBlendAttachmentState& attachment_state)
{
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &attachment_state;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional
    return color_blending;
}

[[nodiscard]]
VkExtent2D scale_window_size_to_capabilities(VkExtent2D decired_extent, 
                                             const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    decired_extent.width = std::clamp(decired_extent.width,
                                    capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    decired_extent.height = std::clamp(decired_extent.height,
                                     capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);
    return decired_extent;
}

[[nodiscard]]
VkPhysicalDevice get_best_physical_device(const VkInstance instance,
                                          const VkSurfaceKHR window_surface,
                                          const DeviceExtensions extensions)
{
    const auto devices = get_available_physical_devices(instance);
    if (devices.empty())
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    
    const auto sorted_score_devices = sort_devices_by_score(devices,
                                                            window_surface,
                                                            extensions);
    if (sorted_score_devices.empty())
        throw std::runtime_error("Failed to find suitable Device!");
    
    std::cout << "Devices in Machine:\n";
    for (const auto& score_device: sorted_score_devices) {
        auto info = get_physical_device_properties_features(score_device.second);
        const auto queue_families = get_queue_families(score_device.second);
        auto render_present_indices = find_graphics_present_indices(queue_families,
                                                                    score_device.second,
                                                                    window_surface);
        
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
    return best_device;
}
    

[[nodiscard]]
VkDevice get_logical_device(const VkPhysicalDevice physical_device,
                            const VkSurfaceKHR window_surface,
                            const DeviceExtensions extensions)
{   
    const auto queue_families = get_queue_families(physical_device);
    auto render_present_indices = find_graphics_present_indices(queue_families,
                                    physical_device,
                                    window_surface);

    if (!render_present_indices.is_complete())
        throw std::runtime_error("Failed to find complete queue family in device!");
 
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = render_present_indices.graphics.value();
    queue_create_info.queueCount = 1;
    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features{};
    // TODO: this is an optional thing but i do not know if it is strictly required
    // for the program to work or if there is an alternative?
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = &device_features;
    // Enable extensions
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    device_create_info.ppEnabledExtensionNames = extensions.data();

    VkDevice logical_device;
    auto status = vkCreateDevice(physical_device,
                         &device_create_info,
                         nullptr,
                         &logical_device);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");
    return logical_device;
} 
    
[[nodiscard]]
VkInstance create_instance(SDL_Window* window,
                           const ValidationLayers& validation_layers)
{
    const auto app_info = create_app_info("noname");
    const auto extensions = get_available_extensions(window);
    auto instance_info = create_instance_info(extensions, &app_info);
    std::cout << "Provided Validation Layers:\n";
    for (const auto& layer: validation_layers) {
        std::cout << "\t" << layer << "\n";
    }
    
    if (!validation_layers.empty()) {
        if (!is_validation_layers_supported(validation_layers))
            throw std::runtime_error("The retrieved validation layers are not supported!");
        instance_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        instance_info.ppEnabledLayerNames = validation_layers.data();
    } else {
        instance_info.enabledLayerCount = 0;
    }
    
    VkInstance instance{};
    const auto status = vkCreateInstance(&instance_info, nullptr, &instance);
    if (status != VK_SUCCESS)
        throw std::runtime_error("vkCreateInstance() returned non-ok");
    return instance;
}

CreatedSwapChain create_swap_chain(const VkPhysicalDevice physical_device,
                                   const VkDevice logical_device,
                                   const VkSurfaceKHR window_surface,
                                   const uint32_t width,
                                   const uint32_t height)
{
    CreatedSwapChain swap_chain;
    /* ===================================================================
     * Create Swap Chain
     */
    const auto swap_chain_info =
        get_rendering_capabilities(physical_device,
                                   window_surface); 
    
    auto ideal_surface_format = find_ideal_swap_chain_surface_format(swap_chain_info.formats);
    if (ideal_surface_format) {
        swap_chain.surface_format = ideal_surface_format.value();
    }
    else {
        if (swap_chain_info.formats.empty())
            throw std::runtime_error("No surface formats exist on physical device!");
        swap_chain.surface_format = swap_chain_info.formats[0];
    }

    auto present_mode = find_ideal_swap_chain_present_mode(swap_chain_info.present_modes);
    if (!present_mode) {
        present_mode = get_default_swap_chain_present_mode();
    }
    
    const auto minimum_swap_chain_image_count =
        get_minimum_swap_chain_image_count(swap_chain_info);
    std::cout << "Wanted image count in swap chain: " << minimum_swap_chain_image_count << std::endl;
    
    // TODO extract create swap chain to function to avoid name redefinitions
    // info on what this stuff means:
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain

    VkSwapchainCreateInfoKHR swap_chain_create_info{};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = window_surface;
    swap_chain_create_info.minImageCount = minimum_swap_chain_image_count;
    swap_chain_create_info.imageFormat = swap_chain.surface_format.format;
    swap_chain_create_info.imageColorSpace = swap_chain.surface_format.colorSpace;
    swap_chain_create_info.imageExtent = {width, height};
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const auto queue_families = get_queue_families(physical_device);
    const auto indices = find_graphics_present_indices(queue_families,
                                                       physical_device,
                                                       window_surface);

    uint32_t queueFamilyIndices[] = {indices.graphics.value(),
                                     indices.present.value()};
    if (indices.graphics != indices.present) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        swap_chain_create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_create_info.queueFamilyIndexCount = 0; // Optional
        swap_chain_create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    swap_chain_create_info.preTransform = swap_chain_info.current_transform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.presentMode = present_mode.value();
    swap_chain_create_info.clipped = VK_TRUE;
    swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;
    
    auto status = vkCreateSwapchainKHR(logical_device,
                                       &swap_chain_create_info, nullptr, 
                                       &swap_chain.swap_chain);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain!");
    
    /* ===================================================================
     * Get Swap Chain Image Views
     */
    swap_chain.image_views = 
        get_swap_chain_image_views(logical_device,
                                   swap_chain.swap_chain,
                                   swap_chain.surface_format.format);
    if (swap_chain.image_views.size() < minimum_swap_chain_image_count)
        throw std::runtime_error("Failed to create enough swap chain image views!");

    std::cout << "Actual image count in swap chain: " 
              << swap_chain.image_views.size()
              << std::endl;
    return swap_chain;
}
   
}
