#include "States/GameOverState.h"
#include "States/MenuState.h"
#include <iostream>
#include <memory>

void GameOverState::onEnter() {
    std::cout << "[GameOverState] onEnter - GAME OVER" << std::endl;
    // TODO: Hien thi "GAME OVER", diem so cuoi cung
}

void GameOverState::onExit() {
    std::cout << "[GameOverState] onExit - Roi khoi man hinh Game Over" << std::endl;
}

void GameOverState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
            // Nhan Enter -> quay ve Menu
            if (stateManager) {
                stateManager->changeState(std::make_unique<MenuState>());
            }
        }
    }
}

void GameOverState::update(float dt) {
    // TODO: Update animation Game Over (neu co)
}

void GameOverState::render(sf::RenderWindow& window) {
    // TODO: Ve man hinh Game Over, diem so, "Press Enter to return to Menu"
}
