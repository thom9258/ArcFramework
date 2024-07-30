#pragma once

#include "Renderer.hpp"
#include "BasicBuffer.hpp"
#include "UniformBuffer.hpp"

#include <vector>
#include <string>


namespace ArcGraphics {
    
//TODO: temporary object to provide viewport in shaders    
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

using ShaderBytecode = std::vector<char>;

[[nodiscard]]
ShaderBytecode read_shader_bytecode(const std::string& filename);

[[nodiscard]]
VkShaderModule compile_shader_bytecode(const VkDevice logical_device,
                                       const ShaderBytecode bytecode);
    
 struct RenderFrame {
    VkBuffer uniform_buffer;
    VkDeviceMemory uniform_buffer_memory;
    void* uniform_buffer_mapped;
    //VkCommandBuffer command_buffer;
    VkSemaphore semaphore_image_available;
    VkSemaphore semaphore_rendering_finished;
    VkFence fence_in_flight;
};
    

class RenderPipeline : public IsNotLvalueCopyable
{
public:
    class Builder;

    RenderPipeline(std::shared_ptr<Device> device,    
                   std::shared_ptr<Renderer> renderer,
                   const VkRenderPass render_pass,
                   const VkDescriptorPool descriptor_pool,
                   const VkDescriptorSetLayout descriptor_layout,
                   const std::vector<VkDescriptorSet> descriptor_sets,
                   const VkPipelineLayout graphics_pipeline_layout,
                   const VkPipeline graphics_pipeline,
                   const std::vector<VkFramebuffer> framebuffers,
                   const std::vector<RenderFrame> renderframes,
                   const std::vector<VkCommandBuffer> commandbuffers
                   );

    ~RenderPipeline();

private:
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Renderer> m_renderer;
    uint32_t m_max_frames_in_flight;
    VkRenderPass m_render_pass;
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_layout;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    std::vector<RenderFrame> m_renderframes;
    std::vector<VkCommandBuffer> m_commandbuffers;

    bool m_swap_chain_framebuffer_resized{false};
    VkCommandPool m_command_pool;
};
    
class RenderPipeline::Builder : protected IsNotLvalueCopyable
{
public:
    Builder(std::shared_ptr<Device> device,
            std::shared_ptr<Renderer> renderer,
            const ShaderBytecode vertex_bytecode,
            const ShaderBytecode fragment_bytecode);

    ~Builder();
    
    Builder& with_frames_in_flight(const uint32_t frames);
    Builder& with_use_alpha_blending(const bool use);
    Builder& with_render_size(const uint32_t width,
                              const uint32_t height);

    [[nodiscard]]
    RenderPipeline produce();
    
private:
    void reset_builder();
    
    uint32_t m_max_frames_in_flight;

    std::shared_ptr<Device> m_device;
    std::shared_ptr<Renderer> m_renderer;
    ShaderBytecode m_vertex_bytecode;
    ShaderBytecode m_fragment_bytecode;
    uint32_t m_render_width;
    uint32_t m_render_height;
    bool m_use_alpha_blending;
};
   
}
