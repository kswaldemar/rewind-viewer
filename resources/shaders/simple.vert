#version 330 core
layout (location = 0) in vec3 a_pos;

layout (std140) uniform Matrix {
    mat4 proj_view;
};
uniform mat4 model;

void main() {
    gl_Position = proj_view * model * vec4(a_pos, 1.0);
}