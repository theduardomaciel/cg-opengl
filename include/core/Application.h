#pragma once
#include <string>
#include <memory>
#include <glm/glm.hpp>

#include "FPSCounter.h"
#include "core/Window.h"
#include "render/Renderer.h"
#include "render/ModelLoader.h"
#include "input/Camera.h"
#include "input/Input.h"

namespace cg
{

    /**
     * @brief Configurações básicas da aplicação
     */
    struct AppConfig
    {
        int width = 1280;
        int height = 720;
        std::string title = "CG OpenGL - Centro Histórico"; // título base (FPS será anexado automaticamente)
        bool vsync = true;
    };

    /**
     * @brief Classe principal da aplicação
     *
     * Responsabilidades:
     * - Orquestrar o loop principal (init -> loop -> cleanup)
     * - Coordenar sistemas (Window, InputManager, FPSCounter, renderização)
     * - Gerenciar ciclo de vida dos recursos OpenGL
     * - Manter cena com modelos 3D carregados de arquivos OBJ
     */
    class Application
    {
    public:
        // Passa por valor para permitir otimização (copy elision) e possível move de string
        explicit Application(AppConfig cfg) : mConfig(std::move(cfg)) {}
        ~Application();

        // Inicialização e execução
        bool init();
        void run();

        // Callbacks do GLFW (encaminhados para sistemas apropriados)
        void handleMousePos(double xpos, double ypos);
        void handleResize(int w, int h);

    private:
        // =================== ETAPAS DE INICIALIZAÇÃO ===================
        bool initGLFW();
        bool initWindow();
        bool initGLAD();
        bool initScene();
        bool initSystems();

        // =================== LOOP PRINCIPAL ===================
        void mainLoop();
        void updateSystems(float deltaTime);
        void renderScene();

        // =================== UTILITÁRIOS ===================
        std::unique_ptr<Model> createTestCube(); // Cria um cubo de teste se nenhum modelo puder ser carregado        // =================== CONFIGURAÇÃO ===================
        AppConfig mConfig;
        bool mInitialized = false;

        // =================== SISTEMAS PRINCIPAIS ===================
        Window mWindow;
        Input mInputManager;
        std::unique_ptr<FPSCounter> mFPSCounter;

        // =================== RENDERIZAÇÃO ===================
        Camera mCamera;
        Renderer mRenderer;
        glm::mat4 mProjectionMatrix{1.0f};
    };

} // namespace cg
