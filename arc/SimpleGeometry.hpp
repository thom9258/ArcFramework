#pragma once

#include "IndexBuffer.hpp" 
#include "BasicBuffer.hpp" 

#include <tuple>

namespace ArcGraphics {
    
struct Vertex_PosTex {
    glm::vec3 pos{};
    glm::vec2 uv{};

    [[nodiscard]]
    static VkVertexInputBindingDescription get_binding_description();
    [[nodiscard]]
    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
};

struct VertexBufferPolicy_PosTex {
    using value_type = Vertex_PosTex;
    static const uint32_t buffer_type_bit;
};

using VertexBuffer_PosTex = ArcGraphics::BasicBuffer<VertexBufferPolicy_PosTex>;
   
[[nodiscard]]
std::pair<VertexBuffer_PosTex::vector_type, ArcGraphics::IndexBuffer::vector_type>
create_unit_plane();

[[nodiscard]]
std::pair<VertexBuffer_PosTex::vector_type, ArcGraphics::IndexBuffer::vector_type>
create_unit_cube();
    
}
