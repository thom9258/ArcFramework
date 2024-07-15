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
    glm::vec2 pos{};
    glm::vec3 color{};

    [[nodiscard]]
    static VkVertexInputBindingDescription get_binding_description();
    [[nodiscard]]
    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions();
};

struct VertexBufferPolicy {
    using value_type = Vertex;
    static const uint32_t buffer_type_bit;
};

using VertexBuffer = BasicBuffer<VertexBufferPolicy>;

}
