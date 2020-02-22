//
// Created by valdemar on 10.02.2020.
//

#include "RenderContext.h"
#include "ShaderCollection.h"

#include <exception>

namespace {

struct point_layout_t {
    glm::vec4 color;
    glm::vec2 point;
} __attribute__((packed));

struct circle_layout_t {
    glm::vec4 color;
    glm::vec2 point;
    float radius;
} __attribute__((packed));

} // anonymous namespace

struct RenderContext::memory_layout_t {
    std::vector<point_layout_t> points;
    std::vector<circle_layout_t> circles;
    std::vector<GLuint> filled_circle_indicies;
    std::vector<GLuint> thin_circle_indicies;
    std::vector<GLuint> triangle_indicies;
    std::vector<GLuint> line_indicies;
};

RenderContext::context_vao_t RenderContext::create_gl_context(ResourceManager &res) {
    RenderContext::context_vao_t ret;

    //Initialize forward pass point vao
    //TODO: How vbo binding really works inside vertex arrays? Using two different vbo breaks lines drawing
    {
        ret.point_vao = res.gen_vertex_array();
        ret.point_vbo = res.gen_buffer();
        glBindVertexArray(ret.point_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ret.point_vbo);
        //Point layout of RenderContext: vec4 color, vec2 pos
        const size_t stride = 6 * sizeof(float);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, cg::offset<float>(4));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    {
        ret.circle_vao = res.gen_vertex_array();
        ret.circle_vbo = res.gen_buffer();
        glBindVertexArray(ret.circle_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ret.circle_vbo);
        const size_t stride = 7 * sizeof(float);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, cg::offset<float>(4));
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, cg::offset<float>(6));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    return ret;
}

RenderContext::RenderContext() {
    impl_ = std::make_unique<memory_layout_t>();
}

RenderContext::~RenderContext() = default;

void RenderContext::add_circle(glm::vec2 center, float r, glm::vec4 color, bool fill) {
    GLuint idx = impl_->circles.size();
    impl_->circles.push_back({color, center, r});
    if (fill) {
        impl_->filled_circle_indicies.push_back(idx);
    } else {
        impl_->thin_circle_indicies.push_back(idx);
    }
}

void RenderContext::add_filled_triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color) {
    GLuint idx = impl_->points.size();
    impl_->points.push_back({color, p1});
    impl_->points.push_back({color, p2});
    impl_->points.push_back({color, p3});
    impl_->triangle_indicies.push_back(idx);
    impl_->triangle_indicies.push_back(idx + 1);
    impl_->triangle_indicies.push_back(idx + 2);
}

void RenderContext::add_rectangle(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, bool fill) {
    auto top_right = glm::vec2{bottom_right.x, top_left.y};
    auto bottom_left = glm::vec2{top_left.x, bottom_right.y};

    if (fill) {
        GLuint idx = impl_->points.size();
        impl_->points.push_back({color, top_left});
        impl_->points.push_back({color, top_right});
        impl_->points.push_back({color, bottom_right});
        impl_->points.push_back({color, bottom_left});

        for (uint8_t t : {0, 1, 3,
                          1, 2, 3}) {
            impl_->triangle_indicies.push_back(idx + t);
        }
    } else {
        add_polyline({top_left, top_right, bottom_right, bottom_left, top_left}, color);
    }
}

void RenderContext::add_polyline(const std::vector<glm::vec2> &points, glm::vec4 color) {
    if (points.size() < 2) {
        throw std::invalid_argument("Got polyline with only one point");
    }

    impl_->points.push_back({color, points[0]});
    for (size_t i = 1; i < points.size(); ++i) {
        impl_->points.push_back({color, points[i]});

        //Add line between two points in sequence
        GLuint idx = impl_->points.size() - 1;
        impl_->line_indicies.push_back(idx - 1);
        impl_->line_indicies.push_back(idx);
    }
}

void RenderContext::draw(const RenderContext::context_vao_t &vaos, const ShaderCollection &shaders) const {
    //glLineWidth(2);
    //glEnable(GL_LINE_SMOOTH);

    //Load data
    glBindBuffer(GL_ARRAY_BUFFER, vaos.point_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        impl_->points.size() * sizeof(point_layout_t),
        impl_->points.data(),
        GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vaos.circle_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        impl_->circles.size() * sizeof(circle_layout_t),
        impl_->circles.data(),
        GL_DYNAMIC_DRAW);

    //Lines
    shaders.line.use();
    glBindVertexArray(vaos.point_vao);
    const auto &line_elements = impl_->line_indicies;
    glDrawElements(GL_LINES, line_elements.size(), GL_UNSIGNED_INT, line_elements.data());

    //Filled triangles, so any polygon
    const auto &triangle_elements = impl_->triangle_indicies;
    glDrawElements(GL_TRIANGLES, triangle_elements.size(), GL_UNSIGNED_INT, triangle_elements.data());

    //Circles
    shaders.circle.use();
    glBindVertexArray(vaos.circle_vao);

    //Filled
    shaders.circle.set_uint("line_width", 0);
    glDrawElements(
        GL_POINTS, impl_->filled_circle_indicies.size(),
        GL_UNSIGNED_INT, impl_->filled_circle_indicies.data());

    //Thin
    shaders.circle.set_uint("line_width", 1);
    glDrawElements(
        GL_POINTS, impl_->thin_circle_indicies.size(),
        GL_UNSIGNED_INT, impl_->thin_circle_indicies.data());

    //glLineWidth(1);
    //glDisable(GL_LINE_SMOOTH);
    glBindVertexArray(0);
}

