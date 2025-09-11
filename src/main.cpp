#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::perspective, lookAt, translate, etc.
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

// Config janela
static int gWindowWidth = 1280;
static int gWindowHeight = 720;

// Camera
static glm::vec3 cameraPos{0.0f, 1.8f, 5.0f}; // "altura dos olhos"
static glm::vec3 cameraFront{0.0f, 0.0f, -1.0f};
static glm::vec3 cameraUp{0.0f, 1.0f, 0.0f};
static float yaw = -90.0f; // olhando para -Zglm
static float pitch = 0.0f;
static bool firstMouse = true;
static double lastX = gWindowWidth / 2.0;
static double lastY = gWindowHeight / 2.0;
static float mouseSensitivity = 0.1f;
static float moveSpeed = 5.0f; // m/s

static bool cursorDisabled = true;

// Timing
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

// Shader helpers
static void checkCompile(GLuint obj, bool program, const char* label) {
    GLint success = 0;
    if (!program) {
        glGetShaderiv(obj, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint len = 0; glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(obj, len, nullptr, log.data());
            std::cerr << "ERRO shader (" << label << "):\n" << log << std::endl;
        }
    } else {
        glGetProgramiv(obj, GL_LINK_STATUS, &success);
        if (!success) {
            GLint len = 0; glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(obj, len, nullptr, log.data());
            std::cerr << "ERRO link program (" << label << "):\n" << log << std::endl;
        }
    }
}

static GLuint makeShaderProgram(const char* vsSrc, const char* fsSrc) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);
    checkCompile(vs, false, "VS");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);
    checkCompile(fs, false, "FS");

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    checkCompile(prog, true, "Program");

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    gWindowWidth = w; gWindowHeight = h;
    glViewport(0, 0, w, h);
}

static void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (!cursorDisabled) return; // ignore quando cursor visível
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos); // invertido
    lastX = xpos; lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw   += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(dir);
}

static void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    float velocity = moveSpeed * deltaTime;
    glm::vec3 forward = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z)); // sem subir/descer
    glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += forward * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= forward * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += right * velocity;

    // Alternar captura do cursor (apertar C)
    static bool cWasPressed = false;
    bool cPressed = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if (cPressed && !cWasPressed) {
        cursorDisabled = !cursorDisabled;
        glfwSetInputMode(window, GLFW_CURSOR, cursorDisabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true; // evita salto
    }
    cWasPressed = cPressed;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(gWindowWidth, gWindowHeight, "Walking Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    const char* vs = R"( #version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMVP;
out float vY;
void main(){
    vY = aPos.y;
    gl_Position = uMVP * vec4(aPos,1.0);
})";

    const char* fs = R"( #version 330 core
in float vY;
out vec4 FragColor;
void main(){
    // tom simples cinza / levemente azulado
    FragColor = vec4(0.55,0.6,0.65,1.0);
})";

    GLuint program = makeShaderProgram(vs, fs);

    // Gera um grid (linhas) no plano XZ em y=0
    std::vector<float> gridVertices; // cada vértice 3 floats
    const int half = 50; // grid de -50 a 50 (101 linhas por eixo)
    for (int i = -half; i <= half; ++i) {
        // linhas paralelas a Z (varia X)
        gridVertices.push_back((float)i); gridVertices.push_back(0.0f); gridVertices.push_back((float)-half);
        gridVertices.push_back((float)i); gridVertices.push_back(0.0f); gridVertices.push_back((float)half);
        // linhas paralelas a X (varia Z)
        gridVertices.push_back((float)-half); gridVertices.push_back(0.0f); gridVertices.push_back((float)i);
        gridVertices.push_back((float)half);  gridVertices.push_back(0.0f); gridVertices.push_back((float)i);
    }
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(gridVertices.size()*sizeof(float)), gridVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glBindVertexArray(0);

    auto startTime = (float)glfwGetTime();
    int frameCount = 0;
    float fpsTimer = 0.f;

    while (!glfwWindowShouldClose(window)) {
        float current = (float)glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;

        processInput(window);

        glClearColor(0.08f,0.09f,0.11f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 proj = glm::perspective(glm::radians(70.0f), (float)gWindowWidth/(float)gWindowHeight, 0.1f, 500.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;

        glUseProgram(program);
        glUniformMatrix4fv(glGetUniformLocation(program, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(vao);
        // Cada linha = 2 vértices; temos 2*(2*half+1) linhas por eixo -> já duplicado no push
        GLsizei totalVertices = (GLsizei)gridVertices.size()/3;
        glDrawArrays(GL_LINES, 0, totalVertices);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // FPS no título
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            float fps = frameCount / fpsTimer;
            std::string title = "Walking Simulator | FPS: " + std::to_string((int)fps);
            glfwSetWindowTitle(window, title.c_str());
            frameCount = 0; fpsTimer = 0.f;
        }
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}