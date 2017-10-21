#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cgutils/Shader.h>
#include <cgutils/utils.h>
#include <viewer/UIController.h>
#include <common/logger.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <exception>
#include <vector>
#include <ctime>
#include <cgutils/ResourceManager.h>

constexpr size_t DEFAULT_WIN_WIDTH = 1200;
constexpr size_t DEFAULT_WIN_HEIGHT = 800;

GLFWwindow *setup_window();
void prepare_and_run_game_loop(GLFWwindow *window);

inline float rnd(float a, float b) {
    return a + (b - a) * (static_cast<float>(std::rand()) / RAND_MAX);
}

int main() {
    // Init GLFW
    if (glfwInit() != GL_TRUE) {
        LOG_ERROR("Failed to initialize GLFW");
        return -1;
    }

    auto window = setup_window();

    if (!gladLoadGL()) {
        LOG_ERROR("Failed to load opengl");
        return -2;
    }

    LOG_INFO("OpenGL %s, GLSL %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

#ifdef OPENGL_DEBUG
#  if (GL_ARB_debug_output)
    LOG_INFO("OpenGL:: Debug output enabled");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallbackARB(cg::debug_output_callback, nullptr);
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#  endif
#endif
    std::srand(std::time(nullptr));
    try {
        prepare_and_run_game_loop(window);
    } catch (const std::exception &e) {
        LOG_ERROR("Exception:: %s", e.what());
    }

    glfwTerminate();
    return 0;
}

GLFWwindow *setup_window() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#ifdef OPENGL_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT,
                                          "OpenGL viewer for Russian AI Cup", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, int width, int height) {
        glViewport(0, 0, width, height);
    });

    return window;
}

void render_grid(ResourceManager &res) {
    static constexpr uint16_t CELL_CNT = 100;
    static GLuint vao = 0;
    static GLsizei grid_size;
    if (vao == 0) {
        vao = res.gen_vertex_array();
        GLuint vbo = res.gen_buffer();

        std::vector<float> coord_line;
        const float step = 1.0f / CELL_CNT;
        for (size_t i = 0; i <= CELL_CNT; ++i) {
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
        grid_size = static_cast<GLsizei>(grid.size());

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, grid.size() * sizeof(float), grid.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, grid_size);
}

struct GridRenderer {
    static constexpr size_t CELL_CNT = 100;

    ~GridRenderer() {
        if (vao_) {
            glDeleteBuffers(1, &vbo_);
            glDeleteVertexArrays(1, &vao_);
        }
    }



private:
    std::vector<float> grid_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
};

void prepare_and_run_game_loop(GLFWwindow *window) {
    const float CAMERA_SPEED_PER_SECOND = 60.0;
    const glm::vec3 grid_color = {0.8f, 0.9f, 0.9f};

    Camera cam({50.0f, 10.0f, 10.0f}, {0.0, 0.0, 1.0}, 90, -30, CAMERA_SPEED_PER_SECOND);
    glfwSetCursorPosCallback(window, Camera::mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    ResourceManager res;

    UIController ui(window, &cam);

    Shader solid_color("shaders/simple.vert", "shaders/simple.frag");
    Shader circle("shaders/circle.vert", "shaders/circle.frag");
    circle.use();
    circle.set_float("radius", 0.25f);
    circle.set_vec3("color", {0.8f, 0.8f, 0.0f});

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

    //Fancy triangle ^_^
    GLuint tr_vao = res.gen_vertex_array();
    float tr_coords[] = {
        -0.5f, -1.0f, 0.1f,
        0.5f, -1.0f, 0.1f,
        0.0f, 1.0f, 0.1f,
    };
    GLuint tr_vbo = res.gen_buffer();
    glBindVertexArray(tr_vao);
    glBindBuffer(GL_ARRAY_BUFFER, tr_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tr_coords), tr_coords, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Operate UI
        ui.next_frame();

        //Non Ui related drawing
        solid_color.use();
        solid_color.set_mat4("view", cam.view());
        solid_color.set_mat4("projection", proj);

        glm::mat4 model;
        //model = glm::rotate(model, glm::radians(-90.0f), {1.0f, 0.0f, 0.0f});
        model = glm::scale(model, {100.0f, 100.0f, 0.0f});
        solid_color.set_vec3("color", grid_color);
        solid_color.set_mat4("model", model);
        render_grid(res);

        circle.use();
        circle.set_mat4("proj_view", proj * cam.view());
        glBindVertexArray(tr_vao);
        //solid_color.set_vec3("color", {0.0f, 0.0f, 1.0f});
        for (int i = 0; i < 5000; ++i) {
            auto shift = glm::vec3{rnd(0, 100), rnd(0, 100), 0.0f};
            model = glm::translate(glm::mat4(), shift);
            circle.set_mat4("model", model);
            circle.set_vec3("center", shift);
            glDrawArrays(GL_TRIANGLES, 0, 9);
        }

        // Render Ui
        glBindVertexArray(0);
        glUseProgram(0);
        ui.frame_end();

        if (ui.close_requested()) {
            glfwSetWindowShouldClose(window, true);
        }

        cam.update(ImGui::GetIO().Framerate);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}