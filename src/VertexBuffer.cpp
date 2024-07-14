#include "../arc/VertexBuffer.hpp"

#include <string.h>

namespace arc {
 
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
   
VertexBuffer::VertexBuffer(const VkDevice& logical_device)
    : m_logical_device(logical_device)
{

}

std::unique_ptr<VertexBuffer>
VertexBuffer::create(const VkPhysicalDevice& physical_device,
                     const VkDevice& logical_device,
                     const std::vector<Vertex>& vertices
                     )
{
    auto vertexbuffer = std::make_unique<VertexBuffer>(logical_device);
    vertexbuffer->m_count = vertices.size();
    create_buffer(physical_device,
                  logical_device,
                  sizeof(vertices[0]) * vertices.size(),
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  vertexbuffer->m_vertice_info,
                  vertexbuffer->m_vertex_buffer,
                  vertexbuffer->m_vertex_buffer_memory);

    void* data_ptr;
    vkMapMemory(logical_device,
                vertexbuffer->m_vertex_buffer_memory,
                0,
                vertexbuffer->get_memsize(),
                0,
                &data_ptr);

    memcpy(data_ptr, vertices.data(), static_cast<size_t>(vertexbuffer->get_memsize()));
    vkUnmapMemory(logical_device, vertexbuffer->m_vertex_buffer_memory);
    return vertexbuffer;
}
    
std::unique_ptr<VertexBuffer>
VertexBuffer::create_staging(const VkPhysicalDevice& physical_device,
                             const VkDevice& logical_device,
                             const VkCommandPool& command_pool,
                             const VkQueue& graphics_queue,
                             const std::vector<Vertex>& vertices)
{
    const auto size = sizeof(vertices[0]) * vertices.size();

    auto staging_buffer = std::make_unique<VertexBuffer>(logical_device);
    staging_buffer->m_count = vertices.size();
    create_buffer(physical_device,
                  logical_device,
                  size,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  staging_buffer->m_vertice_info,
                  staging_buffer->m_vertex_buffer,
                  staging_buffer->m_vertex_buffer_memory);

    void* data_ptr;
    vkMapMemory(logical_device,
                staging_buffer->m_vertex_buffer_memory,
                0,
                staging_buffer->get_memsize(),
                0,
                &data_ptr);
    
    memcpy(data_ptr, vertices.data(), staging_buffer->get_memsize());
    vkUnmapMemory(logical_device, staging_buffer->m_vertex_buffer_memory);

    auto vertex_buffer = std::make_unique<VertexBuffer>(logical_device);
    vertex_buffer->m_count = vertices.size();
    create_buffer(physical_device,
                  logical_device,
                  size,
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
                  | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                  | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  vertex_buffer->m_vertice_info,
                  vertex_buffer->m_vertex_buffer,
                  vertex_buffer->m_vertex_buffer_memory);

    copy_buffer(logical_device,
                command_pool,
                graphics_queue,
                size,
                staging_buffer->m_vertex_buffer,
                vertex_buffer->m_vertex_buffer);
   
    return vertex_buffer;
}

VkBuffer VertexBuffer::get_buffer()
{
    return m_vertex_buffer;
}

size_t VertexBuffer::get_memsize()
{
    return m_vertice_info.size;
}

size_t VertexBuffer::get_count()
{
    return m_count;
}

VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(m_logical_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_logical_device, m_vertex_buffer_memory, nullptr);
}
 
}
