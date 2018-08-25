#include "Scene.h"

#include <cgutils/utils.h>
#include <cgutils/Shader.h>
#include <common/logger.h>

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

namespace {

bool hittest(const glm::vec2 &wmouse, const pod::Popup &popup) {
    auto d = wmouse - popup.center;
    return (d.x * d.x + d.y * d.y) <= popup.radius * popup.radius;
}

} // anonymous namespace

struct Scene::render_attrs_t {
    GLuint grid_vao = 0;
    GLsizei grid_vertex_count = 0;
    //Vertex array to draw any rectangle and circle
    GLuint rect_vao = 0;
    //Lines designed to dynamic draw
    GLuint lines_vao = 0;
    GLuint uniform_buf;
    glm::mat4 grid_model;
};

struct Scene::shaders_t {
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

Scene::Scene(ResourceManager *res, const Config::SceneConf *conf)
    : mgr_(res)
    , conf_(*conf) {

    LOG_INFO("Initialize needed attributes")
    attr_ = std::make_unique<render_attrs_t>();
    //Init needed attributes
    attr_->grid_model = glm::scale(glm::mat4{}, {conf_.grid_dim.x, conf_.grid_dim.y, 0.0f});

    //Shaders
    LOG_INFO("Compile shaders")
    shaders_ = std::make_unique<shaders_t>();

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
}

Scene::~Scene() = default;

void Scene::update_and_render(const glm::mat4 &proj_view, int y_axes_invert) {
    //Update world origin position
    y_axes_invert_ = y_axes_invert;

    //Update current frame
    {
        std::lock_guard<std::mutex> f(frames_mutex_);
        frames_count_ = static_cast<int>(frames_.size());
        if (cur_frame_idx_ >= 0 && cur_frame_idx_ < frames_count_) {
            active_frame_ = frames_[cur_frame_idx_];
        }
    }

    //Update projection matrix
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), glm::value_ptr(proj_view), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //Main scene area
    shaders_->color.use();
    auto model = glm::scale(glm::mat4(1.0f), {conf_.grid_dim * 0.5f, 1.0f});
    model = glm::translate(model, {1.0f, 1.0f, -0.2f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec4("color", conf_.scene_color);
    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //Grid
    if (conf_.show_grid) {
        shaders_->color.use();
        shaders_->color.set_mat4("model", attr_->grid_model);
        shaders_->color.set_vec4("color", conf_.grid_color);
        render_grid();
    }

    //Frame
    if (active_frame_) {
        for (size_t idx = 0; idx < active_frame_->primitives.size(); ++idx) {
            if (conf_.enabled_layers[idx]) {
                render_frame_layer(active_frame_->primitives[idx]);
            }
        }
    }
}

void Scene::set_frame_index(int idx) {
    cur_frame_idx_ = cg::clamp(idx, 0, frames_count_ - 1);
}

int Scene::get_frame_index() {
    return cur_frame_idx_;
}

int Scene::get_frames_count() {
    return frames_count_;
}

const char *Scene::get_frame_user_message() {
    if (active_frame_) {
        return active_frame_->user_message.c_str();
    }
    return "";
}

void Scene::add_frame(std::unique_ptr<Frame> &&frame) {
    std::lock_guard<std::mutex> f(frames_mutex_);
    //Sort units for proper draw order
    frames_.emplace_back(std::move(frame));
}

void Scene::show_detailed_info(const glm::vec2 &mouse) const {
    if (!active_frame_) {
        return;
    }

    for (const auto &popup : active_frame_->popups) {
        if (hittest(mouse, popup)) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", popup.text.c_str());
            ImGui::EndTooltip();
        }
    }
}

void Scene::clear_data(bool clean_active) {
    std::lock_guard<std::mutex> _(frames_mutex_);
    frames_.clear();
    if (clean_active) {
        active_frame_ = nullptr;
    }
    frames_count_ = 0;
    cur_frame_idx_ = 0;
}

bool Scene::has_data() const {
    return frames_count_ > 0;
}

void Scene::render_frame_layer(const Frame::primitives_t &slice) {
    //if (!slice.facilities.empty()) {
    //    shaders_->textured.use();
    //    shaders_->textured.set_vec2("tex_scale", glm::vec2(1.0));
    //    for (const auto &obj : slice.facilities) {
    //        render_facility_object(obj);
    //    }
    //}

    if (!slice.circles.empty()) {
        shaders_->circle.use();
        shaders_->circle.set_int("textured", 0);
        for (const auto &obj : slice.circles) {
            render_circle(obj);
        }
    }

    if (!slice.rectangles.empty()) {
        shaders_->color.use();
        for (const auto &obj : slice.rectangles) {
            render_rectangle(obj);
        }
    }

    if (!slice.lines.empty()) {
        shaders_->lines.use();
        render_lines(slice.lines);
    }

    //if (!slice.units.empty()) {
    //    glLineWidth(2); //Bold outlining
    //    glEnable(GL_DEPTH_TEST);
    //    for (const auto &unit : slice.units) {
    //        render_unit(unit);
    //    }
    //    glDisable(GL_DEPTH_TEST);
    //    glLineWidth(1);
    //}
}

void Scene::render_grid() {
    if (attr_->grid_vao == 0) {
        attr_->grid_vao = mgr_->gen_vertex_array();
        GLuint vbo = mgr_->gen_buffer();

        std::vector<float> coord_line;
        const float step = 1.0f / conf_.grid_cells_count;
        for (size_t i = 0; i <= conf_.grid_cells_count; ++i) {
            coord_line.push_back(step * i);
        }

        std::vector<float> grid;
        for (float shift : coord_line) {
            grid.push_back(shift);
            grid.push_back(0.0);
            grid.push_back(0.0);

            grid.push_back(shift);
            grid.push_back(1.0);
            grid.push_back(0.0);

            grid.push_back(0);
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

void Scene::render_circle(const pod::Circle &circle) {
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

void Scene::render_rectangle(const pod::Rectangle &rect) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                     glm::vec3{rect.center.x, rect.center.y, 0.0f});
    model = glm::scale(model, glm::vec3{rect.w * 0.5, rect.h * 0.5, 1.0f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec4("color", rect.color);

    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Scene::render_lines(const std::vector<pod::Line> &lines) {
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

void Scene::render_progress_bar(const glm::vec2 up_left, float w, float h, const glm::vec4 &color) {
    auto model = glm::translate(glm::mat4(1.0f), {up_left.x, up_left.y, 0.0f});
    model = glm::scale(model, {w * 0.5f, h * 0.5f, 0.0f});
    model = glm::translate(model, {1.0f, 1.0f, 0.0f});
    shaders_->color.use();
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec4("color", color);
    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
