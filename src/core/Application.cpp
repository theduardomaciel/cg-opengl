#include "core/Application.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cg
{
    Application::~Application()
    {
        if (mInitialized)
        {
            glfwTerminate();
            mInitialized = false;
        }
    }

    bool Application::init()
    {
        // Sequência de inicialização ordenada
        if (!initGLFW())
            return false;
        if (!initWindow())
            return false;
        if (!initGLAD())
            return false;
        if (!initScene())
            return false;
        if (!initSystems())
            return false;

        // Configuração de VSync
        if (mConfig.vsync)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        mInitialized = true;
        return true;
    }

    bool Application::initGLFW()
    {
        if (!glfwInit())
        {
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

    bool Application::initWindow()
    {
        if (!mWindow.create(mConfig.width, mConfig.height, mConfig.title))
        {
            return false;
        }

        // Configura callbacks da janela
        glfwSetWindowUserPointer(mWindow.handle(), this);

        // Callback de resize
        glfwSetFramebufferSizeCallback(mWindow.handle(), [](GLFWwindow *win, int w, int h)
                                       {
            if (auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win))) {
                app->handleResize(w, h);
            } });

        // Callback de mouse (encaminha para InputManager)
        glfwSetCursorPosCallback(mWindow.handle(), [](GLFWwindow *win, double x, double y)
                                 {
            if (auto *app = static_cast<Application *>(glfwGetWindowUserPointer(win))) {
                app->handleMousePos(x, y);
            } });

        return true;
    }

    bool Application::initGLAD()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
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

    bool Application::initScene()
    {
        std::cout << "Inicializando cena..." << std::endl;

        // =================== INICIALIZAÇÃO DO RENDERER ===================
        if (!mRenderer.init())
        {
            std::cerr << "ERRO: Falha ao inicializar o sistema de renderização" << std::endl;
            return false;
        }

        // =================== CARREGAMENTO DO MODELO PRINCIPAL ===================
        std::cout << "Carregando modelo principal..." << std::endl;

        // Lista de caminhos possíveis para o modelo (em ordem de prioridade)
        std::vector<std::string> possiblePaths = {
            "models/structure_v6.obj",       // Caminho relativo do projeto
            "../models/structure_v6.obj",    // Uma pasta acima (se executando do build)
            "../../models/structure_v6.obj", // Duas pastas acima
        };

        std::unique_ptr<Model> model = nullptr;
        std::string usedPath;

        // Tenta carregar o modelo usando diferentes caminhos
        for (const auto &path : possiblePaths)
        {
            std::cout << "Tentando carregar modelo de: " << path << std::endl;
            model = ModelLoader::loadModel(path, "CentroHistorico");
            if (model)
            {
                usedPath = path;
                break;
            }
        }

        if (!model)
        {
            std::cerr << "ERRO: Falha ao carregar qualquer modelo. Criando modelo de teste..." << std::endl;

            // Cria um modelo de teste simples (cubo) se nenhum arquivo puder ser carregado
            model = createTestCube();
            if (!model)
            {
                std::cerr << "ERRO: Falha ao criar modelo de teste" << std::endl;
                return false;
            }
            usedPath = "modelo_teste_cubo";
        }

        std::cout << "Modelo carregado com sucesso de: " << usedPath << std::endl;

        // Posiciona o modelo no centro da cena
        model->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        model->setScale(glm::vec3(1.0f, 1.0f, 1.0f)); // Tamanho original

        // Adiciona o modelo ao renderer
        mRenderer.addModel(std::move(model), "centro_historico");

        // =================== CONFIGURAÇÃO DA CÂMERA ===================
        // Posição inicial: elevada e afastada para ter visão geral do modelo
        mCamera.setPosition({0.f, 7.5f, 20.f});

        // =================== ESTATÍSTICAS ===================
        mRenderer.printStats();

        std::cout << "Cena inicializada com sucesso!" << std::endl;
        return true;
    }

    bool Application::initSystems()
    {
        // =================== SISTEMA DE ENTRADA ===================
        mInputManager.init(mWindow.handle());

        // Configurações personalizadas de entrada
        mInputManager.setMouseSensitivity(0.15f); // sensibilidade moderada
        mInputManager.setMoveSpeed(5.0f);         // 5 unidades por segundo

        // =================== CONTADOR DE FPS ===================
        mFPSCounter = std::make_unique<FPSCounter>(&mWindow, mConfig.title);
        mFPSCounter->setUpdateInterval(1.0f); // atualiza FPS a cada 1 segundo

        return true;
    }

    void Application::run()
    {
        if (!mInitialized)
        {
            std::cerr << "Aplicação não foi inicializada corretamente" << std::endl;
            return;
        }
        mainLoop();
    }

    void Application::mainLoop()
    {
        // Controle de timing
        double lastTime = glfwGetTime();

        while (!glfwWindowShouldClose(mWindow.handle()))
        {
            // =================== CÁLCULO DE DELTA TIME ===================
            double currentTime = glfwGetTime();
            auto deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

            // =================== ATUALIZAÇÃO DE SISTEMAS ===================
            glfwPollEvents();         // processa eventos do GLFW
            updateSystems(deltaTime); // atualiza sistemas por frame

            // =================== RENDERIZAÇÃO ===================
            renderScene(); // desenha a cena

            // =================== APRESENTAÇÃO ===================
            mWindow.swapBuffers(); // apresenta frame na tela
        }
    }

    void Application::updateSystems(float deltaTime)
    {
        // =================== SISTEMA DE ENTRADA ===================
        // Processa entrada de teclado/mouse e atualiza câmera
        mInputManager.processInput(mWindow.handle(), mCamera, deltaTime);

        // =================== CONTROLES ESPECIAIS ===================
        // Alterna entre modo wireframe e sólido (Ctrl + W)
        if (mInputManager.wasKeyPressed(mWindow.handle(), GLFW_KEY_W))
        {
            auto settings = mRenderer.getRenderSettings();
            settings.enableWireframe = !settings.enableWireframe;
            mRenderer.setRenderSettings(settings);

            std::cout << "Modo wireframe: " << (settings.enableWireframe ? "ATIVADO" : "DESATIVADO") << std::endl;
        }

        // =================== ATUALIZAÇÃO DO SKYBOX ===================
        // Atualiza animações do skybox (nuvens, etc.)
        mRenderer.updateSkybox(deltaTime);

        // =================== CONTADOR DE FPS ===================
        // Atualiza estatísticas e título da janela
        mFPSCounter->update(deltaTime);
    }

    void Application::renderScene()
    {
        // =================== CÁLCULO DE MATRIZES ===================
        glm::mat4 view = mCamera.viewMatrix();    // Matriz da câmera
        glm::mat4 projection = mProjectionMatrix; // Matriz de projeção

        // =================== RENDERIZAÇÃO DA CENA ===================
        mRenderer.render(view, projection);
    }

    void Application::handleResize(int w, int h)
    {
        if (w < 1 || h < 1)
            return; // evita divisão por zero

        // Atualiza dimensões da janela
        mWindow.resize(w, h);

        // Atualiza viewport do OpenGL
        glViewport(0, 0, w, h);

        // Recalcula matriz de projeção com novo aspect ratio
        float aspect = static_cast<float>(w) / static_cast<float>(std::max(1, h));
        mProjectionMatrix = glm::perspective(glm::radians(70.f), aspect, 0.1f, 500.f);
    }

    void Application::handleMousePos(double xpos, double ypos)
    {
        // Encaminha movimento do mouse para o InputManager com acesso à câmera
        mInputManager.handleMouseMovement(xpos, ypos, mCamera);
    }

    std::unique_ptr<Model> Application::createTestCube()
    {
        std::cout << "Criando cubo de teste..." << std::endl;

        // =================== VÉRTICES DO CUBO ===================
        // Cubo simples centrado na origem com lado de 2 unidades
        std::vector<Vertex> vertices = {
            // Face frontal (Z+)
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 0
            {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // 1
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 2
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // 3

            // Face traseira (Z-)
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // 4
            {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},  // 5
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},   // 6
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},  // 7

            // Face esquerda (X-)
            {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 8
            {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},  // 9
            {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},   // 10
            {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},  // 11

            // Face direita (X+)
            {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 12
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},  // 13
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},   // 14
            {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},  // 15

            // Face superior (Y+)
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 16
            {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},  // 17
            {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // 18
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},  // 19

            // Face inferior (Y-)
            {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // 20
            {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},  // 21
            {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},   // 22
            {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}   // 23
        };

        // =================== ÍNDICES DO CUBO ===================
        // Cada face é composta por 2 triângulos (6 índices por face)
        std::vector<GLuint> indices = {
            // Face frontal
            0, 1, 2, 2, 3, 0,
            // Face traseira
            4, 6, 5, 6, 4, 7,
            // Face esquerda
            8, 9, 10, 10, 11, 8,
            // Face direita
            12, 14, 13, 14, 12, 15,
            // Face superior
            16, 17, 18, 18, 19, 16,
            // Face inferior
            20, 22, 21, 22, 20, 23};

        // =================== CRIAÇÃO DO MODELO ===================
        auto model = std::make_unique<Model>("CuboTeste");
        auto mesh = std::make_unique<Mesh>(vertices, indices, "CuboMesh");
        model->addMesh(std::move(mesh));

        std::cout << "Cubo de teste criado com sucesso!" << std::endl;
        return model;
    }
} // namespace cg