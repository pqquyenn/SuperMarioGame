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
#include "Core/GameSettings.h"
#include "Entities/Luigi.h"
#include "Entities/Mario.h"
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
  const std::string levelName =
      std::filesystem::path(initialMapPath).stem().string();
  hud.setLevelName(levelName.empty() ? "1-1" : levelName);

  const CharacterChoice choice =
      GameSettings::getInstance().getCharacterChoice();
  if (choice == CharacterChoice::Luigi) {
    player = std::make_unique<Luigi>(0.f, 0.f);
    hud.setPlayerName("LUIGI");
  } else {
    player = std::make_unique<Mario>(0.f, 0.f);
    hud.setPlayerName("MARIO");
  }
  player->setTexture(
      AssetManager::getInstance().getTexture("PlayerSpriteSheet"));
  player->setProjectileRequestHandler(
      [this](const ProjectileRequest &request) { spawnFireball(request); });
  player->update(0.f);
  refreshPlayerSpawnPoint();
  player->setPosition(playerSpawnPoint);

  player->addObserver(&hud);

  Camera &cam = level.getCamera();
  cam.setSize(400.f, 225.f);

  const float levelPixelW = level.getTileMap().getMapWidth() * 16.f;
  const float levelPixelH = level.getTileMap().getMapHeight() * 16.f;
  cam.setLevelBounds(levelPixelW, levelPixelH);
  centerCameraOnPlayerSpawn();

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
  std::cout << "[PlayState] Leaving gameplay" << std::endl;
}

void PlayState::handleInput(sf::Event &event, sf::RenderWindow &window) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::T) {
      adminDebugView.toggle();
      std::cout << "[AdminDebugView] "
                << (adminDebugView.isVisible() ? "ENABLED" : "DISABLED")
                << std::endl;
    } else if (event.key.code == sf::Keyboard::Escape) {
      // Nhan Escape -> push PauseState (PlayState van con trong stack)
      if (stateManager) {
        stateManager->pushState(std::make_unique<PauseState>());
      }
    } else if (event.key.code == sf::Keyboard::U ||
               event.key.code == sf::Keyboard::H) {
      std::cout << "[PlayState] Load Underground Map..." << std::endl;
      if (level.loadHiddenMap("underground.txt")) {
        if (player) {
          player->setPosition(32.f, 32.f);
        }
        level.getCamera().setCenter(160.f, 120.f);
        level.getTileMap().setNeedsRedraw(true);
      }
    } else if (event.key.code == sf::Keyboard::M ||
               event.key.code == sf::Keyboard::Num1) {
      std::cout << "[PlayState] Return to Main Overworld Map ("
                << initialMapPath << ")..." << std::endl;
      if (level.loadLevel(initialMapPath)) {
        if (player) {
          refreshPlayerSpawnPoint();
          player->setPosition(playerSpawnPoint);
          player->setVelocity(0.f, 0.f);
        }
        centerCameraOnPlayerSpawn();
        level.getTileMap().setNeedsRedraw(true);
      }
    } else if (event.key.code == sf::Keyboard::P) {
      // Debug: Spawn StarItem immediately in front of the player.
      if (player) {
        sf::Vector2f pos = player->getPosition();
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
        if (player) {
          level.getCamera().update(player->getPosition());
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

  if (player) {
    if (player->isDying()) {
      player->update(dt);

      if (!player->isActive()) {
        if (hud.getLives() > 0 && hud.getTimeRemaining() > 0.f) {
          // Reset underground/bonus-room flags so camera follows player
          // correctly after respawning back on the overworld.
          level.setIsUnderground(false);
          level.setIsInBonusRoom(false);
          player->respawn(playerSpawnPoint.x, playerSpawnPoint.y);
          centerCameraOnPlayerSpawn();
        } else if (stateManager) {
          stateManager->changeState(
              std::make_unique<GameOverState>(hud.getScore()));
        }
      }
    } else if (player->isActive()) {
      inputHandler.handleInput(*player, dt);

      player->update(dt);

      CollisionManager::resolveTileCollisions(*player, level.getTileMap(),
                                              &level);

      for (auto &enemy : level.getEnemies()) {
        if (enemy && enemy->isActive()) {
          CollisionManager::resolveEntityCollisions(*player, *enemy);
        }
      }

      for (auto &item : level.getItems()) {
        if (item && item->isActive()) {
          CollisionManager::resolveEntityCollisions(*player, *item);
        }
      }

      for (auto &platform : level.getMovingPlatforms()) {
        if (platform && platform->isActive()) {
          CollisionManager::resolveMovingPlatform(*player, *platform);
        }
      }

      // 7. Kiểm tra rơi xuống vực (Void Death)
      // Threshold is 900px to cover the full 1-2 vertical layout
      // (overworld+underground+bonus room)
      if (player->getPosition().y > 900.f) {
        player->die(DeathCause::Void);
      }

      // 8. Kiểm tra hết giờ (Time Out)
      if (hud.getTimeRemaining() <= 0.f && !player->isDying()) {
        player->die(DeathCause::TimeOut);
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
        if (player) {
          player->notify(GameEvent{GameEventType::ENEMY_DEFEATED, 100});
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

  // Camera automatically follows the selected character.
  if (player) {
    if (level.getIsInBonusRoom()) {
      // Hidden room (rows 35-43, y=544-720): camera follows Mario
      float camX = std::max(200.f, player->getPosition().x);
      level.getCamera().setCenter(camX, player->getPosition().y);
    } else if (level.getIsUnderground() && player->getPosition().x < 3600.f) {
      // Underground corridor in 1-2: ceiling y=304, floor y=480, midpoint=400
      float camX = std::max(200.f, player->getPosition().x);
      level.getCamera().setCenter(camX, 400.f);
    } else if (player->getPosition().x >= 3600.f) {
      const float camX = std::clamp(player->getPosition().x, 3700.f, 3980.f);
      const float camY =
          std::clamp(player->getPosition().y + 8.f, 80.f, 160.f);
      level.getCamera().setCenter(camX, camY);
    } else {
      // Overworld: follow player X but lock Y to 112 so underground
      // tiles (y >= 256) are never visible through the 225px viewport.
      float camX = std::max(200.f, player->getPosition().x);
      level.getCamera().setCenter(camX, 112.f);
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
                           level.getIsInBonusRoom() || camX > 3600.f;
  sf::Color bgColor =
      isUndergroundArea ? sf::Color::Black : sf::Color(92, 148, 252);

  window.clear(bgColor);
  level.render(window);

  if (player && player->isActive()) {
    player->render(window);
  }

  for (const auto &fireball : fireballs) {
    if (fireball && fireball->isActive()) {
      fireball->render(window);
    }
  }

  // Vẽ HUD (score, coins, world, time) cố định trên màn hình
  hud.render(window);

  if (player) {
    adminDebugView.render(window, *player, level);
  }

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

void PlayState::refreshPlayerSpawnPoint() {
  if (!player) {
    return;
  }

  const sf::FloatRect playerBounds = player->getBounds();
  playerSpawnPoint = level.getStartPosition({
      std::max(1.f, playerBounds.width),
      std::max(1.f, playerBounds.height)});
}

void PlayState::centerCameraOnPlayerSpawn() {
  if (!player) {
    return;
  }

  const sf::FloatRect playerBounds = player->getBounds();
  level.getCamera().setCenter(
      playerSpawnPoint.x + playerBounds.width * 0.5f,
      playerSpawnPoint.y + playerBounds.height * 0.5f);
}
