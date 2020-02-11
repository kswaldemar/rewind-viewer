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
    std::vector<circle_layout_t> filled_circles;
    std::vector<GLuint> line_indicies;
};

RenderContext::context_vao_t RenderContext::create_gl_context(ResourceManager &res) {
    RenderContext::context_vao_t ret;

    //Initialize forward pass point vao
    {
        ret.point_vao = res.gen_vertex_array();
        GLuint vbo = res.gen_buffer();
        //GLuint vbo = res.gen_buffer();
        glBindVertexArray(ret.point_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
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
        GLuint vbo = res.gen_buffer();
        glBindVertexArray(ret.circle_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
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

void RenderContext::add_circle(glm::vec2 center, float r, glm::vec4 color) {
    impl_->filled_circles.push_back({color, center, r});
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

void RenderContext::add_polygone(const std::vector<glm::vec2> &points, glm::vec4 color) {

}

void RenderContext::draw(const RenderContext::context_vao_t &vaos, const ShaderCollection &shaders) const {
    //glLineWidth(2);
    //glEnable(GL_LINE_SMOOTH);

    //Load points
    glBindVertexArray(vaos.point_vao);
    glBufferData(GL_ARRAY_BUFFER, impl_->points.size() * sizeof(point_layout_t), impl_->points.data(), GL_DYNAMIC_DRAW);

    //Draw every polyline with linewidth 1

    shaders.line.use();
    const auto &line_elements = impl_->line_indicies;
    glDrawElements(GL_LINES, line_elements.size(), GL_UNSIGNED_INT, line_elements.data());

    //Activate second shader and so on...
    glBindVertexArray(vaos.circle_vao);
    glBufferData(GL_ARRAY_BUFFER, impl_->filled_circles.size() * sizeof(circle_layout_t),
                 impl_->filled_circles.data(), GL_DYNAMIC_DRAW);
    shaders.circle.use();
    shaders.circle.set_uint("line_width", 1);
    glDrawArrays(GL_POINTS, 0, impl_->filled_circles.size());

    //glLineWidth(1);
    //glDisable(GL_LINE_SMOOTH);
    glBindVertexArray(0);
}

