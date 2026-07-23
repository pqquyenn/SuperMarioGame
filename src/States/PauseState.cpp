#include "States/PauseState.h"

void PauseState::handleInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }
}

void PauseState::update(float dt) {}

void PauseState::render(sf::RenderWindow& window) {}
