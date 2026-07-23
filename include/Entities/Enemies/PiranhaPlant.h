#pragma once

#include "Entities/Enemies/Enemy.h"

class PiranhaPlant : public Enemy {
public:
    PiranhaPlant(float x = 0.f, float y = 0.f);
    void update(float dt) override;
    void onStomped() override;
};
