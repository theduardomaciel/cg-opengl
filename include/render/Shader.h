#pragma once
#include <string>
#include <glm/glm.hpp>

namespace cg {

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool compile(const char* vsSrc, const char* fsSrc);
    void bind() const;
    static void unbind();

    void setMat4(const char* name, const glm::mat4& m) const;

private:
    unsigned int mProgram = 0;
};

} // namespace cg

