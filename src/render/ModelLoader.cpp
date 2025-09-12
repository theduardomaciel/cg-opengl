#include "render/ModelLoader.h"
#include "render/Material.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <filesystem>

namespace cg
{

    // Inicialização do membro estático
    ModelLoader::LoadStats ModelLoader::sLastStats;

    void ModelLoader::LoadStats::print() const
    {
        std::cout << "=== Estatísticas de Carregamento ===" << std::endl;
        std::cout << "Meshes criadas: " << totalMeshes << std::endl;
        std::cout << "Vértices únicos: " << totalVertices << std::endl;
        std::cout << "Triângulos: " << totalTriangles << std::endl;
        std::cout << "Tempo de carregamento: " << loadTimeMs << " ms" << std::endl;
        std::cout << "===================================" << std::endl;
    }

    std::unique_ptr<Model> ModelLoader::loadModel(const std::string &filePath, const std::string &modelName)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Reseta estatísticas
        sLastStats = LoadStats{};

        std::cout << "Carregando modelo OBJ: " << filePath << std::endl;

        // =================== ABERTURA DO ARQUIVO ===================
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "ERRO: Não foi possível abrir o arquivo: " << filePath << std::endl;
            return nullptr;
        }

        // =================== CRIAÇÃO DO MODELO ===================
        std::string finalName = modelName;
        if (finalName.empty())
        {
            // Extrai nome do arquivo se não foi fornecido
            std::filesystem::path path(filePath);
            finalName = path.stem().string();
        }

        auto model = std::make_unique<Model>(finalName);
        ParseData data;

        // =================== PARSING LINHA POR LINHA ===================
        std::string line;
        size_t lineNumber = 0;

        while (std::getline(file, line))
        {
            lineNumber++;

            // Mostra progresso a cada 100.000 linhas para arquivos grandes
            if (lineNumber % 100000 == 0)
            {
                std::cout << "Processando linha " << lineNumber << "..." << std::endl;
            }

            try
            {
                parseLine(line, data, *model, filePath);
            }
            catch (const std::exception &e)
            {
                std::cerr << "ERRO na linha " << lineNumber << ": " << e.what() << std::endl;
                std::cerr << "Linha: " << line << std::endl;
                // Continua o processamento mesmo com erros
            }
        }

        // =================== FINALIZAÇÃO ===================
        // Finaliza a mesh atual se houver dados pendentes
        finalizeMesh(data, *model);

        file.close();

        // =================== CÁLCULO DE ESTATÍSTICAS ===================
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        sLastStats.totalMeshes = model->getMeshCount();
        sLastStats.totalVertices = model->getTotalVertexCount();
        sLastStats.totalTriangles = model->getTotalTriangleCount();
        sLastStats.loadTimeMs = duration.count() / 1000.0f;

        std::cout << "Modelo carregado com sucesso!" << std::endl;
        sLastStats.print();

        // Verifica se o modelo está vazio
        if (model->isEmpty())
        {
            std::cerr << "AVISO: Modelo carregado está vazio (nenhuma mesh válida)" << std::endl;
        }

        return model;
    }

    void ModelLoader::parseLine(const std::string &line, ParseData &data, Model &model, const std::string &objFilePath)
    {
        std::string trimmedLine = trim(line);

        // Ignora linhas vazias e comentários
        if (trimmedLine.empty() || trimmedLine[0] == '#')
        {
            return;
        }

        // Identifica o tipo de linha baseado no primeiro token
        if (trimmedLine.size() >= 2)
        {
            if (trimmedLine[0] == 'v' && trimmedLine[1] == ' ')
            {
                parseVertex(trimmedLine, data);
            }
            else if (trimmedLine.substr(0, 2) == "vn")
            {
                parseNormal(trimmedLine, data);
            }
            else if (trimmedLine.substr(0, 2) == "vt")
            {
                parseTexCoord(trimmedLine, data);
            }
            else if (trimmedLine[0] == 'f' && trimmedLine[1] == ' ')
            {
                parseFace(trimmedLine, data);
            }
            else if (trimmedLine[0] == 'o' && trimmedLine[1] == ' ')
            {
                parseObject(trimmedLine, data, model);
            }
            else if (trimmedLine.substr(0, 6) == "mtllib")
            {
                parseMaterialLib(trimmedLine, data, objFilePath);
            }
            else if (trimmedLine.substr(0, 6) == "usemtl")
            {
                parseUseMaterial(trimmedLine, data);
            }
            // Ignora outras linhas (s, g, etc.)
        }
    }

    void ModelLoader::parseVertex(const std::string &line, ParseData &data)
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // consome o 'v'

        float x, y, z;
        if (iss >> x >> y >> z)
        {
            data.positions.emplace_back(x, y, z);
        }
        else
        {
            throw std::runtime_error("Formato inválido para vértice");
        }
    }

    void ModelLoader::parseNormal(const std::string &line, ParseData &data)
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // consome o 'vn'

        float x, y, z;
        if (iss >> x >> y >> z)
        {
            // Normaliza o vetor (alguns arquivos OBJ têm normais não-normalizadas)
            glm::vec3 normal(x, y, z);
            float length = glm::length(normal);
            if (length > 0.0f)
            {
                normal /= length;
            }
            data.normals.push_back(normal);
        }
        else
        {
            throw std::runtime_error("Formato inválido para normal");
        }
    }

    void ModelLoader::parseTexCoord(const std::string &line, ParseData &data)
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // consome o 'vt'

        float u, v = 0.0f;
        if (iss >> u)
        {
            iss >> v; // v é opcional em alguns arquivos
            data.texCoords.emplace_back(u, v);
        }
        else
        {
            throw std::runtime_error("Formato inválido para coordenada de textura");
        }
    }

    void ModelLoader::parseFace(const std::string &line, ParseData &data)
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // consome o 'f'

        std::vector<FaceIndex> faceIndices;

        // Lê todos os índices da face
        while (iss >> token)
        {
            faceIndices.push_back(parseFaceIndex(token));
        }

        // Converte índices relativos para absolutos
        for (auto &faceIndex : faceIndices)
        {
            faceIndex.makeAbsolute(data.positions.size(), data.texCoords.size(), data.normals.size());
        }

        // Triangula a face se necessário (para faces com mais de 3 vértices)
        if (faceIndices.size() >= 3)
        {
            // Cria triângulos usando fan triangulation (v0-v1-v2, v0-v2-v3, etc.)
            for (size_t i = 2; i < faceIndices.size(); ++i)
            {
                // Adiciona triângulo (0, i-1, i)
                data.currentIndices.push_back(getOrCreateVertex(faceIndices[0], data));
                data.currentIndices.push_back(getOrCreateVertex(faceIndices[i - 1], data));
                data.currentIndices.push_back(getOrCreateVertex(faceIndices[i], data));
            }
        }
    }

    void ModelLoader::parseObject(const std::string &line, ParseData &data, Model &model)
    {
        // Finaliza a mesh atual antes de começar uma nova
        finalizeMesh(data, model);

        // Extrai o nome do objeto
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos && spacePos + 1 < line.size())
        {
            data.currentObjectName = trim(line.substr(spacePos + 1));
        }
        else
        {
            data.currentObjectName = "Objeto_" + std::to_string(model.getMeshCount() + 1);
        }

        std::cout << "Iniciando objeto: " << data.currentObjectName << std::endl;
    }

    void ModelLoader::finalizeMesh(ParseData &data, Model &model)
    {
        if (!data.currentVertices.empty() && !data.currentIndices.empty())
        {
            // Se não há normais no arquivo, calcula automaticamente
            if (data.normals.empty())
            {
                calculateNormals(data);
            }

            std::string meshName = data.currentObjectName;
            if (meshName.empty())
            {
                meshName = "Mesh_" + std::to_string(model.getMeshCount() + 1);
            }

            // =================== APLICAÇÃO DE MATERIAL ===================
            std::shared_ptr<Material> material = data.currentMaterial;

            // Se não há material definido, aplica detecção automática
            if (!material)
            {
                material = detectMaterialFromName(meshName);
            }

            auto mesh = std::make_unique<Mesh>(data.currentVertices, data.currentIndices, meshName, material);
            model.addMesh(std::move(mesh));

            // Limpa dados da mesh atual
            data.currentVertices.clear();
            data.currentIndices.clear();
            data.vertexMap.clear();
            data.currentObjectName.clear();
            data.currentMaterial = nullptr; // Reset do material atual
        }
    }

    ModelLoader::FaceIndex ModelLoader::parseFaceIndex(const std::string &indexStr)
    {
        FaceIndex faceIndex;

        std::vector<std::string> parts = split(indexStr, '/');

        if (!parts.empty() && !parts[0].empty())
        {
            faceIndex.positionIndex = std::stoi(parts[0]);
        }

        if (parts.size() > 1 && !parts[1].empty())
        {
            faceIndex.texCoordIndex = std::stoi(parts[1]);
        }

        if (parts.size() > 2 && !parts[2].empty())
        {
            faceIndex.normalIndex = std::stoi(parts[2]);
        }

        return faceIndex;
    }

    void ModelLoader::FaceIndex::makeAbsolute(size_t posCount, size_t texCount, size_t normCount)
    {
        // Converte índices negativos (relativos ao final) para positivos
        if (positionIndex < 0)
        {
            positionIndex = static_cast<int>(posCount) + positionIndex + 1;
        }
        if (texCoordIndex < 0)
        {
            texCoordIndex = static_cast<int>(texCount) + texCoordIndex + 1;
        }
        if (normalIndex < 0)
        {
            normalIndex = static_cast<int>(normCount) + normalIndex + 1;
        }

        // Converte de 1-indexado (OBJ) para 0-indexado (C++)
        if (positionIndex > 0)
            positionIndex--;
        if (texCoordIndex > 0)
            texCoordIndex--;
        if (normalIndex > 0)
            normalIndex--;
    }

    std::string ModelLoader::FaceIndex::getKey() const
    {
        return std::to_string(positionIndex) + "/" +
               std::to_string(texCoordIndex) + "/" +
               std::to_string(normalIndex);
    }

    GLuint ModelLoader::getOrCreateVertex(const FaceIndex &faceIndex, ParseData &data)
    {
        std::string key = faceIndex.getKey();

        // Verifica se já existe um vértice com estes índices
        auto it = data.vertexMap.find(key);
        if (it != data.vertexMap.end())
        {
            return it->second;
        }

        // Cria novo vértice
        Vertex vertex;

        // Posição (obrigatória)
        if (faceIndex.positionIndex >= 0 && faceIndex.positionIndex < static_cast<int>(data.positions.size()))
        {
            vertex.position = data.positions[faceIndex.positionIndex];
        }

        // Coordenada de textura (opcional)
        if (faceIndex.texCoordIndex >= 0 && faceIndex.texCoordIndex < static_cast<int>(data.texCoords.size()))
        {
            vertex.texCoords = data.texCoords[faceIndex.texCoordIndex];
        }

        // Normal (opcional)
        if (faceIndex.normalIndex >= 0 && faceIndex.normalIndex < static_cast<int>(data.normals.size()))
        {
            vertex.normal = data.normals[faceIndex.normalIndex];
        }

        GLuint vertexIndex = static_cast<GLuint>(data.currentVertices.size());
        data.currentVertices.push_back(vertex);
        data.vertexMap[key] = vertexIndex;

        return vertexIndex;
    }

    void ModelLoader::calculateNormals(ParseData &data)
    {
        // Calcula normais por triângulo se não existirem normais no arquivo
        for (size_t i = 0; i < data.currentIndices.size(); i += 3)
        {
            if (i + 2 < data.currentIndices.size())
            {
                GLuint i0 = data.currentIndices[i];
                GLuint i1 = data.currentIndices[i + 1];
                GLuint i2 = data.currentIndices[i + 2];

                if (i0 < data.currentVertices.size() &&
                    i1 < data.currentVertices.size() &&
                    i2 < data.currentVertices.size())
                {

                    glm::vec3 v0 = data.currentVertices[i0].position;
                    glm::vec3 v1 = data.currentVertices[i1].position;
                    glm::vec3 v2 = data.currentVertices[i2].position;

                    glm::vec3 edge1 = v1 - v0;
                    glm::vec3 edge2 = v2 - v0;
                    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

                    // Adiciona a normal calculada aos três vértices do triângulo
                    data.currentVertices[i0].normal = normal;
                    data.currentVertices[i1].normal = normal;
                    data.currentVertices[i2].normal = normal;
                }
            }
        }
    }

    std::string ModelLoader::trim(const std::string &str)
    {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            return "";

        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    std::vector<std::string> ModelLoader::split(const std::string &str, char delimiter)
    {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;

        while (std::getline(iss, token, delimiter))
        {
            tokens.push_back(token);
        }

        return tokens;
    }

    void ModelLoader::parseMaterialLib(const std::string &line, ParseData &data, const std::string &objFilePath)
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // consome o 'mtllib'

        std::string mtlFile;
        if (iss >> mtlFile)
        {
            // Resolve o caminho relativo
            std::filesystem::path objPath(objFilePath);
            std::filesystem::path mtlPath = objPath.parent_path() / mtlFile;

            std::cout << "Carregando biblioteca de materiais: " << mtlPath << std::endl;

            auto materials = loadMaterials(mtlPath.string());

            // Adiciona os materiais ao ParseData
            for (const auto &pair : materials)
            {
                data.materials[pair.first] = pair.second;
            }

            std::cout << "Carregados " << materials.size() << " materiais." << std::endl;
        }
    }

    void ModelLoader::parseUseMaterial(const std::string &line, ParseData &data)
    {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // consome o 'usemtl'

        std::string materialName;
        if (iss >> materialName)
        {
            auto it = data.materials.find(materialName);
            if (it != data.materials.end())
            {
                data.currentMaterial = it->second;
                std::cout << "Usando material: " << materialName << std::endl;
            }
            else
            {
                std::cerr << "AVISO: Material não encontrado: " << materialName << std::endl;
                data.currentMaterial = createDefaultMaterial(materialName);
            }
        }
    }

    std::unordered_map<std::string, std::shared_ptr<Material>> ModelLoader::loadMaterials(const std::string &filePath)
    {
        std::unordered_map<std::string, std::shared_ptr<Material>> materials;

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "ERRO: Não foi possível abrir o arquivo MTL: " << filePath << std::endl;
            return materials;
        }

        std::string line;
        std::string currentMaterial;

        while (std::getline(file, line))
        {
            try
            {
                parseMaterialLine(line, materials, currentMaterial);
            }
            catch (const std::exception &e)
            {
                std::cerr << "ERRO ao processar linha MTL: " << e.what() << std::endl;
                std::cerr << "Linha: " << line << std::endl;
            }
        }

        file.close();
        return materials;
    }

    void ModelLoader::parseMaterialLine(const std::string &line,
                                        std::unordered_map<std::string, std::shared_ptr<Material>> &materials,
                                        std::string &currentMaterial)
    {
        std::string trimmedLine = trim(line);

        // Ignora linhas vazias e comentários
        if (trimmedLine.empty() || trimmedLine[0] == '#')
        {
            return;
        }

        std::istringstream iss(trimmedLine);
        std::string token;
        iss >> token;

        if (token == "newmtl")
        {
            // Novo material
            if (iss >> currentMaterial)
            {
                std::cout << "Definindo material: " << currentMaterial << std::endl;

                // Detecta automaticamente o tipo pelo nome do material
                auto detectedMaterial = detectMaterialFromName(currentMaterial);
                if (detectedMaterial)
                {
                    // Usa o material detectado como base, mas preserva o nome original
                    materials[currentMaterial] = detectedMaterial;
                    std::cout << "  -> Tipo detectado automaticamente pelo nome" << std::endl;
                }
                else
                {
                    // Cria material padrão se não detectou tipo específico
                    materials[currentMaterial] = std::make_shared<Material>(currentMaterial);
                }
            }
        }
        else if (!currentMaterial.empty() && materials.find(currentMaterial) != materials.end())
        {
            auto material = materials[currentMaterial];

            if (token == "Kd") // Cor difusa (albedo)
            {
                float r, g, b;
                if (iss >> r >> g >> b)
                {
                    material->setAlbedo(glm::vec3(r, g, b));
                }
            }
            else if (token == "Ks") // Cor especular
            {
                float r, g, b;
                if (iss >> r >> g >> b)
                {
                    material->setSpecular(glm::vec3(r, g, b));
                }
            }
            else if (token == "Ke") // Cor emissiva
            {
                float r, g, b;
                if (iss >> r >> g >> b)
                {
                    material->setEmissive(glm::vec3(r, g, b));
                }
            }
            else if (token == "Ns") // Shininess
            {
                float shininess;
                if (iss >> shininess)
                {
                    material->setShininess(shininess);
                }
            }
            else if (token == "d") // Dissolve (opacity)
            {
                float alpha;
                if (iss >> alpha)
                {
                    material->setAlpha(alpha);
                }
            }
            else if (token == "Tr") // Transparency (inverso de d)
            {
                float transparency;
                if (iss >> transparency)
                {
                    material->setAlpha(1.0f - transparency);
                }
            }
            else if (token == "Ni") // Index of refraction
            {
                float ior;
                if (iss >> ior)
                {
                    material->setIndexOfRefraction(ior);
                }
            }
            // Ignora outras propriedades como map_Kd, illum, etc. por enquanto
        }
    }

    std::shared_ptr<Material> ModelLoader::createDefaultMaterial(const std::string &name)
    {
        auto material = std::make_shared<Material>(name);
        // Usa valores padrão da classe Material
        return material;
    }

    std::shared_ptr<Material> ModelLoader::detectMaterialFromName(const std::string &name)
    {
        std::string nameLower = name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        // =================== DETECÇÃO DE VIDRO ===================
        // Padrões comuns do Blender e nomes em português/inglês
        if (nameLower.find("glass") != std::string::npos || nameLower.find("vidro") != std::string::npos)
        {
            std::cout << "  -> Material de vidro detectado: " << name << std::endl;
            return std::make_shared<Material>(Material::createGlass(0.4f, glm::vec3(0.9f, 0.95f, 1.0f)));
        }

        // =================== DETECÇÃO DE METAL ===================
        if (nameLower.find("metal") != std::string::npos || nameLower.find("steel") != std::string::npos)
        {
            std::cout << "  -> Material metálico detectado: " << name << std::endl;

            // Cores específicas por tipo de metal
            glm::vec3 metalColor = glm::vec3(0.7f, 0.7f, 0.7f); // Padrão cinza

            if (nameLower.find("copper") != std::string::npos || nameLower.find("cobre") != std::string::npos)
                metalColor = glm::vec3(0.72f, 0.45f, 0.20f); // Cobre
            else if (nameLower.find("brass") != std::string::npos)
                metalColor = glm::vec3(0.71f, 0.65f, 0.26f); // Latão
            else if (nameLower.find("gold") != std::string::npos || nameLower.find("ouro") != std::string::npos)
                metalColor = glm::vec3(1.0f, 0.84f, 0.0f); // Ouro

            return std::make_shared<Material>(Material::createMetal(metalColor));
        }

        // =================== DETECÇÃO DE EMISSIVOS/LUZES ===================
        if (nameLower.find("light") != std::string::npos || nameLower.find("lamp") != std::string::npos)
        {
            std::cout << "  -> Material emissivo detectado: " << name << std::endl;
            auto material = std::make_shared<Material>(MaterialType::EMISSIVE, name);

            // Cores específicas por tipo de luz
            if (nameLower.find("red") != std::string::npos || nameLower.find("vermelho") != std::string::npos)
                material->setEmissive(glm::vec3(1.0f, 0.2f, 0.2f));
            else if (nameLower.find("blue") != std::string::npos || nameLower.find("azul") != std::string::npos)
                material->setEmissive(glm::vec3(0.2f, 0.2f, 1.0f));
            else if (nameLower.find("green") != std::string::npos || nameLower.find("verde") != std::string::npos)
                material->setEmissive(glm::vec3(0.2f, 1.0f, 0.2f));
            else
                material->setEmissive(glm::vec3(1.0f, 1.0f, 0.8f)); // Luz branca amarelada padrão

            material->setAlbedo(glm::vec3(0.9f, 0.9f, 0.8f));
            return material;
        }

        // =================== DETECÇÃO DE PLÁSTICO ===================
        if (nameLower.find("plastic") != std::string::npos ||
            nameLower.find("plastico") != std::string::npos ||
            nameLower.find("rubber") != std::string::npos ||
            nameLower.find("borracha") != std::string::npos)
        {
            std::cout << "  -> Material plástico detectado: " << name << std::endl;
            return std::make_shared<Material>(Material::createPlastic(glm::vec3(0.8f, 0.8f, 0.8f)));
        }

        // Retorna nullptr se não detectou nenhum tipo específico
        return nullptr;
    }

} // namespace cg
