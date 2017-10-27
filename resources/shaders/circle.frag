#version 330 core
out vec4 frag_color;

in vec3 coord;

uniform vec3 center;
uniform vec3 color;
uniform float radius;

void main() {
    if (length(coord - center) > radius)
        discard;
    frag_color = vec4(color, 1.0);
}