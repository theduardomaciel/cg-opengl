#pragma once
#include <glad/glad.h>
#include <vector>

namespace cg {

class Grid {
public:
    explicit Grid(int halfExtent = 50);
    ~Grid();

    void draw() const;

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;

private:
    GLuint mVAO = 0;
    GLuint mVBO = 0;
    GLsizei mVertexCount = 0;
};

} // namespace cg

