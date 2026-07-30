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

void Game::render() {
    window.clear(sf::Color(107, 140, 255)); // Classic Mario sky blue
    stateManager.render(window);
    window.display();
}

// === Constructor ===

Game::Game() : accumulator(0.0f) {
    initWindow();
    initStates();
}

// === Main Game Loop ===

void Game::run() {
    clock.restart();

    while (window.isOpen()) {
        // 1. Tinh DeltaTime: thoi gian thuc te cua frame truoc
        float dt = clock.restart().asSeconds();

        // 2. Chong "Spiral of Death": neu dt qua lon (vd: debug breakpoint,
        //    lag dot ngot), gioi han lai de tranh fixedUpdate chay hang tram lan
        if (dt > 0.25f) {
            dt = 0.25f;
        }

        // 3. Xu ly input/event
        processEvents();

        // 4. Fixed Timestep: tich luy thoi gian va chay physics deu dan
        //    VD: may chay 120 FPS -> fixedUpdate chay 1 lan moi 2 frame
        //        may chay 30 FPS  -> fixedUpdate chay 2 lan moi frame
        accumulator += dt;
        while (accumulator >= TIME_PER_FRAME) {
            fixedUpdate(TIME_PER_FRAME);
            accumulator -= TIME_PER_FRAME;
        }

        // 5. Variable update (animation, UI, camera...)
        update(dt);

        // 6. Render
        render();

        // 7. Manual FPS Capping: neu frame xu ly xong som, sleep cho du 1/60s
        sf::Time frameTime = clock.getElapsedTime();
        if (frameTime.asSeconds() < TIME_PER_FRAME) {
            sf::sleep(sf::seconds(TIME_PER_FRAME - frameTime.asSeconds()));
        }
    }
}
