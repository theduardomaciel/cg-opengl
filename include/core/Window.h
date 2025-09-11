#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace cg {

class Window {
public:
    Window() = default;
    ~Window();

    bool create(int width, int height, const std::string& title);
    void setTitle(const std::string& title);
    void swapBuffers();

    GLFWwindow* handle() { return mWindow; }
    int width() const { return mWidth; }
    int height() const { return mHeight; }
    void resize(int w, int h);

private:
    GLFWwindow* mWindow = nullptr;
    int mWidth = 0;
    int mHeight = 0;
};

} // namespace cg

