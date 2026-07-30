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

// === Core Loop Steps ===

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        // Chuyen event cho state hien tai xu ly (menu, play, pause...)
        stateManager.handleInput(window);
    }
}

void Game::update(float dt) {
    // Variable timestep update: dung cho animation, UI, camera...
    // dt thay doi tuy theo toc do frame thuc te
    stateManager.update(dt);
}

void Game::fixedUpdate(float fixedDt) {
    // Fixed timestep update: dung cho physics, collision, movement...
    // fixedDt LUON = TIME_PER_FRAME (1/60s) -> dam bao physics
    // chay giong nhau tren moi may bat ke FPS thuc te
    // TODO: Goi physics update cua stateManager khi co physics system
    // Vi du: stateManager.fixedUpdate(fixedDt);
}
