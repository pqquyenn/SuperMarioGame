#pragma once

#include <SFML/Graphics.hpp>

class GameState {
public:
    virtual ~GameState() = default;
    virtual void handleInput(sf::RenderWindow& window) = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};
