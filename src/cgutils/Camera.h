//
// Created by valdemar on 18.10.17.
//

#pragma once


#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    static void mouse_callback(GLFWwindow *, double xpos, double ypos);

    Camera(const glm::vec3 &initial_pos, const glm::vec3 &up, float yaw, float pitch, float move_speed);
    ~Camera() = default;

    void forward();
    void backward();
    void up();
    void down();
    void left();
    void right();

    float *view_ptr();

    void update_speed(float fps);

private:
    void mouse_move(double xpos, double ypos);

    glm::vec3 pos_;
    glm::vec3 up_dir_;
    glm::vec3 front_dir_;
    float yaw_;
    float pitch_;
    float des_move_speed_;
    float move_per_frame_;
    glm::vec3 fake_up_;

    glm::mat4 view_;

    //Previous position
    double xprev_;
    double yprev_;
    bool first_capture_ = false;
};




