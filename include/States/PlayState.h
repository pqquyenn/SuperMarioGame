#pragma once

#include "States/GameState.h"
#include "States/GameStateManager.h"
#include "Level/Level.h"
#include "Entities/Mario.h"
#include "Input/InputHandler.h"
#include <memory>
#include <string>

class PlayState : public GameState {
private:
    Level level;
    std::unique_ptr<Mario> mario;
    InputHandler inputHandler;
    std::string initialMapPath;

public:
    PlayState(const std::string& mapPath = "1.1/1-1.txt");

    void onEnter() override;
    void onExit() override;
    void handleInput(sf::Event& event, sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    Mario* getMario() { return mario.get(); }
    const Mario* getMario() const { return mario.get(); }
};

