#pragma once

enum class GameEventType {
    COIN_COLLECTED,
    ENEMY_DEFEATED,
    PLAYER_HIT,
    PLAYER_DIED,
    POWERUP_COLLECTED
};

struct GameEvent {
    GameEventType type;
    int value = 0;
};
