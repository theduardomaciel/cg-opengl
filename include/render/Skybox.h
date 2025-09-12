#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "render/Shader.h"

namespace cg
{
    /**
     * @brief Classe para renderização de skybox com céu azul, sol e nuvens
     *
     * O Skybox é renderizado como um cubo que segue a câmera e sempre aparece
     * no fundo da cena. Suporta texturas cubemap ou geração procedural.
     */
    class Skybox
    {
    public:
        /**
         * @brief Configurações do skybox
         */
        struct SkyboxConfig
        {
            glm::vec3 skyColor = glm::vec3(0.5f, 0.8f, 1.0f);     // Cor base do céu (azul claro)
            glm::vec3 horizonColor = glm::vec3(0.9f, 0.9f, 0.8f); // Cor do horizonte
            glm::vec3 sunColor = glm::vec3(1.0f, 0.9f, 0.7f);     // Cor do sol
            glm::vec3 sunDirection = glm::vec3(0.3f, 0.8f, 0.5f); // Direção do sol (normalizada)
            float sunSize = 0.04f;                                // Tamanho do sol
            float sunIntensity = 2.0f;                            // Intensidade do sol
            bool enableClouds = true;                             // Habilitar nuvens
            float cloudDensity = 0.4f;                            // Densidade das nuvens
            float cloudSpeed = 0.1f;                              // Velocidade das nuvens
            float time = 0.0f;                                    // Tempo para animação
        };

        /**
         * @brief Construtor padrão
         */
        Skybox();

        /**
         * @brief Destrutor
         */
        ~Skybox();

        /**
         * @brief Inicializa o skybox
         * @return true se inicializado com sucesso
         */
        bool init();

        /**
         * @brief Renderiza o skybox
         * @param viewMatrix Matriz de visualização (sem translação)
         * @param projectionMatrix Matriz de projeção
         */
        void render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);

        /**
         * @brief Atualiza as configurações do skybox
         * @param config Novas configurações
         */
        void setConfig(const SkyboxConfig &config) { mConfig = config; }

        /**
         * @brief Obtém as configurações atuais
         */
        const SkyboxConfig &getConfig() const { return mConfig; }

        /**
         * @brief Atualiza o tempo para animação das nuvens
         * @param deltaTime Tempo decorrido desde a última atualização
         */
        void update(float deltaTime);

    private:
        // Configurações
        SkyboxConfig mConfig;

        // OpenGL objects
        GLuint mVAO = 0;
        GLuint mVBO = 0;
        GLuint mEBO = 0;

        // Shader para renderização procedural
        Shader mShader;

        /**
         * @brief Cria a geometria do cubo para o skybox
         */
        void createCubeGeometry();

        /**
         * @brief Compila os shaders do skybox
         */
        bool compileShaders();

        /**
         * @brief Limpa os recursos OpenGL
         */
        void cleanup();
    };

} // namespace cg