#pragma once

#include <SFML/Graphics.hpp>
#include "States/GameStateManager.h"

class Game {
private:
    sf::RenderWindow window;
    GameStateManager stateManager;
    sf::Clock clock;

    void initWindow();
    void initStates();

public:
    Game();
    ~Game() = default;

    void run();
};
