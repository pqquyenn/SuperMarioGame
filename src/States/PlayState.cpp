#include "States/PlayState.h"
#include "Core/AssetManager.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/StarItem.h"
#include "Factories/EntityFactory.h"
#include "Physics/CollisionManager.h"
#include "States/GameOverState.h"
#include "States/PauseState.h"
#include "UI/HUD.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>


PlayState::PlayState(const std::string &mapPath) : initialMapPath(mapPath) {}

void PlayState::onEnter() {
  std::cout << "[PlayState] onEnter - Bat dau map " << initialMapPath
            << std::endl;
  if (!level.loadLevel(initialMapPath)) {
    std::cerr << "[PlayState] Failed to load level " << initialMapPath << "!"
              << std::endl;
  }

  // Khởi tạo con Mario tại vị trí xuất phát (40, 160)
  mario = std::make_unique<Mario>(40.f, 160.f);
  mario->setTexture(
      AssetManager::getInstance().getTexture("PlayerSpriteSheet"));
  mario->setProjectileRequestHandler(
      [this](const ProjectileRequest &request) { spawnFireball(request); });
  mario->update(0.f);

  // Đăng ký HUD làm Observer của Mario (nhận sự kiện coin, enemy, die, powerup)
  mario->addObserver(&hud);

  Camera &cam = level.getCamera();
  cam.setSize(400.f, 225.f);

  const float levelPixelW = level.getTileMap().getMapWidth() * 16.f;
  const float levelPixelH = level.getTileMap().getMapHeight() * 16.f;
  cam.setLevelBounds(levelPixelW, levelPixelH);
  cam.setCenter(200.f, 112.f);

  // Khởi tạo font & text cho chế độ Map Viewer
  if (!freeCamFontLoaded) {
    const std::string fontPaths[] = {
        "assets/fonts/press-start-2p.ttf",
        "../assets/fonts/press-start-2p.ttf",
        "../../assets/fonts/press-start-2p.ttf",
        "../../../assets/fonts/press-start-2p.ttf"
    };
    for (const auto &path : fontPaths) {
      if (std::filesystem::exists(path) && freeCamFont.loadFromFile(path)) {
        freeCamFontLoaded = true;
        break;
      }
    }
  }
  if (freeCamFontLoaded) {
    freeCamText.setFont(freeCamFont);
    freeCamText.setCharacterSize(8);
    freeCamText.setFillColor(sf::Color::Yellow);
    freeCamText.setOutlineColor(sf::Color::Black);
    freeCamText.setOutlineThickness(1.f);
    freeCamText.setString("[MAP VIEWER MODE] WASD/Arrows: Pan | Shift: Fast | V: Exit");
  }
}

void PlayState::onExit() {
  std::cout << "[PlayState] onExit - Tam dung / Roi khoi PlayState"
            << std::endl;
}

void PlayState::handleInput(sf::Event &event, sf::RenderWindow &window) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape) {
      // Nhan Escape -> push PauseState (PlayState van con trong stack)
      if (stateManager) {
        stateManager->pushState(std::make_unique<PauseState>());
      }
    } else if (event.key.code == sf::Keyboard::U ||
               event.key.code == sf::Keyboard::H) {
      std::cout << "[PlayState] Load Underground Map..." << std::endl;
      if (level.loadHiddenMap("underground.txt")) {
        if (mario) {
          mario->setPosition(32.f, 32.f);
        }
        level.getCamera().setCenter(160.f, 120.f);
        level.getTileMap().setNeedsRedraw(true);
      }
    } else if (event.key.code == sf::Keyboard::M ||
               event.key.code == sf::Keyboard::Num1) {
      std::cout << "[PlayState] Return to Main Overworld Map ("
                << initialMapPath << ")..." << std::endl;
      if (level.loadLevel(initialMapPath)) {
        if (mario) {
          mario->setPosition(40.f, 160.f);
        }
        level.getCamera().setCenter(200.f, 112.f);
        level.getTileMap().setNeedsRedraw(true);
      }
    } else if (event.key.code == sf::Keyboard::P) {
      // Debug: Spawn StarItem ngay phía trước Mario để test
      if (mario) {
        sf::Vector2f pos = mario->getPosition();
        if (auto star = EntityFactory::getInstance().create("StarItem", {pos.x + 32.f, pos.y - 16.f})) {
          if (auto *item = dynamic_cast<Item *>(star.get())) {
            star.release();
            level.getItems().push_back(std::unique_ptr<Item>(item));
          }
        }
      }
    } else if (event.key.code == sf::Keyboard::V) {
      isFreeCameraMode = !isFreeCameraMode;
      if (isFreeCameraMode) {
        std::cout << "[PlayState] Free Camera Mode ENABLED (Map Viewer)" << std::endl;
      } else {
        std::cout << "[PlayState] Free Camera Mode DISABLED" << std::endl;
        if (mario) {
          level.getCamera().update(mario->getPosition());
        }
      }
    }
  }
}

void PlayState::update(float dt) {
  if (isFreeCameraMode) {
    float speed = freeCamSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
      speed *= 2.5f;
    }

    float dx = 0.f;
    float dy = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
      dx -= speed * dt;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
      dx += speed * dt;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
      dy -= speed * dt;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      dy += speed * dt;
    }

    if (dx != 0.f || dy != 0.f) {
      level.getCamera().move(dx, dy);
    }

    // Vẫn cập nhật hoạt ảnh entity trong level (sàn di chuyển...)
    level.update(dt);
    return;
  }

  if (mario) {
    if (mario->isDying()) {
      // Cập nhật hoạt ảnh chết của Mario (đứng yên -> bật lên -> rơi xuống)
      mario->update(dt);

      // Khi hoạt ảnh chết kết thúc và Mario bị ẩn
      if (!mario->isActive()) {
        if (hud.getLives() > 0 && hud.getTimeRemaining() > 0.f) {
          mario->respawn(40.f, 160.f);
        } else if (stateManager) {
          stateManager->changeState(
              std::make_unique<GameOverState>(hud.getScore()));
        }
      }
    } else if (mario->isActive()) {
      // 1. Phím bấm điều khiển Mario
      inputHandler.handleInput(*mario, dt);

      // 2. Cập nhật vật lý & animation của Mario
      mario->update(dt);

      // 3. Xử lý va chạm Mario với địa hình gạch / đất
      CollisionManager::resolveTileCollisions(*mario, level.getTileMap(),
                                              &level);

      // 4. Xử lý va chạm Mario với Quái (Enemies)
      for (auto &enemy : level.getEnemies()) {
        if (enemy && enemy->isActive()) {
          CollisionManager::resolveEntityCollisions(*mario, *enemy);
        }
      }

      // 5. Xử lý va chạm Mario với Vật phẩm (Items)
      for (auto &item : level.getItems()) {
        if (item && item->isActive()) {
          CollisionManager::resolveEntityCollisions(*mario, *item);
        }
      }

      // 6. Resolve Mario vs. Moving Platforms (carry riding logic)
      for (auto &platform : level.getMovingPlatforms()) {
        if (platform && platform->isActive()) {
          CollisionManager::resolveMovingPlatform(*mario, *platform);
        }
      }

      // 7. Kiểm tra rơi xuống vực (Void Death)
      // Threshold is 900px to cover the full 1-2 vertical layout
      // (overworld+underground+bonus room)
      if (mario->getPosition().y > 900.f) {
        mario->die(DeathCause::Void);
      }

      // 8. Kiểm tra hết giờ (Time Out)
      if (hud.getTimeRemaining() <= 0.f && !mario->isDying()) {
        mario->die(DeathCause::TimeOut);
      }
    }
  }

  // Cập nhật Fireball: di chuyển, va chạm tile, va chạm enemy
  for (auto &fireball : fireballs) {
    if (!fireball || !fireball->isActive())
      continue;

    fireball->update(dt);
    CollisionManager::resolveTileCollisions(*fireball, level.getTileMap(),
                                            &level);
    if (!fireball->isActive())
      continue;

    const sf::FloatRect fbBounds = fireball->getBounds();
    for (auto &enemy : level.getEnemies()) {
      if (!enemy || !enemy->isActive())
        continue;
      sf::FloatRect overlap;
      if (CollisionManager::checkAABB(fbBounds, enemy->getBounds(), overlap)) {
        enemy->onFireball();
        fireball->explode();
        if (mario) {
          mario->notify(GameEvent{GameEventType::ENEMY_DEFEATED, 100});
        }
        break;
      }
    }

    // Huỷ fireball nếu bay quá xa khỏi tầm camera
    if (fireball->isActive()) {
      const sf::FloatRect camBounds = level.getCamera().getViewBounds();
      if (fireball->getPosition().x < camBounds.left - 64.f ||
          fireball->getPosition().x > camBounds.left + camBounds.width + 64.f ||
          fireball->getPosition().y > 400.f) {
        fireball->explode();
      }
    }
  }

  fireballs.erase(
      std::remove_if(fireballs.begin(), fireballs.end(),
                     [](const auto &f) { return !f || !f->isActive(); }),
      fireballs.end());

  // Cập nhật các entity trong level
  level.update(dt);

  // Camera tự động cuộn theo vị trí Mario
  if (mario) {
    if (level.getIsInBonusRoom()) {
      // Bonus room (rows 31-45, y=480-720): fix camera on vault
      level.getCamera().setCenter(200.f, 600.f);
    } else if (level.getIsUnderground() && mario->getPosition().x < 3600.f) {
      // Underground corridor in 1-2: ceiling y=304, floor y=480, midpoint=400
      float camX = std::max(200.f, mario->getPosition().x);
      level.getCamera().setCenter(camX, 400.f);
    } else if (mario->getPosition().x >= 3600.f) {
      // Appended underground area in 1-1.txt
      level.getCamera().setCenter(3840.f, 120.f);
    } else {
      level.getCamera().update(mario->getPosition());
    }
  }

  // Cập nhật HUD (thời gian, điểm số...)
  hud.update(dt);
}

void PlayState::render(sf::RenderWindow &window) {
  Camera &cam = level.getCamera();
  cam.applyTo(window);

  float camX = cam.getView().getCenter().x;
  float camY = cam.getView().getCenter().y;
  bool isUndergroundArea = level.getIsUnderground() ||
                           level.getIsInBonusRoom() || camY >= 240.f ||
                           camX > 3600.f;
  sf::Color bgColor =
      isUndergroundArea ? sf::Color::Black : sf::Color(92, 148, 252);

  window.clear(bgColor);
  level.render(window);

  if (mario && mario->isActive()) {
    mario->render(window);
  }

  for (const auto &fireball : fireballs) {
    if (fireball && fireball->isActive()) {
      fireball->render(window);
    }
  }

  // Vẽ HUD (score, coins, world, time) cố định trên màn hình
  hud.render(window);

  // Hiển thị Overlay khi đang ở chế độ Map Viewer
  if (isFreeCameraMode && freeCamFontLoaded) {
    sf::Vector2f camCenter = cam.getView().getCenter();
    sf::Vector2f camSize = cam.getView().getSize();
    sf::FloatRect textBounds = freeCamText.getLocalBounds();
    freeCamText.setOrigin(textBounds.left + textBounds.width / 2.f,
                          textBounds.top + textBounds.height);
    freeCamText.setPosition(camCenter.x, camCenter.y + camSize.y / 2.f - 6.f);
    window.draw(freeCamText);
  }
}

void PlayState::spawnFireball(const ProjectileRequest &request) {
  if (request.type != ProjectileType::Fireball) {
    return;
  }

  sf::Texture &sheet = AssetManager::getInstance().getTexture("BlockTileSheet");
  if (sheet.getSize().x == 0) {
    return; // Sheet chưa load, bỏ qua để tránh crash
  }

  // Giới hạn 2 fireball cùng lúc như Mario 1985
  if (fireballs.size() >= 2) {
    return;
  }

  auto fireball =
      std::make_unique<Fireball>(request.position.x, request.position.y - 4.f,
                                 request.facingRight, sheet, 8.f);
  fireballs.push_back(std::move(fireball));
}
