#include "PlayerEffects/StarEffect.h"
#include "Core/SoundManager.h"
#include <algorithm>

StarEffect::StarEffect(float duration)
    : remainingTime{std::max(0.f, duration)} {}

void StarEffect::onApply(Character& character) {
    (void)character;
    previousBgm = SoundManager::getInstance().getCurrentBGM();
    if (previousBgm.empty()) {
        previousBgm = "assets/audio/music/overworld.wav";
    }
    SoundManager::getInstance().playBGM("assets/audio/music/superstar.wav");
}

void StarEffect::onRemove(Character& character) {
    (void)character;
    if (!previousBgm.empty()) {
        SoundManager::getInstance().playBGM(previousBgm);
    } else {
        SoundManager::getInstance().playBGM("assets/audio/music/overworld.wav");
    }
}

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

float StarEffect::getMoveSpeedMultiplier() const {
    return hasExpired() ? 1.f : 1.5f;
}

bool StarEffect::isCharacterVisible() const {
    return true;
}


