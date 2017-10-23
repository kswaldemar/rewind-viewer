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
{
    attr_ = std::make_unique<render_attrs_t>();
    //Init needed attributes
    attr_->grid_model = glm::scale(glm::mat4{}, {opt_.grid_dim.x, opt_.grid_dim.y, 0.0f});
}

Scene::~Scene() = default;

void Scene::render(const glm::mat4 &view, const glm::mat4 &proj) {
    glClearColor(opt_.clear_color.r, opt_.clear_color.g, opt_.clear_color.b, 1.0f);

    color_sh_.use();
    color_sh_.set_mat4("projection", proj);
    color_sh_.set_mat4("view", view);

    color_sh_.set_mat4("model", attr_->grid_model);
    color_sh_.set_vec3("color", opt_.grid_color);
    render_grid();

    color_sh_.set_vec3("color", {0.0f, 0.0f, 1.0f});
    color_sh_.set_mat4(
        "model", glm::translate(glm::mat4{},
                                glm::vec3{opt_.fancy_triangle_pos_.x, opt_.fancy_triangle_pos_.y, 0.01f})
    );
    render_fancy_triangle();
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

void Scene::set_frame_index(int idx) {
    if (idx >= 0 && idx < get_frames_count() && idx != cur_frame_idx_) {
        cur_frame_idx_ = idx;
    }
}

int Scene::get_frame_index() {
    return cur_frame_idx_;
}

int Scene::get_frames_count() {
    return static_cast<int>(frames_.size()) + 20000;
}

const char *Scene::get_frame_user_message() {
    static std::string ret;
    if (ret.empty()) {
        for (int i = 0; i < 1000; ++i) {
            ret += "Sample message\nMay span multiple lines";
        }
    }
    return ret.c_str();
}


