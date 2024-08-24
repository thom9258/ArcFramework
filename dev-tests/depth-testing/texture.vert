#version 450

layout(set=0, binding=0) uniform ViewPort {
    mat4 view;
    mat4 proj;
    mat4 model;
}viewport;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    gl_Position = viewport.proj * viewport.view * viewport.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}
