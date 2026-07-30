#include "States/PauseState.h"
#include "States/MenuState.h"
#include <iostream>
#include <memory>

void PauseState::onEnter() {
    std::cout << "[PauseState] onEnter - Game da tam dung" << std::endl;
    // TODO: Hien thi overlay "PAUSED", menu Resume/Quit
}

void PauseState::onExit() {
    std::cout << "[PauseState] onExit - Tiep tuc game" << std::endl;
}

void PauseState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            // Nhan Escape -> pop PauseState -> quay ve PlayState
            if (stateManager) {
                stateManager->popState();
            }
        }
        else if (event.key.code == sf::Keyboard::Q) {
            // Nhan Q -> quay ve Menu (thay the toan bo stack)
            if (stateManager) {
                stateManager->changeState(std::make_unique<MenuState>());
            }
        }
    }
}

void PauseState::update(float dt) {
    // Pause state: khong update game logic
    // TODO: Update animation pause menu (neu co)
}

void PauseState::render(sf::RenderWindow& window) {
    // TODO: Ve overlay ban trong suot len tren PlayState
    // TODO: Ve text "PAUSED", "Press ESC to Resume", "Press Q to Quit"
}
