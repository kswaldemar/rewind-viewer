//
// Created by valdemar on 29.11.18.
//

#include "Renderer.h"

#include <cgutils/utils.h>
#include <cgutils/Shader.h>
#include <common/logger.h>

//#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Renderer::render_attrs_t {
    GLuint grid_vao = 0;
    GLsizei grid_vertex_count = 0;
    //Vertex array to draw any rectangle and circle
    GLuint rect_vao = 0;
    //Lines designed to dynamic draw
    GLuint lines_vao = 0;
    GLuint uniform_buf;
    glm::mat4 grid_model;
};

struct Renderer::shaders_t {
    shaders_t()
        : color("simple.vert", "uniform_color.frag"),
          circle("circle.vert", "circle.frag"),
          lines("lines.vert", "lines.frag"),
          textured("simple.vert", "textured.frag") {
        //Setup variables, which will never change
        circle.use();
        circle.set_int("tex_smp", 0);
        textured.use();
        textured.set_int("tex_smp", 0);
    }

    Shader color;
    Shader circle;
    Shader lines;
    Shader textured;
};


Renderer::Renderer(ResourceManager *res, glm::u32vec2 area_size, glm::u16vec2 grid_cells)
    : mgr_(res),
      area_size_(area_size),
      grid_cells_(grid_cells) {

    //TODO: Logger scope for pretty printing

    LOG_INFO("Initialize needed attributes")
    attr_ = std::make_unique<render_attrs_t>();
    //Init needed attributes
    attr_->grid_model = glm::scale(glm::mat4{1.0}, {area_size_.x, area_size_.y, 0.0f});

    //Shaders
    LOG_INFO("Compile shaders")
    shaders_ = std::make_unique<shaders_t>();

    //Even more shaders
    LOG_INFO("Even moar shaders");
    Shader::set_shaders_folder("resources/shaders2");
    shaders2_ = std::make_unique<ShaderCollection>();

    //Preload rectangle to memory for further drawing
    LOG_INFO("Create rectangle for future rendering")
    attr_->rect_vao = mgr_->gen_vertex_array();
    GLuint vbo = mgr_->gen_buffer();
    //@formatter:off
    const float points[] = {
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
    };
    //@formatter:on

    glBindVertexArray(attr_->rect_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    const GLsizei stride = 5 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, cg::offset<float>(3));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Uniform buffer
    LOG_INFO("Create Uniform buffer")
    attr_->uniform_buf = mgr_->gen_buffer();
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, 64, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, attr_->uniform_buf);

    LOG_INFO("Bind Uniform buffer to shaders")
    shaders_->color.bind_uniform_block("MatrixBlock", 0);
    shaders_->circle.bind_uniform_block("MatrixBlock", 0);
    shaders_->lines.bind_uniform_block("MatrixBlock", 0);
    shaders_->textured.bind_uniform_block("MatrixBlock", 0);

    //*** DO NOT FORGET UPDATE SHADERS2 HERE ***//
    shaders2_->line.bind_uniform_block("MatrixBlock", 0);

    //Create one gl context (needed vertex array pointers) for all RenderContext's
    gl_ctx_ = RenderContext::create_gl_context(*res);

    //*** Testing data ***//
    const glm::vec4 color_red = {1.0, 0.0, 0.0, 1.0};
    const glm::vec4 color_blue = {0, 0, 1.0, 1.0};
    const glm::vec4 color_green = {0, 1.0, 0.0, 1.0};
    //test_context_.add_polyline({{0, 0}, {100, 10}, {10, 100}, {50, 50}, {40, 30}}, color_red);
    //test_context_.add_polyline({{10, 0}, {30, 15}, {40, 60}, {10, 90}, {5, 25}}, color_blue);

    for (int i = 0; i < 1200; i += 5) {
        for (int j = 0; j < 800; j += 5) {
            test_context_.add_circle({i, j}, 5, {i / 1200.0, j / 800.0, 0.5, 1.0}, false);
        }
    }
    test_context_.add_circle({8, 8}, 8, color_red, true);
    test_context_.add_circle({20, 10}, 8, color_green, true);
    test_context_.add_circle({10, 20}, 8, color_blue, true);
}

Renderer::~Renderer() = default;

void Renderer::update_frustum(const Camera &cam) {
    //Update projection matrix
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), glm::value_ptr(cam.proj_view()), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::render_background(glm::vec3 color) {
    //Main scene area
    shaders_->color.use();
    auto model = glm::scale(glm::mat4(1.0f), {area_size_ * 0.5f, 1.0f});
    model = glm::translate(model, {1.0f, 1.0f, -0.2f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec4("color", glm::vec4{color, 1.0f});
    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //JUST FOR TESTING PURPOSES
    test_context_.draw(gl_ctx_, *shaders2_);
}

void Renderer::render_grid(glm::vec3 color) {
    shaders_->color.use();
    shaders_->color.set_mat4("model", attr_->grid_model);
    shaders_->color.set_vec4("color", glm::vec4{color, 1.0f});

    if (attr_->grid_vao == 0) {
        attr_->grid_vao = mgr_->gen_vertex_array();
        GLuint vbo = mgr_->gen_buffer();

        std::vector<float> grid;

        const float step_x = 1.0f / grid_cells_.x;
        for (size_t i = 0; i <= grid_cells_.x; ++i) {
            const float shift = step_x * i;

            grid.push_back(shift);
            grid.push_back(0.0);
            grid.push_back(0.0);

            grid.push_back(shift);
            grid.push_back(1.0);
            grid.push_back(0.0);
        }

        const float step_y = 1.0f / grid_cells_.y;
        for (size_t i = 0; i <= grid_cells_.y; ++i) {
            const float shift = step_y * i;

            grid.push_back(0.0);
            grid.push_back(shift);
            grid.push_back(0.0);

            grid.push_back(1.0);
            grid.push_back(shift);
            grid.push_back(0.0);
        }

        attr_->grid_vertex_count = static_cast<GLsizei>(grid.size() / 3);

        glBindVertexArray(attr_->grid_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, grid.size() * sizeof(float), grid.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
    glBindVertexArray(attr_->grid_vao);
    glDrawArrays(GL_LINES, 0, attr_->grid_vertex_count);
}

void Renderer::render_frame_layer(const Frame::primitives_t &slice) {
    if (!slice.circles.empty()) {
        shaders_->circle.use();
        shaders_->circle.set_int("textured", 0);
        for (const auto &obj : slice.circles) {
            do_render_circle(obj);
        }
    }

    if (!slice.rectangles.empty()) {
        shaders_->color.use();
        for (const auto &obj : slice.rectangles) {
            do_render_rectangle(obj);
        }
    }

    if (!slice.lines.empty()) {
        shaders_->lines.use();
        do_render_lines(slice.lines);
    }
}


void Renderer::do_render_circle(const pod::Circle &circle) {
    auto vcenter = glm::vec3{circle.center.x, circle.center.y, 0.0f};
    glm::mat4 model = glm::translate(glm::mat4(1.0f), vcenter);
    model = glm::scale(model, glm::vec3{circle.radius, circle.radius, 1.0f});
    shaders_->circle.set_float("radius2", circle.radius * circle.radius);
    shaders_->circle.set_vec3("center", vcenter);
    shaders_->circle.set_vec4("color", circle.color);
    shaders_->circle.set_mat4("model", model);

    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::do_render_rectangle(const pod::Rectangle &rect) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                     glm::vec3{rect.center.x, rect.center.y, 0.0f});
    model = glm::scale(model, glm::vec3{rect.w * 0.5, rect.h * 0.5, 1.0f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec4("color", rect.color);

    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::do_render_lines(const std::vector<pod::Line> &lines) {
    if (attr_->lines_vao == 0) {
        attr_->lines_vao = mgr_->gen_vertex_array();
        GLuint vbo = mgr_->gen_buffer();

        glBindVertexArray(attr_->lines_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        //Plain float format: vec3 color, alpha, vec2 pos
        const size_t stride = 6 * sizeof(float);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, cg::offset<float>(4));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    glBindVertexArray(attr_->lines_vao);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(pod::Line), lines.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() * 2));
}
