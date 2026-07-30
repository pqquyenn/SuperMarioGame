#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"

class MenuState : public GameState {
public:
    MenuState() = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};
