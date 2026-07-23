#pragma once

#include "Entities/Enemies/Enemy.h"

class Koopa : public Enemy {
private:
    bool inShell = false;
    bool shellSpinning = false;

public:
    Koopa(float x = 0.f, float y = 0.f);
    void update(float dt) override;
    void onStomped() override;
};
