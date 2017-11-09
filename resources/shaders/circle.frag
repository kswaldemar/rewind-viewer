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

const float circle_alpha = 0.6; //outer unit circle alpha
const float texture_coloring = 0.3; //how much add color to texture, 0 - use texture as is

void main() {
    vec3 rv = fs_in.coord - center;
    float cur_r2 = dot(rv, rv);
    if (cur_r2 > radius2)
        discard;
    vec3 solid = mix(color, color * 0.3, cur_r2 / radius2);

    if (textured) {
        vec4 tex = texture(tex_smp, fs_in.uv);
        vec4 colored_tex = mix(tex, vec4(solid, 1.0), texture_coloring);
        frag_color = mix(vec4(solid, circle_alpha), colored_tex, tex.a);
    } else {
        frag_color = vec4(color, 1.0f);
    }

}