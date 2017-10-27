#version 330 core
layout (location = 0) in vec3 a_pos;

uniform mat4 model;
uniform mat4 proj_view;

void main() {
    gl_Position = proj_view * model * vec4(a_pos, 1.0);
}