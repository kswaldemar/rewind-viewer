#version 330 core
layout (location = 0) in vec4 a_color;
layout (location = 1) in vec2 a_pos;
layout (location = 2) in float a_radius;

out VS_OUT {
    vec4 color;
    float radius;
} vs_out;

void main() {
    gl_Position = vec4(a_pos, 0.2, 1.0);
    vs_out.color = a_color;
    vs_out.radius = a_radius;
}