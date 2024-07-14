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
    
uint32_t find_memory_type(const VkPhysicalDeviceMemoryProperties mem_properties,
                          const uint32_t type_filter,
                          const VkMemoryPropertyFlags property_flags)
{
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i)
        && (mem_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
            return i;
    }
    throw std::runtime_error("failed to find suitable memory type!");
}
    
VkBufferCreateInfo create_vertex_buffer_info(const std::vector<Vertex>& vertices)
{
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = sizeof(vertices[0]) * vertices.size();
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return info;
}
    
VkBuffer create_vertex_buffer(VkBufferCreateInfo buffer_info,
                              const VkDevice& m_logical_device
)
{
    VkBuffer buffer;
    const auto status = vkCreateBuffer(m_logical_device, &buffer_info, nullptr, &buffer);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create vertex buffer!");
    return buffer;
}

[[nodiscard]]
VkPhysicalDeviceMemoryProperties
get_physical_device_memory_properties(const VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(device, &properties);
    return properties;
}

/*
const std::vector<arc::Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
*/
    


VertexBuffer::VertexBuffer(const VkDevice& logical_device)
    : m_logical_device(logical_device)
{

}

/*
VkBufferCreateInfo VertexBuffer::create_buffer(const VkPhysicalDevice& physical_device,
                                               const VkDevice& logical_device,
                                               const std::vector<Vertex>& vertices
                                               )
{
    const auto buffer_info = create_vertex_buffer_info(vertices);
    m_vertice_count = vertices.size();
    m_vertex_buffer = create_vertex_buffer(buffer_info, logical_device);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(logical_device,
                                  m_vertex_buffer,
                                  &mem_requirements);
    
    const auto physical_memory_properties =
        get_physical_device_memory_properties(physical_device);
    
    VkMemoryAllocateInfo memalloc_info{};
    memalloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memalloc_info.allocationSize = mem_requirements.size;
    memalloc_info.memoryTypeIndex = find_memory_type(physical_memory_properties, 
                                                     mem_requirements.memoryTypeBits,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                                                     | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto status = vkAllocateMemory(logical_device,
                                   &memalloc_info,
                                   nullptr,
                                   &m_vertex_buffer_memory);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory! now you have a leak because we did not clean up anyhting..");
    
    vkBindBufferMemory(logical_device,
                       m_vertex_buffer,
                       m_vertex_buffer_memory, 0);
    
    return buffer_info;
}
*/


struct CreateBufferResult {
    VkBufferCreateInfo info{};
    VkBuffer buffer{};
    VkDeviceMemory memory{};
};
    
CreateBufferResult create_buffer(const VkPhysicalDevice& physical_device,
                                 const VkDevice& logical_device,
                                 const VkDeviceSize size,
                                 const VkBufferUsageFlags usage,
                                 const VkMemoryPropertyFlags properties)
{
    CreateBufferResult result{};
    result.info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    result.info.size = size;
    result.info.usage = usage;
    result.info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    auto status = vkCreateBuffer(logical_device, &result.info, nullptr, &result.buffer);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create buffer!");

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(logical_device,
                                  result.buffer,
                                  &mem_requirements);
    
    const auto physical_memory_properties =
        get_physical_device_memory_properties(physical_device);
    
    VkMemoryAllocateInfo memalloc_info{};
    memalloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memalloc_info.allocationSize = mem_requirements.size;
    memalloc_info.memoryTypeIndex = find_memory_type(physical_memory_properties, 
                                                     mem_requirements.memoryTypeBits,
                                                     properties);
    status = vkAllocateMemory(logical_device,
                              &memalloc_info,
                              nullptr,
                              &result.memory);
    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!"
                                 " now you have a leak because we did not clean up anyhting..");
    
    vkBindBufferMemory(logical_device,
                       result.buffer,
                       result.memory, 0);
    return result;
}

std::unique_ptr<VertexBuffer>
VertexBuffer::create(const VkPhysicalDevice& physical_device,
                     const VkDevice& logical_device,
                     const std::vector<Vertex>& vertices
                     )
{
    auto vertexbuffer = std::make_unique<VertexBuffer>(logical_device);

    try {
        auto created_buffer = create_buffer(physical_device,
                                            logical_device,
                                            sizeof(vertices[0]) * vertices.size(),
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                                            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vertexbuffer->m_vertex_buffer = created_buffer.buffer;
        vertexbuffer->m_vertex_buffer_memory = created_buffer.memory;
        vertexbuffer->m_vertice_count = created_buffer.info.size;

        void* data_ptr;
        vkMapMemory(logical_device,
                    vertexbuffer->m_vertex_buffer_memory,
                    0,
                    created_buffer.info.size,
                    0,
                    &data_ptr);

        memcpy(data_ptr, vertices.data(), static_cast<size_t>(created_buffer.info.size));
        vkUnmapMemory(logical_device, vertexbuffer->m_vertex_buffer_memory);
    } catch (const std::runtime_error) {
        return nullptr;
    }
    return vertexbuffer;
}
    
std::unique_ptr<VertexBuffer>
VertexBuffer::create_staging(const VkPhysicalDevice& physical_device,
                             const VkDevice& logical_device,
                             const std::vector<Vertex>& vertices)
{
    auto staging_buffer = create_buffer(physical_device,
                                        logical_device,
                                        sizeof(vertices[0]) * vertices.size(),
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                                        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data_ptr;
    vkMapMemory(logical_device,
                staging_buffer.memory,
                0,
                staging_buffer.info.size,
                0,
                &data_ptr);
    
    memcpy(data_ptr, vertices.data(), static_cast<size_t>(staging_buffer.info.size));
    vkUnmapMemory(logical_device, staging_buffer.memory);

    auto vertex_buffer = create_buffer(physical_device,
                                        logical_device,
                                        sizeof(vertices[0]) * vertices.size(),
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                                        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


    // TODO: CONTINUE HERE
    // https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
    // STORE THIS OTHER COOL VULKAN RESOURCE
    // https://marcelbraghetto.github.io/a-simple-triangle/2019/09/21/part-25/
    


    auto vertexbuffer = std::make_unique<VertexBuffer>(logical_device);
    vertexbuffer->m_vertex_buffer = created_buffer.buffer;
    vertexbuffer->m_vertex_buffer_memory = created_buffer.memory;
    vertexbuffer->m_vertice_count = created_buffer.info.size;
 
    return vertexbuffer;
}

VkBuffer VertexBuffer::get_buffer()
{
    return m_vertex_buffer;
}

size_t VertexBuffer::vertice_count()
{
    return m_vertice_count;
}

VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(m_logical_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_logical_device, m_vertex_buffer_memory, nullptr);
}
 
}
