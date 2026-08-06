#include "UI/HUD.h"

HUD::HUD() {}

void HUD::onNotify(const GameEvent& event) {
    switch (event.type) {
        case GameEventType::COIN_COLLECTED:
            coins += 1;
            score += 200;
            break;
        case GameEventType::ENEMY_DEFEATED:
            score += 100;
            break;
        case GameEventType::PLAYER_DIED:
            lives -= 1;
            break;
        default:
            break;
    }
}

void HUD::update(float dt) {
    if (timeRemaining > 0) {
        timeRemaining -= dt;
    }
}

void HUD::render(sf::RenderWindow& window) {
    // Render HUD overlay
}
