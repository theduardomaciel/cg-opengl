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
            // Expande estrutura de pais mantendo sem pai por padrão
            mParents.push_back(-1);
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

    glm::mat4 Model::getWorldMatrixForMesh(const Mesh *mesh) const
    {
        if (!mesh)
            return getModelMatrix();
        int idx = indexOf(mesh);
        if (idx < 0)
            return getModelMatrix();

        // world = Model * (ParentLocal * ... * ChildLocal)
        return getModelMatrix() * accumulateLocalUpToRoot(idx);
    }

    bool Model::setParentByName(const std::string &childName, const std::string &parentName)
    {
        Mesh *child = findMeshByName(childName);
        Mesh *parent = findMeshByName(parentName);
        return setParent(child, parent);
    }

    bool Model::setParent(Mesh *child, Mesh *parent)
    {
        if (!child)
            return false;

        int cIdx = indexOf(child);
        if (cIdx < 0)
            return false;

        int pIdx = -1;
        if (parent)
        {
            pIdx = indexOf(parent);
            if (pIdx < 0)
                return false;

            // Evita ciclo simples (pai não pode ser descendente do filho)
            int cursor = pIdx;
            while (cursor != -1)
            {
                if (cursor == cIdx)
                {
                    std::cerr << "ERRO: Tentativa de criar ciclo na hierarquia de meshes" << std::endl;
                    return false;
                }
                cursor = mParents[cursor];
            }
        }

        mParents[cIdx] = pIdx;
        return true;
    }

    int Model::indexOf(const Mesh *mesh) const
    {
        for (size_t i = 0; i < mMeshes.size(); ++i)
        {
            if (mMeshes[i].get() == mesh)
                return static_cast<int>(i);
        }
        return -1;
    }

    glm::mat4 Model::accumulateLocalUpToRoot(int meshIndex) const
    {
        glm::mat4 acc(1.0f);
        int idx = meshIndex;
        // Constrói cadeia de baixo para cima e acumula em ordem correta (pai antes do filho)
        // Estratégia: empilhar índices e depois multiplicar na ordem da raiz até o filho
        std::vector<int> chain;
        while (idx != -1)
        {
            chain.push_back(idx);
            idx = (idx >= 0 && idx < static_cast<int>(mParents.size())) ? mParents[idx] : -1;
        }
        for (int i = static_cast<int>(chain.size()) - 1; i >= 0; --i)
        {
            const Mesh *m = mMeshes[chain[i]].get();
            acc = acc * (m ? m->getLocalTransform() : glm::mat4(1.0f));
        }
        return acc;
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
