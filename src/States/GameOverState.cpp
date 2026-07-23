#include "States/GameOverState.h"

void GameOverState::handleInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }
}

void GameOverState::update(float dt) {}

void GameOverState::render(sf::RenderWindow& window) {}
