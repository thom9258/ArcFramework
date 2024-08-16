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
    
/**
* @brief shader viewport and model matrix combined.
* @todo temporary object to provide viewport in shaders    
*/
struct ViewPort {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 model;
};

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
    
struct DrawableGeometry {
    DrawableGeometry();
    DrawableGeometry(glm::mat4 model,
                     VertexBuffer* vertices,
                     IndexBuffer* indices);
    glm::mat4 model;
    VertexBuffer* vertices;
    IndexBuffer* indices;
};

class RenderPipeline : public IsNotLvalueCopyable
{
public:
    class Builder;

    RenderPipeline(Device* device,    
                   Renderer* renderer,
                   const VkRenderPass render_pass,
                   //const VkDescriptorPool descriptor_pool,
                   //const VkDescriptorSetLayout descriptor_layout,
                   //const std::vector<VkDescriptorSet> descriptor_sets,
                   const VkPipelineLayout graphics_pipeline_layout,
                   const VkPipeline graphics_pipeline,
                   const VkExtent2D render_size,
                   const std::vector<VkFramebuffer> framebuffers,
                   const std::vector<RenderFrameLocks> framelocks,
                   const std::vector<VkCommandBuffer> commandbuffers,
                   const VkCommandPool command_pool
                   );
    
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
            const VkDescriptorSetLayout descriptorset_layout
            );

    ~Builder() = default;
    
    //TODO: Rasterizer specifically needs lots of control in the builder
    Builder& with_frames_in_flight(const uint32_t frames);
    Builder& with_use_alpha_blending(const bool use);
    //TODO This is considered disabled as i do not yet know how to scale up
    // the resulting image rendered in lower resolution to the window size.
    Builder& with_render_size(const uint32_t width,
                              const uint32_t height);
    [[nodiscard]]
    RenderPipeline produce();
    
private:
    Device* m_device{nullptr};
    Renderer* m_renderer{nullptr};
    ShaderBytecode m_vertex_bytecode;
    ShaderBytecode m_fragment_bytecode;
    VkDescriptorSetLayout m_descriptorset_layout;

    uint32_t m_max_frames_in_flight{2};
    std::optional<VkExtent2D> m_render_size = std::nullopt;
    bool m_use_alpha_blending{false};
};
   
}
