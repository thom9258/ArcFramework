#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
//#include <SDL2/SDL_events.h>
//#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan.h>

#ifdef _WIN32
    #pragma comment(linker, "/subsystem:windows")
    #define VK_USE_PLATFORM_WIN32_KHR
    #define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#include <memory>
#include <atomic>
#include <vector>
#include <string>

#include "DeclareNotCopyable.hpp"

namespace arc {

using ShaderBytecode = std::vector<char>;

[[nodiscard]]
ShaderBytecode read_shader_bytecode(const std::string& filename);


class ShaderModule : public DeclareNotCopyable
{
public:
    ShaderModule(const ShaderBytecode& code, const VkDevice& device);
    ~ShaderModule();
    const VkShaderModule& get_module() const noexcept;
private:
    VkShaderModule m_shader_module;
    const VkDevice& m_logical_device;
};
    
class VertexShaderModule : public ShaderModule
{
    using ShaderModule::ShaderModule;
};

class FragmentShaderModule : public ShaderModule
{
    using ShaderModule::ShaderModule;
};
    
class GraphicsContext : public DeclareNotCopyable
{
public:
    using ValidationLayers = std::vector<const char*>;
    using DeviceExtensions = std::vector<const char*>;

    GraphicsContext() = default;
    ~GraphicsContext();

    [[nodiscard]]
    static std::unique_ptr<GraphicsContext> create(const uint32_t width,
                                                   const uint32_t height,
                                                   const ValidationLayers validation_layers,
                                                   const DeviceExtensions device_extensions,
                                                   // TODO: This is disgusting,
                                                   // Split up shit into multiple stages
                                                   const ShaderBytecode& vertex_bytecode,
                                                   const ShaderBytecode& fragment_bytecode
                                                   );

    void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void draw_frame();

private:
    SDL_Window* m_window{nullptr};
    VkSurfaceKHR m_window_surface;
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    
    // TODO: is this a good split for when creating logical device?
    VkDevice m_logical_device{VK_NULL_HANDLE};
    VkQueue m_graphics_queue;
    VkSwapchainKHR m_swap_chain;
    VkExtent2D m_swap_chain_extent;
    std::vector<VkImageView> m_swap_chain_image_views;
    VkRenderPass m_render_pass;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    VkCommandPool m_command_pool;
    VkCommandBuffer m_command_buffer;
    
    VkSemaphore m_semaphore_image_available;
    VkSemaphore m_semaphore_rendering_finished;
    VkFence m_fence_in_flight;


};


}
