#include "../arc/Context.hpp"
#include "../arc/UniformBuffer.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <set>
#include <optional>
#include <sstream>
#include <string>
#include <cstdint>
#include <limits>

// https://github.com/AndreVallestero/sdl-vulkan-tutorial/blob/master/hello-triangle/main.cpp
// https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Instance

// TODO: Get message callbacks with vulkan errors:
// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers

namespace arc {
    
namespace global {

std::atomic_bool GraphicsContext_created{false};

}
   
ShaderBytecode read_shader_bytecode(const std::string& filename) 
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t file_size = static_cast<size_t>(file.tellg());
    ShaderBytecode buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

ShaderModule::ShaderModule(const ShaderBytecode& code, const VkDevice& device)
    : m_logical_device(device)
{
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    const auto status = vkCreateShaderModule(m_logical_device, &create_info, nullptr, &m_shader_module);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");
}
    
ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(m_logical_device, m_shader_module, nullptr);
}
    
const VkShaderModule& ShaderModule::get_module() const noexcept
{
    return m_shader_module;
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


[[nodiscard]]
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
    
[[nodiscard]]
std::vector<const char*> get_available_extensions(SDL_Window* window)
{
    uint32_t count{0};
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    std::vector<const char*> available(count);
    SDL_Vulkan_GetInstanceExtensions(window, &count, available.data());
    return available;
}
    
[[nodiscard]]
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
  
[[nodiscard]]
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
    
[[nodiscard]]
std::vector<VkPhysicalDevice> get_available_physical_devices(const VkInstance& instance)
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
SwapChainSupportInfo get_swap_chain_support_info(const VkPhysicalDevice& device, 
                                                 const VkSurfaceKHR& surface)
{
    SwapChainSupportInfo details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    details.formats = get_swap_chain_formats(device, surface);
    details.present_modes = get_swap_chain_present_modes(device, surface);
    return details;
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
uint32_t get_minimum_swap_chain_image_count(const VkSurfaceCapabilitiesKHR& capabilities)
{
    auto count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && count > capabilities.maxImageCount) 
        count = capabilities.maxImageCount;
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
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
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
VkExtent2D GraphicsContext::get_window_size()
{
    int width, height;
    //SDL_GetWindowSize(window, &width, &height);
    vkDeviceWaitIdle(m_logical_device);
    SDL_Vulkan_GetDrawableSize(m_window, &width, &height);
    
    const VkExtent2D size = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
   return size;
}

VkExtent2D GraphicsContext::scale_window_size_to_be_suitable(VkExtent2D decired)
{
    const auto info =  get_swap_chain_support_info(m_physical_device, 
                                                   m_window_surface);
    return scale_window_size_to_capabilities(decired, info.capabilities);
}

void GraphicsContext::record_command_buffer(VkCommandBuffer command_buffer,
                                            uint32_t image_index) 
{
    /* ===================================================================
     * Begin Command Buffer
     */
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0; // Optional
    /*
    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:
    The command buffer will be rerecorded right after executing it once.
    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT:
    This is a secondary command buffer that will be entirely within a single render pass.
    VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:
    The command buffer can be resubmitted while it is also already pending execution.

    None of these flags are applicable for us right now.
    */

    begin_info.pInheritanceInfo = nullptr; // Optional
    /*
    The pInheritanceInfo parameter is only relevant for secondary command buffers.
    It specifies which state to inherit from the calling primary command buffers.   
     */

    auto status = vkBeginCommandBuffer(command_buffer, &begin_info);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");
    
    /* ===================================================================
     * Begin Command Buffer
     */
     const auto swap_chain_info =
        get_swap_chain_support_info(m_physical_device,
                                    m_window_surface); 
    
    const auto extent = scale_window_size_to_be_suitable(get_window_size());

    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass = m_render_pass;
    renderpass_info.framebuffer = m_swap_chain_framebuffers[image_index];
    renderpass_info.renderArea.offset = {0, 0};
    renderpass_info.renderArea.extent = extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderpass_info.clearValueCount = 1;
    renderpass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);   
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    

    /* ===================================================================
     * Draw
     */
    VkBuffer vertex_buffers[] = {m_vertex_buffer->get_buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, m_index_buffer->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_graphics_pipeline_layout,
                            0,
                            1,
                            &m_descriptor_sets[m_flight_frame],
                            0,
                            nullptr);
    //std::cout << "flightframe: " << m_flight_frame << std::endl;
    
    const uint32_t instanceCount = 1;
    const uint32_t firstIndex = 0;
    const int32_t  vertexOffset = 0;
    const uint32_t firstInstance = 0;
    vkCmdDrawIndexed(command_buffer,
                     m_index_buffer->get_count(),
                     instanceCount,
                     firstIndex,
                     vertexOffset,
                     firstInstance);
    
    //const uint32_t vertex_count = m_vertex_buffer->get_count();
    //const uint32_t instance_count = 1;
    //const uint32_t first_vertex_index = 0;
    //const uint32_t first_instance = 0;
    //vkCmdDraw(command_buffer,
    //          vertex_count,
    //          instance_count,
    //          first_vertex_index,
    //          first_instance);

    vkCmdEndRenderPass(command_buffer);
    status = vkEndCommandBuffer(command_buffer);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer!");
}

GraphicsContext::GraphicsContext(const uint32_t width,
                                 const uint32_t height,
                                 const ShaderBytecode& vertex_bytecode,
                                 const ShaderBytecode& fragment_bytecode
                                 )
{
    const arc::GraphicsContext::ValidationLayers validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const arc::GraphicsContext::DeviceExtensions device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    if (global::GraphicsContext_created)
        throw std::runtime_error("Constructor GraphicsContext() can only be called once");

    global::GraphicsContext_created = true;

    /* ===================================================================
     * Create Instance
     */

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Vulkan_LoadLibrary(nullptr);
   
    m_window = SDL_CreateWindow("Unnamed Window",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                width,
                                height, 
                                SDL_WINDOW_SHOWN 
                                | SDL_WINDOW_RESIZABLE 
                                | SDL_WINDOW_VULKAN);
    
    const auto extension_properties = get_available_extension_properties();
    std::cout << "Supported Extensions:\n";
    for (const auto& property: extension_properties) {
        std::cout << "\t" << property.extensionName << "\n";
    }

    const auto app_info = create_app_info("noname");
    const auto extensions = get_available_extensions(m_window);
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
    
    VkResult err{VK_ERROR_UNKNOWN};
    err = vkCreateInstance(&instance_info, nullptr, &m_instance);
    if (err)
        throw std::runtime_error("vkCreateInstance() returned non-ok");
    
    /* ===================================================================
     * Create Window Surface as the Main Rendertarget
     */

    SDL_Vulkan_CreateSurface(m_window,
                             m_instance, 
                             &m_window_surface);

    /* ===================================================================
     * Find Best Physical Device for our Instance & Window
     */

    const auto devices = get_available_physical_devices(m_instance);
    if (devices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    const auto sorted_score_devices = sort_devices_by_score(devices,
                                                      m_window_surface,
                                                      device_extensions);
    if (sorted_score_devices.empty())
        throw std::runtime_error("Failed to find suitable Device!");

    std::cout << "Devices in Machine:\n";
    for (const auto& score_device: sorted_score_devices) {
        auto info = get_physical_device_properties_features(score_device.second);
        const auto queue_families = get_queue_families(score_device.second);
        auto render_present_indices = find_graphics_present_indices(queue_families,
                                    score_device.second,
                                    m_window_surface);

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
    m_physical_device = best_device;

    /* ===================================================================
     * Create Logical Device from our best device
     */

    const auto queue_families = get_queue_families(m_physical_device);
    auto render_present_indices = find_graphics_present_indices(queue_families,
                                    m_physical_device,
                                    m_window_surface);

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

    err = vkCreateDevice(m_physical_device,
                         &device_create_info,
                         nullptr,
                         &m_logical_device);
    if (err) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(m_logical_device, 
                     render_present_indices.graphics.value(),
                     0,
                     &m_graphics_queue);
    
    /* ===================================================================
     * Create Swap Chain
     */
    create_swap_chain(width, height);
   
    /* ===================================================================
     * Create Shader Stages
     */
    
    const auto vertex = VertexShaderModule(vertex_bytecode,
                                           m_logical_device);
    
    VkPipelineShaderStageCreateInfo vertex_create_info{};
    vertex_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_create_info.module = vertex.get_module();
    vertex_create_info.pName = "main";

    const auto fragment = VertexShaderModule(fragment_bytecode,
                                             m_logical_device);

    VkPipelineShaderStageCreateInfo fragment_create_info{};
    fragment_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_create_info.module = fragment.get_module();
    fragment_create_info.pName = "main";   

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_create_info,
                                                       fragment_create_info};
    
    /* ===================================================================
     * Create Shader Stages
     */
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    const auto binding_description = Vertex::get_binding_description();
    const auto attribute_descriptions = Vertex::get_attribute_descriptions();

    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;

    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    

    const std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    /* ===================================================================
     * Create Pipeline Viewport
     */
    const auto swap_chain_info = get_swap_chain_support_info(m_physical_device,
                                                             m_window_surface); 

    const auto extent = scale_window_size_to_be_suitable(get_window_size());
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;
    
    /* ===================================================================
     * Create Rasterizer
     */

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    constexpr bool use_alpha_blending = false;

    const auto color_blending_attachment_state =
        create_color_blend_attachement_state(use_alpha_blending);
    const auto color_blending =
        create_color_blend_state_info(color_blending_attachment_state);

    /* ===================================================================
     * Create Multisample State
     */
    
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    /* ===================================================================
     * Create Pipeline Layout
     */
    
    // Here we add our uniform buffer layouts
    m_descriptor_layout = 
        create_uniform_vertex_descriptorset_layout(m_logical_device, 0, 1);
    
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &m_descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    auto status = vkCreatePipelineLayout(m_logical_device,
                                         &pipeline_layout_info,
                                         nullptr,
                                         &m_graphics_pipeline_layout);
    if (status != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    
    /* ===================================================================
     * Create Render Passes
     */

    VkAttachmentDescription color_attachment{};
    color_attachment.format = m_swap_chain_surface_format.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    status = vkCreateRenderPass(m_logical_device,
                                &render_pass_info,
                                nullptr,
                                &m_render_pass);

    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass!");
    
    
    /* ===================================================================
     * Create Pipeline
     */

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = nullptr; // Optional
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = m_graphics_pipeline_layout;
    pipeline_info.renderPass = m_render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1; // Optional
    
    status = vkCreateGraphicsPipelines(m_logical_device,
                                       VK_NULL_HANDLE,
                                       1,
                                       &pipeline_info,
                                       nullptr,
                                       &m_graphics_pipeline);

    if (status  != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline!");
    

   /* ===================================================================
    * Create Swap Chain Framebuffers
    */
    create_swap_chain_framebuffers(width, height);
   
    /* ===================================================================
     * Create Command Pool
     */
    { //NOTE: We add a scope here to avoid variable renaming
        const auto queue_families = get_queue_families(m_physical_device);
        auto queue_families_indices =
            find_graphics_present_indices(queue_families,
                                        m_physical_device,
                                        m_window_surface);

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = queue_families_indices.graphics.value();

        const auto status = vkCreateCommandPool(m_logical_device,
                                                &pool_info,
                                                nullptr,
                                                &m_command_pool);
        if (status != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool!");
    }

    /* ===================================================================
     * Create Vertex Buffers
     */
    const std::vector<arc::Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    const IndexBuffer::vector_type indices = {
        0, 1, 2, 2, 3, 0
    };
    // Normal Copy
    //m_vertex_buffer = VertexBuffer::create(m_physical_device, m_logical_device, vertices);

    // Staging Buffer Copy
    m_vertex_buffer = VertexBuffer::create_staging(m_physical_device,
                                                   m_logical_device,
                                                   m_command_pool,
                                                   m_graphics_queue,
                                                   vertices);

    if (m_vertex_buffer == nullptr)
            throw std::runtime_error("Failed to create vertex buffer!");
    
    m_index_buffer = IndexBuffer::create(m_physical_device,
                                                   m_logical_device,
                                                   indices);

    if (m_index_buffer == nullptr)
            throw std::runtime_error("Failed to create index buffer!");
    
    /* ===================================================================
     * Create Uniform Buffers
     */
    m_uniform_buffers.resize(m_max_frames_in_flight);
    m_uniform_buffers_memory.resize(m_max_frames_in_flight);
    m_uniform_buffers_mapped.resize(m_max_frames_in_flight);
    
    for (uint32_t i = 0; i < m_max_frames_in_flight; i++) {
        create_uniform_buffer(m_physical_device,
                              m_logical_device,
                              sizeof(UniformBufferObject),
                              m_uniform_buffers[i],
                              m_uniform_buffers_memory[i],
                              m_uniform_buffers_mapped[i]);
    }
    
    std::cout << "created uniform buffers" << std::endl;

    /* ===================================================================
     * Create Descriptor Pool and Sets
     */
    m_descriptor_pool = 
        create_uniform_descriptor_pool(m_logical_device, m_max_frames_in_flight);

    std::vector<VkDescriptorSetLayout> layouts(m_max_frames_in_flight,
                                               m_descriptor_layout);
    
    VkDescriptorSetAllocateInfo descriptor_pool_alloc_info{};
    descriptor_pool_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_pool_alloc_info.descriptorPool = m_descriptor_pool;
    descriptor_pool_alloc_info.descriptorSetCount = m_max_frames_in_flight;
    descriptor_pool_alloc_info.pSetLayouts = layouts.data();

    m_descriptor_sets.resize(m_max_frames_in_flight);
    status = vkAllocateDescriptorSets(m_logical_device,
                                      &descriptor_pool_alloc_info,
                                      m_descriptor_sets.data());
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets!");
    
    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = m_uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);
        
        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = m_descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr; // Optional
        descriptor_write.pTexelBufferView = nullptr; // Optional
        vkUpdateDescriptorSets(m_logical_device, 1, &descriptor_write, 0, nullptr);
    }

    std::cout << "created and allocated descriptor pool" << std::endl;
    /* ===================================================================
     * Create Command Buffers
     */
    
    m_command_buffers.resize(m_max_frames_in_flight);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    /*
    VK_COMMAND_BUFFER_LEVEL_PRIMARY:
    Can be submitted to a queue for execution, but cannot be called from other command buffers.
    VK_COMMAND_BUFFER_LEVEL_SECONDARY:
    Cannot be submitted directly, but can be called from primary command buffers.
    */
    alloc_info.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());
    status = vkAllocateCommandBuffers(m_logical_device,
                                      &alloc_info,
                                      m_command_buffers.data());
    if (status  != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");

    std::cout << "allocated command buffers" << std::endl;
    
    /* ===================================================================
     * Create Sync Objects
     */

    m_semaphores_image_available.resize(m_max_frames_in_flight);
    m_semaphores_rendering_finished.resize(m_max_frames_in_flight);
    m_fences_in_flight.resize(m_max_frames_in_flight);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        status = vkCreateSemaphore(m_logical_device,
                                   &semaphore_info,
                                   nullptr,
                                   &m_semaphores_image_available[i]);
        if (status != VK_SUCCESS)
            throw std::runtime_error("failed to create image available semaphore["
                                     + std::to_string(i) + "]");
        
        status = vkCreateSemaphore(m_logical_device,
                                   &semaphore_info,
                                   nullptr,
                                   &m_semaphores_rendering_finished[i]);
        if (status != VK_SUCCESS)
            throw std::runtime_error("failed to create rendering finished semaphore["
                                     + std::to_string(i) + "]");

        status = vkCreateFence(m_logical_device,
                            &fence_info,
                            nullptr,
                            &m_fences_in_flight[i]);
        if (status != VK_SUCCESS)
            throw std::runtime_error("failed to create fence in flight ["
                                     + std::to_string(i) + "]");
    }

    if (m_swap_chain_framebuffer_resized) {
        recreate_swap_chain();
        return;
    } 

    std::cout << "Constructor GraphicsContext() complete!" << std::endl;
}

void GraphicsContext::window_resized_event()
{
    m_swap_chain_framebuffer_resized = true;
}

void GraphicsContext::update_ubo_rotation(uint32_t flight_frame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f),
                            time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, 1.0f));

    const auto windowsize = get_window_size();
    ubo.proj = glm::perspective(glm::radians(45.0f),
                                windowsize.width / (float)windowsize.height,
                                0.1f,
                                10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(m_uniform_buffers_mapped[flight_frame], &ubo, sizeof(ubo));
} 
 

   
void GraphicsContext::draw_frame()
{
    vkWaitForFences(m_logical_device,
                    1,
                    &m_fences_in_flight[m_flight_frame],
                    VK_TRUE,
                    UINT64_MAX);
    
    if (m_swap_chain_framebuffer_resized) {
        recreate_swap_chain();
        return;
    }

    uint32_t image_index;
    auto status = vkAcquireNextImageKHR(m_logical_device,
                                        m_swap_chain,
                                        UINT64_MAX,
                                        m_semaphores_image_available[m_flight_frame],
                                        VK_NULL_HANDLE,
                                        &image_index);

    if (status == VK_ERROR_OUT_OF_DATE_KHR || m_swap_chain_framebuffer_resized) {
        recreate_swap_chain();
        return;
    } 
    else if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image!");
    
    // Reset fences now we could get next image to draw on
    vkResetFences(m_logical_device, 1, &m_fences_in_flight[m_flight_frame]);

    vkResetCommandBuffer(m_command_buffers[m_flight_frame], 0);
    record_command_buffer(m_command_buffers[m_flight_frame], image_index);
    
    /* Update Uniform buffers here
     */
    update_ubo_rotation(m_flight_frame);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkSemaphore> waitSemaphores = {m_semaphores_image_available[m_flight_frame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submit_info.waitSemaphoreCount = waitSemaphores.size();
    submit_info.pWaitSemaphores = waitSemaphores.data();
    submit_info.pWaitDstStageMask = waitStages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffers[m_flight_frame];
    
    std::vector<VkSemaphore> signalSemaphores =
        {m_semaphores_rendering_finished[m_flight_frame]};
    submit_info.signalSemaphoreCount = signalSemaphores.size();
    submit_info.pSignalSemaphores = signalSemaphores.data();

    status = vkQueueSubmit(m_graphics_queue,
                           1,
                           &submit_info,
                           m_fences_in_flight[m_flight_frame]); 
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer!");
    

    /* ===================================================================
     * Presentation
     */
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = signalSemaphores.size();
    present_info.pWaitSemaphores = signalSemaphores.data();
    std::vector<VkSwapchainKHR> swapChains = {m_swap_chain};
    present_info.swapchainCount = swapChains.size();
    present_info.pSwapchains = swapChains.data();
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr; // Optional

    status = vkQueuePresentKHR(m_graphics_queue, &present_info);
    if (status == VK_ERROR_OUT_OF_DATE_KHR || m_swap_chain_framebuffer_resized) {
        recreate_swap_chain();
        return;
    } 
    else if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image!");

    m_flight_frame = (m_flight_frame + 1) % m_max_frames_in_flight;
}


void GraphicsContext::destroy_swap_chain() {
    for (auto& framebuffer: m_swap_chain_framebuffers)
        vkDestroyFramebuffer(m_logical_device, framebuffer, nullptr);

    for (auto& view: m_swap_chain_image_views)
        vkDestroyImageView(m_logical_device, view, nullptr);

    vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
}


void GraphicsContext::create_swap_chain(const uint32_t width,
                                        const uint32_t height)
{
    /* ===================================================================
     * Create Swap Chain
     */
    const auto swap_chain_info =
        get_swap_chain_support_info(m_physical_device,
                                    m_window_surface); 
    
    auto ideal_surface_format = find_ideal_swap_chain_surface_format(swap_chain_info.formats);
    if (ideal_surface_format) {
        m_swap_chain_surface_format = ideal_surface_format.value();
    }
    else {
        if (swap_chain_info.formats.empty())
            throw std::runtime_error("No surface formats exist on physical device!");
        m_swap_chain_surface_format = swap_chain_info.formats[0];
    }

    auto present_mode = find_ideal_swap_chain_present_mode(swap_chain_info.present_modes);
    if (!present_mode) {
        present_mode = get_default_swap_chain_present_mode();
    }
    
    const auto minimum_swap_chain_image_count =
        get_minimum_swap_chain_image_count(swap_chain_info.capabilities);
    std::cout << "Wanted image count in swap chain: " << minimum_swap_chain_image_count << std::endl;
    
    // TODO extract create swap chain to function to avoid name redefinitions
    // info on what this stuff means:
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain

    VkSwapchainCreateInfoKHR swap_chain_create_info{};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = m_window_surface;
    swap_chain_create_info.minImageCount = minimum_swap_chain_image_count;
    swap_chain_create_info.imageFormat = m_swap_chain_surface_format.format;
    swap_chain_create_info.imageColorSpace = m_swap_chain_surface_format.colorSpace;
    swap_chain_create_info.imageExtent = {width, height};
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const auto queue_families = get_queue_families(m_physical_device);
    const auto indices = find_graphics_present_indices(queue_families,
                                                       m_physical_device,
                                                       m_window_surface);

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

    swap_chain_create_info.preTransform = swap_chain_info.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.presentMode = present_mode.value();
    swap_chain_create_info.clipped = VK_TRUE;
    swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;
    
    auto status = vkCreateSwapchainKHR(m_logical_device,
                                       &swap_chain_create_info, nullptr, 
                                       &m_swap_chain);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain!");
    
    /* ===================================================================
     * Get Swap Chain Image Views
     */
    m_swap_chain_image_views = 
        get_swap_chain_image_views(m_logical_device,
                                   m_swap_chain,
                                   m_swap_chain_surface_format.format);
    if (m_swap_chain_image_views.size() < minimum_swap_chain_image_count)
        throw std::runtime_error("Failed to create enough swap chain image views!");

    std::cout << "Actual image count in swap chain: " 
              << m_swap_chain_image_views.size()
              << std::endl;
}

void GraphicsContext::create_swap_chain_framebuffers(const uint32_t width,
                                                     const uint32_t height)
{
     /* ===================================================================
     * Create Swap Chain Framebuffers
     */
    const auto framebuffer_size = m_swap_chain_image_views.size();
    m_swap_chain_framebuffers.resize(framebuffer_size);
    for (size_t i = 0; i < framebuffer_size; i++) {
        VkImageView attachments[] = {m_swap_chain_image_views[i]};
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = width;
        framebuffer_info.height = height;
        framebuffer_info.layers = 1;
        auto status = vkCreateFramebuffer(m_logical_device,
                                          &framebuffer_info,
                                          nullptr,
                                          &m_swap_chain_framebuffers[i]);

        if (status != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer["
                                     + std::to_string(i) + "]!");
    }
    
    std::cout << "Created "
              <<  m_swap_chain_framebuffers.size()
              << " framebuffers" 
              << std::endl;
}

void GraphicsContext::recreate_swap_chain() 
{
    std::cout << "recreating swap chain" << std::endl;
    // Wait with recreation of swap chain if window is minimized
    // NOTE: This is not really testable on my setup..
    auto size = get_window_size();
    while (size.width == 0 || size.height == 0)
        size = get_window_size();

    vkDeviceWaitIdle(m_logical_device);
    destroy_swap_chain();
    const auto info = get_swap_chain_support_info(m_physical_device, 
                                                  m_window_surface);

    auto extent = get_window_size();
    create_swap_chain(extent.width, extent.height);
    create_swap_chain_framebuffers(extent.width, extent.height);
    m_swap_chain_framebuffer_resized = false;
}

GraphicsContext::~GraphicsContext()
{
    vkDeviceWaitIdle(m_logical_device);
    
    destroy_swap_chain();
    
    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        vkDestroyBuffer(m_logical_device, m_uniform_buffers[i], nullptr);
        vkFreeMemory(m_logical_device, m_uniform_buffers_memory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_logical_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_logical_device, m_descriptor_layout, nullptr);

    m_vertex_buffer.reset();
    m_index_buffer.reset();

    for (uint32_t i = 0; i < m_max_frames_in_flight; i++) {
        vkDestroySemaphore(m_logical_device, m_semaphores_image_available[i], nullptr);
        vkDestroySemaphore(m_logical_device, m_semaphores_rendering_finished[i], nullptr);
        vkDestroyFence(m_logical_device, m_fences_in_flight[i], nullptr);
    }
    vkDestroyCommandPool(m_logical_device, m_command_pool, nullptr);

    vkDestroyPipeline(m_logical_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_logical_device, m_graphics_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_logical_device, m_render_pass, nullptr);

    vkDestroySurfaceKHR(m_instance, m_window_surface, nullptr);
    vkDestroyDevice(m_logical_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    SDL_DestroyWindow(m_window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

}
