#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include "Level/Level.h"

class PlayState : public GameState {
private:
    Level level;

public:
    PlayState() = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
};

