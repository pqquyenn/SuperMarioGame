#include "PlayerEffects/ShieldEffect.h"

#include <algorithm>

ShieldEffect::ShieldEffect(int durability, float duration)
    : remainingHits{std::max(0, durability)},
      remainingTime{std::max(0.f, duration)},
      expiresByTime{duration >= 0.f} {}

void ShieldEffect::update(Character&, float dt) {
    if (expiresByTime) {
        remainingTime = std::max(0.f, remainingTime - dt);
    }
}

bool ShieldEffect::hasExpired() const {
    return remainingHits <= 0 ||
           (expiresByTime && remainingTime <= 0.f);
}

bool ShieldEffect::tryAbsorbDamage(Character&) {
    if (hasExpired()) {
        return false;
    }

    --remainingHits;
    return true;
}

int ShieldEffect::getDamagePriority() const {
    return 10;
}

int ShieldEffect::getRemainingHits() const {
    return remainingHits;
}
