#include "../arc/Renderer.hpp"
#include "../arc/UniformBuffer.hpp"

namespace ArcGraphics {
    

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

Renderer::Builder::Builder(const Device* device)
{
    reset_builder();
    m_device = device;
}
    
void Renderer::Builder::reset_builder()
{
    m_window_name = "Unknown Window";
    m_window_width = 800;
    m_window_height = 600;
    m_window_flags = 0;
    m_device = nullptr;
}

Renderer::Builder& Renderer::Builder::with_wanted_window_size(const uint32_t width,
                                                              const uint32_t height)
{
    m_window_width = width;
    m_window_height = height;
    return *this;
}

Renderer::Builder& Renderer::Builder::with_window_name(const std::string& name)
{
    m_window_name = name;
    return *this;
}
    
Renderer::Builder& Renderer::Builder::with_window_flags(const uint32_t flags)
{
    m_window_flags |= flags;
    return *this;
}

Renderer::Builder::~Builder()
{
    reset_builder();
}

Renderer Renderer::Builder::produce()
{
    
    VkExtent2D decired_window_size;
    decired_window_size.width = m_window_width;
    decired_window_size.height = m_window_height;

    const auto window_size =
        scale_window_size_to_capabilities(decired_window_size,
                                          m_device->capabilities().surface_capabilities);

    auto window = SDL_CreateWindow(m_window_name.c_str(),
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   window_size.width,
                                   window_size.height,
                                   m_window_flags | SDL_WINDOW_VULKAN);
   
    VkSurfaceKHR window_surface;
    SDL_Vulkan_CreateSurface(window,
                             m_device->instance(), 
                             &window_surface);
    
    const auto swap_chain = create_swap_chain(m_device->physical_device(),
                                              m_device->logical_device(),
                                              window_surface,
                                              m_window_width,
                                              m_window_height);
    
    /* ===================================================================
     * Create Pipeline Viewport
     */
#if 0
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = window_size;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)window_size.width;
    viewport.height = (float)window_size.height;
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
    if (status != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
 
    
    /* ===================================================================
     * Create Render Passes
     */

    VkAttachmentDescription color_attachment{};
    color_attachment.format = swap_chain.surface_format.format;
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
    
    VkPipeline graphics_pipeline;
    status = vkCreateGraphicsPipelines(m_logical_device,
                                       VK_NULL_HANDLE,
                                       1,
                                       &pipeline_info,
                                       nullptr,
                                       &graphics_pipeline);

    if (status  != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline!");
#endif 








    reset_builder();
    return Renderer();
}
   

}
