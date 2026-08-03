#include "PlayerEffects/StarEffect.h"

#include <algorithm>

StarEffect::StarEffect(float duration)
    : remainingTime{std::max(0.f, duration)} {}

void StarEffect::update(Character&, float dt) {
    remainingTime = std::max(0.f, remainingTime - dt);
}

bool StarEffect::hasExpired() const {
    return remainingTime <= 0.f;
}

bool StarEffect::tryAbsorbDamage(Character&) {
    return !hasExpired();
}

int StarEffect::getDamagePriority() const {
    return 100;
}

bool StarEffect::defeatsEnemiesOnContact() const {
    return !hasExpired();
}
