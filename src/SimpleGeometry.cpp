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
    const IndexBuffer::vector_type indices = {
        0, 1, 2, 2, 3, 0
    };
    return {vertices, indices};
}
    
std::pair<VertexBuffer_PosTex::vector_type, ArcGraphics::IndexBuffer::vector_type> 
create_unit_cube()
{
    const VertexBuffer_PosTex::vector_type vertices = {
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f}}, // A 0
        {{0.5f, -0.5f, -0.5f},  {1.0f, 0.0f}},  // B 1
        {{0.5f,  0.5f, -0.5f},  {1.0f, 1.0f }}, // C 2
        {{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f}}, // D 3
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f}}, // E 4
        {{0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},  // F 5
        {{0.5f,  0.5f,  0.5f},  {1.0f, 1.0f}},  // G 6
        {{-0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f}}, // H 7

        {{-0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f}}, // D 8
        {{-0.5f, -0.5f, -0.5f}, { 1.0f, 0.0f}}, // A 9
        {{-0.5f, -0.5f,  0.5f}, { 1.0f, 1.0f}}, // E 10
        {{-0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f}}, // H 11
        {{0.5f, -0.5f, -0.5f},  {0.0f, 0.0f}},  // B 12
        {{0.5f,  0.5f, -0.5f},  {1.0f, 0.0f}},  // C 13
        {{0.5f,  0.5f,  0.5f},  {1.0f, 1.0f}},  // G 14
        {{0.5f, -0.5f,  0.5f},  {0.0f, 1.0f}},  // F 15

        {{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f}}, // A 16
        {{0.5f, -0.5f, -0.5f},  {1.0f, 0.0f}},  // B 17
        {{0.5f, -0.5f,  0.5f},  {1.0f, 1.0f}},  // F 18
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, 1.0f}}, // E 19
        {{0.5f,  0.5f, -0.5f},  { 0.0f, 0.0f}}, // C 20
        {{-0.5f,  0.5f, -0.5f}, { 1.0f, 0.0f}}, // D 21
        {{-0.5f,  0.5f,  0.5f}, { 1.0f, 1.0f}}, // H 22
        {{0.5f,  0.5f,  0.5f},  { 0.0f, 1.0f}}  // G 23
    };

    const ArcGraphics::IndexBuffer::vector_type indices = {
        // front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,
        // left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        // bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };

    return {vertices, indices};
}
    

}
