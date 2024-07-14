#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include "VertexBuffer.hpp"

#ifdef _WIN32
    #pragma comment(linker, "/subsystem:windows")
    #define VK_USE_PLATFORM_WIN32_KHR
    #define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#ifndef UNREFERENCED
#define UNREFERENCED(param) void(param)
#endif

#include <memory>
#include <atomic>
#include <vector>
#include <array>
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

    [[nodiscard]]
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
public: using ValidationLayers = std::vector<const char*>;
    using DeviceExtensions = std::vector<const char*>;

    ~GraphicsContext();
    GraphicsContext(const uint32_t width,
                    const uint32_t height,
                    // TODO: This is disgusting,
                    // Split up shit into multiple stages
                    // Here we need to accept modules instead of bytecode for type safety
                    const ShaderBytecode& vertex_bytecode,
                    const ShaderBytecode& fragment_bytecode
                    );

    [[nodiscard]]
    VkExtent2D get_window_size();
    [[nodiscard]]
    VkExtent2D scale_window_size_to_be_suitable(VkExtent2D decired);
    void window_resized_event();
    void recreate_swap_chain();
    void create_swap_chain(const uint32_t width, const uint32_t height);
    void create_swap_chain_framebuffers(const uint32_t width, const uint32_t height);
    void destroy_swap_chain();
    void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void draw_frame();

    VkBuffer create_vertex_buffer(const std::vector<Vertex>& vertices);


private:
    SDL_Window* m_window{nullptr};
    VkSurfaceKHR m_window_surface;
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    
    // TODO: This could maybe be split into a GraphicsPipeline?
    VkDevice m_logical_device{VK_NULL_HANDLE};
    VkQueue m_graphics_queue;
    VkSwapchainKHR m_swap_chain;
    VkSurfaceFormatKHR m_swap_chain_surface_format;
    std::vector<VkImageView> m_swap_chain_image_views;
    VkRenderPass m_render_pass;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    bool m_swap_chain_framebuffer_resized{false};
    VkCommandPool m_command_pool;

    std::unique_ptr<VertexBuffer> m_vertex_buffer;
    
    // TODO: This could maybe be split into a frame handler?
    const uint32_t m_max_frames_in_flight{2};
    std::vector<VkCommandBuffer> m_command_buffers;
    std::vector<VkSemaphore> m_semaphores_image_available;
    std::vector<VkSemaphore> m_semaphores_rendering_finished;
    std::vector<VkFence> m_fences_in_flight;
    uint32_t m_flight_frame = 0;
};


}
