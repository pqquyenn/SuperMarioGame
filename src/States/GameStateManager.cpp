#include "States/GameStateManager.h"

void GameStateManager::pushState(std::unique_ptr<GameState> state) {
    states.push(std::move(state));
}

void GameStateManager::popState() {
    if (!states.empty()) {
        states.pop();
    }
}

void GameStateManager::changeState(std::unique_ptr<GameState> state) {
    if (!states.empty()) {
        states.pop();
    }
    states.push(std::move(state));
}

void GameStateManager::handleInput(sf::RenderWindow& window) {
    if (!states.empty()) {
        states.top()->handleInput(window);
    }
}

void GameStateManager::update(float dt) {
    if (!states.empty()) {
        states.top()->update(dt);
    }
}

void GameStateManager::render(sf::RenderWindow& window) {
    if (!states.empty()) {
        states.top()->render(window);
    }
}
