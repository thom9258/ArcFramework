#include "Context.hpp"
#include "DeclareNotCopyable.hpp"

#include <vector>
#include <string>


namespace arc {

using ShaderBytecode = std::vector<char>;

[[nodiscard]]
ShaderBytecode read_shader_bytecode(const std::string& filename);


class ShaderModule : public DeclareNotCopyable
{
public:
    ShaderModule(const ShaderBytecode& code, const VkDevice& device);
    ~ShaderModule();
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
    
class GraphicsPipeline : public DeclareNotCopyable
{
public:
    GraphicsPipeline(const VertexShaderModule& vertex,
                     const FragmentShaderModule& fragment,
                     const VkDevice& logical_device,
                     const VkExtent2D swap_chain_extent,
                     const VkFormat swap_chain_surface_format
                     );

    ~GraphicsPipeline();
private:
    const VkDevice& m_logical_device;
    VkPipelineLayout m_pipeline_layout;
    VkRenderPass m_render_pass;
};
   
}
