#pragma once

#include "TypeTraits.hpp"
#include "Device.hpp"

#include <vector>
#include <string>


namespace ArcGraphics {

using ShaderBytecode = std::vector<char>;

[[nodiscard]]
ShaderBytecode read_shader_bytecode(const std::string& filename);

[[nodiscard]]
VkShaderModule compile_shader_bytecode(const VkDevice logical_device,
                                       const ShaderBytecode bytecode);
    
class ShaderPipeline : public IsNotLvalueCopyable
{
public:
    ShaderPipeline(const VkShaderModule vertex,
                   const VkShaderModule fragment
                   //const VkDevice logical_device
                   );

    ~ShaderPipeline();
private:
    //const VkDevice& m_logical_device;
    VkPipelineLayout m_pipeline_layout;
    VkRenderPass m_render_pass;
};
   
}
