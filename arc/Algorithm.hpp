#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <string>
#include <sstream>
#include <vector>
#include <optional>

namespace ArcGraphics {
    
using ValidationLayers = std::vector<const char*>;
using DeviceExtensions = std::vector<const char*>;
    
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
    
   
//std::tuple<VkSurfaceFormatKHR, VkSwapchainKHR, std::vector<VkImageView>>
struct CreatedSwapChain {
    VkSwapchainKHR swap_chain;
    std::vector<VkImageView> image_views;
    VkSurfaceFormatKHR surface_format;
};

struct DeviceRenderingCapabilities {
    // A condensed collection of members from VkSurfaceCapabilitiesKHR
    uint32_t min_image_count;
    uint32_t max_image_count;
    uint32_t max_image_array_layers;
    VkSurfaceTransformFlagsKHR supported_transforms;
    VkSurfaceTransformFlagBitsKHR current_transform;
    VkCompositeAlphaFlagsKHR supported_composite_alpha;
    VkImageUsageFlags supported_usage_flags;

    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

[[nodiscard]]
VkApplicationInfo create_app_info(const char* appname);

    
[[nodiscard]]
std::vector<const char*> get_available_extensions(SDL_Window* window);

[[nodiscard]]
VkDescriptorPool create_uniform_descriptor_pool(const VkDevice& logical_device,
                                                const uint32_t frames_in_flight);
    
[[nodiscard]]
VkInstanceCreateInfo create_instance_info(const std::vector<const char*>& extensions,
                                          const VkApplicationInfo* app_info);

[[nodiscard]]
std::vector<VkExtensionProperties> get_available_extension_properties();

[[nodiscard]]
std::vector<VkLayerProperties> get_available_validation_layers();
    
[[nodiscard]]
bool is_validation_layers_supported(const ValidationLayers layers);
    
[[nodiscard]]
std::vector<VkPhysicalDevice> get_available_physical_devices(const VkInstance instance);

[[nodiscard]]
VkPhysicalDeviceProperties get_physical_device_properties(const VkPhysicalDevice device);

[[nodiscard]]
VkPhysicalDeviceFeatures get_physical_device_features(const VkPhysicalDevice device);

[[nodiscard]]
PhysicalDevicePropertyFeatureSet get_physical_device_properties_features(const VkPhysicalDevice device);

[[nodiscard]]
QueueFamilyIndices 
find_graphics_present_indices(const std::vector<VkQueueFamilyProperties> families,
                              const VkPhysicalDevice& device,
                              const VkSurfaceKHR& surface);

[[nodiscard]]
std::vector<VkQueueFamilyProperties> get_queue_families(const VkPhysicalDevice& device);

[[nodiscard]]
std::vector<VkExtensionProperties> get_device_extensions(const VkPhysicalDevice& device);

    
[[nodiscard]]
bool is_device_extensions_supported(const VkPhysicalDevice device,
                                    const std::vector<const char*> needed_extensions);

[[nodiscard]]
std::vector<VkSurfaceFormatKHR> get_swap_chain_formats(const VkPhysicalDevice& device,
                                                       const VkSurfaceKHR& surface);

[[nodiscard]]
std::vector<VkPresentModeKHR> get_swap_chain_present_modes(const VkPhysicalDevice& device,
                                                           const VkSurfaceKHR& surface);

[[nodiscard]]
DeviceRenderingCapabilities get_rendering_capabilities(const VkPhysicalDevice& device, 
                                                       const VkSurfaceKHR& surface);

[[nodiscard]]
std::optional<VkSurfaceFormatKHR>
find_swap_chain_surface_format(const std::vector<VkSurfaceFormatKHR>& surfaceformats,
                               const VkFormat format,
                               const VkColorSpaceKHR colorspace);

[[nodiscard]]
std::optional<VkSurfaceFormatKHR>
find_ideal_swap_chain_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);

[[nodiscard]]
std::optional<VkPresentModeKHR>
find_swap_chain_present_mode(const std::vector<VkPresentModeKHR>& presentmodes,
                             const VkPresentModeKHR mode);

[[nodiscard]]
VkPresentModeKHR get_default_swap_chain_present_mode();

[[nodiscard]]
std::optional<VkPresentModeKHR>
find_ideal_swap_chain_present_mode(const std::vector<VkPresentModeKHR>& presentmodes);

[[nodiscard]]
uint32_t get_minimum_swap_chain_image_count(const DeviceRenderingCapabilities& capabilities);
    
[[nodiscard]]
uint32_t calculate_device_score(VkPhysicalDevice device,
                                const VkSurfaceKHR& surface,
                                const std::vector<const char*> needed_extensions);

[[nodiscard]]
std::vector<ScoredDevice> sort_devices_by_score(const std::vector<VkPhysicalDevice>& devices,
                                                const VkSurfaceKHR& surface,
                                                const std::vector<const char*> extensions);

[[nodiscard]]
std::vector<ScoredDevice>
remove_zero_score_devices(const std::vector<ScoredDevice>& score_devices);
    
[[nodiscard]]
VkExtent2D get_window_size(const VkDevice logical_device, SDL_Window* window);

[[nodiscard]]
std::optional<VkImageView> create_image_view(const VkDevice& device,
                                             const VkImage image,
                                             const VkFormat format);

[[nodiscard]]
std::vector<VkImage> get_swap_chain_images(const VkDevice& device,
                                           const VkSwapchainKHR& swap_chain);

[[nodiscard]]
std::vector<VkImageView> get_swap_chain_image_views(const VkDevice& device,
                                                    const VkSwapchainKHR& swap_chain,
                                                    const VkFormat format);

[[nodiscard]]
VkPipelineColorBlendAttachmentState
create_color_blend_attachement_state(const bool use_alpha_blending);

[[nodiscard]]
VkPipelineColorBlendStateCreateInfo
create_color_blend_state_info(const VkPipelineColorBlendAttachmentState& attachment_state);

[[nodiscard]]
VkExtent2D scale_window_size_to_capabilities(VkExtent2D decired_extent, 
                                             const VkSurfaceCapabilitiesKHR& capabilities);
    
[[nodiscard]]
VkPhysicalDevice get_best_physical_device(const VkInstance instance,
                                          const VkSurfaceKHR window_surface,
                                          const DeviceExtensions extensions);

[[nodiscard]]
VkDevice get_logical_device(const VkPhysicalDevice physical_device,
                            const VkSurfaceKHR window_surface,
                            const DeviceExtensions extensions);


[[nodiscard]]
VkInstance create_instance(SDL_Window* window,
                           const ValidationLayers& validation_layers);


[[nodiscard]]
CreatedSwapChain create_swap_chain(const VkPhysicalDevice physical_device,
                                   const VkDevice logical_device,
                                   const VkSurfaceKHR window_surface,
                                   const uint32_t width,
                                   const uint32_t height);

}
