#pragma once
#include "render/Model.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace cg
{

    /**
     * @brief Utilitário para carregar modelos 3D do formato OBJ
     *
     * Esta classe implementa um parser simples para arquivos .obj que suporta:
     * - Vértices (v)
     * - Normais (vn)
     * - Coordenadas de textura (vt)
     * - Faces triangulares (f)
     * - Múltiplos objetos em um arquivo (o nome)
     * - Comentários (#)
     *
     * Limitações atuais:
     * - Apenas faces triangulares (não quads)
     * - Não suporta grupos (g)
     */
    class ModelLoader
    {
    public:
        /**
         * @brief Estrutura para armazenar estatísticas do carregamento
         */
        struct LoadStats
        {
            size_t totalVertices = 0;  // Total de vértices únicos carregados
            size_t totalTriangles = 0; // Total de triângulos carregados
            size_t totalMeshes = 0;    // Total de meshes criadas
            float loadTimeMs = 0.0f;   // Tempo de carregamento em milissegundos

            void print() const; // Imprime estatísticas no console
        };

        /**
         * @brief Carrega um modelo 3D de um arquivo OBJ
         * @param filePath Caminho para o arquivo .obj
         * @param modelName Nome opcional para o modelo (se vazio, usa o nome do arquivo)
         * @return Ponteiro único para o modelo carregado, ou nullptr se houve erro
         */
        static std::unique_ptr<Model> loadModel(const std::string &filePath,
                                                const std::string &modelName = "");

        /**
         * @brief Obtém as estatísticas do último carregamento
         * @return Estrutura com estatísticas detalhadas
         */
        static const LoadStats &getLastLoadStats() { return sLastStats; }

    private:
        // =================== ESTRUTURAS INTERNAS ===================

        /**
         * @brief Estrutura temporária para armazenar dados durante o parsing
         */
        struct ParseData
        {
            std::vector<glm::vec3> positions; // Posições de vértices (v)
            std::vector<glm::vec3> normals;   // Normais (vn)
            std::vector<glm::vec2> texCoords; // Coordenadas de textura (vt)

            // Dados da mesh atual sendo processada
            std::vector<Vertex> currentVertices;
            std::vector<GLuint> currentIndices;
            std::string currentObjectName;
            std::shared_ptr<Material> currentMaterial; // Material atual

            // Mapa para evitar vértices duplicados (otimização)
            std::unordered_map<std::string, GLuint> vertexMap;

            // Biblioteca de materiais carregados
            std::unordered_map<std::string, std::shared_ptr<Material>> materials;
        };

        /**
         * @brief Estrutura para representar um índice de face no formato OBJ
         */
        struct FaceIndex
        {
            int positionIndex = -1; // Índice da posição (obrigatório)
            int texCoordIndex = -1; // Índice da coordenada de textura (opcional)
            int normalIndex = -1;   // Índice da normal (opcional)

            // Converte índices relativos do OBJ para absolutos
            void makeAbsolute(size_t posCount, size_t texCount, size_t normCount);

            // Cria string única para identificar este vértice (para evitar duplicatas)
            std::string getKey() const;
        };

        // =================== MÉTODOS DE PARSING ===================

        /**
         * @brief Faz o parsing de uma linha do arquivo OBJ
         * @param line Linha a ser processada
         * @param data Dados de parsing onde armazenar resultados
         * @param model Modelo onde adicionar meshes completas
         * @param objFilePath Caminho do arquivo OBJ (para resolver caminhos relativos de .mtl)
         */
        static void parseLine(const std::string &line, ParseData &data, Model &model, const std::string &objFilePath);

        /**
         * @brief Processa uma linha de vértice (v x y z)
         */
        static void parseVertex(const std::string &line, ParseData &data);

        /**
         * @brief Processa uma linha de normal (vn x y z)
         */
        static void parseNormal(const std::string &line, ParseData &data);

        /**
         * @brief Processa uma linha de coordenada de textura (vt u v)
         */
        static void parseTexCoord(const std::string &line, ParseData &data);

        /**
         * @brief Processa uma linha de face (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3)
         */
        static void parseFace(const std::string &line, ParseData &data);

        /**
         * @brief Processa uma linha de objeto (o nome)
         */
        static void parseObject(const std::string &line, ParseData &data, Model &model);

        /**
         * @brief Processa uma linha de biblioteca de materiais (mtllib arquivo.mtl)
         */
        static void parseMaterialLib(const std::string &line, ParseData &data, const std::string &objFilePath);

        /**
         * @brief Processa uma linha de uso de material (usemtl nome_material)
         */
        static void parseUseMaterial(const std::string &line, ParseData &data);

        /**
         * @brief Finaliza a mesh atual e a adiciona ao modelo
         */
        static void finalizeMesh(ParseData &data, Model &model);

        /**
         * @brief Processa um índice de face no formato "v/vt/vn" ou "v//vn" ou "v"
         */
        static FaceIndex parseFaceIndex(const std::string &indexStr);

        /**
         * @brief Cria ou reutiliza um vértice baseado nos índices da face
         */
        static GLuint getOrCreateVertex(const FaceIndex &faceIndex, ParseData &data);

        /**
         * @brief Calcula normais automaticamente se não estiverem presentes no arquivo
         */
        static void calculateNormals(ParseData &data);

        /**
         * @brief Remove espaços em branco do início e fim de uma string
         */
        static std::string trim(const std::string &str);

        /**
         * @brief Divide uma string em tokens usando um delimitador
         */
        static std::vector<std::string> split(const std::string &str, char delimiter);

        // =================== MÉTODOS PARA MATERIAIS ===================

        /**
         * @brief Carrega um arquivo .mtl e retorna um mapa de materiais
         * @param filePath Caminho para o arquivo .mtl
         * @return Mapa de nome_material -> Material
         */
        static std::unordered_map<std::string, std::shared_ptr<Material>> loadMaterials(const std::string &filePath);

        /**
         * @brief Faz o parsing de uma linha do arquivo .mtl
         * @param line Linha a ser processada
         * @param materials Mapa de materiais onde armazenar resultados
         * @param currentMaterial Nome do material sendo processado atualmente
         */
        static void parseMaterialLine(const std::string &line,
                                      std::unordered_map<std::string, std::shared_ptr<Material>> &materials,
                                      std::string &currentMaterial);

        /**
         * @brief Cria um material padrão caso não seja encontrado
         * @param name Nome do material
         * @return Material padrão
         */
        static std::shared_ptr<Material> createDefaultMaterial(const std::string &name);

        /**
         * @brief Detecta automaticamente o tipo de material baseado no nome
         * @param name Nome do objeto/material
         * @return Material apropriado ou nullptr se não detectado
         */
        static std::shared_ptr<Material> detectMaterialFromName(const std::string &name);

        // =================== DADOS ESTÁTICOS ===================
        static LoadStats sLastStats; // Estatísticas do último carregamento
    };

} // namespace cg
