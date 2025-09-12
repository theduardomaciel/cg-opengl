#pragma once
#include <GLFW/glfw3.h>

namespace cg
{

    // Forward declaration
    class Camera;

    class Input
    {
    public:
        // Construtor
        Input() = default;

        // Inicialização do sistema de entrada
        void init(GLFWwindow *window);

        // Processamento de entrada por frame
        void processInput(GLFWwindow *window, Camera &camera, float deltaTime);

        // Tratamento de movimento do mouse
        void handleMouseMovement(double xpos, double ypos, Camera &camera);

        // Verifica se uma tecla foi pressionada (apenas uma vez, não continuamente)
        bool wasKeyPressed(GLFWwindow *window, int key);

        // Configurações
        void setMouseSensitivity(float sensitivity) { mMouseSensitivity = sensitivity; }
        void setMoveSpeed(float speed) { mMoveSpeed = speed; }

        // Método estático original para compatibilidade
        static bool keyDown(GLFWwindow *win, int key)
        {
            return glfwGetKey(win, key) == GLFW_PRESS;
        }

        // Callbacks estáticos do GLFW
        static void mouseCallback(GLFWwindow *window, double xpos, double ypos);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    private:
        // Controle do cursor
        void setCursorDisabled(GLFWwindow *window, bool disabled);

        // Estado do mouse
        float mLastX = 0.0f;
        float mLastY = 0.0f;
        bool mFirstMouse = true;
        bool mCursorDisabled = false;

        // Estado do teclado
        bool mPrevToggleKey = false;
        bool mPrevWireframeKey = false; // Para alternar wireframe

        // Configurações
        float mMouseSensitivity = 0.1f;
        float mMoveSpeed = 2.5f;

        // Instância estática para callbacks
        static Input *sInstance;
    };

} // namespace cg
