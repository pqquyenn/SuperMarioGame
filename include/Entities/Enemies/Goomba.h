#pragma once

#include "Entities/Enemies/Enemy.h"

class Goomba : public Enemy {
public:
    Goomba(float x = 0.f, float y = 0.f);
    void update(float dt) override;
    void onStomped() override;
};
