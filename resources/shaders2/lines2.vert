#version 330 core
layout (location = 0) in vec4 a_color;
layout (location = 1) in vec2 a_pos;

out VS_OUT {
    vec4 color;
} vs_out;

layout (std140) uniform MatrixBlock {
    mat4 proj_view;
};

void main() {
    gl_Position = proj_view * vec4(a_pos, 0.2, 1.0);
    vs_out.color = a_color;
}