#include "States/MenuState.h"

void MenuState::handleInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }
}

void MenuState::update(float dt) {
    // Menu logic update
}

void MenuState::render(sf::RenderWindow& window) {
    // Render Menu UI
}
