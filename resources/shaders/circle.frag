#version 330 core
out vec4 frag_color;

in GS_OUT {
    vec4 color;
    vec2 cur_pt;
    vec2 center;
    float radius;
} fs_in;

//Zero line width mean filled circle
uniform uint line_width;

void main() {
    float dist = distance(fs_in.cur_pt, fs_in.center);
    float delta = fwidth(dist);

    float alpha = step(fs_in.radius, dist);
    alpha += (1.0 - step(fs_in.radius - delta * line_width, dist)) *
             step(float(1), float(line_width));

    frag_color = mix(fs_in.color, vec4(0.0), alpha);
}