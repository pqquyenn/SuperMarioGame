#pragma once
#include "Entities/Enemies/RedKoopa.h"

class RedParatroopa : public RedKoopa {
private:
    bool hasWings = true;
    float startY;
    float flyMinY;
    float flyMaxY;
    static constexpr float FlySpeed = 60.f;
    int flyDir = -1; // -1 = up, +1 = down

public:
    RedParatroopa(float x = 0.f, float y = 0.f, const TileMap* map = nullptr,
                  float rangeUp = 48.f, float rangeDown = 48.f);
    ~RedParatroopa() override = default;

    void update(float dt) override;
    void onStomped() override;

    bool isWinged() const { return hasWings; }
};
