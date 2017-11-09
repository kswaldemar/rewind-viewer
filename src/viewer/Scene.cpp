#include "Scene.h"

#include <cgutils/utils.h>
#include <cgutils/Shader.h>
#include <common/logger.h>

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

bool hittest(const glm::vec2 &wmouse, const pod::Unit &unit) {
    auto d = wmouse - unit.center;
    return (d.x * d.x + d.y * d.y) <= unit.radius * unit.radius;
}

const std::map<int, const char *> side2str = {
    {-1, "Ally"},
    {0,  "Neutral"},
    {1,  "Enemy"},
};

} // anonymous namespace

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
    LOG_INFO("Initialize needed attributes")
    attr_ = std::make_unique<render_attrs_t>();
    //Init needed attributes
    attr_->grid_model = glm::scale(glm::mat4{}, {opt_.grid_dim.x, opt_.grid_dim.y, 0.0f});

    //Shaders
    LOG_INFO("Compile shaders")
    shaders_ = std::make_unique<shaders_t>();

    //Load textures
    LOG_INFO("Load background texture")
    attr_->grass_tex = mgr_->load_texture("resources/textures/grass.png", false, GL_REPEAT, GL_REPEAT);

    //Unit textures
    LOG_INFO("Load unit textures")
    unit2tex_[Frame::UnitType::TANK] = mgr_->load_texture("resources/textures/tank.png", false);
    unit2tex_[Frame::UnitType::IFV] = mgr_->load_texture("resources/textures/ifv.png", false);
    unit2tex_[Frame::UnitType::ARRV] = mgr_->load_texture("resources/textures/arrv.png", false);
    unit2tex_[Frame::UnitType::HELICOPTER] = mgr_->load_texture("resources/textures/helicopter.png", false);
    unit2tex_[Frame::UnitType::FIGHTER] = mgr_->load_texture("resources/textures/fighter.png", false);

    //AreaDesc textures
    terrain2tex_[Frame::AreaType::FOREST] = mgr_->load_texture("resources/textures/forest.png",
                                                               true, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_NEAREST);
    terrain2tex_[Frame::AreaType::SWAMP] = mgr_->load_texture("resources/textures/swamp.png",
                                                              true, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_NEAREST);
    terrain2tex_[Frame::AreaType::CLOUD] = mgr_->load_texture("resources/textures/clouds.png",
                                                              true, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_NEAREST);
    terrain2tex_[Frame::AreaType::RAIN] = mgr_->load_texture("resources/textures/rain.png",
                                                             true, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_NEAREST);

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
            active_frame_ = frames_[cur_frame_idx_].get();
        }
    }

    //Update projection matrix
    glBindBuffer(GL_UNIFORM_BUFFER, attr_->uniform_buf);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), glm::value_ptr(proj_view), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //Grid
    if (opt_.draw_grid) {
        //TODO: Rendering garbage lines if disabled by default
        shaders_->color.use();
        shaders_->color.set_mat4("model", attr_->grid_model);
        shaders_->color.set_vec3("color", opt_.grid_color);
        render_grid();
    }

    //Main grass texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, attr_->grass_tex);
    shaders_->textured.use();
    auto model = glm::scale(glm::mat4(1.0f), {opt_.grid_dim * 0.5f, 1.0f});
    model = glm::translate(model, {1.0f, 1.0f, -0.2f});
    shaders_->textured.set_mat4("model", model);
    shaders_->textured.set_vec2("tex_scale", glm::vec2(opt_.grid_cells_count));
    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //AreaDesc
    render_terrain();

    //Frame
    if (!frames_.empty()) {
        const Frame *frame = frames_[cur_frame_idx_].get();
        render_frame(*frame);
    }
}

void Scene::add_frame(std::unique_ptr<Frame> &&frame) {
    std::lock_guard<std::mutex> f(frames_mutex_);
    //Sort units for proper draw order
    std::sort(frame->units.begin(), frame->units.end(), [](const pod::Unit &lhs, const pod::Unit &rhs) {
        return lhs.utype < rhs.utype;
    });
    frames_.emplace_back(std::move(frame));
}

void Scene::add_area_description(pod::AreaDesc area) {
    std::lock_guard<std::mutex> f(terrain_mutex_);
    terrains_.emplace_back(area);
}

void Scene::show_detailed_info(const glm::vec2 &mouse) const {
    if (!opt_.show_detailed_info_on_hover || frames_.empty()) {
        return;
    }

    const Frame &frame = *frames_[cur_frame_idx_].get();
    for (const auto &unit : frame.units) {
        if (hittest(mouse, unit)) {
            ImGui::BeginTooltip();
            ImGui::Text(
                "%s %s:"
                    "\nHP: %d / %d"
                    "\nPosition: %0.3lf, %0.3lf"
                    "\nCooldown: %d (%d)"
                    "\nSelected: %s",
                side2str.at(unit.enemy),
                Frame::unit_name(unit.utype),
                unit.hp, unit.max_hp,
                unit.center.x, unit.center.y,
                unit.rem_cooldown, unit.cooldown,
                unit.selected ? "yes" : "no"
            );
            ImGui::EndTooltip();
        }
    }
}

void Scene::render_terrain() {
    std::lock_guard<std::mutex> f(terrain_mutex_);
    if (terrains_.empty()) {
        return;
    }

    const auto cell_dim = opt_.grid_dim / static_cast<float>(opt_.grid_cells_count);
    shaders_->textured.use();
    shaders_->textured.set_vec2("tex_scale", glm::vec2(1.0f, y_axes_invert_));
    glBindVertexArray(attr_->rect_vao);
    for (const auto &tm : terrains_) {
        float z = -0.1f;
        if (tm.type == Frame::AreaType::RAIN || tm.type == Frame::AreaType::CLOUD) {
            z += 0.05f;
        }

        auto model = glm::translate(glm::mat4(1.0), {cell_dim.x * tm.x, cell_dim.y * tm.y, z});
        model = glm::scale(model, glm::vec3(cell_dim * 0.5f, 0.0f));
        model = glm::translate(model, {1.0f, 1.0f, 0.0f});
        shaders_->textured.set_mat4("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, terrain2tex_[tm.type]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
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
    glDisable(GL_DEPTH_TEST);
    for (const auto &unit : frame.units) {
        render_unit(unit);
    }
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1);
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
        glBindVertexArray(0);
    }
    glBindVertexArray(attr_->grid_vao);
    glDrawArrays(GL_LINES, 0, attr_->grid_vertex_count);
    //glBindVertexArray(0);
}

void Scene::render_circle(const pod::Circle &circle) {
    auto vcenter = glm::vec3{circle.center.x, circle.center.y, 0.1f};
    glm::mat4 model = glm::translate(glm::mat4(1.0f), vcenter);
    model = glm::scale(model, glm::vec3{circle.radius, circle.radius, 1.0f});
    shaders_->circle.set_float("radius2", circle.radius * circle.radius);
    shaders_->circle.set_vec3("center", vcenter);
    shaders_->circle.set_vec3("color", circle.color);
    shaders_->circle.set_mat4("model", model);

    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Scene::render_rectangle(const pod::Rectangle &rect) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{rect.center.x, rect.center.y, 0.1f});
    model = glm::scale(model, glm::vec3{rect.w * 0.5, rect.h * 0.5, 1.0f});
    shaders_->color.set_mat4("model", model);
    shaders_->color.set_vec3("color", rect.color);

    glBindVertexArray(attr_->rect_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
    if (unit.selected) {
        shaders_->circle.set_vec3("color", opt_.selected_unit_color);
    } else if (unit.enemy == 1) {
        shaders_->circle.set_vec3("color", opt_.enemy_unit_color);
    } else if (unit.enemy == -1) {
        shaders_->circle.set_vec3("color", opt_.ally_unit_color);
    } else {
        shaders_->circle.set_vec3("color", opt_.neutral_unit_color);
    }

    auto vcenter = glm::vec3{unit.center.x, unit.center.y, 0.1f};
    glm::mat4 model = glm::translate(glm::mat4(1.0f), vcenter);
    if (unit.utype != Frame::UnitType::UNKNOWN) {
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

    const glm::vec3 bar_shift{vcenter.x - unit.radius,
                              vcenter.y + unit.radius * 1.15 * y_axes_invert_,
                              vcenter.z + 0.01};
    const float hp_bar_height = std::max(unit.radius * 0.10f, 0.1f);
    if (opt_.show_full_hp_bars || unit.hp != unit.max_hp) {
        //HP bar
        float hp_length = static_cast<float>(cg::lerp(unit.hp, 0, unit.max_hp, 0, unit.radius));
        model = glm::translate(glm::mat4(1.0f), bar_shift);
        model = glm::scale(model, {hp_length, hp_bar_height, 0.0f});
        model = glm::translate(model, {1.0f, 0.0f, 0.0f});
        float color_shift = static_cast<float>(unit.hp) / unit.max_hp;
        glm::vec3 color{1.0f - color_shift, color_shift, 0.0};
        auto m = 1.0f / std::max(color.r, color.g);
        shaders_->color.use();
        shaders_->color.set_mat4("model", model);
        shaders_->color.set_vec3("color", color * m);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //Hp bar outlining
        model = glm::translate(glm::mat4(1.0f), bar_shift);
        model = glm::scale(model, {unit.radius, hp_bar_height, 0.0f});
        model = glm::translate(model, {1.0f, 0.0f, 0.0f});
        shaders_->color.set_mat4("model", model);
        shaders_->color.set_vec3("color", glm::vec3(0.0f));
        const uint8_t indicies[] = {0, 1, 3, 2};
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, indicies);
    }
    if (opt_.show_cooldown_bars && unit.cooldown > 0) {
        model = glm::translate(glm::mat4(1.0f),
                               {bar_shift.x, bar_shift.y - hp_bar_height, bar_shift.z});
        const double cooldown_fraction = cg::lerp(unit.rem_cooldown, 0, unit.cooldown, 0, unit.radius);
        model = glm::scale(model, {cooldown_fraction, hp_bar_height * 0.5f, 0.0f});
        model = glm::translate(model, {1.0f, 1.0f, 0.0f});
        shaders_->color.set_mat4("model", model);
        shaders_->color.set_vec3("color", glm::vec3(0.5));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void Scene::set_frame_index(int idx) {
    if (idx >= 0 && idx < frames_count_ && idx != cur_frame_idx_) {
        cur_frame_idx_ = idx;
    }
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



