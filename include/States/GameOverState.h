#pragma once

#include "States/GameState.h"

class GameOverState : public GameState {
public:
    GameOverState() = default;
    void handleInput(sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
