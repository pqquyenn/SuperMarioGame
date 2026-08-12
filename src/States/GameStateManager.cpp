#include "States/GameStateManager.h"
#include <iostream>

// === Quan ly state (them vao pending, xu ly sau) ===

void GameStateManager::pushState(std::unique_ptr<GameState> state) {
    if (!state || hasPendingTransition()) return;
    pendingActions.push_back({Action::Push, std::move(state)});
}

void GameStateManager::popState() {
    if (hasPendingTransition()) return;
    pendingActions.push_back({Action::Pop, nullptr});
}

void GameStateManager::changeState(std::unique_ptr<GameState> state) {
    if (!state || hasPendingTransition()) return;
    pendingActions.push_back({Action::Change, std::move(state)});
}

void GameStateManager::clearAndPushState(std::unique_ptr<GameState> state) {
    if (!state || hasPendingTransition()) return;
    pendingActions.push_back({Action::ClearAndPush, std::move(state)});
}

// === Xu ly pending actions: goi moi dau frame ===
// Tranh thay doi stack khi dang duyet -> crash

void GameStateManager::processPendingActions() {
    for (auto& pa : pendingActions) {
        switch (pa.action) {
            case Action::Push:
                // The covered state remains alive; it is paused, not exited.
                if (!states.empty()) {
                    states.back()->onPause();
                }
                pa.state->setStateManager(this);
                states.push_back(std::move(pa.state));
                states.back()->onEnter();
                std::cout << "[StateManager] Pushed new state. Stack size: " << states.size() << std::endl;
                break;

            case Action::Pop:
                if (!states.empty()) {
                    states.back()->onExit();
                    states.pop_back();
                    std::cout << "[StateManager] Popped state. Stack size: " << states.size() << std::endl;
                    if (!states.empty()) {
                        states.back()->onResume();
                    }
                }
                break;

            case Action::Change:
                // Pop state cu
                if (!states.empty()) {
                    states.back()->onExit();
                    states.pop_back();
                }
                // Push state moi
                pa.state->setStateManager(this);
                states.push_back(std::move(pa.state));
                states.back()->onEnter();
                std::cout << "[StateManager] Changed state. Stack size: " << states.size() << std::endl;
                break;

            case Action::ClearAndPush:
                while (!states.empty()) {
                    states.back()->onExit();
                    states.pop_back();
                }
                pa.state->setStateManager(this);
                states.push_back(std::move(pa.state));
                states.back()->onEnter();
                std::cout << "[StateManager] Replaced state stack. Stack size: "
                          << states.size() << std::endl;
                break;
        }
    }
    pendingActions.clear();
}

// === Core loop: delegate cho state tren cung ===

void GameStateManager::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (!states.empty()) {
        states.back()->handleInput(event, window);
    }
}

void GameStateManager::update(float dt) {
    if (!states.empty()) {
        states.back()->update(dt);
    }
}

void GameStateManager::fixedUpdate(float dt) {
    if (!states.empty()) {
        states.back()->fixedUpdate(dt);
    }
}

void GameStateManager::render(sf::RenderWindow& window) {
    if (!states.empty()) {
        std::size_t firstState = states.size() - 1;
        while (firstState > 0 && states[firstState]->isTransparent()) {
            --firstState;
        }
        for (std::size_t i = firstState; i < states.size(); ++i) {
            states[i]->render(window);
        }
    }
}

// === Tien ich ===

GameState* GameStateManager::getCurrentState() const {
    if (!states.empty()) {
        return states.back().get();
    }
    return nullptr;
}
