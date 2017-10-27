#include "Scene.h"

#include <cgutils/Shader.h>

#include <glm/gtc/matrix_transform.hpp>

struct Scene::render_attrs_t {
    GLuint grid_vao = 0;
    GLuint grid_vertex_count = 0;
    glm::mat4 grid_model;
};

Scene::Scene(ResourceManager *res)
    : rsm_(res)
    , color_sh_("shaders/simple.vert", "shaders/simple.frag")
    , circle_sh_("shaders/circle.vert", "shaders/circle.frag")
{
    attr_ = std::make_unique<render_attrs_t>();
    //Init needed attributes
    attr_->grid_model = glm::scale(glm::mat4{}, {opt_.grid_dim.x, opt_.grid_dim.y, 0.0f});

    //Preload rectangle to memory for further drawing
    rect_vao_ = rsm_->gen_vertex_array();
    GLuint vbo = rsm_->gen_buffer();
    const float points[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };
    glBindVertexArray(rect_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

Scene::~Scene() = default;

void Scene::render(const glm::mat4 &proj_view) {
    glClearColor(opt_.clear_color.r, opt_.clear_color.g, opt_.clear_color.b, 1.0f);

    color_sh_.use();
    color_sh_.set_mat4("proj_view", proj_view);
    color_sh_.set_mat4("model", attr_->grid_model);
    color_sh_.set_vec3("color", opt_.grid_color);
    render_grid();

    color_sh_.set_vec3("color", {0.0f, 0.0f, 1.0f});
    color_sh_.set_mat4(
        "model", glm::translate(glm::mat4{},
                                glm::vec3{opt_.fancy_triangle_pos_.x, opt_.fancy_triangle_pos_.y, 0.01f})
    );
    render_fancy_triangle();

    if (!frames_.empty()) {
        circle_sh_.use();
        circle_sh_.set_mat4("proj_view", proj_view);

        const Frame *frame = frames_[cur_frame_idx_].get();
        render_frame(*frame);
    }
}

void Scene::add_frame(std::unique_ptr<Frame> &&frame) {
    //TODO: Is that thread safe?!
    frames_.emplace_back(std::move(frame));
}

void Scene::render_frame(const Frame &frame) {
    if (!frame.circles.empty()) {
        circle_sh_.use();
        for (const auto &obj : frame.circles) {
            render_circle(obj);
        }
    }
}

void Scene::render_grid() {
    if (attr_->grid_vao == 0) {
        attr_->grid_vao = rsm_->gen_vertex_array();
        GLuint vbo = rsm_->gen_buffer();

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

void Scene::render_fancy_triangle() {
    static GLuint vao = 0;
    if (vao == 0) {
        //Fancy triangle ^_^
        vao = rsm_->gen_vertex_array();
        float tr_coords[] = {
            -0.5f, -1.0f, 0.0f,
            0.5f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
        };
        GLuint vbo = rsm_->gen_buffer();
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tr_coords), tr_coords, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Scene::render_circle(const pod::Circle &circle) {
    glBindVertexArray(rect_vao_);
    glm::mat4 model;
    auto vcenter = glm::vec3{circle.center.x, circle.center.y, 0.1f};
    model = glm::translate(model, vcenter);
    model = glm::scale(model, glm::vec3{circle.radius, circle.radius, 0.0f});
    circle_sh_.set_float("radius", circle.radius);
    circle_sh_.set_vec3("center", vcenter);
    circle_sh_.set_vec3("color", circle.color);
    circle_sh_.set_mat4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, 18);
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
    if (cur_frame_idx_ >= 0 && cur_frame_idx_ < frames_.size()) {
        return frames_[cur_frame_idx_]->user_message.c_str();
    }
    return "";
}


