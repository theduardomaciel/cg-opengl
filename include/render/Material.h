#pragma once
#include <glm/glm.hpp>
#include <string>

namespace cg
{

    /**
     * @brief Enumeração dos tipos de material disponíveis
     */
    enum class MaterialType
    {
        OPAQUE,      // Material opaco padrão
        TRANSPARENT, // Material transparente (vidro)
        EMISSIVE     // Material emissivo (que emite luz)
    };

    /**
     * @brief Classe que representa um material com propriedades físicas para renderização
     *
     * Um material define como uma superfície interage com a luz, incluindo:
     * - Cor base (albedo)
     * - Propriedades especulares
     * - Transparência
     * - Emissão de luz
     */
    class Material
    {
    public:
        /**
         * @brief Construtor que cria um material opaco padrão
         * @param name Nome do material (para debug)
         */
        explicit Material(const std::string &name = "Default Material");

        /**
         * @brief Construtor que cria um material com tipo específico
         * @param type Tipo do material
         * @param name Nome do material
         */
        Material(MaterialType type, const std::string &name = "Material");

        /**
         * @brief Destrutor padrão
         */
        ~Material() = default;

        // =================== PROPRIEDADES BÁSICAS ===================

        /**
         * @brief Define a cor base do material (albedo)
         * @param color Cor RGB (valores entre 0.0 e 1.0)
         */
        void setAlbedo(const glm::vec3 &color) { mAlbedo = color; }

        /**
         * @brief Define a cor especular (reflexos)
         * @param color Cor RGB dos reflexos
         */
        void setSpecular(const glm::vec3 &color) { mSpecular = color; }

        /**
         * @brief Define o brilho especular (shininess)
         * @param shininess Valor do brilho (tipicamente entre 1.0 e 128.0)
         */
        void setShininess(float shininess) { mShininess = shininess; }

        /**
         * @brief Define a transparência do material
         * @param alpha Valor alpha (0.0 = totalmente transparente, 1.0 = opaco)
         */
        void setAlpha(float alpha);

        /**
         * @brief Define a cor emissiva (luz própria)
         * @param color Cor RGB da emissão
         */
        void setEmissive(const glm::vec3 &color) { mEmissive = color; }

        /**
         * @brief Define o índice de refração (para materiais transparentes)
         * @param ior Índice de refração (1.0 = ar, 1.33 = água, 1.5 = vidro comum)
         */
        void setIndexOfRefraction(float ior) { mIndexOfRefraction = ior; }

        // =================== GETTERS ===================

        const glm::vec3 &getAlbedo() const { return mAlbedo; }
        const glm::vec3 &getSpecular() const { return mSpecular; }
        float getShininess() const { return mShininess; }
        float getAlpha() const { return mAlpha; }
        const glm::vec3 &getEmissive() const { return mEmissive; }
        float getIndexOfRefraction() const { return mIndexOfRefraction; }
        MaterialType getType() const { return mType; }
        const std::string &getName() const { return mName; }

        /**
         * @brief Verifica se o material é transparente
         */
        bool isTransparent() const { return mType == MaterialType::TRANSPARENT || mAlpha < 1.0f; }

        /**
         * @brief Verifica se o material é emissivo
         */
        bool isEmissive() const { return mType == MaterialType::EMISSIVE || glm::length(mEmissive) > 0.0f; }

        // =================== MATERIAIS PRÉ-DEFINIDOS ===================

        /**
         * @brief Cria um material de vidro transparente
         * @param alpha Transparência (0.0 = invisível, 1.0 = opaco)
         * @param tint Cor do vidro (padrão: ligeiramente azulado)
         * @return Material de vidro configurado
         */
        static Material createGlass(float alpha = 0.3f, const glm::vec3 &tint = glm::vec3(0.9f, 0.95f, 1.0f));

        /**
         * @brief Cria um material metálico
         * @param color Cor base do metal
         * @param shininess Brilho metálico
         * @return Material metálico configurado
         */
        static Material createMetal(const glm::vec3 &color, float shininess = 64.0f);

        /**
         * @brief Cria um material plástico
         * @param color Cor base do plástico
         * @return Material plástico configurado
         */
        static Material createPlastic(const glm::vec3 &color);

    private:
        // =================== PROPRIEDADES DO MATERIAL ===================
        std::string mName;  // Nome do material
        MaterialType mType; // Tipo do material

        glm::vec3 mAlbedo{0.8f, 0.8f, 0.8f};   // Cor base (difusa)
        glm::vec3 mSpecular{1.0f, 1.0f, 1.0f}; // Cor especular
        float mShininess = 32.0f;              // Brilho especular
        float mAlpha = 1.0f;                   // Transparência
        glm::vec3 mEmissive{0.0f, 0.0f, 0.0f}; // Cor emissiva
        float mIndexOfRefraction = 1.5f;       // Índice de refração (vidro comum)
    };

} // namespace cg
