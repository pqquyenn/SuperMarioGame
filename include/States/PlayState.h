#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include "Level/Level.h"
#include "Entities/Mario.h"
#include "Input/InputHandler.h"
#include "UI/HUD.h"
#include <memory>

class PlayState : public GameState {
private:
    Level level;
    std::unique_ptr<Mario> mario;
    InputHandler inputHandler;
    HUD hud;

public:
    PlayState() = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    Mario* getMario() { return mario.get(); }
    const Mario* getMario() const { return mario.get(); }
};

