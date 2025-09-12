#include "render/Skybox.h"
#include <glad/glad.h>
#include <iostream>
#include <cmath>

namespace cg
{

    Skybox::Skybox() = default;

    Skybox::~Skybox()
    {
        cleanup();
    }

    bool Skybox::init()
    {
        // Criar geometria do cubo
        createCubeGeometry();

        // Compilar shaders
        if (!compileShaders())
        {
            std::cerr << "Falha ao compilar shaders do skybox" << std::endl;
            return false;
        }

        // Normalizar direção do sol
        mConfig.sunDirection = glm::normalize(mConfig.sunDirection);

        return true;
    }

    void Skybox::render(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
    {
        // Debug: verificar se o skybox está sendo chamado
        // std::cout << "Renderizando skybox..." << std::endl;

        // Salvar estado anterior
        GLint prevDepthFunc;
        glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);

        // Configurar estados para skybox
        glDepthFunc(GL_LEQUAL); // Permitir que o skybox seja desenhado no fundo

        // Usar shader
        mShader.bind();

        // Remover translação da matriz de view (apenas rotação)
        glm::mat4 skyboxView = glm::mat4(glm::mat3(viewMatrix));

        // Configurar uniforms
        mShader.setMat4("view", skyboxView);
        mShader.setMat4("projection", projectionMatrix);
        mShader.setVec3("skyColor", mConfig.skyColor);
        mShader.setVec3("horizonColor", mConfig.horizonColor);
        mShader.setVec3("sunColor", mConfig.sunColor);
        mShader.setVec3("sunDirection", mConfig.sunDirection);
        mShader.setFloat("sunSize", mConfig.sunSize);
        mShader.setFloat("sunIntensity", mConfig.sunIntensity);
        mShader.setInt("enableClouds", mConfig.enableClouds ? 1 : 0);
        mShader.setFloat("cloudDensity", mConfig.cloudDensity);
        mShader.setFloat("time", mConfig.time);

        // Renderizar cubo
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36); // Usar DrawArrays em vez de DrawElements
        glBindVertexArray(0);

        // Restaurar estado anterior
        glDepthFunc(prevDepthFunc);
    }

    void Skybox::update(float deltaTime)
    {
        mConfig.time += deltaTime * mConfig.cloudSpeed;
    }

    void Skybox::createCubeGeometry()
    {
        // Vértices do cubo (posições apenas, já que usaremos as coordenadas como direções)
        float vertices[] = {
            // Posições
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f};

        // Gerar e configurar VAO, VBO
        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);

        glBindVertexArray(mVAO);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Atributo de posição
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    bool Skybox::compileShaders()
    {
        // Vertex shader
        const char *vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Truque para garantir que o skybox fique sempre no fundo
}
)";

        // Fragment shader com céu procedural
        const char *fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 skyColor;
uniform vec3 horizonColor;
uniform vec3 sunColor;
uniform vec3 sunDirection;
uniform float sunSize;
uniform float sunIntensity;
uniform int enableClouds;
uniform float cloudDensity;
uniform float time;

// Função de ruído simples para nuvens
float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    
    return value;
}

void main()
{
    vec3 direction = normalize(TexCoords);
    
    // Calcular gradiente do céu baseado na altura (Y)
    float height = direction.y;
    float t = max(0.0, height);
    
    // Interpolar entre cor do horizonte e cor do céu
    vec3 skyGradient = mix(horizonColor, skyColor, t);
    
    // Calcular sol
    float sunDot = dot(direction, normalize(sunDirection));
    float sunMask = smoothstep(1.0 - sunSize, 1.0 - sunSize * 0.5, sunDot);
    vec3 sunContribution = sunColor * sunMask * sunIntensity;
    
    // Adicionar halo do sol
    float sunHalo = smoothstep(1.0 - sunSize * 3.0, 1.0 - sunSize, sunDot);
    sunContribution += sunColor * sunHalo * 0.3;
    
    // Calcular nuvens (apenas na parte superior do céu)
    vec3 cloudColor = vec3(1.0);
    float cloudMask = 0.0;
    
    if (enableClouds > 0 && height > 0.0) {
        vec3 cloudPos = direction * 10.0;
        cloudPos.x += time * 0.5;
        cloudPos.z += time * 0.3;
        
        float cloudNoise = fbm(cloudPos);
        cloudMask = smoothstep(1.0 - cloudDensity, 1.0, cloudNoise);
        cloudMask *= smoothstep(0.0, 0.3, height); // Nuvens apenas na parte superior
        
        // Variar cor das nuvens baseado na proximidade do sol
        float cloudSunInfluence = max(0.0, dot(direction, normalize(sunDirection)));
        cloudColor = mix(vec3(0.8, 0.8, 0.9), vec3(1.0, 0.95, 0.8), cloudSunInfluence);
    }
    
    // Combinar todas as contribuições
    vec3 finalColor = skyGradient + sunContribution;
    finalColor = mix(finalColor, cloudColor, cloudMask);
    
    FragColor = vec4(finalColor, 1.0);
}
)";

        return mShader.compile(vertexShaderSource, fragmentShaderSource);
    }

    void Skybox::cleanup()
    {
        if (mVAO)
        {
            glDeleteVertexArrays(1, &mVAO);
            mVAO = 0;
        }
        if (mVBO)
        {
            glDeleteBuffers(1, &mVBO);
            mVBO = 0;
        }
        if (mEBO)
        {
            glDeleteBuffers(1, &mEBO);
            mEBO = 0;
        }
    }

} // namespace cg