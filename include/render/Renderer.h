#pragma once
#include "render/Model.h"
#include "render/Shader.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace cg
{

    /**
     * @brief Sistema de renderização responsável por desenhar todos os modelos na cena
     *
     * O Renderer mantém uma lista de modelos e os renderiza usando shaders apropriados.
     * Ele gerencia multiple shaders e permite diferentes configurações de renderização
     * para diferentes tipos de objetos.
     */
    class Renderer
    {
    public:
        /**
         * @brief Configurações de renderização
         */
        struct RenderSettings
        {
            bool enableWireframe = false;                    // Ativa modo wireframe (apenas linhas)
            bool enableBackfaceCulling = true;               // Ativa descarte de faces traseiras
            bool enableDepthTest = true;                     // Ativa teste de profundidade
            glm::vec4 clearColor{0.08f, 0.09f, 0.11f, 1.0f}; // Cor de fundo
        };

        /**
         * @brief Construtor padrão
         */
        Renderer();

        /**
         * @brief Destrutor padrão
         */
        ~Renderer() = default;

        /**
         * @brief Inicializa o sistema de renderização
         * @return true se inicializado com sucesso, false caso contrário
         */
        bool init();

        /**
         * @brief Adiciona um modelo à cena para ser renderizado
         * @param model Ponteiro único para o modelo a ser adicionado
         * @param id Identificador único para o modelo (para remoção posterior)
         */
        void addModel(std::unique_ptr<Model> model, const std::string &id = "");

        /**
         * @brief Remove um modelo da cena
         * @param id Identificador do modelo a ser removido
         * @return true se o modelo foi encontrado e removido, false caso contrário
         */
        bool removeModel(const std::string &id);

        /**
         * @brief Obtém um ponteiro para um modelo específico
         * @param id Identificador do modelo
         * @return Ponteiro para o modelo ou nullptr se não encontrado
         */
        Model *getModel(const std::string &id);

        /**
         * @brief Renderiza todos os modelos na cena
         * @param viewMatrix Matriz de visualização da câmera
         * @param projectionMatrix Matriz de projeção
         */
        void render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);

        /**
         * @brief Limpa todos os modelos da cena
         */
        void clear();

        /**
         * @brief Define as configurações de renderização
         * @param settings Novas configurações
         */
        void setRenderSettings(const RenderSettings &settings);

        /**
         * @brief Obtém as configurações atuais de renderização
         */
        const RenderSettings &getRenderSettings() const { return mSettings; }

        /**
         * @brief Obtém estatísticas de renderização
         */
        struct RenderStats
        {
            size_t totalModels = 0;    // Número total de modelos na cena
            size_t totalMeshes = 0;    // Número total de meshes renderizadas
            size_t totalTriangles = 0; // Número total de triângulos renderizados
            size_t totalVertices = 0;  // Número total de vértices renderizados
        };

        /**
         * @brief Calcula e retorna estatísticas da cena atual
         */
        RenderStats calculateStats() const;

        /**
         * @brief Imprime estatísticas da cena no console
         */
        void printStats() const;

    private:
        // =================== DADOS DA CENA ===================
        std::unordered_map<std::string, std::unique_ptr<Model>> mModels; // Modelos por ID
        std::vector<std::string> mModelOrder;                            // Ordem de renderização dos modelos
        size_t mNextAutoId = 0;                                          // Contador para IDs automáticos

        // =================== SHADERS ===================
        Shader mBasicShader;       // Shader básico para geometria sólida
        Shader mTransparentShader; // Shader para materiais transparentes

        // =================== CONFIGURAÇÕES ===================
        RenderSettings mSettings;

        /**
         * @brief Gera um ID automático único para um modelo
         */
        std::string generateAutoId();

        /**
         * @brief Configura os estados OpenGL baseado nas configurações atuais
         */
        void setupRenderState();

        /**
         * @brief Limpa os buffers de cor e profundidade
         */
        void clearBuffers();

        /**
         * @brief Renderiza um modelo específico (objetos opacos)
         * @param model Modelo a ser renderizado
         * @param viewMatrix Matriz de visualização
         * @param projectionMatrix Matriz de projeção
         */
        void renderModelOpaque(const Model &model, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);

        /**
         * @brief Renderiza um modelo específico (objetos transparentes)
         * @param model Modelo a ser renderizado
         * @param viewMatrix Matriz de visualização
         * @param projectionMatrix Matriz de projeção
         */
        void renderModelTransparent(const Model &model, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
    };

} // namespace cg
