#pragma once

#include "PlayerEffects/PlayerEffect.h"

class StarEffect : public PlayerEffect {
private:
    float remainingTime;

public:
    explicit StarEffect(float duration = 10.f);

    void update(Character& character, float dt) override;
    bool hasExpired() const override;
    bool tryAbsorbDamage(Character& character) override;
    int getDamagePriority() const override;
    bool defeatsEnemiesOnContact() const override;
};
