#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace arc {

[[nodiscard]]
glm::vec3 global_up(void) noexcept;

[[nodiscard]]
glm::vec3 global_right(void) noexcept;

[[nodiscard]]
glm::vec3 global_front(void) noexcept;

[[nodiscard]]
std::pair<float, glm::vec3> euler_to_angleaxis(glm::vec3 _euler) noexcept;

[[nodiscard]]
glm::vec3 local_up(const glm::mat4 _m) noexcept;

[[nodiscard]]
glm::vec3 local_right(const glm::mat4 _m) noexcept;

[[nodiscard]]
glm::vec3 local_front(const glm::mat4 _m) noexcept;


[[nodiscard]]
glm::vec3 position(const glm::mat4 _m) noexcept;

[[nodiscard]]
glm::mat4 set_position(const glm::mat4 _m, const glm::vec3 _v) noexcept;

[[nodiscard]]
glm::mat4 translate(const glm::mat4 _m, const glm::vec3 _v) noexcept;

[[nodiscard]]
glm::mat4 local_translate(const glm::mat4 _m, const glm::vec3 _v) noexcept;

[[nodiscard]]
glm::mat4 translation_matrix(const glm::vec3 _v) noexcept;

glm::mat4 view_projection(const glm::mat4 _transform,
                          const glm::mat4 _projection) noexcept;

[[nodiscard]]
glm::mat4 rotate(const glm::mat4 _m,
                 const float _angle,
                 const glm::vec3 _axis) noexcept;

[[nodiscard]]
glm::mat4 rotation_towards(const glm::vec3 _position,
                           const glm::vec3 _target,
                           const glm::vec3 _up) noexcept;

[[nodiscard]]
std::string stringify(const glm::vec3 _vec) noexcept;

[[nodiscard]]
std::string stringify(const glm::vec4 _vec) noexcept;

[[nodiscard]]
std::string stringify(const glm::mat4 _m) noexcept;

[[nodiscard]]
std::string stringify(const glm::quat _q) noexcept;

}
