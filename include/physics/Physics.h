#pragma once

namespace cg {

// Stub de física (futuro: integrar Bullet)
class Physics {
public:
    bool init() { return true; }
    void step(float /*dt*/) { /* placeholder */ }
};

} // namespace cg

