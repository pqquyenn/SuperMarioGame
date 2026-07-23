#include "States/PlayState.h"

void PlayState::handleInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }
}

void PlayState::update(float dt) {
    // Game loop physics & entity updates
}

void PlayState::render(sf::RenderWindow& window) {
    // Render entities, tilemaps, HUD
}
