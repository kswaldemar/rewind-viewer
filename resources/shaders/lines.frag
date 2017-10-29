#version 330 core
out vec4 frag_color;

in VS_OUT {
    vec3 color;
} fs_in;

in vec3 color;

void main() {
    frag_color = vec4(fs_in.color, 1.0);
}