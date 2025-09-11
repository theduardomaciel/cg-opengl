#include "core/Application.h"
#include <iostream>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cg {
    Application::~Application() {
        if (mInitialized) {
            glfwTerminate();
            mInitialized = false;
        }
    }

    bool Application::init() {
        // Sequência de inicialização ordenada
        if (!initGLFW()) return false;
        if (!initWindow()) return false;
        if (!initGLAD()) return false;
        if (!initScene()) return false;
        if (!initSystems()) return false;

        // Configuração de VSync
        if (mConfig.vsync) glfwSwapInterval(1);
        else glfwSwapInterval(0);

        mInitialized = true;
        return true;
    }

    bool Application::initGLFW() {
        if (!glfwInit()) {
            std::cerr << "Falha ao inicializar GLFW" << std::endl;
            return false;
        }

        // Configuração do contexto OpenGL 3.3 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        return true;
    }

    bool Application::initWindow() {
        if (!mWindow.create(mConfig.width, mConfig.height, mConfig.title)) {
            return false;
        }

        // Configura callbacks da janela
        glfwSetWindowUserPointer(mWindow.handle(), this);

        // Callback de resize
        glfwSetFramebufferSizeCallback(mWindow.handle(), [](GLFWwindow *win, int w, int h) {
            if (auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win))) {
                app->handleResize(w, h);
            }
        });

        // Callback de mouse (encaminha para InputManager)
        glfwSetCursorPosCallback(mWindow.handle(), [](GLFWwindow *win, double x, double y) {
            if (auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win))) {
                app->handleMousePos(x, y);
            }
        });

        return true;
    }

    bool Application::initGLAD() {
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cerr << "Falha ao carregar GLAD" << std::endl;
            return false;
        }

        // Configuração inicial do viewport
        glViewport(0, 0, mWindow.width(), mWindow.height());

        // Calcula matriz de projeção inicial
        float aspect = static_cast<float>(mWindow.width()) / static_cast<float>(std::max(1, mWindow.height()));
        mProjectionMatrix = glm::perspective(glm::radians(70.f), aspect, 0.1f, 500.f);

        return true;
    }

    bool Application::initScene() {
        // Habilita teste de profundidade
        glEnable(GL_DEPTH_TEST);

        // =================== SHADER UNIFICADO (MVP) ===================
        // Mantém simplicidade do exemplo original com shader minimalista
        const char *vertexShader = R"GLSL(
        #version 330 core
        layout(location=0) in vec3 aPos;
        uniform mat4 uMVP;
        void main(){
            gl_Position = uMVP * vec4(aPos, 1.0);
        }
    )GLSL";

        const char *fragmentShader = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        void main(){
            FragColor = vec4(0.55, 0.55, 0.60, 1.0);
        }
    )GLSL";

        if (!mGridShader.compile(vertexShader, fragmentShader)) {
            std::cerr << "Falha ao compilar shader da grade" << std::endl;
            return false;
        }

        // =================== GEOMETRIA DA CENA ===================
        // Grade de referência (-50 a 50 unidades no plano XZ)
        mGrid = std::make_unique<Grid>(50);

        // =================== CONFIGURAÇÃO DA CÂMERA ===================
        // Posição inicial: levemente elevada e afastada da origem
        mCamera.setPosition({0.f, 1.8f, 5.f});

        return true;
    }

    bool Application::initSystems() {
        // =================== SISTEMA DE ENTRADA ===================
        mInputManager.init(mWindow.handle());

        // Configurações personalizadas de entrada
        mInputManager.setMouseSensitivity(0.15f); // sensibilidade moderada
        mInputManager.setMoveSpeed(5.0f); // 5 unidades por segundo

        // =================== CONTADOR DE FPS ===================
        mFPSCounter = std::make_unique<FPSCounter>(&mWindow, mConfig.title);
        mFPSCounter->setUpdateInterval(1.0f); // atualiza FPS a cada 1 segundo

        return true;
    }

    void Application::run() {
        if (!mInitialized) {
            std::cerr << "Aplicação não foi inicializada corretamente" << std::endl;
            return;
        }
        mainLoop();
    }

    void Application::mainLoop() {
        // Controle de timing
        double lastTime = glfwGetTime();

        while (!glfwWindowShouldClose(mWindow.handle())) {
            // =================== CÁLCULO DE DELTA TIME ===================
            double currentTime = glfwGetTime();
            auto deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

            // =================== ATUALIZAÇÃO DE SISTEMAS ===================
            glfwPollEvents(); // processa eventos do GLFW
            updateSystems(deltaTime); // atualiza sistemas por frame

            // =================== RENDERIZAÇÃO ===================
            renderScene(); // desenha a cena

            // =================== APRESENTAÇÃO ===================
            mWindow.swapBuffers(); // apresenta frame na tela
        }
    }

    void Application::updateSystems(float deltaTime) {
        // =================== SISTEMA DE ENTRADA ===================
        // Processa entrada de teclado/mouse e atualiza câmera
        mInputManager.processInput(mWindow.handle(), mCamera, deltaTime);

        // =================== CONTADOR DE FPS ===================
        // Atualiza estatísticas e título da janela
        mFPSCounter->update(deltaTime);
    }

    void Application::renderScene() {
        // =================== LIMPEZA DE BUFFERS ===================
        glClearColor(0.08f, 0.09f, 0.11f, 1.0f); // cor de fundo escura
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // =================== CÁLCULO DE MATRIZES ===================
        glm::mat4 model(1.0f); // modelo identidade (cena estática)
        glm::mat4 view = mCamera.viewMatrix(); // matriz da câmera
        glm::mat4 mvp = mProjectionMatrix * view * model; // MVP final

        // =================== RENDERIZAÇÃO DA GRADE ===================
        mGridShader.bind();
        mGridShader.setMat4("uMVP", mvp);
        if (mGrid) {
            mGrid->draw(); // desenha linhas da grade
        }
        Shader::unbind();
    }

    void Application::handleResize(int w, int h) {
        if (w < 1 || h < 1) return; // evita divisão por zero

        // Atualiza dimensões da janela
        mWindow.resize(w, h);

        // Atualiza viewport do OpenGL
        glViewport(0, 0, w, h);

        // Recalcula matriz de projeção com novo aspect ratio
        float aspect = static_cast<float>(w) / static_cast<float>(std::max(1, h));
        mProjectionMatrix = glm::perspective(glm::radians(70.f), aspect, 0.1f, 500.f);
    }

    void Application::handleMousePos(double xpos, double ypos) {
        // Encaminha movimento do mouse para o InputManager com acesso à câmera
        mInputManager.handleMouseMovement(xpos, ypos, mCamera);
    }
} // namespace cg