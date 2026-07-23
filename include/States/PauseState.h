#pragma once

#include "States/GameState.h"

class PauseState : public GameState {
public:
    PauseState() = default;
    void handleInput(sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
