#version 330 core
layout (location = 0) in vec3 a_pos;

out vec3 coord;

layout (std140) uniform Matrix {
    mat4 proj_view;
};
uniform mat4 model;

void main() {
    vec4 world_pos = model * vec4(a_pos, 1.0);
    gl_Position = proj_view * world_pos;
    coord = world_pos.xyz;
}