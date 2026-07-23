#pragma once

#include "States/GameState.h"

class PlayState : public GameState {
public:
    PlayState() = default;
    void handleInput(sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
