#version 330 core
out vec4 frag_color;

in VS_OUT {
    vec4 color;
} fs_in;

void main() {
    frag_color = fs_in.color;
}