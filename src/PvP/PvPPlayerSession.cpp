#include "PvP/PvPPlayerSession.h"

#include "PvP/PvPMatchRules.h"

#include <algorithm>

PvPPlayerSession::PvPPlayerSession(PlayerId id, int startingLives)
    : playerId{id}, lifeCount{std::max(0, startingLives)} {}

void PvPPlayerSession::onNotify(const GameEvent& event) {
    if (event.type == GameEventType::COIN_COLLECTED) {
        ++coinCount;
        scoreValue += event.value;
    } else if (event.type == GameEventType::ENEMY_DEFEATED) {
        scoreValue += event.value;
    }
}

void PvPPlayerSession::update(float dt) {
    const float elapsed = std::max(0.f, dt);
    protectionRemaining = std::max(0.f, protectionRemaining - elapsed);
    projectileCooldownRemaining =
        std::max(0.f, projectileCooldownRemaining - elapsed);
    firePowerRemaining = std::max(0.f, firePowerRemaining - elapsed);
}

void PvPPlayerSession::beginSpawnProtection(float duration) {
    protectionRemaining = std::max(0.f, duration);
}

void PvPPlayerSession::beginFireCooldown(float duration) {
    projectileCooldownRemaining = std::max(0.f, duration);
}

void PvPPlayerSession::grantFirePower(float duration) {
    firePowerRemaining = std::max(0.f, duration);
    firePowerGranted = firePowerRemaining > 0.f;
}

bool PvPPlayerSession::consumeExpiredFirePower() {
    if (!firePowerGranted || firePowerRemaining > 0.f) {
        return false;
    }
    firePowerGranted = false;
    firePowerRemaining = 0.f;
    return true;
}

void PvPPlayerSession::clearFirePower() {
    firePowerGranted = false;
    firePowerRemaining = 0.f;
}

void PvPPlayerSession::loseLife() {
    lifeCount = std::max(0, lifeCount - 1);
}

void PvPPlayerSession::applyFriendlyDeathPenalty() {
    scoreValue = PvPMatchRules::friendlyDeathScore(scoreValue);
}
