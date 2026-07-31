#include "States/MenuState.h"
#include "States/PlayState.h"
#include <iostream>
#include <memory>

void MenuState::onEnter() {
    std::cout << "[MenuState] onEnter - Hien thi menu chinh" << std::endl;
    // TODO: Load font, tao text "SUPER MARIO BROS", "Press Enter to Start"
}

void MenuState::onExit() {
    std::cout << "[MenuState] onExit - Roi khoi menu" << std::endl;
}

void MenuState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
            // Nhan Enter -> chuyen sang PlayState
            if (stateManager) {
                stateManager->changeState(std::make_unique<PlayState>());
            }
        }
    }
}

void MenuState::update(float dt) {
    // TODO: Update animation menu (nhap nhay text, animation background...)
}

void MenuState::render(sf::RenderWindow& window) {
    // TODO: Ve background menu, title, "Press Enter to Start"
}
