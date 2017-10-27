#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cgutils/Shader.h>
#include <cgutils/utils.h>
#include <cgutils/ResourceManager.h>
#include <viewer/UIController.h>
#include <viewer/Scene.h>
#include <common/logger.h>
#include <net/NetListener.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <exception>
#include <vector>
#include <ctime>
#include <thread>

constexpr size_t DEFAULT_WIN_WIDTH = 1200;
constexpr size_t DEFAULT_WIN_HEIGHT = 800;

constexpr const char *WINDOW_TITLE = "Rewind viewer for Russian AI Cup";

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

    GLFWwindow *window = glfwCreateWindow(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, int width, int height) {
        glViewport(0, 0, width, height);
    });

    return window;
}

void prepare_and_run_game_loop(GLFWwindow *window) {
    Camera cam({0.0f, 0.0f}, 1000);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    ResourceManager res;
    Scene scene(&res);
    UIController ui(&cam);

    //Start network listening
    NetListener net(&scene, "127.0.0.1", 7000);
    std::thread network_thread([&net]{
        net.run();
    });
    network_thread.detach();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Updates
        ui.next_frame(&scene);
        cam.update();

        if (ui.close_requested()) {
            glfwSetWindowShouldClose(window, true);
        }

        //Non Ui related drawing
        scene.render(cam.proj_view());

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