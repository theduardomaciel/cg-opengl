#pragma once
#include "render/Mesh.h"
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cg
{

    /**
     * @brief Classe que representa um modelo 3D completo
     *
     * Um modelo pode conter múltiplas meshes (por exemplo, um prédio pode ter
     * meshes separadas para paredes, telhado, janelas, etc.). Cada modelo
     * mantém sua própria transformação (posição, rotação, escala) no mundo.
     */
    class Model
    {
    public:
        /**
         * @brief Construtor que cria um modelo vazio
         * @param name Nome do modelo (opcional, usado para debug)
         */
        explicit Model(const std::string &name = "");

        /**
         * @brief Destrutor padrão
         */
        ~Model() = default;

        /**
         * @brief Adiciona uma mesh ao modelo
         * @param mesh Ponteiro único para a mesh a ser adicionada
         */
        void addMesh(std::unique_ptr<Mesh> mesh);

        /**
         * @brief Renderiza todas as meshes do modelo
         */
        void draw() const;

        // =================== TRANSFORMAÇÕES ===================

        /**
         * @brief Define a posição do modelo no mundo
         * @param position Nova posição (x, y, z)
         */
        void setPosition(const glm::vec3 &position);

        /**
         * @brief Define a rotação do modelo
         * @param rotation Rotação em radianos (x, y, z)
         */
        void setRotation(const glm::vec3 &rotation);

        /**
         * @brief Define a escala do modelo
         * @param scale Fator de escala (x, y, z). Use (1,1,1) para tamanho original
         */
        void setScale(const glm::vec3 &scale);

        /**
         * @brief Obtém a matriz de transformação do modelo
         * @return Matriz 4x4 que representa a transformação completa (posição + rotação + escala)
         */
        glm::mat4 getModelMatrix() const;

    /**
     * @brief Obtém a matriz mundo de uma mesh específica (modelo * cadeia de pais * local)
     * @param mesh Ponteiro para a mesh pertencente a este Model
     * @return Matriz 4x4 da transformação no espaço do mundo
     */
    glm::mat4 getWorldMatrixForMesh(const Mesh *mesh) const;

    /**
     * @brief Define a relação pai-filho entre duas meshes por nome
     * @param childName Nome da mesh filha
     * @param parentName Nome da mesh pai
     * @return true se encontrado e vinculado, false caso contrário
     */
    bool setParentByName(const std::string &childName, const std::string &parentName);

    /**
     * @brief Define a relação pai-filho entre duas meshes
     * @param child Ponteiro para a mesh filha
     * @param parent Ponteiro para a mesh pai (ou nullptr para remover pai)
     * @return true em caso de sucesso
     */
    bool setParent(Mesh *child, Mesh *parent);

        // =================== GETTERS ===================

        const glm::vec3 &getPosition() const { return mPosition; }
        const glm::vec3 &getRotation() const { return mRotation; }
        const glm::vec3 &getScale() const { return mScale; }
        const std::string &getName() const { return mName; }

        /**
         * @brief Obtém o número total de meshes no modelo
         */
        size_t getMeshCount() const { return mMeshes.size(); }

        /**
         * @brief Obtém o número total de triângulos em todas as meshes
         */
        size_t getTotalTriangleCount() const;

        /**
         * @brief Obtém o número total de vértices em todas as meshes
         */
        size_t getTotalVertexCount() const;

        /**
         * @brief Verifica se o modelo está vazio (sem meshes)
         */
        bool isEmpty() const { return mMeshes.empty(); }

        /**
         * @brief Obtém acesso somente leitura às meshes para renderização personalizada
         * @return Referência constante ao vetor de meshes
         */
        const std::vector<std::unique_ptr<Mesh>> &getMeshes() const { return mMeshes; }

        /**
         * @brief Acesso mutável às meshes (para aplicar transformações locais)
         */
        std::vector<std::unique_ptr<Mesh>> &getMeshesMutable() { return mMeshes; }

        /**
         * @brief Obtém um ponteiro para a mesh pelo nome (somente leitura)
         * @param meshName Nome da mesh
         * @return Ponteiro para Mesh ou nullptr se não encontrada
         */
        const Mesh *findMeshByName(const std::string &meshName) const
        {
            for (const auto &m : mMeshes)
            {
                if (m && m->name == meshName)
                    return m.get();
            }
            return nullptr;
        }

        /**
         * @brief Obtém um ponteiro para a mesh pelo nome (mutável)
         * @param meshName Nome da mesh
         * @return Ponteiro para Mesh ou nullptr se não encontrada
         */
        Mesh *findMeshByName(const std::string &meshName)
        {
            for (auto &m : mMeshes)
            {
                if (m && m->name == meshName)
                    return m.get();
            }
            return nullptr;
        }

        // Desabilita cópia (usa unique_ptr)
        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        // Permite movimentação
        Model(Model &&other) noexcept = default;
        Model &operator=(Model &&other) noexcept = default;

    private:
        // =================== DADOS DO MODELO ===================
        std::vector<std::unique_ptr<Mesh>> mMeshes; // Lista de meshes que compõem o modelo
        std::string mName;                          // Nome do modelo

    // Índice do pai por mesh (alinha com mMeshes). -1 indica sem pai.
    std::vector<int> mParents;

        // =================== TRANSFORMAÇÃO ===================
        glm::vec3 mPosition{0.0f}; // Posição no mundo (x, y, z)
        glm::vec3 mRotation{0.0f}; // Rotação em radianos (x, y, z)
        glm::vec3 mScale{1.0f};    // Escala (x, y, z) - (1,1,1) = tamanho original

        /**
         * @brief Recalcula a matriz de transformação baseada na posição, rotação e escala atuais
         */
        mutable glm::mat4 mModelMatrix{1.0f};   // Cache da matriz de transformação
        mutable bool mMatrixNeedsUpdate = true; // Flag para indicar se a matriz precisa ser recalculada

        /**
         * @brief Marca que a matriz de transformação precisa ser recalculada
         */
        void markMatrixDirty() { mMatrixNeedsUpdate = true; }

        // Utilitário: retorna o índice de uma mesh pelo ponteiro; -1 se não pertencer ao modelo
        int indexOf(const Mesh *mesh) const;

        // Utilitário: calcula a transformação local acumulada ao longo da cadeia de pais até a raiz
        glm::mat4 accumulateLocalUpToRoot(int meshIndex) const;
    };

} // namespace cg
