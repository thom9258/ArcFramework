#include <string>
#include <optional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "error.hpp"
#include "sdlgl.hpp"

namespace arc {

class Shader;
using ShaderPtr = std::shared_ptr<const Shader>;
using ShaderUniform = std::optional<int>;

class Shader {
public:
    explicit Shader(void) = delete;
    explicit Shader(unsigned int program) noexcept;
    ~Shader(void) noexcept;

    Shader(Shader&&) = delete;
    Shader(const Shader&) = delete;
    Shader operator=(Shader&&) = delete;
    Shader operator=(const Shader&) = delete;

    friend ShaderPtr make_shader(const char* _vertex_source, const char* _fragment_source);
    friend ShaderUniform get_uniform(ShaderPtr shader, const std::string& name);
    friend void activate(ShaderPtr shader) noexcept;

private:
    unsigned int m_program{0};
};

[[nodiscard]]
ShaderPtr make_shader(std::string _vertex_source, std::string _fragment_source);

[[nodiscard]]
ShaderPtr make_shader(const char* _vertex_source, const char* _fragment_source);

void activate(ShaderPtr shader) noexcept;

[[nodiscard]]
ShaderUniform get_uniform(ShaderPtr shader, const std::string &name);

void set_mat4(ShaderPtr shader, const std::string &name, glm::mat4 value);
void set_int(ShaderPtr shader, const std::string &name, int value);
void set_float(ShaderPtr shader, const std::string &name, float value);
void set_vec3(ShaderPtr shader, const std::string &name, glm::vec3 value);
void set_vec4(ShaderPtr shader, const std::string &name, glm::vec4 value);

}
