#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

#include <chrono>

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

namespace ArcGraphics {
    
//TODO: temporary object here    
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
    
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
public: 
    using ValidationLayers = std::vector<const char*>;
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
    void update_ubo_rotation(uint32_t flight_frame);

    VkBuffer create_vertex_buffer(const std::vector<Vertex>& vertices);


private:
    SDL_Window* m_window{nullptr};
    VkSurfaceKHR m_window_surface;
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    
    // TODO: This could maybe be split into a GraphicsPipeline?
    VkDevice m_logical_device{VK_NULL_HANDLE};
    
    // TODO: Could this be split up into a renderer?
    // this would allow us to destroy the entire renderer pipeline and recreate it
    VkQueue m_graphics_queue;
    VkSwapchainKHR m_swap_chain;
    VkSurfaceFormatKHR m_swap_chain_surface_format;
    std::vector<VkImageView> m_swap_chain_image_views;
    VkRenderPass m_render_pass;
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_layout;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    bool m_swap_chain_framebuffer_resized{false};
    VkCommandPool m_command_pool;

    // TODO: This is all stuff that should not be handled internally, but be provided..
    std::unique_ptr<VertexBuffer> m_vertex_buffer;
    std::unique_ptr<IndexBuffer> m_index_buffer;

    // TODO: This could maybe be split into a frame handler?
    const uint32_t m_max_frames_in_flight{2};
    std::vector<VkBuffer> m_uniform_buffers;
    std::vector<VkDeviceMemory> m_uniform_buffers_memory;
    std::vector<void*> m_uniform_buffers_mapped;
    std::vector<VkCommandBuffer> m_command_buffers;
    std::vector<VkSemaphore> m_semaphores_image_available;
    std::vector<VkSemaphore> m_semaphores_rendering_finished;
    std::vector<VkFence> m_fences_in_flight;
    uint32_t m_flight_frame = 0;
};


}
