#pragma once

#include "BasicBuffer.hpp"

namespace arc {

struct IndexTypeTag {
    static uint32_t bit;
};

using Index = uint32_t;
using IndexBuffer = BasicBuffer<Index, IndexTypeTag>;

}
