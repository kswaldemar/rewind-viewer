#version 330 core
layout (location = 0) in vec3 a_pos;

out vec3 coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 world_pos = model * vec4(a_pos, 1.0);
    gl_Position = projection * view * world_pos;
    coord = world_pos.xyz;
}