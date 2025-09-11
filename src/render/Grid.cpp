#include "render/Grid.h"
#include <vector>

#include "glad/glad.h"

namespace cg {

Grid::Grid(int halfExtent) {
    // Gera vértices para uma grade de linhas no plano XZ (y=0)
    std::vector<float> verts;

    // Para cada linha de -halfExtent até +halfExtent
    for (int i = -halfExtent; i <= halfExtent; ++i) {
        // =================== LINHAS PARALELAS AO EIXO Z ===================
        // Linha que vai de (i, 0, -halfExtent) até (i, 0, +halfExtent)
        verts.push_back(static_cast<float>(i));           // x = i (constante)
        verts.push_back(0.f);                             // y = 0 (plano XZ)
        verts.push_back(static_cast<float>(-halfExtent)); // z inicial

        verts.push_back(static_cast<float>(i));           // x = i (constante)
        verts.push_back(0.f);                             // y = 0 (plano XZ)
        verts.push_back(static_cast<float>(halfExtent));  // z final

        // =================== LINHAS PARALELAS AO EIXO X ===================
        // Linha que vai de (-halfExtent, 0, i) até (+halfExtent, 0, i)
        verts.push_back(static_cast<float>(-halfExtent)); // x inicial
        verts.push_back(0.f);                             // y = 0 (plano XZ)
        verts.push_back(static_cast<float>(i));           // z = i (constante)

        verts.push_back(static_cast<float>(halfExtent));  // x final
        verts.push_back(0.f);                             // y = 0 (plano XZ)
        verts.push_back(static_cast<float>(i));           // z = i (constante)
    }

    // Total de vértices: cada linha tem 2 pontos, 3 floats por ponto
    mVertexCount = static_cast<GLsizei>(verts.size() / 3);

    // =================== CONFIGURAÇÃO OPENGL ===================
    // Gera VAO (Vertex Array Object) e VBO (Vertex Buffer Object)
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);

    // Vincula VAO (armazena configuração de atributos)
    glBindVertexArray(mVAO);

    // Vincula e carrega dados no VBO
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(float),
                 verts.data(),
                 GL_STATIC_DRAW);

    // Configura atributo de posição (location 0, 3 floats por vértice)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Desvincula VAO (boa prática)
    glBindVertexArray(0);
}

Grid::~Grid() {
    // Limpa recursos OpenGL
    if (mVBO) glDeleteBuffers(1, &mVBO);
    if (mVAO) glDeleteVertexArrays(1, &mVAO);
}

void Grid::draw() const {
    // Vincula VAO (ativa configuração de atributos)
    glBindVertexArray(mVAO);

    // Desenha como linhas (GL_LINES: cada par de vértices forma uma linha)
    glDrawArrays(GL_LINES, 0, mVertexCount);

    // Desvincula VAO (boa prática)
    glBindVertexArray(0);
}

} // namespace cg
