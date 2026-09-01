#pragma once
#include "Entities/Enemies/Koopa.h"

class GreenParatroopa : public Koopa {
private:
    bool hasWings = true;
    static constexpr float HopVelocity = -220.f;
    bool onGround = false;

public:
    GreenParatroopa(float x = 0.f, float y = 0.f);
    ~GreenParatroopa() override = default;

    void update(float dt) override;
    void onStomped() override;
    void onLanded() override;
    void notifyLanded();

    bool isWinged() const { return hasWings; }
};
