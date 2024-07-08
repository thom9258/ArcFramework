#include "../arc/Context.hpp"

#include <iostream>
#include <algorithm>
#include <map>
#include <set>
#include <optional>

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

struct PhysicalDevicePropertyFeatureSet {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};
    
// TODO: if this thing is only needed here, then delete it and use a optional instead
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    bool is_complete() {
        return graphics_family && present_family;
    }
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
    
QueueFamilyIndices find_queue_family_indices(const VkPhysicalDevice& device,
                                             const VkSurfaceKHR& surface,
                                             std::vector<VkQueueFamilyProperties> families)
{
    QueueFamilyIndices curr_indices;
    uint32_t i = 0;
    for (const auto& family : families) {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            curr_indices.graphics_family = i;
        }
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

        if (curr_indices.is_complete())
            break;
        i++;
    }
    return curr_indices;
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

uint32_t calculate_device_score(VkPhysicalDevice device,
                                const VkSurfaceKHR& surface,
                                const std::vector<const char*> needed_extensions)
{
    uint32_t score{0};
    if (device == VK_NULL_HANDLE)
        return 0;
    
    const auto info = get_physical_device_properties_features(device);
    if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;
    if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        score += 100;
    
    const auto queue_families = get_queue_families(device);
    auto queue_family_indices = find_queue_family_indices(device, surface, queue_families);
    if (!queue_family_indices.is_complete())
        return 0;
    if (!is_device_extensions_supported(device, needed_extensions))
        return 0;

    //TODO get more score paramters...
    return score;
}
    
std::vector<VkPhysicalDevice> sort_devices_by_score(
  const std::vector<VkPhysicalDevice>& devices,
  const VkSurfaceKHR& surface,
  const std::vector<const char*> needed_extensions) 
{
    std::multimap<uint32_t, VkPhysicalDevice> reverse_sorted;
    for (const auto& dpf : devices)
        reverse_sorted.insert({calculate_device_score(dpf, surface, needed_extensions), dpf});
    
    std::vector<VkPhysicalDevice> sorted;
    for (auto it = reverse_sorted.rbegin(); it != reverse_sorted.rend(); it++)
        sorted.push_back(it->second);

    return sorted;
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

    //const auto devices_property_features = get_physical_devices_properties_features(devices);
    std::cout << "Devices in Machine:\n";
    for (const auto& device: devices) {
        auto info = get_physical_device_properties_features(device);
        std::cout << "\t" << info.properties.deviceName << "\n";
    }
    
    const auto sorted_devices = sort_devices_by_score(devices,
                                                      graphics_context->m_window_surface,
                                                      device_extensions);
    if (sorted_devices.empty())
        throw std::runtime_error("Failed to find suitable Device!");
    const auto best_device = sorted_devices[0];
    const auto best_properties = get_physical_device_properties(best_device);
 
    std::cout << "Best Device: " << best_properties.deviceName << "\n";
    graphics_context->m_physical_device = best_device;

    /* ===================================================================
     * Create Logical Device from our best device
     */

    const auto queue_families = get_queue_families(graphics_context->m_physical_device);
    const auto queue_indices = find_queue_family_indices(graphics_context->m_physical_device,
                                                         graphics_context->m_window_surface,
                                                         queue_families);

    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_indices.graphics_family.value();
    queue_create_info.queueCount = 1;
    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();

    err = vkCreateDevice(graphics_context->m_physical_device,
                         &device_create_info,
                         nullptr,
                         &graphics_context->m_device);
    if (err) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(graphics_context->m_device, 
                     queue_indices.graphics_family.value(),
                     0,
                     &graphics_context->m_graphics_queue);
    

    return graphics_context;
}
    

GraphicsContext::~GraphicsContext()
{
    vkDestroySurfaceKHR(m_instance, m_window_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    SDL_DestroyWindow(m_window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}
    
}
