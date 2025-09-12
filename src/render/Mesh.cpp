#include "render/Mesh.h"
#include <iostream>

namespace cg
{

    Mesh::Mesh(const std::vector<Vertex> &vertices,
               const std::vector<GLuint> &indices,
               const std::string &name,
               std::shared_ptr<Material> material)
        : vertices(vertices), indices(indices), name(name), material(material)
    {

        // Configura os buffers OpenGL para esta mesh
        setupMesh();

        std::string materialInfo = material ? " com material " + material->getName() : " sem material";
        std::cout << "Mesh criada: " << name
                  << " (Vértices: " << vertices.size()
                  << ", Triângulos: " << getTriangleCount() << ")"
                  << materialInfo << std::endl;
    }

    Mesh::~Mesh()
    {
        cleanup();
    }

    Mesh::Mesh(Mesh &&other) noexcept
        : vertices(std::move(other.vertices)), indices(std::move(other.indices)), name(std::move(other.name)), material(std::move(other.material)), mVAO(other.mVAO), mVBO(other.mVBO), mEBO(other.mEBO)
    {

        // Zera os recursos do objeto movido para evitar double-deletion
        other.mVAO = 0;
        other.mVBO = 0;
        other.mEBO = 0;
    }

    Mesh &Mesh::operator=(Mesh &&other) noexcept
    {
        if (this != &other)
        {
            // Libera recursos atuais
            cleanup();

            // Move dados do outro objeto
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            name = std::move(other.name);
            material = std::move(other.material);
            mVAO = other.mVAO;
            mVBO = other.mVBO;
            mEBO = other.mEBO;

            // Zera recursos do objeto movido
            other.mVAO = 0;
            other.mVBO = 0;
            other.mEBO = 0;
        }
        return *this;
    }

    void Mesh::setupMesh()
    {
        // =================== GERAÇÃO DE BUFFERS ===================
        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);
        glGenBuffers(1, &mEBO);

        // =================== CONFIGURAÇÃO DO VAO ===================
        glBindVertexArray(mVAO);

        // =================== CONFIGURAÇÃO DO VBO (VÉRTICES) ===================
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(Vertex),
                     vertices.data(),
                     GL_STATIC_DRAW);

        // =================== CONFIGURAÇÃO DO EBO (ÍNDICES) ===================
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(GLuint),
                     indices.data(),
                     GL_STATIC_DRAW);

        // =================== CONFIGURAÇÃO DE ATRIBUTOS DE VÉRTICE ===================

        // Atributo 0: Posição (vec3)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

        // Atributo 1: Normal (vec3)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

        // Atributo 2: Coordenadas de textura (vec2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));

        // =================== DESVINCULAÇÃO (BOA PRÁTICA) ===================
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void Mesh::draw() const
    {
        // Vincula o VAO que contém toda a configuração desta mesh
        glBindVertexArray(mVAO);

        // Desenha os triângulos usando os índices
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

        // Desvincula o VAO (boa prática)
        glBindVertexArray(0);
    }

    void Mesh::cleanup()
    {
        // Libera recursos OpenGL se ainda não foram liberados
        if (mEBO)
        {
            glDeleteBuffers(1, &mEBO);
            mEBO = 0;
        }
        if (mVBO)
        {
            glDeleteBuffers(1, &mVBO);
            mVBO = 0;
        }
        if (mVAO)
        {
            glDeleteVertexArrays(1, &mVAO);
            mVAO = 0;
        }
    }

} // namespace cg
