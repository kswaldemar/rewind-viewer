#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec4 color;
    float radius;
} gs_in[];

out GS_OUT {
    vec4 color;
    vec2 cur_pt;
    vec2 center;
    float radius;
} gs_out;

layout (std140) uniform MatrixBlock {
    mat4 proj_view;
};

void main() {
    float r = gs_in[0].radius;
    gs_out.color = gs_in[0].color;
    gs_out.center = gl_in[0].gl_Position.xy;
    gs_out.radius = gs_in[0].radius;

    vec4 center_pos = gl_in[0].gl_Position;
    vec4 point;

    point = center_pos + vec4(-r, -r, 0.0, 0.0);
    gl_Position = proj_view * point;
    gs_out.cur_pt = point.xy;
    EmitVertex();

    point = center_pos + vec4(r, -r, 0.0, 0.0);
    gl_Position = proj_view * point;
    gs_out.cur_pt = point.xy;
    EmitVertex();

    point = center_pos + vec4(-r, r, 0.0, 0.0);
    gl_Position = proj_view * point;
    gs_out.cur_pt = point.xy;
    EmitVertex();

    point = center_pos + vec4(r, r, 0.0, 0.0);
    gl_Position = proj_view * point;
    gs_out.cur_pt = point.xy;
    EmitVertex();

    EndPrimitive();
}
