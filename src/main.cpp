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
#include <viewer/Scene.h>

constexpr size_t DEFAULT_WIN_WIDTH = 1200;
constexpr size_t DEFAULT_WIN_HEIGHT = 800;

GLFWwindow *setup_window();
void prepare_and_run_game_loop(GLFWwindow *window);

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

void prepare_and_run_game_loop(GLFWwindow *window) {
    const float CAMERA_SPEED_PER_SECOND = 60.0;

    Camera cam({0.0f, 0.0f, 20.0f}, {0.0, 0.0, 1.0}, 120, -50, CAMERA_SPEED_PER_SECOND);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    ResourceManager res;
    Scene scene(&res);
    UIController ui(window, &cam);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Updates
        ui.next_frame();
        const auto &io = ImGui::GetIO();
        cam.update(io.Framerate, io.MouseWheel);

        if (ui.close_requested()) {
            glfwSetWindowShouldClose(window, true);
        }

        //Playback control

        //Non Ui related drawing
        scene.render(cam.view(), proj);

        // Cleanup opengl state
        glBindVertexArray(0);
        glUseProgram(0);

        // Render UI
        ui.frame_end();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}