//
// Created by valdemar on 29.11.18.
//

#include "Renderer.h"

#include <cgutils/Shader.h>
#include <cgutils/utils.h>
#include <common/logger.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

[[maybe_unused]] const RenderContext *test_draw() {
    static std::unique_ptr<RenderContext> context;
    if (!context) {
        context = std::make_unique<RenderContext>();
        auto &to = *context;

        const glm::vec4 color_red = {1.0, 0.0, 0.0, 1.0};
        const glm::vec4 color_blue = {0, 0, 1.0, 1.0};
        const glm::vec4 color_green = {0, 1.0, 0.0, 1.0};
        to.add_polyline({{0, 0}, {100, 10}, {10, 100}, {50, 50}, {40, 30}}, color_red);
        to.add_polyline({{10, 0}, {30, 15}, {40, 60}, {10, 90}, {5, 25}}, color_blue);

        for (int i = 0; i < 1200; i += 5) {
            for (int j = 0; j < 800; j += 5) {
                to.add_circle({i, j}, 5, {i / 1200.0, j / 800.0, 0.5, 1.0}, false);
            }
        }

        to.add_rectangle({5, 5}, {45, 35}, {1.0, 1.0, 0.0, 0.7}, true);
        to.add_triangle({10, 10}, {60, 30}, {10, 40}, {0.0, 1.0, 1.0, 0.8}, true);

        to.add_circle({8, 8}, 8, color_red, true);
        to.add_circle({20, 10}, 8, color_green, true);
        to.add_circle({10, 20}, 8, color_blue, false);
    }

    return context.get();
}

}  // anonymous namespace

struct Renderer::render_attrs_t {
    GLuint grid_vao = 0;
    GLsizei grid_vertex_count = 0;
    // Vertex array to draw any rectangle and circle
    GLuint rect_vao = 0;
    // Lines designed to dynamic draw
    GLuint lines_vao = 0;
    GLuint uniform_buf{};
    glm::mat4 grid_model{};
};

Renderer::Renderer(ResourceManager *res, glm::u32vec2 area_size, glm::u16vec2 grid_cells)
    : mgr_(res)
    , ctx_render_params_(RenderContext::create_gl_context(*mgr_))
    , area_size_(area_size)
    , grid_cells_(grid_cells) {
    // TODO: Logger scope for pretty printing

    LOG_INFO("Initialize needed attributes");
    attr_ = std::make_unique<render_attrs_t>();
    // Init needed attributes
    attr_->grid_model = glm::scale(glm::mat4{1.0}, {area_size_.x, area_size_.y, 0.0f});

    // Shaders
    LOG_INFO("Compile shaders");
    shaders_ = std::make_unique<ShaderCollection>();

    // Preload rectangle to memory for further drawing
    LOG_INFO("Create rectangle for future rendering");
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

    // Uniform buffer
    LOG_INFO("Create Uniform buffer");
    attr_->uniform_buf = mgr_->gen_buffer();
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, 64, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, attr_->uniform_buf);

    LOG_INFO("Bind Uniform buffer to shaders");
    shaders_->color.bind_uniform_block("MatrixBlock", 0);
    shaders_->color_pos.bind_uniform_block("MatrixBlock", 0);
    shaders_->circle.bind_uniform_block("MatrixBlock", 0);
}

Renderer::~Renderer() = default;

void Renderer::update_frustum(const Camera &cam) {
    // Update projection matrix
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), glm::value_ptr(cam.proj_view()),
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::render_background(glm::vec3 color) {
    // Main scene area
    shaders_->color.use();
    auto model = glm::scale(glm::mat4(1.0f), {area_size_ * 0.5f, 1.0f});
    model = glm::translate(model, {1.0f, 1.0f, -0.2f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec4("color", glm::vec4{color, 1.0f});
    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //if (auto context = test_draw()) {
    //    context->draw(ctx_render_params_, *shaders_);
    //}
}

void Renderer::render_grid(glm::vec3 color) {
    shaders_->color.use();
    shaders_->color.set_mat4("model", attr_->grid_model);
    shaders_->color.set_vec4("color", glm::vec4{color, 1.0f});

    if (attr_->grid_vao == 0) {
        attr_->grid_vao = mgr_->gen_vertex_array();
        GLuint vbo = mgr_->gen_buffer();

        std::vector<float> grid;

        const float step_x = 1.0f / static_cast<float>(grid_cells_.x);
        for (size_t i = 0; i <= grid_cells_.x; ++i) {
            const float shift = step_x * i;

            grid.push_back(shift);
            grid.push_back(0.0);
            grid.push_back(0.0);

            grid.push_back(shift);
            grid.push_back(1.0);
            grid.push_back(0.0);
        }

        const float step_y = 1.0f / static_cast<float>(grid_cells_.y);
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
    glBindVertexArray(0);
}

void Renderer::render_primitives(const RenderContext &ctx) {
    ctx.draw(ctx_render_params_, *shaders_);
}
