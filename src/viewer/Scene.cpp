#include "Scene.h"

#include <cgutils/utils.h>
#include <cgutils/Shader.h>

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Scene::render_attrs_t {
    GLuint grid_vao = 0;
    GLuint grid_vertex_count = 0;
    //Vertex array to draw any rectangle and circle
    GLuint rect_vao = 0;
    //Lines designed to dynamic draw
    GLuint lines_vao = 0;
    GLuint uniform_buf;
    GLuint grass_tex = 0;
    glm::mat4 grid_model;
};

struct Scene::shaders_t {
    shaders_t()
        : color("resources/shaders/simple.vert", "resources/shaders/uniform_color.frag")
        , circle("resources/shaders/circle.vert", "resources/shaders/circle.frag")
        , lines("resources/shaders/lines.vert", "resources/shaders/lines.frag")
        , textured("resources/shaders/simple.vert", "resources/shaders/textured.frag") {
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

Scene::Scene(ResourceManager *res)
    : mgr_(res) {
    attr_ = std::make_unique<render_attrs_t>();
    //Init needed attributes
    attr_->grid_model = glm::scale(glm::mat4{}, {opt_.grid_dim.x, opt_.grid_dim.y, 0.0f});

    //Shaders
    shaders_ = std::make_unique<shaders_t>();

    //Load textures
    attr_->grass_tex = mgr_->load_texture("resources/textures/grass_seamless.jpg", false, GL_REPEAT, GL_REPEAT);

    //Unit textures
    unit2tex_[Frame::UnitType::helicopter] = mgr_->load_texture("resources/textures/helicopter.png", false);

    //Preload rectangle to memory for further drawing
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
    attr_->uniform_buf = mgr_->gen_buffer();
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, 64, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, attr_->uniform_buf);

    shaders_->color.bind_uniform_block("Matrix", 0);
    shaders_->circle.bind_uniform_block("Matrix", 0);
    shaders_->lines.bind_uniform_block("Matrix", 0);
    shaders_->textured.bind_uniform_block("Matrix", 0);
}

Scene::~Scene() = default;

void Scene::render(const glm::mat4 &proj_view) {
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), glm::value_ptr(proj_view), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    shaders_->color.use();
    shaders_->color.set_mat4("model", attr_->grid_model);
    shaders_->color.set_vec3("color", opt_.grid_color);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, attr_->grass_tex);
    render_grid();

    shaders_->textured.use();
    auto model = glm::scale(glm::mat4(1.0f), {opt_.grid_dim * 0.5f, -1.0f});
    model = glm::translate(model, {1.0f, 1.0f, 0.0f});
    shaders_->textured.set_mat4("model", model);
    shaders_->textured.set_vec2("tex_scale", {10, 10});
    shaders_->textured.set_vec3("color", glm::vec3(0.6));
    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (!frames_.empty()) {
        const Frame *frame = frames_[cur_frame_idx_].get();
        render_frame(*frame);
    }
}

void Scene::add_frame(std::unique_ptr<Frame> &&frame) {
    //Very careful with that call from other thread
    frames_.emplace_back(std::move(frame));
}

void Scene::render_frame(const Frame &frame) {
    if (!frame.circles.empty()) {
        shaders_->circle.use();
        shaders_->circle.set_int("textured", 0);
        for (const auto &obj : frame.circles) {
            render_circle(obj);
        }
    }

    if (!frame.rectangles.empty()) {
        shaders_->color.use();
        for (const auto &obj : frame.rectangles) {
            render_rectangle(obj);
        }
    }

    if (!frame.lines.empty()) {
        shaders_->lines.use();
        render_lines(frame.lines);
    }

    glLineWidth(2); //Bold outlining
    for (const auto &unit : frame.units) {
        render_unit(unit);
    }
    glLineWidth(1);

#ifndef NDEBUG
    ImGui::LabelText("Circles", "%zu", frame.circles.size());
    ImGui::LabelText("Rectangles", "%zu", frame.rectangles.size());
    ImGui::LabelText("Lines", "%zu", frame.lines.size());
    ImGui::LabelText("Units", "%zu", frame.units.size());
#endif
}

void Scene::render_grid() {
    if (attr_->grid_vao == 0) {
        attr_->grid_vao = mgr_->gen_vertex_array();
        GLuint vbo = mgr_->gen_buffer();

        std::vector<float> coord_line;
        const float step = 1.0f / opt_.grid_cells_count;
        for (size_t i = 0; i <= opt_.grid_cells_count; ++i) {
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
        attr_->grid_vertex_count = static_cast<GLsizei>(grid.size());

        glBindVertexArray(attr_->grid_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, grid.size() * sizeof(float), grid.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    }

    glBindVertexArray(attr_->grid_vao);
    glDrawArrays(GL_LINES, 0, attr_->grid_vertex_count);
}

void Scene::render_circle(const pod::Circle &circle) {
    auto vcenter = glm::vec3{circle.center.x, circle.center.y, 0.1f};
    glm::mat4 model = glm::translate(glm::mat4(1.0f), vcenter);
    model = glm::scale(model, glm::vec3{circle.radius, circle.radius, 0.0f});
    shaders_->circle.set_float("radius2", circle.radius * circle.radius);
    shaders_->circle.set_vec3("center", vcenter);
    shaders_->circle.set_vec3("color", circle.color);
    shaders_->circle.set_mat4("model", model);

    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Scene::render_rectangle(const pod::Rectangle &rect) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{rect.center.x, rect.center.y, 0.1f});
    model = glm::scale(model, glm::vec3{rect.w * 0.5, rect.h * 0.5, 0.0f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec3("color", rect.color);

    glBindVertexArray(attr_->rect_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
}

void Scene::render_lines(const std::vector<pod::Line> &lines) {
    if (attr_->lines_vao == 0) {
        attr_->lines_vao = mgr_->gen_vertex_array();
        GLuint vbo = mgr_->gen_buffer();

        glBindVertexArray(attr_->lines_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        //Plain float format: vec3 color, vec2 pos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), cg::offset<float>(3));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    glBindVertexArray(attr_->lines_vao);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(pod::Line), lines.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() * 2));
}

void Scene::render_unit(const pod::Unit &unit) {
    //Circle
    shaders_->circle.use();
    shaders_->circle.set_float("radius2", unit.radius * unit.radius);
    shaders_->circle.set_vec3("color", unit.color);

    auto vcenter = glm::vec3{unit.center.x, unit.center.y, 0.1f};
    glm::mat4 model = glm::translate(glm::mat4(1.0f), vcenter);
    if (unit.utype != Frame::UnitType::undefined) {
        shaders_->circle.set_int("textured", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, unit2tex_[unit.utype]);
        static const float tex_compensation = glm::radians(90.0f); //Because texture by default aligned to Y axes
        model = glm::rotate(model, unit.course - tex_compensation, {0.0f, 0.0f, 1.0f});
    } else {
        shaders_->circle.set_int("textured", 0);
    }
    model = glm::scale(model, glm::vec3{unit.radius, unit.radius, 0.0f});
    shaders_->circle.set_vec3("center", vcenter);
    shaders_->circle.set_mat4("model", model);

    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //HP bar
    float hp_length = static_cast<float>(cg::lerp(unit.hp, 0, unit.max_hp, 0, unit.radius));
    model = glm::translate(glm::mat4(1.0f), {vcenter.x - unit.radius, vcenter.y + unit.radius * 1.1, vcenter.z + 0.1});
    model = glm::scale(model, {hp_length, std::max(unit.radius * 0.06, 1.0), 0.0f});
    model = glm::translate(model, {1.0f, 0.0f, 0.0f});
    float color_shift = static_cast<float>(unit.hp) / unit.max_hp;
    glm::vec3 color{1.0f - color_shift, color_shift, 0.0};
    auto m = 1.0f / std::max(color.r, color.g);
    shaders_->color.use();
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec3("color", color * m);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //Hp bar outlining
    model = glm::translate(glm::mat4(1.0f), {vcenter.x - unit.radius, vcenter.y + unit.radius * 1.1, vcenter.z + 0.1});
    model = glm::scale(model, {unit.radius, std::max(unit.radius * 0.06, 1.0), 0.0f});
    model = glm::translate(model, {1.0f, 0.0f, 0.0f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec3("color", glm::vec3(0.0f));
    const uint8_t indicies[] = {0, 1, 3, 2};
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, indicies);
}

void Scene::set_frame_index(int idx) {
    if (idx >= 0 && idx < get_frames_count() && idx != cur_frame_idx_) {
        cur_frame_idx_ = idx;
    }
}

int Scene::get_frame_index() {
    return cur_frame_idx_;
}

int Scene::get_frames_count() {
    return static_cast<int>(frames_.size());
}

const char *Scene::get_frame_user_message() {
    if (cur_frame_idx_ >= 0 && cur_frame_idx_ < static_cast<int>(frames_.size())) {
        return frames_[cur_frame_idx_]->user_message.c_str();
    }
    return "";
}



