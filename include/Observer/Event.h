#pragma once

enum class GameEventType {
    COIN_COLLECTED,
    ENEMY_DEFEATED,
    PLAYER_HIT,
    PLAYER_DIED,
    POWERUP_COLLECTED,
    LIFE_GAINED,
    FLAGPOLE_REACHED,
    MYSTERY_BLOCK_TOUCHED
};

struct GameEvent {
    GameEventType type;
    // For score-bearing events this is the score delta supplied by the
    // producer. For other events it is the event-specific payload, such as
    // the number of lives gained or the death cause.
    int value = 0;

    // Some events carry a non-score payload in value but still award score.
    // LIFE_GAINED is the current example: value is the number of lives and
    // scoreDelta preserves the separate score contribution.
    int scoreDelta = 0;

    static GameEvent coinCollected(int score) {
        return {GameEventType::COIN_COLLECTED, score};
    }

    static GameEvent enemyDefeated(int score) {
        return {GameEventType::ENEMY_DEFEATED, score};
    }

    static GameEvent powerupCollected(int score = 0) {
        return {GameEventType::POWERUP_COLLECTED, score};
    }

    static GameEvent lifeGained(int amount, int score = 0) {
        return {GameEventType::LIFE_GAINED, amount, score};
    }

    static GameEvent playerHit() {
        return {GameEventType::PLAYER_HIT};
    }

    static GameEvent playerDied(int cause) {
        return {GameEventType::PLAYER_DIED, cause};
    }
};
