#pragma once

#include "Entities/Entity.h"

class Enemy : public Entity {
protected:
    float speed = 50.f;
    int direction = -1; // -1 for left, 1 for right

public:
    Enemy(float x = 0.f, float y = 0.f) : Entity(x, y) {}
    virtual ~Enemy() = default;

    virtual void onStomped() = 0;
};
