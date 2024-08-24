#pragma once

#include "Renderer.hpp"
#include "VertexBuffer.hpp"
#include "Texture.hpp"
#include "IndexBuffer.hpp"
#include "UniformBuffer.hpp"
#include "Algorithm.hpp"

#include <vector>
#include <string>


namespace ArcGraphics {
    
using ShaderBytecode = std::vector<char>;

/**
 * @brief read shader as binary bytecode.
 */
[[nodiscard]]
ShaderBytecode read_shader_bytecode(const std::string& filename);

/**
 * @brief Compile shader bytecode into a shader module.
 */
[[nodiscard]]
VkShaderModule compile_shader_bytecode(const VkDevice logical_device,
                                       const ShaderBytecode bytecode);
    
struct RenderFrameLocks {
    VkSemaphore semaphore_image_available;
    VkSemaphore semaphore_rendering_finished;
    VkFence fence_in_flight;
};
    
class RenderPipeline : public IsNotLvalueCopyable
{
public:
    class Builder;

    RenderPipeline(Device* device,    
                   Renderer* renderer,
                   const VkRenderPass render_pass,
                   const VkPipelineLayout graphics_pipeline_layout,
                   const VkPipeline graphics_pipeline,
                   const VkExtent2D render_size,
                   const VkClearValue clear_value,
                   const std::vector<VkFramebuffer> framebuffers,
                   const std::vector<RenderFrameLocks> framelocks,
                   const std::vector<VkCommandBuffer> commandbuffers,
                   const VkCommandPool command_pool);
    
    VkExtent2D render_size() const;
    uint32_t max_frames_in_flight() const;
    uint32_t current_flight_frame() const;
    const VkPipelineLayout& layout() const;

    std::optional<uint32_t> wait_for_next_frame();
  
    VkCommandBuffer begin_command_buffer(uint32_t image_index);

    void end_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
    
    ~RenderPipeline() = default;
    void destroy();
    
    const VkCommandPool& command_pool() const;

private:
    Device* m_device{nullptr};
    Renderer* m_renderer{nullptr};
    VkRenderPass m_render_pass;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    VkExtent2D m_render_size;
    VkClearValue m_clear_value;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    std::vector<RenderFrameLocks> m_framelocks;
    std::vector<VkCommandBuffer> m_commandbuffers;
    uint32_t m_current_flight_frame{0};

    bool m_swap_chain_framebuffer_resized{false};
    VkCommandPool m_command_pool;
};
    
class RenderPipeline::Builder : protected IsNotLvalueCopyable
{
public:
    Builder(Device* device,
            Renderer* renderer,
            const ShaderBytecode vertex_bytecode,
            const ShaderBytecode fragment_bytecode,
            const VkDescriptorSetLayout descriptorset_layout,
            const VkVertexInputBindingDescription vertex_binding_description,
            const std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions
            );

    ~Builder() = default;
    
    //TODO: Rasterizer specifically needs lots of control in the builder
    Builder& with_frames_in_flight(const uint32_t frames);
    Builder& with_use_alpha_blending(const bool use);
    //TODO This is considered disabled as i do not yet know how to scale up
    // the resulting image rendered in lower resolution to the window size.
    Builder& with_render_size(const uint32_t width,
                              const uint32_t height);

    Builder& with_clear_color(const float r, const float g, const float b);
    [[nodiscard]]
    RenderPipeline produce();
    
private:
    Device* m_device{nullptr};
    Renderer* m_renderer{nullptr};
    ShaderBytecode m_vertex_bytecode;
    ShaderBytecode m_fragment_bytecode;
    VkDescriptorSetLayout m_descriptorset_layout{};
    VkVertexInputBindingDescription m_vertex_binding_description{};
    std::vector<VkVertexInputAttributeDescription> m_vertex_attribute_descriptions{};

    uint32_t m_max_frames_in_flight{2};
    std::optional<VkExtent2D> m_render_size = std::nullopt;

    VkClearValue m_clear_value = {{{0.0f, 0.0f, 0.5f, 1.0f}}};
    bool m_use_alpha_blending{false};
};
   
}
