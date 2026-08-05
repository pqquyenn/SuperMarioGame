#include "States/PlayState.h"
#include "States/PauseState.h"
#include "Core/AssetManager.h"
#include "Physics/CollisionManager.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Items/Item.h"
#include <iostream>
#include <memory>

void PlayState::onEnter() {
    std::cout << "[PlayState] onEnter - Bat dau map 1-1" << std::endl;
    if (!level.loadLevel("1.1/1-1.txt")) {
        std::cerr << "[PlayState] Failed to load level 1.1/1-1.txt!" << std::endl;
    }

    // Khởi tạo con Mario tại vị trí xuất phát (40, 160)
    mario = std::make_unique<Mario>(40.f, 160.f);
    mario->setTexture(AssetManager::getInstance().getTexture("PlayerSpriteSheet"));
    mario->update(0.f);

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
                if (mario) {
                    mario->setPosition(32.f, 32.f);
                }
                level.getCamera().setCenter(160.f, 120.f);
                level.getTileMap().setNeedsRedraw(true);
            }
        }
        else if (event.key.code == sf::Keyboard::M || event.key.code == sf::Keyboard::Num1) {
            std::cout << "[PlayState] Return to Main Overworld Map..." << std::endl;
            if (level.loadLevel("1.1/1-1.txt")) {
                if (mario) {
                    mario->setPosition(40.f, 160.f);
                }
                level.getCamera().setCenter(200.f, 112.f);
                level.getTileMap().setNeedsRedraw(true);
            }
        }
    }
}

void PlayState::update(float dt) {
    if (mario && mario->isActive()) {
        // 1. Phím bấm điều khiển Mario
        inputHandler.handleInput(*mario, dt);

        // 2. Cập nhật vật lý & animation của Mario
        mario->update(dt);

        // 3. Xử lý va chạm Mario với địa hình gạch / đất
        CollisionManager::resolveTileCollisions(*mario, level.getTileMap(), &level);

        // 4. Xử lý va chạm Mario với Quái (Enemies)
        for (auto& enemy : level.getEnemies()) {
            if (enemy && enemy->isActive()) {
                CollisionManager::resolveEntityCollisions(*mario, *enemy);
            }
        }

        // 5. Xử lý va chạm Mario với Vật phẩm (Items)
        for (auto& item : level.getItems()) {
            if (item && item->isActive()) {
                CollisionManager::resolveEntityCollisions(*mario, *item);
            }
        }

        // 6. Kiểm tra rơi xuống vực (Void Death & Respawn)
        if (mario->getPosition().y > 300.f) {
            mario->die(DeathCause::Void);
            mario->respawn(40.f, 160.f);
        }
    }

    // Cập nhật các entity trong level
    level.update(dt);

    // Cập nhật HUD (đếm ngược thời gian)
    hud.update(dt);

    // Camera tự động cuộn theo vị trí Mario
    if (mario) {
        level.getCamera().update(mario->getPosition());
    }
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

    if (mario && mario->isActive()) {
        mario->render(window);
    }

    // Vẽ HUD overlay (luôn cố định trên cùng màn hình)
    hud.render(window);
}

