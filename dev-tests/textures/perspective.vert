#version 450

layout(set=0, binding=0) uniform ViewPort {
    mat4 view;
    mat4 proj;
    mat4 model;
}viewport;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = viewport.proj * viewport.view * viewport.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
