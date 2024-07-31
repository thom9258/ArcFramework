#pragma once

#include "Renderer.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "UniformBuffer.hpp"
#include "Algorithm.hpp"


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

    RenderPipeline(Device* device,    
                   Renderer* renderer,
                   const VkRenderPass render_pass,
                   const VkDescriptorPool descriptor_pool,
                   const VkDescriptorSetLayout descriptor_layout,
                   const std::vector<VkDescriptorSet> descriptor_sets,
                   const VkPipelineLayout graphics_pipeline_layout,
                   const VkPipeline graphics_pipeline,
                   const VkExtent2D render_extent,
                   const std::vector<VkFramebuffer> framebuffers,
                   const std::vector<RenderFrame> renderframes,
                   const std::vector<VkCommandBuffer> commandbuffers,
                   const VkCommandPool command_pool
                   );

    void draw_frame();

    // TODO: this ugly thing needs to be moved
    void record_command_buffer(VkCommandBuffer command_buffer,
                               uint32_t image_index);

    ~RenderPipeline() = default;
    void destroy();
    
    void add_geometry(VertexBuffer* vertex_buffer,
                      IndexBuffer* index_buffer);
    
    const VkCommandPool& command_pool() const;

private:
    // TODO: TEMPORARY BUFFERS TO TEST DRAWING WORKS
    VertexBuffer* m_vertex_buffer;
    IndexBuffer* m_index_buffer;

    Device* m_device{nullptr};
    Renderer* m_renderer{nullptr};
    VkRenderPass m_render_pass;
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_layout;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    VkExtent2D m_render_extent;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    std::vector<RenderFrame> m_renderframes;
    std::vector<VkCommandBuffer> m_commandbuffers;
    uint32_t m_current_frame{0};

    bool m_swap_chain_framebuffer_resized{false};
    VkCommandPool m_command_pool;
};
    
class RenderPipeline::Builder : protected IsNotLvalueCopyable
{
public:
    Builder(Device* device,
            Renderer* renderer,
            const ShaderBytecode vertex_bytecode,
            const ShaderBytecode fragment_bytecode);

    ~Builder() = default;
    
    Builder& with_frames_in_flight(const uint32_t frames);
    Builder& with_use_alpha_blending(const bool use);

    [[nodiscard]]
    RenderPipeline produce();
    
private:
    // TODO: this is not possible at the moment
    Builder& with_render_size(const uint32_t width,
                              const uint32_t height);

    uint32_t m_max_frames_in_flight{2};

    Device* m_device{nullptr};
    Renderer* m_renderer{nullptr};
    ShaderBytecode m_vertex_bytecode;
    ShaderBytecode m_fragment_bytecode;
    uint32_t m_render_width{0};
    uint32_t m_render_height{0};
    bool m_use_alpha_blending{false};
};
   
}
