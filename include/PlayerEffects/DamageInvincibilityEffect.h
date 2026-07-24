#pragma once

#include "PlayerEffects/PlayerEffect.h"

class DamageInvincibilityEffect : public PlayerEffect {
private:
    float remainingTime;
    float flashInterval;

public:
    explicit DamageInvincibilityEffect(
        float duration = 2.f,
        float flashInterval = 0.1f
    );

    void update(Character& character, float dt) override;
    bool hasExpired() const override;
    bool tryAbsorbDamage(Character& character) override;
    int getDamagePriority() const override;
    bool isCharacterVisible() const override;
};
