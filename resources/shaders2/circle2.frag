#version 330 core
out vec4 frag_color;

in GS_OUT {
    vec4 color;
    vec2 cur_pt;
    vec2 center;
    float radius2;
} fs_in;

void main() {
    vec2 rvec = fs_in.cur_pt - fs_in.center;
    float cur_r2 = dot(rvec, rvec);
    if (cur_r2 > fs_in.radius2) {
        discard;
    }

    vec4 solid = fs_in.color;
//    solid.w = 0.5;
//    vec4 border = fs_in.color * 0.5;
//    border.w = fs_in.color.w;
//    vec4 solid = mix(fs_in.color, border, cur_r2 / fs_in.radius2);

    frag_color = solid;
}