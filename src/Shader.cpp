#include "../arc/Shader.hpp"

#include <fstream>

namespace arc {

std::vector<char> read_shader(const std::string& filename) 
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
}

}
