#include "render/Model.h"
#include <iostream>

namespace cg
{

    Model::Model(const std::string &name)
        : mName(name)
    {
        std::cout << "Modelo criado: " << mName << std::endl;
    }

    void Model::addMesh(std::unique_ptr<Mesh> mesh)
    {
        if (mesh)
        {
            std::cout << "Adicionando mesh '" << mesh->name << "' ao modelo '" << mName << "'" << std::endl;
            mMeshes.push_back(std::move(mesh));
        }
    }

    void Model::draw() const
    {
        // Renderiza todas as meshes do modelo
        for (const auto &mesh : mMeshes)
        {
            if (mesh)
            {
                mesh->draw();
            }
        }
    }

    void Model::setPosition(const glm::vec3 &position)
    {
        if (mPosition != position)
        {
            mPosition = position;
            markMatrixDirty();
        }
    }

    void Model::setRotation(const glm::vec3 &rotation)
    {
        if (mRotation != rotation)
        {
            mRotation = rotation;
            markMatrixDirty();
        }
    }

    void Model::setScale(const glm::vec3 &scale)
    {
        if (mScale != scale)
        {
            mScale = scale;
            markMatrixDirty();
        }
    }

    glm::mat4 Model::getModelMatrix() const
    {
        // Recalcula a matriz apenas se necessário (lazy evaluation)
        if (mMatrixNeedsUpdate)
        {
            // =================== ORDEM DAS TRANSFORMAÇÕES ===================
            // 1. Escala primeiro (altera o tamanho)
            // 2. Rotação (gira em torno da origem)
            // 3. Translação por último (move para posição final)

            mModelMatrix = glm::mat4(1.0f); // Matriz identidade

            // Aplicar translação (posição)
            mModelMatrix = glm::translate(mModelMatrix, mPosition);

            // Aplicar rotações (ordem: X, Y, Z)
            if (mRotation.x != 0.0f)
            {
                mModelMatrix = glm::rotate(mModelMatrix, mRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            }
            if (mRotation.y != 0.0f)
            {
                mModelMatrix = glm::rotate(mModelMatrix, mRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            if (mRotation.z != 0.0f)
            {
                mModelMatrix = glm::rotate(mModelMatrix, mRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            }

            // Aplicar escala
            mModelMatrix = glm::scale(mModelMatrix, mScale);

            mMatrixNeedsUpdate = false;
        }

        return mModelMatrix;
    }

    size_t Model::getTotalTriangleCount() const
    {
        size_t total = 0;
        for (const auto &mesh : mMeshes)
        {
            if (mesh)
            {
                total += mesh->getTriangleCount();
            }
        }
        return total;
    }

    size_t Model::getTotalVertexCount() const
    {
        size_t total = 0;
        for (const auto &mesh : mMeshes)
        {
            if (mesh)
            {
                total += mesh->getVertexCount();
            }
        }
        return total;
    }

} // namespace cg
