#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include "render/Material.h"

namespace cg
{

    /**
     * @brief Estrutura que representa um vértice com posição, normal e coordenadas de textura
     */
    struct Vertex
    {
        glm::vec3 position;  // Posição do vértice no espaço 3D
        glm::vec3 normal;    // Vetor normal para iluminação
        glm::vec2 texCoords; // Coordenadas de textura (UV mapping)

        Vertex() : position(0.0f), normal(0.0f), texCoords(0.0f) {}

        Vertex(const glm::vec3 &pos, const glm::vec3 &norm = glm::vec3(0.0f), const glm::vec2 &tex = glm::vec2(0.0f))
            : position(pos), normal(norm), texCoords(tex) {}
    };

    /**
     * @brief Classe que representa uma malha de triângulos (mesh)
     *
     * Uma mesh é uma coleção de vértices e índices que formam triângulos.
     * Cada mesh tem seus próprios buffers OpenGL (VAO, VBO, EBO) e pode
     * ser renderizada independentemente.
     */
    class Mesh
    {
    public:
        // =================== DADOS DA GEOMETRIA ===================
        std::vector<Vertex> vertices;       // Lista de vértices da mesh
        std::vector<GLuint> indices;        // Lista de índices (3 índices = 1 triângulo)
        std::string name;                   // Nome opcional da mesh (para debug)
        std::shared_ptr<Material> material; // Material da mesh

        /**
         * @brief Construtor que inicializa a mesh com vértices e índices
         * @param vertices Lista de vértices
         * @param indices Lista de índices para formar triângulos
         * @param name Nome opcional da mesh
         * @param material Material da mesh (opcional)
         */
        Mesh(const std::vector<Vertex> &vertices,
             const std::vector<GLuint> &indices,
             const std::string &name = "",
             std::shared_ptr<Material> material = nullptr);

        /**
         * @brief Destrutor que libera recursos OpenGL
         */
        ~Mesh();

        /**
         * @brief Renderiza a mesh usando OpenGL
         */
        void draw() const;

        /**
         * @brief Obtém o número de triângulos da mesh
         * @return Número de triângulos (índices / 3)
         */
        size_t getTriangleCount() const { return indices.size() / 3; }

        /**
         * @brief Obtém o número de vértices da mesh
         * @return Número de vértices
         */
        size_t getVertexCount() const { return vertices.size(); }

        // =================== GERENCIAMENTO DE MATERIAIS ===================

        /**
         * @brief Define o material da mesh
         * @param mat Ponteiro compartilhado para o material
         */
        void setMaterial(std::shared_ptr<Material> mat) { material = mat; }

        /**
         * @brief Obtém o material da mesh
         * @return Ponteiro compartilhado para o material (pode ser nullptr)
         */
        std::shared_ptr<Material> getMaterial() const { return material; }

        /**
         * @brief Verifica se a mesh tem material
         * @return true se tem material, false caso contrário
         */
        bool hasMaterial() const { return material != nullptr; }

        /**
         * @brief Verifica se a mesh é transparente
         * @return true se tem material transparente, false caso contrário
         */
        bool isTransparent() const { return material && material->isTransparent(); }

        // Desabilita cópia para evitar problemas com recursos OpenGL
        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        // Permite movimentação (move semantics)
        Mesh(Mesh &&other) noexcept;
        Mesh &operator=(Mesh &&other) noexcept;

        // =================== TRANSFORMAÇÃO LOCAL ===================
        /**
         * @brief Define a transformação local da mesh (relativa ao Model)
         * @param transform Matriz 4x4 de transformação local
         */
        void setLocalTransform(const glm::mat4 &transform) { mLocalTransform = transform; }

        /**
         * @brief Obtém a transformação local da mesh
         * @return Matriz 4x4 da transformação local
         */
        const glm::mat4 &getLocalTransform() const { return mLocalTransform; }

    private:
        // =================== RECURSOS OPENGL ===================
        GLuint mVAO = 0; // Vertex Array Object - armazena configuração de atributos
        GLuint mVBO = 0; // Vertex Buffer Object - armazena dados dos vértices
        GLuint mEBO = 0; // Element Buffer Object - armazena índices dos triângulos

        // =================== TRANSFORMAÇÃO LOCAL ===================
        glm::mat4 mLocalTransform{1.0f};

        /**
         * @brief Configura os buffers OpenGL (VAO, VBO, EBO)
         */
        void setupMesh();

        /**
         * @brief Libera recursos OpenGL
         */
        void cleanup();
    };

} // namespace cg
