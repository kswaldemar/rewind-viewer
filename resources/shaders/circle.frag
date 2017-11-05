#version 330 core
out vec4 frag_color;

in vec3 coord;

uniform vec3 center;
uniform vec3 color;
uniform float radius2;

const vec3 selected_color = vec3(0.4, 0.8, 0.0);

void main() {
    vec3 rv = coord - center;
    float cur_r2 = dot(rv, rv);
    if (cur_r2 > radius2)
        discard;
    vec3 mixed_color = mix(color, color * 0.3, cur_r2 / radius2);
    frag_color = vec4(mixed_color, 1.0);
}