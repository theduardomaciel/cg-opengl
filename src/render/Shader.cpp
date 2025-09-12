#include "render/Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace cg
{

    // Função auxiliar para logar erros de compilação ou linkagem
    static bool checkCompile(GLuint obj, bool program, const char *label)
    {
        GLint success = 0;
        if (!program)
        {
            glGetShaderiv(obj, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                GLint len = 0;
                glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
                std::string log(len, '\0');
                glGetShaderInfoLog(obj, len, nullptr, log.data());
                std::cerr << "ERRO shader (" << label << "):\n"
                          << log << std::endl;
                return false;
            }
        }
        else
        {
            glGetProgramiv(obj, GL_LINK_STATUS, &success);
            if (!success)
            {
                GLint len = 0;
                glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
                std::string log(len, '\0');
                glGetProgramInfoLog(obj, len, nullptr, log.data());
                std::cerr << "ERRO link program (" << label << "):\n"
                          << log << std::endl;
                return false;
            }
        }
        return true;
    }

    Shader::~Shader()
    {
        if (mProgram)
            glDeleteProgram(mProgram);
    }

    bool Shader::compile(const char *vsSrc, const char *fsSrc)
    {
        // Compila vertex shader
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vsSrc, nullptr);
        glCompileShader(vs);
        bool okVS = checkCompile(vs, false, "VS");

        // Compila fragment shader
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fsSrc, nullptr);
        glCompileShader(fs);
        bool okFS = checkCompile(fs, false, "FS");

        if (!okVS || !okFS)
        {
            glDeleteShader(vs);
            glDeleteShader(fs);
            return false; // falhou compilação de algum estágio
        }

        // Linka programa
        mProgram = glCreateProgram();
        glAttachShader(mProgram, vs);
        glAttachShader(mProgram, fs);
        glLinkProgram(mProgram);
        bool okLink = checkCompile(mProgram, true, "Program");

        glDeleteShader(vs);
        glDeleteShader(fs);

        if (!okLink)
        {
            glDeleteProgram(mProgram);
            mProgram = 0;
            return false;
        }
        return true;
    }

    void Shader::bind() const { glUseProgram(mProgram); }
    void Shader::unbind() { glUseProgram(0); }

    // =================== IMPLEMENTAÇÃO DOS MÉTODOS PARA UNIFORMS ===================

    void Shader::setMat4(const char *name, const glm::mat4 &m) const
    {
        GLint loc = glGetUniformLocation(mProgram, name);
        if (loc != -1)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
        }
    }

    void Shader::setMat3(const char *name, const glm::mat3 &m) const
    {
        GLint loc = glGetUniformLocation(mProgram, name);
        if (loc != -1)
        {
            glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
        }
    }

    void Shader::setVec3(const char *name, const glm::vec3 &v) const
    {
        GLint loc = glGetUniformLocation(mProgram, name);
        if (loc != -1)
        {
            glUniform3fv(loc, 1, glm::value_ptr(v));
        }
    }

    void Shader::setVec4(const char *name, const glm::vec4 &v) const
    {
        GLint loc = glGetUniformLocation(mProgram, name);
        if (loc != -1)
        {
            glUniform4fv(loc, 1, glm::value_ptr(v));
        }
    }

    void Shader::setFloat(const char *name, float value) const
    {
        GLint loc = glGetUniformLocation(mProgram, name);
        if (loc != -1)
        {
            glUniform1f(loc, value);
        }
    }

    void Shader::setInt(const char *name, int value) const
    {
        GLint loc = glGetUniformLocation(mProgram, name);
        if (loc != -1)
        {
            glUniform1i(loc, value);
        }
    }

} // namespace cg
