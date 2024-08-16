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
                   const VkDescriptorPool descriptor_pool,
                   const VkDescriptorSetLayout descriptor_layout,
                   const std::vector<VkDescriptorSet> descriptor_sets,
                   const VkPipelineLayout graphics_pipeline_layout,
                   const VkPipeline graphics_pipeline,
                   const VkExtent2D render_size,
                   const std::vector<VkFramebuffer> framebuffers,
                   const std::vector<RenderFrameLocks> framelocks,
                   const std::vector<std::shared_ptr<BasicUniformBuffer>> uniform_viewport,
                   const std::vector<VkCommandBuffer> commandbuffers,
                   const VkCommandPool command_pool,
                   
                   Texture* TMP_texture
                   );
    
    VkExtent2D render_size() const;
    std::optional<uint32_t> wait_for_next_frame();

    void start_frame(const glm::mat4 view, const glm::mat4 projection);
    void add_geometry(const DrawableGeometry geometry);
    void draw_frame();

    // TODO: this ugly thing needs to be moved
    void record_command_buffer(const std::vector<DrawableGeometry> geometries,
                               VkCommandBuffer command_buffer,
                               uint32_t image_index);
    
    ~RenderPipeline() = default;
    void destroy();
    
    const VkCommandPool& command_pool() const;

private:
    glm::mat4 m_view;
    glm::mat4 m_projection;
    std::vector<DrawableGeometry> m_geometries;

    Device* m_device{nullptr};
    Renderer* m_renderer{nullptr};
    VkRenderPass m_render_pass;
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_layout;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    VkExtent2D m_render_size;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    std::vector<RenderFrameLocks> m_framelocks;
    std::vector<std::shared_ptr<BasicUniformBuffer>> m_uniform_viewports;
    std::vector<VkCommandBuffer> m_commandbuffers;
    uint32_t m_current_frame{0};

    bool m_swap_chain_framebuffer_resized{false};
    VkCommandPool m_command_pool;
    

    Texture* m_TMP_texture;
};
    
class RenderPipeline::Builder : protected IsNotLvalueCopyable
{
public:
    Builder(Device* device,
            Renderer* renderer,
            const ShaderBytecode vertex_bytecode,
            const ShaderBytecode fragment_bytecode);

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
    uint32_t m_max_frames_in_flight{2};

    Device* m_device{nullptr};
    Renderer* m_renderer{nullptr};
    ShaderBytecode m_vertex_bytecode;
    ShaderBytecode m_fragment_bytecode;
    std::optional<VkExtent2D> m_render_size = std::nullopt;
    bool m_use_alpha_blending{false};
};
   
}
