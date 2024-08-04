#include "../arc/Shader.hpp"

#include <fstream>
#include <iostream>

namespace ArcGraphics {
    
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

VkShaderModule compile_shader_bytecode(const VkDevice logical_device,
                                       const ShaderBytecode bytecode)
{
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = bytecode.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());
    VkShaderModule shader_module;
    const auto status = vkCreateShaderModule(logical_device,
                                             &create_info,
                                             nullptr,
                                             &shader_module);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create shader module!");
    return shader_module;
}

VkPipelineColorBlendStateCreateInfo create_color_blend_state_info(const bool use_alpha_blending)
{
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
                                          | VK_COLOR_COMPONENT_G_BIT 
                                          | VK_COLOR_COMPONENT_B_BIT 
                                          | VK_COLOR_COMPONENT_A_BIT;
    
    if (use_alpha_blending) {
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

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional
    return color_blending;
}
    
DrawableGeometry::DrawableGeometry()
: DrawableGeometry(glm::mat4(1), nullptr, nullptr)
{
}

DrawableGeometry::DrawableGeometry(glm::mat4 model,
                                   VertexBuffer* vertices,
                                   IndexBuffer* indices)
: model(model)
, vertices(vertices)
, indices(indices)
{
}
    
const VkCommandPool& RenderPipeline::command_pool() const
{
    return m_command_pool;
}

void RenderPipeline::record_command_buffer(const std::vector<DrawableGeometry> geometries,
                                           VkCommandBuffer command_buffer,
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
     const auto capabilities = get_rendering_capabilities(m_device->physical_device(),
                                                          m_renderer->window_surface()); 
    
    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass = m_render_pass;
    renderpass_info.framebuffer = m_swap_chain_framebuffers[image_index];
    renderpass_info.renderArea.offset = {0, 0};
    renderpass_info.renderArea.extent = m_render_extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderpass_info.clearValueCount = 1;
    renderpass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);   
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_render_extent.width);
    viewport.height = static_cast<float>(m_render_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_render_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    

    /* ===================================================================
     * Draw
     */
     // TODO: note we load data to uniform buffers using this UniformBufferObject thing..
    for (auto& geometry: geometries) {
        if (!geometry.vertices)
            throw std::runtime_error("vertex buffer ptr was nullptr!");
        if (!geometry.indices)
            throw std::runtime_error("index buffer ptr was nullptr!");

        //UniformBufferObject ubo{};
        //ubo.view = m_viewport.view;
        //ubo.proj = m_viewport.proj;
        //ubo.model = geometry.model;
        //m_uniformbuffers[m_current_frame]->set_uniform(&ubo);

        /* Bind Viewport
         */
        m_uniform_viewports[m_current_frame]->set_uniform(&viewport);

        // https://community.khronos.org/t/how-to-pass-two-uniform-bbuffers-in-shader/106753/2
        vkCmdBindDescriptorSets(command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_graphics_pipeline_layout,
                                0,
                                1,
                                &m_descriptor_sets[m_current_frame],
                                0,
                                nullptr);

        glm::mat4 model = geometry.model;
        m_uniform_models[m_current_frame]->set_uniform(&model);

        vkCmdBindDescriptorSets(command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_graphics_pipeline_layout,
                                1,
                                1,
                                &m_descriptor_sets[m_current_frame],
                                0,
                                nullptr);
        
        /* Bind Vertices and Indices
         */
        VkBuffer vertex_buffers[] = {geometry.vertices->get_buffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffer,
                             geometry.indices->get_buffer(),
                             0,
                             VK_INDEX_TYPE_UINT32);
        
        const uint32_t instanceCount = 1;
        const uint32_t firstIndex = 0;
        const int32_t  vertexOffset = 0;
        const uint32_t firstInstance = 0;
        vkCmdDrawIndexed(command_buffer,
                         geometry.indices->get_count(),
                         instanceCount,
                         firstIndex,
                         vertexOffset,
                         firstInstance);
    }
   
    vkCmdEndRenderPass(command_buffer);
    status = vkEndCommandBuffer(command_buffer);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer!");
}

VkExtent2D RenderPipeline::render_size() const
{
    return m_render_extent;
}

void RenderPipeline::start_frame(const ViewPort viewport)
{
    m_geometries.clear();
    m_viewport = viewport;
}

void RenderPipeline::add_geometry(const DrawableGeometry geometry)
{
    m_geometries.push_back(geometry);
}



std::optional<uint32_t> RenderPipeline::wait_for_next_frame()
{
    vkWaitForFences(m_device->logical_device(),
                    1,
                    &m_framelocks[m_current_frame].fence_in_flight,
                    VK_TRUE,
                    UINT64_MAX);
    
    if (m_swap_chain_framebuffer_resized) {
    //    recreate_swap_chain();
        return std::nullopt; //TODO call resizing
    }

    uint32_t image_index;
    auto status = vkAcquireNextImageKHR(m_device->logical_device(),
                                        m_renderer->swapchain(),
                                        UINT64_MAX,
                                        m_framelocks[m_current_frame].semaphore_image_available,
                                        VK_NULL_HANDLE,
                                        &image_index);

    if (status == VK_ERROR_OUT_OF_DATE_KHR || m_swap_chain_framebuffer_resized) {
        //recreate_swap_chain();
        return std::nullopt; //TODO call resizing
    } 
    else if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image!");
    
    // Reset fences now we could get next image to draw on
    vkResetFences(m_device->logical_device(),
                  1,
                  &m_framelocks[m_current_frame].fence_in_flight);

    vkResetCommandBuffer(m_commandbuffers[m_current_frame], 0);
    return image_index;
}

void RenderPipeline::draw_frame()
{
    auto image_index = wait_for_next_frame();
    if (!image_index)
        return;

    record_command_buffer(m_geometries,
                          m_commandbuffers[m_current_frame],
                          *image_index);
   
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkSemaphore> waitSemaphores =
        {m_framelocks[m_current_frame].semaphore_image_available};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submit_info.waitSemaphoreCount = waitSemaphores.size();
    submit_info.pWaitSemaphores = waitSemaphores.data();
    submit_info.pWaitDstStageMask = waitStages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandbuffers[m_current_frame];
    
    std::vector<VkSemaphore> signalSemaphores =
        {m_framelocks[m_current_frame].semaphore_rendering_finished};
    submit_info.signalSemaphoreCount = signalSemaphores.size();
    submit_info.pSignalSemaphores = signalSemaphores.data();

    auto status = vkQueueSubmit(m_renderer->graphics_queue(),
                           1,
                           &submit_info,
                           m_framelocks[m_current_frame].fence_in_flight); 
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer!");
    
    /* ===================================================================
     * Presentation
     */
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = signalSemaphores.size();
    present_info.pWaitSemaphores = signalSemaphores.data();
    std::vector<VkSwapchainKHR> swapChains = {m_renderer->swapchain()};
    present_info.swapchainCount = swapChains.size();
    present_info.pSwapchains = swapChains.data();
    present_info.pImageIndices = &*image_index;
    present_info.pResults = nullptr; // Optional

    status = vkQueuePresentKHR(m_renderer->graphics_queue(), &present_info);

    if (status == VK_ERROR_OUT_OF_DATE_KHR || m_swap_chain_framebuffer_resized) {
        //recreate_swap_chain();
        return;
    } 
    else if (status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image!");

    m_current_frame = (m_current_frame + 1) % m_framelocks.size();
}
  
RenderPipeline::RenderPipeline(Device* device,
    Renderer* renderer,
    const VkRenderPass render_pass,
    const VkDescriptorPool descriptor_pool,
    const VkDescriptorSetLayout descriptor_layout,
    const std::vector<VkDescriptorSet> descriptor_sets,
    const VkPipelineLayout graphics_pipeline_layout,
    const VkPipeline graphics_pipeline,
    const VkExtent2D render_extent,
    const std::vector<VkFramebuffer> framebuffers,
    const std::vector<RenderFrameLocks> framelocks,
    //const std::vector<std::shared_ptr<BasicUniformBuffer>> uniformbuffers,
    const std::vector<std::shared_ptr<BasicUniformBuffer>> uniform_viewport,
    const std::vector<std::shared_ptr<BasicUniformBuffer>> uniform_model,
    const std::vector<VkCommandBuffer> commandbuffers,
    const VkCommandPool command_pool)
    : m_device(device)
    , m_renderer(renderer)
    , m_render_pass(render_pass)
    , m_descriptor_pool(descriptor_pool)
    , m_descriptor_layout(descriptor_layout)
    , m_descriptor_sets(descriptor_sets)
    , m_graphics_pipeline_layout(graphics_pipeline_layout)
    , m_graphics_pipeline(graphics_pipeline)
    , m_render_extent(render_extent)
    , m_swap_chain_framebuffers(framebuffers)
    , m_framelocks(framelocks)
    , m_uniform_viewports(uniform_viewport)
    , m_uniform_models(uniform_model)
    , m_commandbuffers(commandbuffers)
    , m_command_pool(command_pool)
{
    if (!m_device)
        throw std::runtime_error("RenderPipeline() device was nullptr!");
    if (!m_renderer)
        throw std::runtime_error("RenderPipeline() renderer was nullptr!");
}
    
void RenderPipeline::destroy()
{
    const auto logical_device = m_device->logical_device();
    vkDeviceWaitIdle(logical_device);
    
    // TODO TJNS this is part of recreating swap chain...
    for (auto& framebuffer: m_swap_chain_framebuffers)
        vkDestroyFramebuffer(logical_device, framebuffer, nullptr);

    vkDestroyDescriptorPool(logical_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(logical_device, m_descriptor_layout, nullptr);
   
    for (size_t i = 0; i < m_framelocks.size(); i++) {
        vkDestroySemaphore(logical_device,
                           m_framelocks[i].semaphore_image_available,
                           nullptr);
        vkDestroySemaphore(logical_device,
                           m_framelocks[i].semaphore_rendering_finished,
                           nullptr);
        vkDestroyFence(logical_device, m_framelocks[i].fence_in_flight, nullptr);
    }

    vkDestroyCommandPool(logical_device, m_command_pool, nullptr);
    vkDestroyPipeline(logical_device,
                      m_graphics_pipeline,
                      nullptr);
    vkDestroyPipelineLayout(logical_device, m_graphics_pipeline_layout, nullptr);
    vkDestroyRenderPass(logical_device, m_render_pass, nullptr);
}
    
RenderPipeline::Builder::Builder(Device* device,
                                 Renderer* renderer,
                                 const ShaderBytecode vertex_bytecode,
                                 const ShaderBytecode fragment_bytecode)
    : m_device(device)
    , m_renderer(renderer)
    , m_vertex_bytecode(vertex_bytecode)
    , m_fragment_bytecode(fragment_bytecode)
{
    if (!m_device)
        throw std::runtime_error("RenderPipeline::Builder() device was nullptr!");
    if (!m_renderer)
        throw std::runtime_error("RenderPipeline::Builder() renderer was nullptr!");
}

RenderPipeline::Builder& RenderPipeline::Builder::with_frames_in_flight(const uint32_t frames)
{
    m_max_frames_in_flight = frames;
    return *this;
}
    
RenderPipeline::Builder& RenderPipeline::Builder::with_render_size(const uint32_t width,
                                                                   const uint32_t height)
{
    m_render_width = width;
    m_render_height = height;
    return *this;
}

RenderPipeline::Builder& RenderPipeline::Builder::with_use_alpha_blending(const bool use)
{
    m_use_alpha_blending = use;
    return *this;
}

RenderPipeline RenderPipeline::Builder::produce()
{
    std::cout << "==================================================\n"
              << " Producing RenderPipeline\n"
              << "=================================================="
              << std::endl;

    const auto vert_module = compile_shader_bytecode(m_device->logical_device(),
                                                     m_vertex_bytecode);

    const auto frag_module = compile_shader_bytecode(m_device->logical_device(),
                                                     m_fragment_bytecode);

    VkPipelineShaderStageCreateInfo vertex_create_info{};
    vertex_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_create_info.module = vert_module;
    vertex_create_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragment_create_info{};
    fragment_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_create_info.module = frag_module;
    fragment_create_info.pName = "main";   
    
    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_create_info,
    fragment_create_info};

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    // TODO: This is not correct, it should be able to bind different descriptions right?
    // This is a per-shader thing, does this mean we could be able to have multiple
    // shaders per pipeline?
    const auto binding_description = Vertex::get_binding_description();
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    const auto attribute_descriptions = Vertex::get_attribute_descriptions();
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
    
    
    VkExtent2D render_extent;
    if (m_render_width == 0 || m_render_height == 0)
        render_extent = m_renderer->window_size();
    else
        render_extent = {m_render_width, m_render_height};

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = render_extent;
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)render_extent.width;
    viewport.height = (float)render_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
   
    /* ===================================================================
     * Create Pipeline Layout
     */
    // Here we add our uniform buffer layouts
    const auto descriptor_layout = 
        create_uniform_vertex_descriptorset_layout(m_device->logical_device(), 0, 1);
    
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    
    VkPipelineLayout graphics_pipeline_layout;
    auto status = vkCreatePipelineLayout(m_device->logical_device(),
                                    &pipeline_layout_info,
                                    nullptr,
                                    &graphics_pipeline_layout);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
    
    /* ===================================================================
     * Create Descriptor Pool and Sets
     */
    auto descriptor_pool = 
        create_uniform_descriptor_pool(m_device->logical_device(),
                                       m_max_frames_in_flight);

    std::vector<VkDescriptorSetLayout> layouts(m_max_frames_in_flight,
                                               descriptor_layout);
    
    VkDescriptorSetAllocateInfo descriptor_pool_alloc_info{};
    descriptor_pool_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_pool_alloc_info.descriptorPool = descriptor_pool;
    descriptor_pool_alloc_info.descriptorSetCount = m_max_frames_in_flight;
    descriptor_pool_alloc_info.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptor_sets(m_max_frames_in_flight);
    status = vkAllocateDescriptorSets(m_device->logical_device(),
                                      &descriptor_pool_alloc_info,
                                      descriptor_sets.data());
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets!");

    std::cout << "Allocated descriptor sets!" << std::endl;
    
    
    /* ===================================================================
     * Create Uniform Buffers
     */
    //std::vector<std::shared_ptr<BasicUniformBuffer>> uniformbuffer;
    std::vector<std::shared_ptr<BasicUniformBuffer>> uniform_viewports;
    std::vector<std::shared_ptr<BasicUniformBuffer>> uniform_models;

    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        //auto uniform = BasicUniformBuffer::create(m_device->physical_device(),
        //                                           m_device->logical_device(),
        //                                           sizeof(UniformBufferObject));
        //uniformbuffer.push_back(std::shared_ptr<BasicUniformBuffer>(uniform.release()));

        auto uniform = BasicUniformBuffer::create(m_device->physical_device(),
                                                   m_device->logical_device(),
                                                   sizeof(ViewPort));
        uniform_viewports.push_back(std::shared_ptr<BasicUniformBuffer>(uniform.release()));
    }

    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        auto uniform = BasicUniformBuffer::create(m_device->physical_device(),
                                                m_device->logical_device(),
                                                sizeof(glm::mat4));
        uniform_models.push_back(std::shared_ptr<BasicUniformBuffer>(uniform.release()));
    }
    
    std::cout << "created uniform buffers" << std::endl;
    
    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        // Setup Descriptor buffer for ViewPort
        VkDescriptorBufferInfo buffer_info =
            uniform_viewports[i]->descriptor_buffer_info();
        
        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr; // Optional
        descriptor_write.pTexelBufferView = nullptr; // Optional

        uint32_t count = 1;
        vkUpdateDescriptorSets(m_device->logical_device(),
                               count,
                               &descriptor_write,
                               0,
                               nullptr);

        // Setup Descriptor buffer for Model
        buffer_info = uniform_models[i]->descriptor_buffer_info();
        
        descriptor_write = VkWriteDescriptorSet{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr; // Optional
        descriptor_write.pTexelBufferView = nullptr; // Optional

        count = 1;
        vkUpdateDescriptorSets(m_device->logical_device(),
                               count,
                               &descriptor_write,
                               0,
                               nullptr);

    }

    std::cout << "Allocated uniform buffers!" << std::endl;

    /* ===================================================================
     * Create Render Passes
     */
    const auto surface_format = m_renderer->surface_format().format;

    VkAttachmentDescription color_attachment{};
    color_attachment.format = surface_format;
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
    
    VkRenderPass render_pass;
    status = vkCreateRenderPass(m_device->logical_device(),
                                &render_pass_info,
                                nullptr,
                                &render_pass);
    
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass!");

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
    
    const auto color_blending_attachment_state =
        create_color_blend_attachement_state(m_use_alpha_blending);
    const auto color_blending =
        create_color_blend_state_info(color_blending_attachment_state);
    
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
    pipeline_info.layout = graphics_pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1; // Optional
    
    VkPipeline graphics_pipeline;
    status = vkCreateGraphicsPipelines(m_device->logical_device(),
                                       VK_NULL_HANDLE,
                                       1,
                                       &pipeline_info,
                                       nullptr,
                                       &graphics_pipeline);
    
    if (status  != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline!");
    

   /* ===================================================================
    * Cleanup shader modules now they are part of the graphics pipeline
    */
    vkDestroyShaderModule(m_device->logical_device(), vert_module, nullptr);
    vkDestroyShaderModule(m_device->logical_device(), frag_module, nullptr);

   /* ===================================================================
    * Create Swap Chain Framebuffers
    */
    const auto framebuffer_count = m_renderer->swapchain_image_view_count();
    std::vector<VkFramebuffer> swapchain_framebuffers(framebuffer_count);

    for (size_t i = 0; i < framebuffer_count; i++) {
        VkImageView attachments[] = {m_renderer->swapchain_image_view(i)};
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = render_extent.width;
        framebuffer_info.height = render_extent.height;
        framebuffer_info.layers = 1;
        auto status = vkCreateFramebuffer(m_device->logical_device(),
                                          &framebuffer_info,
                                          nullptr,
                                          &swapchain_framebuffers[i]);

        if (status != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer["
                                     + std::to_string(i) + "]!");
    }
    
    std::cout << "Created "
              <<  swapchain_framebuffers.size()
              << " framebuffers" 
              << std::endl;
    /* ===================================================================
     * Create Command Pool
     */
    const auto queue_families = get_queue_families(m_device->physical_device());
    auto queue_families_indices =
        find_graphics_present_indices(queue_families,
                                    m_device->physical_device(),
                                    m_renderer->window_surface());

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_families_indices.graphics.value();


    VkCommandPool command_pool;
    status = vkCreateCommandPool(m_device->logical_device(),
                                 &pool_info,
                                 nullptr,
                                 &command_pool);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");

 
   
    /* ===================================================================
     * Create Command Buffers
     */
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    /*
    VK_COMMAND_BUFFER_LEVEL_PRIMARY:
    Can be submitted to a queue for execution, but cannot be called from other command buffers.
    VK_COMMAND_BUFFER_LEVEL_SECONDARY:
    Cannot be submitted directly, but can be called from primary command buffers.
    */
    std::vector<VkCommandBuffer> commandbuffers(m_max_frames_in_flight);
    alloc_info.commandBufferCount = static_cast<uint32_t>(commandbuffers.size());
    status = vkAllocateCommandBuffers(m_device->logical_device(),
                                      &alloc_info,
                                      commandbuffers.data());

    if (status  != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");
    std::cout << "allocated command buffers" << std::endl;
    
    /* ===================================================================
     * Create Sync Objects
     */

    std::vector<RenderFrameLocks> framelocks(m_max_frames_in_flight);
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < framelocks.size(); i++) {
        status = vkCreateSemaphore(m_device->logical_device(),
                                   &semaphore_info,
                                   nullptr,
                                   &framelocks[i].semaphore_image_available);
        if (status != VK_SUCCESS)
            throw std::runtime_error("failed to create image available semaphore["
                                     + std::to_string(i) + "]");
        
        status = vkCreateSemaphore(m_device->logical_device(),
                                   &semaphore_info,
                                   nullptr,
                                   &framelocks[i].semaphore_rendering_finished);
        if (status != VK_SUCCESS)
            throw std::runtime_error("failed to create rendering finished semaphore["
                                     + std::to_string(i) + "]");

        status = vkCreateFence(m_device->logical_device(),
                            &fence_info,
                            nullptr,
                            &framelocks[i].fence_in_flight);
        if (status != VK_SUCCESS)
            throw std::runtime_error("failed to create fence in flight ["
                                     + std::to_string(i) + "]");
    }
    
    std::cout << "Created synchronization objects" << std::endl;

    return RenderPipeline(m_device,
                          m_renderer,
                          render_pass,
                          descriptor_pool,
                          descriptor_layout,
                          descriptor_sets,
                          graphics_pipeline_layout,
                          graphics_pipeline,
                          render_extent,
                          swapchain_framebuffers,
                          framelocks,
                          //uniformbuffers,
                          uniform_viewports,
                          uniform_models,
                          commandbuffers,
                          command_pool
                          );
}

}
