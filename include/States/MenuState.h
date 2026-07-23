#pragma once

#include "States/GameState.h"

class MenuState : public GameState {
public:
    MenuState() = default;
    void handleInput(sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
