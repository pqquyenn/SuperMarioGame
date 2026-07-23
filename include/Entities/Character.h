#pragma once

#include "Entities/Entity.h"

class Character : public Entity {
protected:
    float moveSpeed = 150.f;
    float jumpForce = 350.f;
    bool isGrounded = false;

public:
    Character(float x = 0.f, float y = 0.f) : Entity(x, y) {}
    virtual ~Character() = default;

    virtual void moveLeft(float dt);
    virtual void moveRight(float dt);
    virtual void jump();
    virtual void shootFireball();

    void setGrounded(bool grounded) { isGrounded = grounded; }
    bool getIsGrounded() const { return isGrounded; }
};
