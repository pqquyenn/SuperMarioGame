#pragma once

#include "Entities/Entity.h"

class Enemy : public Entity {
protected:
    float speed{50.f};
    int direction{-1}; // -1: Left, 1: Right
    bool isAlive{true};
    bool squished{false};

public:
    Enemy(float x = 0.f, float y = 0.f);
    virtual ~Enemy() = default;

    virtual void onStomped() = 0;
    virtual void reverseDirection();

    int getDirection() const;
    void setDirection(int dir);

    float getSpeed() const;
    void setSpeed(float spd);

    bool isSquished() const;
    bool isEnemyAlive() const;
};

