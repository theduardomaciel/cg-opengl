#include "input/Input.h"
#include "input/Camera.h"
#include <iostream>

namespace cg {

// Instância estática para callbacks do GLFW
Input* Input::sInstance = nullptr;

void Input::init(GLFWwindow* window) {
    // Define esta instância como a ativa para callbacks
    sInstance = this;

    // Inicializa captura do cursor (estilo FPS)
    setCursorDisabled(window, true);

    // Obtém posição inicial da janela para calcular centro
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mLastX = static_cast<float>(width) / 2.0f;
    mLastY = static_cast<float>(height) / 2.0f;
}

void Input::processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    // =================== MOVIMENTO DA CÂMERA ===================
    // Calcula velocidade baseada no delta time para movimento suave
    float velocity = mMoveSpeed * deltaTime;

    // Movimento WASD no plano XZ (ignora componente Y do vetor front)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.moveForward(velocity);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.moveForward(-velocity);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.moveRight(velocity);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.moveRight(-velocity);
    }

    // =================== CONTROLES GERAIS ===================
    // ESC para fechar a aplicação
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // =================== TOGGLE CURSOR (TECLA C) ===================
    // Detecta borda de subida da tecla C para alternar captura do cursor
    bool togglePressed = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if (togglePressed && !mPrevToggleKey) {
        setCursorDisabled(window, !mCursorDisabled);
        std::cout << "Cursor " << (mCursorDisabled ? "capturado" : "liberado") << std::endl;
    }
    mPrevToggleKey = togglePressed;
}

void Input::handleMouseMovement(double xpos, double ypos, Camera& camera) {
    // Ignora movimento quando cursor está visível
    if (!mCursorDisabled) return;

    auto xf = static_cast<float>(xpos);
    auto yf = static_cast<float>(ypos);

    // Primeira vez: apenas registra posição sem calcular offset
    if (mFirstMouse) {
        mLastX = xf;
        mLastY = yf;
        mFirstMouse = false;
        return;
    }

    // Calcula offset do movimento do mouse
    float xoffset = xf - mLastX;
    float yoffset = mLastY - yf; // Invertido (mouse para cima = olhar para cima)

    mLastX = xf;
    mLastY = yf;

    // Aplica movimento à câmera
    camera.processMouse(xoffset, yoffset, mMouseSensitivity);
}

void Input::setCursorDisabled(GLFWwindow* window, bool disabled) {
    mCursorDisabled = disabled;
    glfwSetInputMode(window, GLFW_CURSOR,
                    disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    // Reset do estado do mouse para evitar "salto" ao reentrar
    if (disabled) {
        mFirstMouse = true;
    }
}

// =================== CALLBACKS ESTÁTICOS DO GLFW ===================

void Input::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    // Este callback não é usado diretamente - a Application encaminha via handleMouseMovement
    (void)window; (void)xpos; (void)ypos; // evita warnings
}

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Callback adicional para teclas específicas se necessário no futuro
    // Por enquanto, processInput() já trata todas as teclas necessárias
    (void)window; (void)key; (void)scancode; (void)action; (void)mods; // evita warnings
}

} // namespace cg
