#include "States/GameStateManager.h"
#include <iostream>

// === Quan ly state (them vao pending, xu ly sau) ===

void GameStateManager::pushState(std::unique_ptr<GameState> state) {
    pendingActions.push_back({Action::Push, std::move(state)});
}

void GameStateManager::popState() {
    pendingActions.push_back({Action::Pop, nullptr});
}

void GameStateManager::changeState(std::unique_ptr<GameState> state) {
    pendingActions.push_back({Action::Change, std::move(state)});
}

// === Xu ly pending actions: goi moi dau frame ===
// Tranh thay doi stack khi dang duyet -> crash

void GameStateManager::processPendingActions() {
    for (auto& pa : pendingActions) {
        switch (pa.action) {
            case Action::Push:
                // Goi onExit cua state cu (neu co)
                if (!states.empty()) {
                    states.top()->onExit();
                }
                // Set manager cho state moi va push
                pa.state->setStateManager(this);
                states.push(std::move(pa.state));
                states.top()->onEnter();
                std::cout << "[StateManager] Pushed new state. Stack size: " << states.size() << std::endl;
                break;

            case Action::Pop:
                if (!states.empty()) {
                    states.top()->onExit();
                    states.pop();
                    std::cout << "[StateManager] Popped state. Stack size: " << states.size() << std::endl;
                    // Goi onEnter cua state phia duoi (quay lai)
                    if (!states.empty()) {
                        states.top()->onEnter();
                    }
                }
                break;

            case Action::Change:
                // Pop state cu
                if (!states.empty()) {
                    states.top()->onExit();
                    states.pop();
                }
                // Push state moi
                pa.state->setStateManager(this);
                states.push(std::move(pa.state));
                states.top()->onEnter();
                std::cout << "[StateManager] Changed state. Stack size: " << states.size() << std::endl;
                break;
        }
    }
    pendingActions.clear();
}

// === Core loop: delegate cho state tren cung ===

void GameStateManager::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (!states.empty()) {
        states.top()->handleInput(event, window);
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

// === Tien ich ===

GameState* GameStateManager::getCurrentState() const {
    if (!states.empty()) {
        return states.top().get();
    }
    return nullptr;
}
