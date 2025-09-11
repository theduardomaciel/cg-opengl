#pragma once
#include <string>

namespace cg {

class Window; // forward declaration

/**
 * @brief Contador de FPS (Frames Por Segundo)
 *
 * Esta classe é responsável por:
 * - Calcular FPS médio baseado em frames acumulados
 * - Atualizar título da janela com informação de FPS
 * - Controlar frequência de atualização (padrão: 1 segundo)
 */
class FPSCounter {
public:
    explicit FPSCounter(Window* window, const std::string& baseTitle);

    // Atualiza contador a cada frame
    void update(float deltaTime);

    // Configuração
    void setUpdateInterval(float interval) { mUpdateInterval = interval; }

    // Acesso ao FPS atual (último calculado)
    float getCurrentFPS() const { return mCurrentFPS; }

private:
    Window* mWindow;           // referência para atualizar título
    std::string mBaseTitle;    // título base da janela

    // Contadores
    int mFrameCount = 0;       // frames acumulados
    float mTimer = 0.f;        // tempo acumulado
    float mUpdateInterval = 1.0f; // intervalo de atualização (segundos)
    float mCurrentFPS = 0.f;   // último FPS calculado

    // Atualiza título da janela com FPS
    void updateWindowTitle();
};

} // namespace cg
