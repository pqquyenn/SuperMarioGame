#include "States/PlayState.h"
#include "States/PauseState.h"
#include <iostream>
#include <memory>

void PlayState::onEnter() {
    std::cout << "[PlayState] onEnter - Bat dau choi game" << std::endl;
    // TODO: Load level, spawn Mario, bat dau nhac nen
}

void PlayState::onExit() {
    std::cout << "[PlayState] onExit - Tam dung game" << std::endl;
}

void PlayState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            // Nhan Escape -> push PauseState (PlayState van con trong stack)
            if (stateManager) {
                stateManager->pushState(std::make_unique<PauseState>());
            }
        }
    }
    // TODO: Xu ly phim di chuyen Mario (Left, Right, Jump...)
}

void PlayState::update(float dt) {
    // TODO: Update entities, physics, camera, timer...
}

void PlayState::render(sf::RenderWindow& window) {
    // TODO: Ve tilemap, entities, HUD (score, coins, time, lives)
}
