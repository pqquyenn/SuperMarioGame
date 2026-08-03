#pragma once

#include "PlayerEffects/PlayerEffect.h"

class ShieldEffect : public PlayerEffect {
private:
    int remainingHits;
    float remainingTime;
    bool expiresByTime;

public:
    explicit ShieldEffect(
        int durability,
        float duration = -1.f
    );

    void update(Character& character, float dt) override;
    bool hasExpired() const override;
    bool tryAbsorbDamage(Character& character) override;
    int getDamagePriority() const override;

    int getRemainingHits() const;
};
