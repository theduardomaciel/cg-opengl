#pragma once
#include <string>
#include <glm/glm.hpp>

namespace cg
{

    class Shader
    {
    public:
        Shader() = default;
        ~Shader();

        bool compile(const char *vsSrc, const char *fsSrc);
        void bind() const;
        static void unbind();

        // =================== MÉTODOS PARA DEFINIR UNIFORMS ===================
        void setMat4(const char *name, const glm::mat4 &m) const;
        void setMat3(const char *name, const glm::mat3 &m) const;
        void setVec3(const char *name, const glm::vec3 &v) const;
        void setVec4(const char *name, const glm::vec4 &v) const;
        void setFloat(const char *name, float value) const;
        void setInt(const char *name, int value) const;

    private:
        unsigned int mProgram = 0;
    };

} // namespace cg
