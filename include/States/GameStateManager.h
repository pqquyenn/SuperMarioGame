#pragma once

#include "States/GameState.h"
#include <stack>
#include <memory>

class GameStateManager {
private:
    std::stack<std::unique_ptr<GameState>> states;

public:
    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void changeState(std::unique_ptr<GameState> state);

    void handleInput(sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderWindow& window);

    bool isEmpty() const { return states.empty(); }
};
