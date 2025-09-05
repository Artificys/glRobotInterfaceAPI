#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>


struct Camera {
    // ---- Core state ----
    float yaw = 30.f;
    float pitch = 20.f; // height/angle from XY plane
    float dist = 2.5f; // radius from target; CHANGE TO RADIUS OR R
    float aspect = 1.0f;

    glm::vec3 target{0.0f};

    // ---- Input state ----
    bool rotating = false;
    bool panning = false;
    double lastX = 0.0, lastY = 0.0;

    // ---- Math ----
    glm::mat4 view() const {
        float cy = glm::radians(yaw);
        float cp = glm::radians(pitch);
        glm::vec3 eye{
            dist * cos(cp) * cos(cy),
            dist * cos(cp) * sin(cy),
            dist * sin(cp)
        };
        return glm::lookAt(eye + target, target, glm::vec3(0, 0, 1));
    }

    glm::vec3 eye() const {
        float cy = glm::radians(yaw);
        float cp = glm::radians(pitch);
        return glm::vec3(
            dist * cos(cp) * cos(cy),
            dist * cos(cp) * sin(cy),
            dist * sin(cp)
        ) + target;
    }

    // ---- Input handling ----
    void onFramebufferResize(int width, int height) {
        if (height > 0)
            aspect = float(width) / float(height);
    }

    void onMouseButton(int button, int action, double x, double y) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            rotating = (action == GLFW_PRESS);
            lastX = x; lastY = y;
        } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            panning = (action == GLFW_PRESS);
            lastX = x; lastY = y;
        }
    }

    void onCursorMove(double x, double y) {
        float dx = float(x - lastX);
        float dy = float(y - lastY);
        lastX = x; lastY = y;

        if (rotating) {
            yaw -= dx * 0.25f;
            pitch += dy * 0.25f;
            pitch = glm::clamp(pitch, -89.0f, 89.0f);
        } else if (panning) {
            float cy = glm::radians(yaw);
            float cp = glm::radians(pitch);
            glm::vec3 forward{
                cos(cp) * cos(cy),
                cos(cp) * sin(cy),
                sin(cp)
            };
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 0, 1)));
            glm::vec3 up = glm::normalize(glm::cross(right, forward));

            float panSpeed = 0.001f * dist;
            glm::vec3 offset = dx * panSpeed * right + dy * panSpeed * up;
            target += offset;
        }
    }

    void onScroll(double yoff) {
        dist *= (yoff > 0 ? 0.9f : 1.1111f);
        dist = glm::clamp(dist, 0.2f, 50.0f);
    }
};
