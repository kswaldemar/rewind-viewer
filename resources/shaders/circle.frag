#version 330 core
out vec4 frag_color;

in VS_OUT {
    vec2 uv;
    vec3 coord;
} fs_in;

uniform vec3 center;
uniform vec3 color;
uniform float radius2;
uniform sampler2D tex_smp;
uniform bool textured; // Whenever use texture or not

const vec3 selected_color = vec3(0.4, 0.8, 0.0);

void main() {
    vec3 rv = fs_in.coord - center;
    float cur_r2 = dot(rv, rv);
    if (cur_r2 > radius2)
        discard;
    vec3 solid = mix(color, color * 0.3, cur_r2 / radius2);
    vec4 tex = texture(tex_smp, fs_in.uv);
    if (textured && tex.a < 0.1)
        discard;
    frag_color = mix(vec4(solid, 1.0), tex, (textured ? 0.5 : 0) * tex.a);
}