#include "interface.h"

// ---------------- Callback Functions ----------------

static void framebufferSizeCb(GLFWwindow* w, int width, int height) {
    auto* self = static_cast<RobotInterface*>(glfwGetWindowUserPointer(w));
    if (self && self->camera) {
        self->camera->onFramebufferResize(width, height);
    }
    glViewport(0, 0, width, height);
}

static void mouseButtonCb(GLFWwindow* w, int button, int action, int) {
    auto* self = static_cast<RobotInterface*>(glfwGetWindowUserPointer(w));
    if (self && self->camera) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        self->camera->onMouseButton(button, action, x, y);
    }
}

static void cursorPosCb(GLFWwindow* w, double x, double y) {
    auto* self = static_cast<RobotInterface*>(glfwGetWindowUserPointer(w));
    if (self && self->camera) {
        self->camera->onCursorMove(x, y);
    }
}

static void scrollCb(GLFWwindow* w, double, double yoff) {
    auto* self = static_cast<RobotInterface*>(glfwGetWindowUserPointer(w));
    if (self && self->camera) {
        self->camera->onScroll(yoff);
    }
}

// ---------------- Interface Implementation ----------------

bool RobotInterface::init(const std::string& modelPath,
                          const std::string& vertShaderPath,
                          const std::string& fragShaderPath) {
    this->modelPath = modelPath;
    defaultVertShaderPath = vertShaderPath;
    defaultFragShaderPath = fragShaderPath;

    running = false;
    thread = std::thread(&RobotInterface::threadController, this);

    return true;
}

bool RobotInterface::init(const std::string& modelPath) {
    this->modelPath = modelPath;

    running = false;
    thread = std::thread(&RobotInterface::threadController, this);

    return true;
}

void RobotInterface::threadController() {
    if (!initThread()) {
        std::cerr << "Failed to initialize interface thread.\n";
        return;
    }

    while (!running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Based time on nothing
    }

    loop();
}

bool RobotInterface::initThread() {
#ifdef DEBUG_MODE
    std::cout << "Initializing interface...\n";
#endif
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#if IS_WINDOW_RESIZABLE
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#else
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#endif

    window = glfwCreateWindow(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, "Robot Interface", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window\n";
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCb);
    glfwSetMouseButtonCallback(window, mouseButtonCb);
    glfwSetCursorPosCallback(window, cursorPosCb);
    glfwSetScrollCallback(window, scrollCb);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    defaultShaderProgram = loadShaders(defaultVertShaderPath, defaultFragShaderPath);
    if (defaultShaderProgram == 0) {
        std::cerr << "Failed to load/compile default shaders\n";
        return false;
    }

    gridShaderProgram = loadShaders(gridVertShaderPath, gridFragShaderPath);
    if (gridShaderProgram == 0) {
        std::cerr << "Failed to load/compile grid shaders\n";
        return false;
    }

    std::lock_guard<std::mutex> lock(modelMutex);
    model = new Model(modelPath.c_str());
    if (!model->load()) {
        delete model;
        model = nullptr;
        std::cerr << "Failed to load model\n";
    }
    model->updateTransforms(0.0f);

    // Initialize camera
    camera = new Camera();
    auto bBox = model->getBoundingBox();
    glm::vec3 camBMin = bBox.first;
    glm::vec3 camBMax = bBox.second;
    glm::vec3 c = 0.5f * (camBMin + camBMax);
    glm::vec3 e = 0.5f * (camBMax - camBMin);
    float radius = std::max({ e.x, e.y, e.z });
    camera->target = c;
    camera->dist = std::max(2.5f * radius, 0.5f);

    glEnable(GL_DEPTH_TEST);
    glClearColor(BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1.0f);

    glEnable(GL_BLEND); // needed for grid alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    framebufferSizeCb(window, w, h);

    initGrid(999, 1.5f); // make sure to make grid size odd int so Y-axis is centered

#ifdef DEBUG_MODE
    std::cout << "Interface initialized successfully.\n";
#endif

    return true;
}

void RobotInterface::spin() {
    if (running) {
        std::cerr << "Interface is already running.\n";
        return;
    }
    running = true;
}

void RobotInterface::loop() {
#ifdef DEBUG_MODE
    std::cout << "Entering interface loop...\n";
#endif

    auto last = std::chrono::steady_clock::now();
    GLint locV, locP, locC;

    while (running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                          camera->aspect, 0.01f, 200.0f);
        glm::mat4 view = camera->view();

        glUseProgram(defaultShaderProgram);
        locV = glGetUniformLocation(defaultShaderProgram, "uView");
        locP = glGetUniformLocation(defaultShaderProgram, "uProj");
        locC = glGetUniformLocation(defaultShaderProgram, "uCameraPos");

        glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(locC, 1, glm::value_ptr(camera->eye()));

        std::lock_guard<std::mutex> lock(modelMutex);
        if (model) {
            for (auto& [jointName, jointPtr] : jointLinks) {
                model->updatePosition(jointName, jointPtr->load());
            }
            model->updateTransforms(dt);
            model->render(defaultShaderProgram);
        }

        glUseProgram(gridShaderProgram);
        locV = glGetUniformLocation(gridShaderProgram, "uView");
        locP = glGetUniformLocation(gridShaderProgram, "uProj");
        locC = glGetUniformLocation(gridShaderProgram, "uCameraPos");

        glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(locC, 1, glm::value_ptr(camera->eye()));

        drawGrid();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwWindowShouldClose(window)) {
#ifdef DEBUG_MODE
            std::cout << "Window close requested.\n";
#endif
            running = false;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::raise(SIGINT);
}

void RobotInterface::shutdown() {
#ifdef DEBUG_MODE
    std::cout << "Shutting down interface...\n";
#endif
    running = false;
    if (thread.joinable()) {
        thread.join();
    }
    if (model) {
        delete model;
        model = nullptr;
    }
    if (camera) {
        delete camera;
        camera = nullptr;
    }
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void RobotInterface::initGrid(int gridSize = 10, float spacing = 1.0f) {
    // Only generate vertices once
    if (gridVertices.empty()) {
        for (int i = -gridSize; i <= gridSize; ++i) {
            // Lines along X axis
            gridVertices.push_back({ -gridSize * spacing, i * spacing, 0.0f });
            gridVertices.push_back({ gridSize * spacing, i * spacing, 0.0f });

            // Lines along Z axis
            gridVertices.push_back({ i * spacing, -gridSize * spacing, 0.0f });
            gridVertices.push_back({ i * spacing, gridSize * spacing, 0.0f });
        }

        glGenVertexArrays(1, &gridVAO);
        glGenBuffers(1, &gridVBO);
        glBindVertexArray(gridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(glm::vec3),
                     gridVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}

void RobotInterface::drawGrid() {
    GLint locColor = glGetUniformLocation(gridShaderProgram, "uColor");

    glBindVertexArray(gridVAO);
    glLineWidth(1.0f);

    glUniform3f(locColor, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b); // Grid lines
    glDrawArrays(GL_LINES, 0, (GLsizei)(gridVertices.size() / 2));
    glLineWidth(2.0f);
    glUniform3f(locColor, 1.0f, 0.0f, 0.0f); // X Axis
    glDrawArrays(GL_LINES, (int)gridVertices.size() / 2, 2);
    glUniform3f(locColor, 0.0f, 1.0f, 0.0f); // Y Axis
    glDrawArrays(GL_LINES, (int)gridVertices.size() / 2 - 2, 2);
    glLineWidth(1.0f);

    glUniform3f(locColor, GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b); // Grid lines
    glDrawArrays(GL_LINES, (int)gridVertices.size() / 2 + 2, (GLsizei)(gridVertices.size() / 2 - 4));
    glBindVertexArray(0);
}

void RobotInterface::linkJoint(const std::string& jointName,
                               std::atomic<float>& jointPosition) {
#ifdef DEBUG_MODE
    std::cout << "Linking joint '" << jointName << "'\n";
#endif
    jointLinks[jointName] = &jointPosition;
}

unsigned int RobotInterface::loadShaders(const std::string& vertShaderPath,
                                         const std::string& fragShaderPath) {
    unsigned int shaderProgram = 0;

    std::ifstream vertFile(vertShaderPath);
    std::ifstream fragFile(fragShaderPath);
    if (!vertFile.is_open() || !fragFile.is_open())
        return 0;

    std::stringstream vertStream, fragStream;
    vertStream << vertFile.rdbuf();
    fragStream << fragFile.rdbuf();
    std::string vertSrcStr = vertStream.str();
    std::string fragSrcStr = fragStream.str();
    const char* vertSrc = vertSrcStr.c_str();
    const char* fragSrc = fragSrcStr.c_str();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSrc, nullptr);
    glCompileShader(vertShader);
    GLint success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(vertShader);
        return 0;
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSrc, nullptr);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return 0;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    if (!success) {
        glDeleteProgram(shaderProgram);
        return 0;
    }
    return shaderProgram;
}
