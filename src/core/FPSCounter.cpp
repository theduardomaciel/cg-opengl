#include "core/FPSCounter.h"
#include "core/Window.h"
#include <string>

namespace cg {

FPSCounter::FPSCounter(Window* window, const std::string& baseTitle)
    : mWindow(window), mBaseTitle(baseTitle) {
}

void FPSCounter::update(float deltaTime) {
    // Incrementa contador de frames e acumula tempo
    ++mFrameCount;
    mTimer += deltaTime;

    // Atualiza FPS quando atingir o intervalo configurado
    if (mTimer >= mUpdateInterval) {
        // Calcula FPS médio no período
        mCurrentFPS = static_cast<float>(mFrameCount) / mTimer;

        // Atualiza título da janela
        updateWindowTitle();

        // Reset dos contadores para próximo período
        mFrameCount = 0;
        mTimer = 0.f;
    }
}

void FPSCounter::updateWindowTitle() {
    if (mWindow) {
        std::string title = mBaseTitle + " | FPS: " + std::to_string(static_cast<int>(mCurrentFPS));
        mWindow->setTitle(title);
    }
}

} // namespace cg
