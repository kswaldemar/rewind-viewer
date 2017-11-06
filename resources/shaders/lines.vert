#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec3 a_color;

out VS_OUT {
    vec3 color;
} vs_out;

out vec3 color;

layout (std140) uniform MatrixBlock {
    mat4 proj_view;
};

void main() {
    gl_Position = proj_view * vec4(a_pos, 0.2, 1.0);
    vs_out.color = a_color;
}