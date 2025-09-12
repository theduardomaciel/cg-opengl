#include "render/Renderer.h"
#include <iostream>
#include <glad/glad.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace cg
{

    Renderer::Renderer() = default;

    bool Renderer::init()
    {
        std::cout << "Inicializando sistema de renderização..." << std::endl;

        // =================== SHADER BÁSICO PARA MODELOS 3D ===================
        // Shader vertex mais avançado que suporta normais e iluminação básica
        const char *vertexShaderSource = R"GLSL(
        #version 330 core
        
        // Atributos de entrada
        layout(location = 0) in vec3 aPosition;  // Posição do vértice
        layout(location = 1) in vec3 aNormal;    // Normal do vértice
        layout(location = 2) in vec2 aTexCoord;  // Coordenada de textura
        
        // Matrizes de transformação
        uniform mat4 uModel;       // Matriz do modelo (posição, rotação, escala)
        uniform mat4 uView;        // Matriz de visualização (câmera)
        uniform mat4 uProjection;  // Matriz de projeção (perspectiva)
        uniform mat3 uNormalMatrix; // Matriz para transformar normais
        
        // Dados para o fragment shader
        out vec3 FragPos;    // Posição do fragmento no espaço mundial
        out vec3 Normal;     // Normal transformada
        out vec2 TexCoord;   // Coordenada de textura
        
        void main() {
            // Calcula posição final do vértice
            vec4 worldPos = uModel * vec4(aPosition, 1.0);
            FragPos = worldPos.xyz;
            
            // Transforma a normal para o espaço mundial
            Normal = normalize(uNormalMatrix * aNormal);
            
            // Passa coordenada de textura inalterada
            TexCoord = aTexCoord;
            
            // Posição final na tela
            gl_Position = uProjection * uView * worldPos;
        }
    )GLSL";

        // Shader fragment com iluminação básica
        const char *fragmentShaderSource = R"GLSL(
        #version 330 core
        
        // Dados de entrada do vertex shader
        in vec3 FragPos;   // Posição do fragmento
        in vec3 Normal;    // Normal do fragmento
        in vec2 TexCoord;  // Coordenada de textura
        
        // Cor final de saída
        out vec4 FragColor;
        
        // Configurações de iluminação
        uniform vec3 uLightPos;     // Posição da luz
        uniform vec3 uLightColor;   // Cor da luz
        uniform vec3 uViewPos;      // Posição da câmera
        uniform vec3 uObjectColor;  // Cor base do objeto
        
        void main() {
            // =================== ILUMINAÇÃO AMBIENTE ===================
            float ambientStrength = 0.3;
            vec3 ambient = ambientStrength * uLightColor;
            
            // =================== ILUMINAÇÃO DIFUSA ===================
            vec3 lightDir = normalize(uLightPos - FragPos);
            float diff = max(dot(Normal, lightDir), 0.0);
            vec3 diffuse = diff * uLightColor;
            
            // =================== ILUMINAÇÃO ESPECULAR ===================
            float specularStrength = 0.5;
            vec3 viewDir = normalize(uViewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, Normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * uLightColor;
            
            // =================== COR FINAL ===================
            vec3 result = (ambient + diffuse + specular) * uObjectColor;
            FragColor = vec4(result, 1.0);
        }
    )GLSL";

        // Compila o shader
        if (!mBasicShader.compile(vertexShaderSource, fragmentShaderSource))
        {
            std::cerr << "ERRO: Falha ao compilar shader básico do renderer" << std::endl;
            return false;
        }

        std::cout << "Shader básico compilado com sucesso" << std::endl;

        // Configura estado inicial do OpenGL
        setupRenderState();

        std::cout << "Sistema de renderização inicializado com sucesso" << std::endl;
        return true;
    }

    void Renderer::addModel(std::unique_ptr<Model> model, const std::string &id)
    {
        if (!model)
        {
            std::cerr << "ERRO: Tentativa de adicionar modelo nulo" << std::endl;
            return;
        }

        std::string finalId = id;
        if (finalId.empty())
        {
            finalId = generateAutoId();
        }

        // Verifica se o ID já existe
        if (mModels.find(finalId) != mModels.end())
        {
            std::cerr << "AVISO: Substituindo modelo existente com ID: " << finalId << std::endl;
            // Remove da ordem de renderização
            auto it = std::find(mModelOrder.begin(), mModelOrder.end(), finalId);
            if (it != mModelOrder.end())
            {
                mModelOrder.erase(it);
            }
        }

        std::cout << "Adicionando modelo '" << model->getName() << "' com ID: " << finalId << std::endl;

        mModels[finalId] = std::move(model);
        mModelOrder.push_back(finalId);
    }

    bool Renderer::removeModel(const std::string &id)
    {
        auto it = mModels.find(id);
        if (it != mModels.end())
        {
            std::cout << "Removendo modelo com ID: " << id << std::endl;

            // Remove da ordem de renderização
            auto orderIt = std::find(mModelOrder.begin(), mModelOrder.end(), id);
            if (orderIt != mModelOrder.end())
            {
                mModelOrder.erase(orderIt);
            }

            mModels.erase(it);
            return true;
        }

        std::cerr << "AVISO: Modelo com ID '" << id << "' não encontrado para remoção" << std::endl;
        return false;
    }

    Model *Renderer::getModel(const std::string &id)
    {
        auto it = mModels.find(id);
        if (it != mModels.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    void Renderer::render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
    {
        // =================== PREPARAÇÃO ===================
        setupRenderState();
        clearBuffers();

        // =================== CONFIGURAÇÃO DO SHADER ===================
        mBasicShader.bind();

        // Define matrizes
        mBasicShader.setMat4("uView", viewMatrix);
        mBasicShader.setMat4("uProjection", projectionMatrix);

        // Define parâmetros de iluminação (luz fixa por enquanto)
        mBasicShader.setVec3("uLightPos", glm::vec3(10.0f, 10.0f, 10.0f));
        mBasicShader.setVec3("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

        // Extrai posição da câmera da matriz de visualização (inversa)
        glm::mat4 invView = glm::inverse(viewMatrix);
        glm::vec3 cameraPos = glm::vec3(invView[3]);
        mBasicShader.setVec3("uViewPos", cameraPos);

        // =================== RENDERIZAÇÃO DOS MODELOS ===================
        for (const std::string &modelId : mModelOrder)
        {
            auto it = mModels.find(modelId);
            if (it != mModels.end() && it->second)
            {
                renderModel(*it->second, viewMatrix, projectionMatrix);
            }
        }

        // =================== FINALIZAÇÃO ===================
        Shader::unbind();
    }

    void Renderer::clear()
    {
        std::cout << "Limpando todos os modelos da cena (" << mModels.size() << " modelos)" << std::endl;
        mModels.clear();
        mModelOrder.clear();
        mNextAutoId = 0;
    }

    void Renderer::setRenderSettings(const RenderSettings &settings)
    {
        mSettings = settings;
        setupRenderState();
    }

    Renderer::RenderStats Renderer::calculateStats() const
    {
        RenderStats stats;
        stats.totalModels = mModels.size();

        for (const auto &pair : mModels)
        {
            if (pair.second)
            {
                stats.totalMeshes += pair.second->getMeshCount();
                stats.totalTriangles += pair.second->getTotalTriangleCount();
                stats.totalVertices += pair.second->getTotalVertexCount();
            }
        }

        return stats;
    }

    void Renderer::printStats() const
    {
        RenderStats stats = calculateStats();

        std::cout << "=== Estatísticas da Cena ===" << std::endl;
        std::cout << "Modelos: " << stats.totalModels << std::endl;
        std::cout << "Meshes: " << stats.totalMeshes << std::endl;
        std::cout << "Triângulos: " << stats.totalTriangles << std::endl;
        std::cout << "Vértices: " << stats.totalVertices << std::endl;
        std::cout << "=============================" << std::endl;
    }

    std::string Renderer::generateAutoId()
    {
        return "modelo_" + std::to_string(mNextAutoId++);
    }

    void Renderer::setupRenderState()
    {
        // =================== TESTE DE PROFUNDIDADE ===================
        if (mSettings.enableDepthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // =================== DESCARTE DE FACES TRASEIRAS ===================
        if (mSettings.enableBackfaceCulling)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW); // Sentido anti-horário para faces frontais
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        // =================== MODO WIREFRAME ===================
        if (mSettings.enableWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    void Renderer::clearBuffers()
    {
        glClearColor(mSettings.clearColor.r,
                     mSettings.clearColor.g,
                     mSettings.clearColor.b,
                     mSettings.clearColor.a);

        GLbitfield clearMask = GL_COLOR_BUFFER_BIT;
        if (mSettings.enableDepthTest)
        {
            clearMask |= GL_DEPTH_BUFFER_BIT;
        }

        glClear(clearMask);
    }

    void Renderer::renderModel(const Model &model, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
    {
        // =================== CONFIGURAÇÃO DAS MATRIZES ===================
        glm::mat4 modelMatrix = model.getModelMatrix();
        mBasicShader.setMat4("uModel", modelMatrix);

        // Matriz normal para transformar corretamente as normais
        // (inversa transposta da matriz modelo)
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        mBasicShader.setMat3("uNormalMatrix", normalMatrix);

        // Cor do objeto (por enquanto uma cor fixa, no futuro pode vir de material)
        mBasicShader.setVec3("uObjectColor", glm::vec3(0.7f, 0.7f, 0.8f));

        // =================== RENDERIZAÇÃO ===================
        model.draw();
    }

} // namespace cg
