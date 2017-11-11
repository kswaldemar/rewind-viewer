#version 330 core
out vec4 frag_color;

in VS_OUT {
    vec2 uv;
} fs_in;

uniform sampler2D tex_smp;
uniform vec2 tex_scale = vec2(1.0);
uniform vec3 color = vec3(1.0);

void main() {
    frag_color = vec4(color, 1.0) * texture(tex_smp, fs_in.uv * tex_scale);
}