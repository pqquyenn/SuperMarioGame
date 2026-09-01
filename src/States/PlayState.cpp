#include "States/PlayState.h"
#include "Core/AchievementSystem.h"
#include "Core/AssetManager.h"
#include "Core/SoundManager.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/DragonLugia.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/StarItem.h"
#include "Factories/EntityFactory.h"
#include "Physics/CollisionManager.h"
#include "States/GameOverState.h"
#include "States/LevelCompleteState.h"
#include "States/PauseState.h"
#include "States/WinState.h"
#include "UI/HUD.h"
#include "Core/GameSettings.h"
#include "Entities/Luigi.h"
#include "Entities/Mario.h"
#include "PlayerEffects/StarEffect.h"
#include "PlayerStates/FireState.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>


PlayState::PlayState(const std::string &mapPath) : initialMapPath(mapPath) {}

void PlayState::onEnter() {
  std::cout << "[PlayState] onEnter - Bat dau map " << initialMapPath
            << std::endl;
  // Disconnect before replacing the player so the old Subject never retains
  // the HUD observer or an RAII connection to the old player.
  hudObserverConnection.disconnect();
  levelWon = false;
  hud.setTimeFrozen(false);
  if (!level.loadLevel(initialMapPath)) {
    std::cerr << "[PlayState] Failed to load level " << initialMapPath << "!"
              << std::endl;
  }
  hud.setTimeRemaining(static_cast<float>(level.getTimeLimit()));
  const std::string levelName = level.getDefinition().name.empty()
      ? std::filesystem::path(initialMapPath).stem().string()
      : level.getDefinition().name;
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

  hudObserverConnection = player->addObserver(&hud);
  achievementObserverConnection =
      player->addObserver(&AchievementSystem::getInstance());
  AchievementSystem::getInstance().beginLevel(player->getCurrentFormName());

  Camera &cam = level.getCamera();
  cam.setSize(400.f, 225.f);

  const float tileSize = level.isDataDriven()
      ? level.getDefinition().tileSize
      : 16.f;
  const float levelPixelW = level.getTileMap().getMapWidth() * tileSize;
  const float levelPixelH = level.getTileMap().getMapHeight() * tileSize;
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

  // Start stage background music
  bool hasBoss = false;
  for (const auto& enemy : level.getEnemies()) {
    if (dynamic_cast<DragonLugia*>(enemy.get())) {
      hasBoss = true;
      break;
    }
  }

  std::string lowerMap = initialMapPath;
  for (char &c : lowerMap) c = static_cast<char>(::tolower(c));
  std::string lowerName = levelName;
  for (char &c : lowerName) c = static_cast<char>(::tolower(c));

  if (hasBoss || lowerName.find("castle") != std::string::npos || lowerMap.find("castle") != std::string::npos) {
    SoundManager::getInstance().playBGM("assets/audio/music/castle.wav");
  } else if (level.usesDarkBackground() || lowerName.find("underground") != std::string::npos || lowerMap.find("underground") != std::string::npos || lowerName.find("1-2") != std::string::npos) {
    SoundManager::getInstance().playBGM("assets/audio/music/underground.wav");
  } else if (lowerName.find("underwater") != std::string::npos || lowerMap.find("underwater") != std::string::npos) {
    SoundManager::getInstance().playBGM("assets/audio/music/underwater.wav");
  } else {
    SoundManager::getInstance().playBGM("assets/audio/music/overworld.wav");
  }
}

void PlayState::onExit() {
  hudObserverConnection.disconnect();
  SoundManager::getInstance().stopBGM();
  std::cout << "[PlayState] Leaving gameplay" << std::endl;
}

void PlayState::handleInput(sf::Event &event, sf::RenderWindow &window) {
  if (event.type == sf::Event::KeyPressed) {
    if (inputHandler.matches(InputAction::Crouch, event.key.code) &&
        player &&
            level.tryActivatePortalForInput(*player, PortalActivation::Down)) {
      return;
    } else if (inputHandler.matches(
                   InputAction::MoveRight,
                   event.key.code) &&
               player &&
               level.tryActivatePortalForInput(
                   *player, PortalActivation::Right)) {
      return;
    } else if (event.key.code == sf::Keyboard::T) {
      adminDebugView.toggle();
      std::cout << "[AdminDebugView] "
                << (adminDebugView.isVisible() ? "ENABLED" : "DISABLED")
                << std::endl;
    } else if (adminDebugView.isVisible() && player &&
               event.key.code == sf::Keyboard::Y) {
      adminDebugView.startMovementTrail(*player);
      std::cout << "[AdminDebugView] Recording movement trail for 8 seconds"
                << std::endl;
    } else if (adminDebugView.isVisible() && player &&
               event.key.code == sf::Keyboard::I) {
      // Apply the same timed invincibility, contact defeat, movement boost,
      // and visual treatment as collecting a Star. Void and timeout deaths
      // still call Character::die directly and remain lethal.
      player->addEffect(std::make_unique<StarEffect>());
      std::cout << "[AdminDebugView] Star invincibility granted"
                << std::endl;
    } else if (adminDebugView.isVisible() && player &&
               event.key.code == sf::Keyboard::K) {
      const std::string_view form = player->getCurrentFormName();
      if (form == "Small" || form == "Super") {
        // Character::receivePowerUp intentionally converts Small + powered
        // state to Super first; the next press advances Super to Fire.
        player->receivePowerUp(std::make_unique<FireState>());
        std::cout << "[AdminDebugView] Player form increased to "
                  << player->getCurrentFormName() << std::endl;
      }
    } else if (adminDebugView.isVisible() && player &&
               event.key.code == sf::Keyboard::L) {
      // Force exactly one normal damage transition even if I was used. The
      // regular damage path still grants the configured post-hit immunity.
      player->takeDamageIgnoringProtection();
      std::cout << "[AdminDebugView] Player form decreased to "
                << (player->isDying()
                        ? std::string_view{"Dying"}
                        : player->getCurrentFormName())
                << std::endl;
    } else if (event.key.code == sf::Keyboard::Escape) {
      // Nhan Escape -> push PauseState (PlayState van con trong stack)
      if (stateManager) {
        stateManager->pushState(
            std::make_unique<PauseState>(initialMapPath));
      }
    } else if (event.key.code == sf::Keyboard::U ||
               event.key.code == sf::Keyboard::H) {
      if (player && level.tryActivateFirstPortalFromCurrentArea(*player)) {
        SoundManager::getInstance().playSound("pipe");
        std::cout << "[PlayState] Activated the first manifest portal from "
                  << "the current area." << std::endl;
      } else {
        std::cout << "[PlayState] Current area has no manifest portal."
                  << std::endl;
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
  if (levelWon) {
    return;
  }

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
      // Solo owns its music lifecycle. Character::die() is shared with PvP
      // and Duo, where one player's death must not stop music for everyone.
      if (!SoundManager::getInstance().getCurrentBGM().empty()) {
        SoundManager::getInstance().stopBGM();
      }
      player->update(dt);

      if (!player->isActive()) {
        if (hud.getLives() > 0 && hud.getTimeRemaining() > 0.f) {
          level.resetToInitialArea();
          player->respawn(playerSpawnPoint.x, playerSpawnPoint.y);
          centerCameraOnPlayerSpawn();
        } else if (stateManager) {
          stateManager->changeState(
              std::make_unique<GameOverState>(hud.getScore(), initialMapPath));
        }
      }
    } else if (player->isActive()) {
      inputHandler.handleInput(*player, dt);

      player->update(dt);

      CollisionManager::resolveTileCollisions(*player, level.getTileMap(),
                                              &level);

      for (auto &enemy : level.getEnemies()) {
        if (enemy && enemy->isActive()) {
          if (auto* dragon = dynamic_cast<DragonLugia*>(enemy.get())) {
            dragon->updateWithPlayer(dt, player.get(), &level.getTileMap());
          }
          CollisionManager::resolveEntityCollisions(*player, *enemy);
        }
      }

      AchievementSystem::getInstance().observeForm(
          player->getCurrentFormName());

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

      // Finalize stance only after static tiles and moving platforms have both
      // contributed their grounded contact for this frame.
      CollisionManager::tryStandUp(*player, level.getTileMap());
      adminDebugView.updateMovementTrail(*player, dt);

      // Keep portal activation aligned with the active Solo profile while a
      // direction is held.
      if (player && player->isActive() && !player->isDying()) {
        if (inputHandler.isHeld(InputAction::Crouch) ||
            player->isCrouching()) {
          level.tryActivatePortalForInput(*player, PortalActivation::Down);
        }
        if (inputHandler.isHeld(InputAction::MoveRight) ||
            player->getVelocity().x > 0.f) {
          level.tryActivatePortalForInput(*player, PortalActivation::Right);
        }
      }

      // 6.5. Kiểm tra chạm Cột Cờ (Win State)
      DragonLugia* stageBoss = nullptr;
      if (!levelWon && player->isActive() && !player->isDying()) {
        const sf::FloatRect pBounds = player->getBounds();
        for (const TileHandle &h : level.getTileMap().getTilesInBounds(pBounds)) {
          const Tile *tile = level.getTileMap().getTile(h);
          if (tile && tile->isFlagpole()) {
            levelWon = true;
            AchievementSystem::getInstance().observeForm(
                player->getCurrentFormName());
            AchievementSystem::getInstance().completeLevel(
                level.getLevelId(), hud.getScore());
            if (stateManager) {
              stateManager->pushState(std::make_unique<WinState>(
                  this,
                  initialMapPath,
                  level.getNextStage()));
            }
            return;
          }
        }

        for (const auto& enemy : level.getEnemies()) {
          if (auto* dragon = dynamic_cast<DragonLugia*>(enemy.get())) {
            stageBoss = dragon;
            if (dragon->isBossDefeated()) {
              levelWon = true;
              AchievementSystem::getInstance().observeForm(
                  player->getCurrentFormName());
              AchievementSystem::getInstance().completeLevel(
                  level.getLevelId(), hud.getScore());
              if (stateManager) {
                stateManager->changeState(std::make_unique<LevelCompleteState>(
                    level.getDefinition().name,
                    initialMapPath,
                    level.getNextStage(),
                    hud.getScore(),
                    hud.getCoins(),
                    static_cast<int>(hud.getTimeRemaining() * 50.f)
                ));
              }
              return;
            }
          }
        }
      }

      // Boss support drops are enabled by placing DragonLugia in the manifest,
      // rather than by branching on a numeric stage ID.
      if (!levelWon && stageBoss && player && player->isActive() &&
          !player->isDying()) {
        skyDropTimer -= dt;
        if (skyDropTimer <= 0.f) {
          skyDropTimer = 6.5f + static_cast<float>(rand() % 40) / 10.f; // 6.5s - 10.5s

          int dropType = rand() % 100;
          if (dropType < 55) {
            // 55% chance: Powerup Items (Mushroom, FireFlower, StarItem)
            // Spawn directly on ground or blue castle blocks so they don't hover in empty sky
            struct SurfaceSpot {
              float x;
              float y;
            };
            static const SurfaceSpot surfaces[] = {
                {80.f, 192.f},   // Ground Left
                {200.f, 192.f},  // Ground Center
                {310.f, 192.f},  // Ground Right
                {88.f, 128.f},   // Left Blue Platform (row 9)
                {184.f, 80.f},   // Middle Blue Platform (row 6)
                {300.f, 112.f}   // Right Blue Platform (row 8)
            };
            int spotIdx = rand() % 6;
            sf::Vector2f spawnPos(surfaces[spotIdx].x + static_cast<float>(rand() % 16 - 8), surfaces[spotIdx].y);

            static const char* itemTypes[] = {"Mushroom", "FireFlower", "StarItem"};
            int itemIdx = rand() % 3;
            std::string selectedItem = itemTypes[itemIdx];
            if (selectedItem == "Mushroom" && player->getCurrentFormName() != "Small") {
              selectedItem = "FireFlower";
            }
            if (auto itemEnt = EntityFactory::getInstance().create(selectedItem, spawnPos)) {
              if (auto* item = dynamic_cast<Item*>(itemEnt.get())) {
                itemEnt.release();
                level.getItems().push_back(std::unique_ptr<Item>(item));
                SoundManager::getInstance().playSound("powerupappear");
              }
            }
          } else {
            // 45% chance: ONLY Goomba spawns and drops from the ceiling
            float spawnX = 60.f + static_cast<float>(rand() % 280);
            float spawnY = 20.f;
            if (auto enemyEnt = EntityFactory::getInstance().create("Goomba", {spawnX, spawnY})) {
              if (auto* enemy = dynamic_cast<Enemy*>(enemyEnt.get())) {
                enemy->setActivated(true);
                enemyEnt.release();
                level.getEnemies().push_back(std::unique_ptr<Enemy>(enemy));
              }
            }
          }
        }
      }

      // The camera is presentation state and can move independently during
      // fullscreen changes, warps, or Map Viewer mode. Constrain the player
      // against stable map-space bounds so camera changes cannot teleport it.
      constrainPlayerHorizontally();

      // 7. Kiểm tra rơi xuống vực (Void Death)
      // Threshold is 900px to cover the full 1-2 vertical layout
      // (overworld+underground+bonus room)
      if (player->getPosition().y > level.getKillPlaneY()) {
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
    CollisionManager::resolveTileCollisions(*fireball, level.getTileMap());
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
          player->notify(GameEvent::enemyDefeated(enemy->getScoreValue()));
        }
        break;
      }
    }

    // Destroy projectiles only after their complete bounds leave the active
    // camera area. A fixed world-space Y limit breaks underground and bonus
    // rooms, whose valid floors are below the overworld's coordinates.
    if (fireball->isActive()) {
      const sf::FloatRect camBounds = level.getCamera().getViewBounds();
      constexpr float CleanupMargin = 64.f;
      const sf::FloatRect activeBounds{
          camBounds.left - CleanupMargin,
          camBounds.top - CleanupMargin,
          camBounds.width + CleanupMargin * 2.f,
          camBounds.height + CleanupMargin * 2.f};

      if (!fireball->getBounds().intersects(activeBounds)) {
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
    level.updateCameraFor(player->getPosition());
  }

  // Cập nhật HUD (thời gian, điểm số...)
  hud.update(dt);
  AchievementSystem::getInstance().recordScore(hud.getScore());
}

void PlayState::render(sf::RenderWindow &window) {
  Camera &cam = level.getCamera();
  cam.applyTo(window);

  bool isUndergroundArea = level.usesDarkBackground();
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

  level.updateCameraFor(player->getPosition());
}

void PlayState::constrainPlayerHorizontally() {
  if (!player || !player->isActive() || player->isDying()) {
    return;
  }

  const sf::FloatRect playerBounds = player->getBounds();
  const float minimumX = level.getLeftBoundaryX();
  const float maximumX = level.getRightBoundaryX(playerBounds.width);
  const float currentX = player->getPosition().x;
  const float constrainedX = std::clamp(currentX, minimumX, maximumX);

  if (constrainedX != currentX) {
    player->setPosition(constrainedX, player->getPosition().y);
    player->setVelocity(0.f, player->getVelocity().y);
  }
}
