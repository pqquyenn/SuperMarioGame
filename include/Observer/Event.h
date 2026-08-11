#pragma once

enum class GameEventType {
    COIN_COLLECTED,
    ENEMY_DEFEATED,
    PLAYER_HIT,
    PLAYER_DIED,
    POWERUP_COLLECTED,
    LIFE_GAINED
};

struct GameEvent {
    GameEventType type;
    int value = 0;
};
