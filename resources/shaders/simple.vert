#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_uv;

out VS_OUT {
    vec2 uv;
} vs_out;

layout (std140) uniform MatrixBlock {
    mat4 proj_view;
};

uniform mat4 model;

void main() {
    gl_Position = proj_view * model * vec4(a_pos, 1.0);
    vs_out.uv = a_tex_uv;
}