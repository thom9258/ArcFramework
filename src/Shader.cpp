#include "../arc/Shader.hpp"

#include <fstream>

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
    
ShaderPipeline::ShaderPipeline(const VkShaderModule vertex,
                               const VkShaderModule fragment
                               //const VkDevice logical_device
                               )
{
    //VkPipelineShaderStageCreateInfo vertex_create_info{};
    //vertex_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //vertex_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    //vertex_create_info.module = vertex;
    //vertex_create_info.pName = "main";
    //
    //VkPipelineShaderStageCreateInfo fragment_create_info{};
    //fragment_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //fragment_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    //fragment_create_info.module = fragment;
    //fragment_create_info.pName = "main";   
    //
    //VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_create_info,
    //fragment_create_info};
    //
    //
    //VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    //vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //vertex_input_info.vertexBindingDescriptionCount = 0;
    //vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
    //vertex_input_info.vertexAttributeDescriptionCount = 0;
    //vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional
    //
    //VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    //input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    //input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //input_assembly.primitiveRestartEnable = VK_FALSE;
    //
    //
    //const std::vector<VkDynamicState> dynamic_states = {
    //VK_DYNAMIC_STATE_VIEWPORT,
    //VK_DYNAMIC_STATE_SCISSOR
//};
    //
    //VkPipelineDynamicStateCreateInfo dynamic_state{};
    //dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    //dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    //dynamic_state.pDynamicStates = dynamic_states.data();

    //VkRect2D scissor{};
    //scissor.offset = {0, 0};
    //scissor.extent = swap_chain_extent;
    //
    //VkViewport viewport{};
    //viewport.x = 0.0f;
    //viewport.y = 0.0f;
    //viewport.width = (float)swap_chain_extent.width;
    //viewport.height = (float)swap_chain_extent.height;
    //viewport.minDepth = 0.0f;
    //viewport.maxDepth = 1.0f;
    //
    //VkPipelineViewportStateCreateInfo viewport_state{};
    //viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    //viewport_state.viewportCount = 1;
    //viewport_state.pViewports = &viewport;
    //viewport_state.scissorCount = 1;
    //viewport_state.pScissors = &scissor;
    //
    //VkPipelineRasterizationStateCreateInfo rasterizer{};
    //rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //rasterizer.depthClampEnable = VK_FALSE;
    //rasterizer.rasterizerDiscardEnable = VK_FALSE;
    //rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //rasterizer.lineWidth = 1.0f;
    //rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //rasterizer.depthBiasEnable = VK_FALSE;
    //rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    //rasterizer.depthBiasClamp = 0.0f; // Optional
    //rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
    //
    ////constexpr bool use_alpha_blending = false;
    ////VkPipelineColorBlendStateCreateInfo color_blending =
    ////    create_color_blend_state_info(use_alpha_blending);
    //
    //VkPipelineLayoutCreateInfo pipeline_layout_info{};
    //pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipeline_layout_info.setLayoutCount = 0; // Optional
    //pipeline_layout_info.pSetLayouts = nullptr; // Optional
    ////pipeline_layout_info.pColorBlendState = &color_blending;
    //pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    //pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    //auto status = vkCreatePipelineLayout(m_logical_device,
    //                                     &pipeline_layout_info,
    //                                     nullptr,
    //                                     &m_pipeline_layout);
    //if (status != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create pipeline layout!");
    //}
    //
    //
    //VkAttachmentDescription color_attachment{};
    //color_attachment.format = swap_chain_surface_format;
    //color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    //color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //
    //VkAttachmentReference color_attachment_ref{};
    //color_attachment_ref.attachment = 0;
    //color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //
    //VkSubpassDescription subpass{};
    //subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //subpass.colorAttachmentCount = 1;
    //subpass.pColorAttachments = &color_attachment_ref;
    //
    //VkRenderPassCreateInfo render_pass_info{};
    //render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //render_pass_info.attachmentCount = 1;
    //render_pass_info.pAttachments = &color_attachment;
    //render_pass_info.subpassCount = 1;
    //render_pass_info.pSubpasses = &subpass;
    //
    //status = vkCreateRenderPass(logical_device,
    //                            &render_pass_info,
    //                            nullptr,
    //                            &m_render_pass);
    //
    //if (status != VK_SUCCESS)
    //    throw std::runtime_error("Failed to create render pass!");

}

ShaderPipeline::~ShaderPipeline()
{
    //vkDestroyPipelineLayout(m_logical_device, m_pipeline_layout, nullptr);
}

}
