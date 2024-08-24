#include "../arc/VertexBuffer.hpp"

namespace ArcGraphics {

#if 0
const uint32_t VertexBufferPolicy::buffer_type_bit = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
 
VkVertexInputBindingDescription Vertex::get_binding_description() {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    /*
    VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
    VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
     */
    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::get_attribute_descriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
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
    attribute_descriptions[0].offset = offsetof(Vertex, pos);

    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(Vertex, color);
    return attribute_descriptions;
}
#endif

}
