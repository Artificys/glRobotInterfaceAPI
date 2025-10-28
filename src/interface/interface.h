#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <atomic>
#include <csignal>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "core/camera.h"
#include "core/model.h"

#define BACKGROUND_COLOR glm::vec3(0.25f, 0.25f, 0.25f)
#define GRID_COLOR glm::vec3(0.4f, 0.4f, 0.4f)
#define INITIAL_WINDOW_WIDTH 1280
#define INITIAL_WINDOW_HEIGHT 720
#define IS_WINDOW_RESIZABLE true

// Namespace/project name RDBugI - Robot Debugging Interface

/**
 * @brief The RobotInterface class manages the OpenGL rendering window, shaders, and
 * user interaction.
 *
 * It handles initialization and shutdown of the rendering context, loading
 * models and shaders, drawing grids and datum axes, and linking joint positions
 * for interactive control. The class supports multi-threaded rendering and
 * provides camera manipulation.
 */
class RobotInterface {
public:
    RobotInterface()
        : window(nullptr)
        , defaultShaderProgram(0)
        , gridShaderProgram(0)
        , model(nullptr)
        , camera(nullptr) {}
    ~RobotInterface() { shutdown(); }

    bool init(const std::string& modelPath,
              const std::string& vertShaderPath,
              const std::string& fragShaderPath);
    bool init(const std::string& modelPath);
    void spin();
    void shutdown();

    void linkJoint(const std::string& jointName,
                   std::atomic<float>& jointPosition);

    void updateShaderPaths(const std::string& vertShaderPath,
                           const std::string& fragShaderPath) {
        defaultVertShaderPath = vertShaderPath;
        defaultFragShaderPath = fragShaderPath;
    }

    Camera* camera = nullptr;

private:
    GLFWwindow* window = nullptr;
    unsigned int defaultShaderProgram = 0;
    unsigned int gridShaderProgram = 0;

    std::thread thread;
    std::atomic<bool> running = false;
    std::mutex modelMutex;

    std::string modelPath;
    std::string defaultVertShaderPath = "shaders/default.vert.glsl";
    std::string defaultFragShaderPath = "shaders/default.frag.glsl";
    std::string gridVertShaderPath = "shaders/grid.vert.glsl";
    std::string gridFragShaderPath = "shaders/grid.frag.glsl";

    Model* model = nullptr;
    std::map<std::string, std::atomic<float>*> jointLinks;

    unsigned int gridVAO = 0, gridVBO = 0;
    std::vector<glm::vec3> gridVertices;
    unsigned int datumVAO = 0, datumVBO = 0;
    std::vector<glm::vec3> datumVertices;

    bool initThread();
    void loop();
    unsigned int loadShaders(const std::string& vertShaderPath,
                             const std::string& fragShaderPath);
    void threadController();
    void initGrid(int gridSize, float spacing);
    void drawGrid();
};