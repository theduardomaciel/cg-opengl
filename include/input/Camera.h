#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cg {

class Camera {
public:
    Camera() = default;

    void setPosition(const glm::vec3& p) { mPosition = p; }
    const glm::vec3& position() const { return mPosition; }
    const glm::vec3& front() const { return mFront; }
    const glm::vec3& up() const { return mUp; }

    float yaw() const { return mYaw; }
    float pitch() const { return mPitch; }

    void processMouse(float xoffset, float yoffset, float sensitivity = 0.1f);
    void moveForward(float amount);
    void moveRight(float amount);

    glm::mat4 viewMatrix() const;

private:
    void updateVectors();

    glm::vec3 mPosition{0.f, 1.8f, 5.f};
    glm::vec3 mFront{0.f, 0.f, -1.f};
    glm::vec3 mUp{0.f, 1.f, 0.f};
    glm::vec3 mRight{1.f, 0.f, 0.f};

    float mYaw = -90.f;
    float mPitch = 0.f;
};

} // namespace cg

