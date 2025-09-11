#include "input/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cg {

void Camera::processMouse(float xoffset, float yoffset, float sensitivity) {
    // Aplica sensibilidade aos offsets do mouse
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Atualiza ângulos de rotação
    mYaw += xoffset;    // rotação horizontal (esquerda/direita)
    mPitch += yoffset;  // rotação vertical (cima/baixo)

    // Limita pitch para evitar flip da câmera (não pode olhar mais de 89° para cima/baixo)
    if (mPitch > 89.f) mPitch = 89.f;
    if (mPitch < -89.f) mPitch = -89.f;

    // Recalcula vetores direcionais baseado nos novos ângulos
    updateVectors();
}

void Camera::moveForward(float amount) {
    // Movimento apenas no plano XZ (ignora componente Y para não "voar")
    // Normaliza o vetor front projetado no plano horizontal
    glm::vec3 forwardXZ = glm::normalize(glm::vec3(mFront.x, 0.f, mFront.z));
    mPosition += forwardXZ * amount;
}

void Camera::moveRight(float amount) {
    // Movimento lateral no plano XZ
    // Usa vetor right projetado no plano horizontal
    glm::vec3 rightXZ = glm::normalize(glm::vec3(mRight.x, 0.f, mRight.z));
    mPosition += rightXZ * amount;
}

glm::mat4 Camera::viewMatrix() const {
    // Constrói matriz de visualização usando lookAt
    // Olha da posição atual na direção (posição + front)
    return glm::lookAt(mPosition, mPosition + mFront, mUp);
}

void Camera::updateVectors() {
    // Calcula novo vetor direcional baseado nos ângulos yaw e pitch
    glm::vec3 dir;
    dir.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));  // componente X
    dir.y = sin(glm::radians(mPitch));                            // componente Y
    dir.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));  // componente Z
    mFront = glm::normalize(dir);

    // Recalcula vetores right e up baseados no novo front
    glm::vec3 worldUp{0.f, 1.f, 0.f};  // up mundial (Y positivo)
    mRight = glm::normalize(glm::cross(mFront, worldUp));  // produto vetorial para right
    mUp = glm::normalize(glm::cross(mRight, mFront));      // produto vetorial para up local
}

} // namespace cg
