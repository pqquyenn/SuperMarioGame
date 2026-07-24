#include "PlayerEffects/DamageInvincibilityEffect.h"

#include <algorithm>

DamageInvincibilityEffect::DamageInvincibilityEffect(
    float duration,
    float interval
)
    : remainingTime{std::max(0.f, duration)},
      flashInterval{std::max(0.01f, interval)} {}

void DamageInvincibilityEffect::update(Character&, float dt) {
    remainingTime = std::max(0.f, remainingTime - dt);
}

bool DamageInvincibilityEffect::hasExpired() const {
    return remainingTime <= 0.f;
}

bool DamageInvincibilityEffect::tryAbsorbDamage(Character&) {
    return !hasExpired();
}

int DamageInvincibilityEffect::getDamagePriority() const {
    return 90;
}

bool DamageInvincibilityEffect::isCharacterVisible() const {
    const int flashPhase =
        static_cast<int>(remainingTime / flashInterval);
    return flashPhase % 2 == 0;
}
