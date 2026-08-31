#pragma once

#include "Observer/Observer.h"
#include "PvP/PvPTypes.h"

class PvPPlayerSession final : public Observer {
private:
    PlayerId playerId;
    int lifeCount;
    int scoreValue{0};
    int coinCount{0};
    float protectionRemaining{0.f};
    float projectileCooldownRemaining{0.f};
    float firePowerRemaining{0.f};
    bool firePowerGranted{false};

public:
    explicit PvPPlayerSession(PlayerId id, int startingLives = 3);

    void onNotify(const GameEvent& event) override;
    void update(float dt);

    PlayerId id() const { return playerId; }
    int lives() const { return lifeCount; }
    int score() const { return scoreValue; }
    int coins() const { return coinCount; }
    float spawnProtection() const { return protectionRemaining; }
    float fireCooldown() const { return projectileCooldownRemaining; }
    float fireTime() const { return firePowerRemaining; }

    bool isSpawnProtected() const { return protectionRemaining > 0.f; }
    bool canFire() const {
        return firePowerGranted && firePowerRemaining > 0.f &&
               projectileCooldownRemaining <= 0.f;
    }

    void beginSpawnProtection(float duration);
    void beginFireCooldown(float duration);
    void grantFirePower(float duration);
    bool consumeExpiredFirePower();
    void clearFirePower();
    void loseLife();
    void applyFriendlyDeathPenalty();
};
