#include "core/Window.h"
#include <iostream>

namespace cg {

Window::~Window() {
    // Limpa recursos da janela se ainda existir
    if (mWindow) {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
    }
}

bool Window::create(int width, int height, const std::string& title) {
    // Armazena dimensões para consulta posterior
    mWidth = width;
    mHeight = height;

    // Cria janela GLFW com contexto OpenGL
    mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!mWindow) {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        return false;
    }

    // Torna o contexto OpenGL desta janela ativo na thread atual
    glfwMakeContextCurrent(mWindow);

    return true;
}

void Window::setTitle(const std::string& title) {
    // Atualiza título da janela (usado pelo FPSCounter)
    if (mWindow) {
        glfwSetWindowTitle(mWindow, title.c_str());
    }
}

void Window::swapBuffers() {
    // Troca buffers front/back (apresenta frame renderizado)
    if (mWindow) {
        glfwSwapBuffers(mWindow);
    }
}

void Window::resize(int w, int h) {
    // Atualiza dimensões internas (chamado pelo callback de resize)
    mWidth = w;
    mHeight = h;
}

} // namespace cg
