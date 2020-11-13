//
// Created by valdemar on 10.02.2020.
//

#include "RenderContext.h"
#include "ShaderCollection.h"

#include <array>
#include <stdexcept>

namespace {

#pragma pack(push, 1)
struct point_layout_t {
    glm::vec4 color;
    glm::vec2 point;
};

struct circle_layout_t {
    glm::vec4 color;
    glm::vec2 point;
    float radius;
};

#pragma pack(pop)

void add_elements(size_t shift, std::vector<GLuint> &to, const std::vector<GLuint> &from) {
    to.reserve(to.size() + from.size());
    for (GLuint i : from) {
        to.push_back(shift + i);
    }
}

}  // anonymous namespace

struct RenderContext::memory_layout_t {
    std::vector<point_layout_t> points;
    std::vector<circle_layout_t> circles;
    std::vector<GLuint> filled_circle_indicies;
    std::vector<GLuint> thin_circle_indicies;
    std::vector<GLuint> triangle_indicies;
    std::vector<GLuint> line_indicies;
};

RenderContext::context_vao_t RenderContext::create_gl_context(ResourceManager &res) {
    RenderContext::context_vao_t ret{};

    ret.common_ebo = res.gen_buffer();
    // Initialize forward pass point vao
    {
        ret.point_vao = res.gen_vertex_array();
        ret.point_vbo = res.gen_buffer();
        glBindVertexArray(ret.point_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ret.point_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.common_ebo);
        // Point layout of RenderContext: vec4 color, vec2 pos
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
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.common_ebo);
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

void RenderContext::add_triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color,
                                 bool fill) {
    add_triangle(p1, p2, p3, {color, color, color}, fill);
}

void RenderContext::add_rectangle(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color,
                                  bool fill) {
    RectangleColors colors;
    colors.fill(color);
    add_rectangle(top_left, bottom_right, colors, fill);
}

void RenderContext::add_polyline(const std::vector<glm::vec2> &points, glm::vec4 color) {
    if (points.size() < 2) {
        throw std::invalid_argument("Cannot create polyline from one point");
    }

    impl_->points.push_back({color, points[0]});
    for (size_t i = 1; i < points.size(); ++i) {
        impl_->points.push_back({color, points[i]});

        // Add line between two points in sequence
        GLuint idx = impl_->points.size() - 1;
        impl_->line_indicies.push_back(idx - 1);
        impl_->line_indicies.push_back(idx);
    }
}

void RenderContext::add_triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3,
                                 const TriangleColors &colors, bool fill) {
    if (fill) {
        GLuint idx = impl_->points.size();
        impl_->points.push_back({colors[0], p1});
        impl_->points.push_back({colors[1], p2});
        impl_->points.push_back({colors[2], p3});
        impl_->triangle_indicies.push_back(idx);
        impl_->triangle_indicies.push_back(idx + 1);
        impl_->triangle_indicies.push_back(idx + 2);
    } else {
        add_polyline({p1, p2, p3, p1}, colors[0]);
    }
}

void RenderContext::add_rectangle(glm::vec2 top_left, glm::vec2 bottom_right,
                                  const RectangleColors &colors, bool fill) {
    auto top_right = glm::vec2{bottom_right.x, top_left.y};
    auto bottom_left = glm::vec2{top_left.x, bottom_right.y};

    if (fill) {
        GLuint idx = impl_->points.size();
        impl_->points.push_back({colors[0], top_left});
        impl_->points.push_back({colors[1], bottom_left});
        impl_->points.push_back({colors[2], top_right});
        impl_->points.push_back({colors[3], bottom_right});

        for (uint8_t t : {0, 2, 1, 2, 3, 1}) {
            impl_->triangle_indicies.push_back(idx + t);
        }
    } else {
        add_polyline({top_left, top_right, bottom_right, bottom_left, top_left}, colors[0]);
    }
}

void RenderContext::update_from(const RenderContext &other) {
    const size_t points_cnt = impl_->points.size();
    add_elements(points_cnt, impl_->line_indicies, other.impl_->line_indicies);
    add_elements(points_cnt, impl_->triangle_indicies, other.impl_->triangle_indicies);

    impl_->points.reserve(points_cnt + other.impl_->points.size());
    for (auto &obj : other.impl_->points) {
        impl_->points.emplace_back(obj);
    }

    const size_t circles_cnt = impl_->circles.size();
    add_elements(circles_cnt, impl_->thin_circle_indicies, other.impl_->thin_circle_indicies);
    add_elements(circles_cnt, impl_->filled_circle_indicies, other.impl_->filled_circle_indicies);

    impl_->circles.reserve(circles_cnt + other.impl_->circles.size());
    for (auto &obj : other.impl_->circles) {
        impl_->circles.emplace_back(obj);
    }
}

void RenderContext::clear() {
    (*impl_) = memory_layout_t();
}

void RenderContext::draw(const RenderContext::context_vao_t &vaos,
                         const ShaderCollection &shaders) const {
    glCheckError();
    // glLineWidth(2);
    // glEnable(GL_LINE_SMOOTH);

    // Load data
    glBindBuffer(GL_ARRAY_BUFFER, vaos.point_vbo);
    glBufferData(GL_ARRAY_BUFFER, impl_->points.size() * sizeof(point_layout_t),
                 impl_->points.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vaos.circle_vbo);
    glBufferData(GL_ARRAY_BUFFER, impl_->circles.size() * sizeof(circle_layout_t),
                 impl_->circles.data(), GL_DYNAMIC_DRAW);

    // Simple pass shader - triangles and lines
    shaders.color_pos.use();
    glBindVertexArray(vaos.point_vao);
    {
        // Filled triangles, so any polygon
        const auto &triangle_elements = impl_->triangle_indicies;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_elements.size() * sizeof(GLuint),
                     triangle_elements.data(), GL_DYNAMIC_DRAW);
        glDrawElements(GL_TRIANGLES, triangle_elements.size(), GL_UNSIGNED_INT, nullptr);

        // Lines
        const auto &line_elements = impl_->line_indicies;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, line_elements.size() * sizeof(GLuint),
                     line_elements.data(), GL_DYNAMIC_DRAW);
        glDrawElements(GL_LINES, line_elements.size(), GL_UNSIGNED_INT, nullptr);
    }

    // Circles shader
    shaders.circle.use();
    glBindVertexArray(vaos.circle_vao);
    {
        // Filled
        shaders.circle.set_uint("line_width", 0);
        const auto &fill_circle_elements = impl_->filled_circle_indicies;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, fill_circle_elements.size() * sizeof(GLuint),
                     fill_circle_elements.data(), GL_DYNAMIC_DRAW);
        glDrawElements(GL_POINTS, fill_circle_elements.size(), GL_UNSIGNED_INT, nullptr);

        // Thin
        shaders.circle.set_uint("line_width", 1);
        const auto &thin_circle_elements = impl_->thin_circle_indicies;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, thin_circle_elements.size() * sizeof(GLuint),
                     thin_circle_elements.data(), GL_DYNAMIC_DRAW);
        glDrawElements(GL_POINTS, thin_circle_elements.size(), GL_UNSIGNED_INT, nullptr);
    }

    // glLineWidth(1);
    // glDisable(GL_LINE_SMOOTH);
    glBindVertexArray(0);
    glCheckError();
}
