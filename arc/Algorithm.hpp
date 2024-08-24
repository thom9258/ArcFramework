#pragma once
/** *******************************************************************
 * @file Algorithm.hpp
 * @brief The grand collection of different Vulkan and SDL related tools.
 *
 * This file is designed to be the backbone of ArcGraphics, yet is
 * also as modular as possible, to the extent where it can be used anywhere.
 *
 * The functions are designed to all be pure, to minimize the mental overload
 * of each.
 *
 * @copyright
 * Copyright 2024 Thomas Alexgaard Jensen
 *********************************************************************/


#include "SDLVulkan.hpp"
#include "GLM.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <optional>

namespace ArcGraphics {
    
/**
 * @brief A collection of validation layers.
 */
using ValidationLayers = std::vector<const char*>;
    
/**
 * @brief A collection of device extensions.
 */
using DeviceExtensions = std::vector<const char*>;
    
/**
 * @brief The internal scoring value for physical device selection.
 */
using DeviceScore = uint32_t;

/**
 * @brief A device and its scoring value.
 */
using ScoredDevice = std::pair<DeviceScore, VkPhysicalDevice>;

/**
 * @brief Properties & Features associated with a device.
 */
struct PhysicalDevicePropertyFeatureSet {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};
    
/**
 * @brief The indices of the relevant queue families of a physical device.
 */
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics; /// graphics index
    std::optional<uint32_t> present;  /// presentation index

    /**
    * @brief Predicate for if all the required indices found.
    */
    bool is_complete() {
        return graphics && present;
    }

    /**
    * @brief Stringifier for debugging.
    */
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
    
/**
 * @brief Return value for creating swap chains.
 */
struct CreatedSwapChain {
    VkSwapchainKHR swap_chain;
    std::vector<VkImageView> image_views;
    VkSurfaceFormatKHR surface_format;
};

/**
 * @brief Create swap chain.
 * @throw std::runtime_error if creating swap chain is not possible.
 */
[[nodiscard]]
CreatedSwapChain create_swap_chain(const VkPhysicalDevice physical_device,
                                   const VkDevice logical_device,
                                   const VkSurfaceKHR window_surface,
                                   const uint32_t width,
                                   const uint32_t height);

/**
 * @brief A collection of Rendering Capabilities for a physical device. 
 * @note This is essencially a condensed version of VkSurfaceCapabilitiesKHR
 * where everything non-constant is removed, such as things related to the
 * current surface.
 */
struct DeviceRenderingCapabilities {
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

/**
 * @brief Get Rendering Capabilities of a physical device and its surface.
 */
[[nodiscard]]
DeviceRenderingCapabilities get_rendering_capabilities(const VkPhysicalDevice& device, 
                                                       const VkSurfaceKHR& surface);

/**
 * @brief Create Application information for the Vulkan instance.
 */
[[nodiscard]]
VkApplicationInfo create_app_info(const char* appname);

    
/**
 * @brief Get extensions for window & instance.
 */
[[nodiscard]]
std::vector<const char*> get_available_extensions(SDL_Window* window);

   
/**
 * @brief Create descriptor pool sizes from the desired bindings and frames in flight.
 */
[[nodiscard]]
std::vector<VkDescriptorPoolSize>
create_descriptor_pool_sizes(const std::vector<VkDescriptorSetLayoutBinding> bindings,
                             const uint32_t frames_in_flight);
    

/**
 * @brief Create instance info with decired extensions.
 */
[[nodiscard]]
VkInstanceCreateInfo create_instance_info(const std::vector<const char*>& extensions,
                                          const VkApplicationInfo* app_info);

/**
 * @brief Get extension properties.
 */
[[nodiscard]]
std::vector<VkExtensionProperties> get_available_extension_properties();

/**
 * @brief Get available validation layers.
 */
[[nodiscard]]
std::vector<VkLayerProperties> get_available_validation_layers();
    
/**
 * @brief Check that a collection of validation layers are available.
 */
[[nodiscard]]
bool is_validation_layers_supported(const ValidationLayers layers);
    
/**
 * @brief Check that a collection of validation layers are available.
 */
[[nodiscard]]
std::vector<VkPhysicalDevice> get_available_physical_devices(const VkInstance instance);

/**
 * @brief Get properties of physical device.
 */
[[nodiscard]]
VkPhysicalDeviceProperties get_physical_device_properties(const VkPhysicalDevice device);
    
/**
 * @brief Get collection of memory properties of physical device.
 */
[[nodiscard]]
VkPhysicalDeviceMemoryProperties
get_physical_device_memory_properties(const VkPhysicalDevice device);

/**
 * @brief Find suitable memory type in collection of memory properties.
 */
[[nodiscard]]
uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties mem_properties,
                          const uint32_t type_filter,
                          const VkMemoryPropertyFlags property_flags);
 
/**
 * @brief Get collection of physical device features.
 */
[[nodiscard]]
VkPhysicalDeviceFeatures get_physical_device_features(const VkPhysicalDevice device);

/**
 * @brief Get both collections of of physical device properties & features.
 */
[[nodiscard]]
PhysicalDevicePropertyFeatureSet
get_physical_device_properties_features(const VkPhysicalDevice device);

/**
 * @brief Get physical device indice families.
 */
[[nodiscard]]
std::vector<VkQueueFamilyProperties> get_queue_families(const VkPhysicalDevice& device);

/**
 * @brief Find graphics & presentation indices in collection of indice families.
 */
[[nodiscard]]
QueueFamilyIndices 
find_graphics_present_indices(const std::vector<VkQueueFamilyProperties> families,
                              const VkPhysicalDevice& device,
                              const VkSurfaceKHR& surface);

/**
 * @brief Get physical device extensions.
 */
[[nodiscard]]
std::vector<VkExtensionProperties> get_device_extensions(const VkPhysicalDevice& device);

/**
* @brief check if physical device supports the specified extensions.
*/
[[nodiscard]]
bool is_device_extensions_supported(const VkPhysicalDevice device,
                                    const std::vector<const char*> needed_extensions);

/**
* @brief check if physical device supports the specified extensions.
*/
[[nodiscard]]
std::vector<VkSurfaceFormatKHR> get_swap_chain_formats(const VkPhysicalDevice& device,
                                                       const VkSurfaceKHR& surface);

/**
* @brief Get collection of presentation modes of a swap chain created from physical device.
*/
[[nodiscard]]
std::vector<VkPresentModeKHR> get_swap_chain_present_modes(const VkPhysicalDevice& device,
                                                           const VkSurfaceKHR& surface);

/**
* @brief Get collection of presentation modes of a swap chain created from physical device.
*/
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
    
    
void create_image(const VkPhysicalDevice physical_device,
                  const VkDevice logical_device,
                  const uint32_t width,
                  const uint32_t height,
                  const VkFormat format,
                  const VkImageTiling tiling,
                  const VkImageUsageFlags usage,
                  const VkMemoryPropertyFlags properties,
                  VkImage& image,
                  VkDeviceMemory& memory);

[[nodiscard]]
std::optional<VkImageView> create_image_view(const VkDevice& device,
                                             const VkImage image,
                                             const VkFormat format,
                                             const VkImageAspectFlags aspect);

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


}
