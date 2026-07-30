#include "Core/Game.h"
#include "States/MenuState.h"
#include <iostream>
#include <thread>   // std::this_thread::sleep_for (dung cho manual FPS capping)
#include <chrono>   // std::chrono::duration (do thoi gian chinh xac)
#include <memory>

// === Initialization ===

void Game::initWindow() {
    window.create(
        sf::VideoMode(800, 600),
        "Super Mario Bros (C++ SFML 2.6.1)",
        sf::Style::Close | sf::Style::Titlebar
    );

    // KHONG dung setFramerateLimit() vi ta tu cap FPS bang tay trong run()
    // window.setFramerateLimit(60);  // <-- bo di

    // Tat VSync de tranh xung dot voi manual FPS capping
    window.setVerticalSyncEnabled(false);
}

void Game::initStates() {
    stateManager.pushState(std::make_unique<MenuState>());
}
