#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "DeclareNotCopyable.hpp"
#include "BasicBuffer.hpp"

#include <memory>
#include <vector>
#include <array>

namespace arc {
    
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    [[nodiscard]]
    static VkVertexInputBindingDescription get_binding_description();
    [[nodiscard]]
    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions();
};

    
class VertexBuffer : public DeclareNotCopyable
{
public:
    [[nodiscard]]
    static std::unique_ptr<VertexBuffer> create(const VkPhysicalDevice& physical_device,
                                                const VkDevice& logical_device,
                                                const std::vector<arc::Vertex>& vertices);
    [[nodiscard]]
    static std::unique_ptr<VertexBuffer> create_staging(const VkPhysicalDevice& physical_device,
                                                        const VkDevice& logical_device,
                                                        const VkCommandPool& command_pool,
                                                        const VkQueue& graphics_queue,
                                                        const std::vector<arc::Vertex>& vertices);

    VertexBuffer() = delete;
    VertexBuffer(const VkDevice& logical_device);
    
    [[nodiscard]]
    size_t get_count();

    [[nodiscard]]
    size_t get_memsize();

    [[nodiscard]]
    VkBuffer get_buffer();

    ~VertexBuffer();

private:
    const VkDevice& m_logical_device;
    VkBuffer m_vertex_buffer;
    VkBufferCreateInfo m_vertice_info;
    size_t m_count;
    VkDeviceMemory m_vertex_buffer_memory;
};

}
