#include "../arc/SimpleGeometry.hpp"

namespace ArcGraphics {
    
VkVertexInputBindingDescription Vertex_PosTex::get_binding_description() {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex_PosTex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    /*
    VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
    VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
     */
    return binding_description;
}

std::vector<VkVertexInputAttributeDescription> Vertex_PosTex::get_attribute_descriptions() {
    /*
    float:  VK_FORMAT_R32_SFLOAT
    vec2:   VK_FORMAT_R32G32_SFLOAT
    vec3:   VK_FORMAT_R32G32B32_SFLOAT
    vec4:   VK_FORMAT_R32G32B32A32_SFLOAT
    ivec2:  VK_FORMAT_R32G32_SINT
            a 2-component vector of 32-bit signed integers
    uvec4:  VK_FORMAT_R32G32B32A32_UINT
            a 4-component vector of 32-bit unsigned integers
    double: VK_FORMAT_R64_SFLOAT
            a double-precision (64-bit) float
     */
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);

    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].offset = offsetof(Vertex_PosTex, pos);
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].offset = offsetof(Vertex_PosTex, uv);
    attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;

    return attribute_descriptions;
}
    
const uint32_t VertexBufferPolicy_PosTex::buffer_type_bit = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    

std::pair<VertexBuffer_PosTex::vector_type, ArcGraphics::IndexBuffer::vector_type> 
create_unit_plane()
{
    const VertexBuffer_PosTex::vector_type vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f},  {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f},   {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f},  {1.0f, 1.0f}}
    };
    const ArcGraphics::IndexBuffer::vector_type indices = {
        0, 1, 2, 2, 3, 0
    };
    return {vertices, indices};
}
    

std::pair<VertexBuffer_PosTex::vector_type, ArcGraphics::IndexBuffer::vector_type> 
create_unit_cube()
{
    const VertexBuffer_PosTex::vector_type vertices = {
        //front
        {{ 0.5f,  0.5f, 0.5f}, {1.0f, 1.0f}},  // top right
        {{ 0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},  // bottom right
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},  // bottom left
        {{-0.5f,  0.5f, 0.5f}, {0.0f, 1.0f}},  // top left 
        //back
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}}, // top right
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}}, // bottom right
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, // bottom left
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}}  // top left 
    }; 

    const ArcGraphics::IndexBuffer::vector_type indices = {
        0, 1, 5,    5, 1, 6,
        1, 2, 6,    6, 2, 7,
        2, 3, 7,    7, 3, 8,
        3, 4, 8,    8, 4, 9,
        10, 11, 0,  0, 11, 1,
        5, 6, 12,   12, 6, 13
     };

    return {vertices, indices};
}

}
