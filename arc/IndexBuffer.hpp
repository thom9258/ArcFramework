#pragma once

#include "BasicBuffer.hpp"

namespace ArcGraphics {
    
struct IndexBufferPolicy {
    using value_type = uint32_t;
    static const uint32_t buffer_type_bit;
};

using IndexBuffer = BasicBuffer<IndexBufferPolicy>;

}
