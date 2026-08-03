#include "PlayerEffects/PlayerEffect.h"

void PlayerEffect::onApply(Character&) {}

void PlayerEffect::onRemove(Character&) {}

float PlayerEffect::getMoveSpeedMultiplier() const {
    return 1.f;
}

float PlayerEffect::getJumpForceMultiplier() const {
    return 1.f;
}

bool PlayerEffect::tryAbsorbDamage(Character&) {
    return false;
}

int PlayerEffect::getDamagePriority() const {
    return 0;
}

bool PlayerEffect::defeatsEnemiesOnContact() const {
    return false;
}

bool PlayerEffect::isCharacterVisible() const {
    return true;
}
