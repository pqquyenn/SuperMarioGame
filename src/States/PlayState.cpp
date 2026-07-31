#include "States/PlayState.h"
#include "States/PauseState.h"
#include <iostream>
#include <memory>

void PlayState::onEnter() {
    std::cout << "[PlayState] onEnter - Bat dau map 1-1" << std::endl;
    if (!level.loadLevel("1.1/1-1.txt")) {
        std::cerr << "[PlayState] Failed to load level 1.1/1-1.txt!" << std::endl;
    }

    Camera& cam = level.getCamera();
    cam.setSize(400.f, 225.f);

    const float levelPixelW = 244.f * 16.f; // 3904px
    const float levelPixelH = 16.f * 16.f;  // 256px
    cam.setLevelBounds(levelPixelW, levelPixelH);
    cam.setCenter(200.f, 112.f);
}

void PlayState::onExit() {
    std::cout << "[PlayState] onExit - Tam dung / Roi khoi PlayState" << std::endl;
}

void PlayState::handleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            // Nhan Escape -> push PauseState (PlayState van con trong stack)
            if (stateManager) {
                stateManager->pushState(std::make_unique<PauseState>());
            }
        }
        else if (event.key.code == sf::Keyboard::U || event.key.code == sf::Keyboard::H) {
            std::cout << "[PlayState] Load Underground Map..." << std::endl;
            if (level.loadHiddenMap("underground.txt")) {
                level.getCamera().setCenter(160.f, 120.f);
                level.getTileMap().setNeedsRedraw(true);
            }
        }
        else if (event.key.code == sf::Keyboard::M || event.key.code == sf::Keyboard::Num1) {
            std::cout << "[PlayState] Return to Main Overworld Map..." << std::endl;
            if (level.loadLevel("1.1/1-1.txt")) {
                level.getCamera().setCenter(200.f, 112.f);
                level.getTileMap().setNeedsRedraw(true);
            }
        }
    }
}



void PlayState::update(float dt) {
    Camera& cam = level.getCamera();
    const float speed = 200.0f * dt;
    bool moved = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        cam.move(speed, 0.f);
        moved = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        cam.move(-speed, 0.f);
        moved = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        cam.move(0.f, speed);
        moved = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        cam.move(0.f, -speed);
        moved = true;
    }

    if (moved) {
        level.getTileMap().setNeedsRedraw(true);
    }

    level.update(dt);
}

void PlayState::render(sf::RenderWindow& window) {
    Camera& cam = level.getCamera();
    cam.applyTo(window);

    float camX = cam.getView().getCenter().x;
    float camY = cam.getView().getCenter().y;
    bool isUndergroundArea = level.getIsUnderground() || camY >= 240.f || camX > 3280.f;
    sf::Color bgColor = isUndergroundArea ? sf::Color::Black : sf::Color(92, 148, 252);

    window.clear(bgColor);
    level.render(window);
}

