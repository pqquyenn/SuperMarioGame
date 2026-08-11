#include "Level/Level.h"
#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/RedKoopa.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/OneUpMushroom.h"
#include "Entities/Items/StarItem.h"
#include "Entities/MovingPlatform.h"
#include "Factories/EntityFactory.h"
#include "Physics/CollisionManager.h"
#include <algorithm>
#include <filesystem>
#include <vector>

Level::Level(int id) : levelId(id) {}
Level::~Level() = default;

struct SpawnData {
  std::string type;
  sf::Vector2f position;
};

void Level::spawnEntitiesFromMap() {
  auto &factory = EntityFactory::getInstance();

  // Data-driven spawn positions for each level
  std::vector<SpawnData> enemySpawns;
  std::vector<SpawnData> itemSpawns;

  if (levelId == 1) {
    enemySpawns = {{"Goomba", {384.f, 192.f}},  {"Goomba", {656.f, 192.f}},
                   {"Goomba", {832.f, 192.f}},  {"Goomba", {848.f, 192.f}},
                   {"Goomba", {1312.f, 64.f}},  {"Goomba", {1344.f, 64.f}},
                   {"Goomba", {1584.f, 192.f}}, {"Goomba", {1600.f, 192.f}},
                   {"Goomba", {1792.f, 192.f}}, {"Goomba", {1808.f, 192.f}},
                   {"Goomba", {2000.f, 192.f}}, {"Goomba", {2016.f, 192.f}},
                   {"Goomba", {2048.f, 192.f}}, {"Goomba", {2064.f, 192.f}},
                   {"Goomba", {2768.f, 192.f}}, {"Goomba", {2784.f, 192.f}},
                   {"Koopa", {1712.f, 176.f}}};
  } else if (levelId == 2) {
    // No hardcoded enemies for 1-2 overworld; underground section has
    // tile-based content
  } else if (levelId == 3) {
  }

  for (const auto &data : enemySpawns) {
    if (auto entity = factory.create(data.type, data.position)) {
      if (auto *enemy = dynamic_cast<Enemy *>(entity.get())) {
        if (auto *rk = dynamic_cast<RedKoopa *>(enemy)) {
          rk->setTileMap(&map);
        }
        entity.release();
        enemies.push_back(std::unique_ptr<Enemy>(enemy));
        std::cout << "[Level] Spawned Enemy: " << data.type << " at ("
                  << data.position.x << ", " << data.position.y << ")"
                  << std::endl;
      }
    }
  }

  for (const auto &data : itemSpawns) {
    if (auto entity = factory.create(data.type, data.position)) {
      if (auto *item = dynamic_cast<Item *>(entity.get())) {
        entity.release();
        items.push_back(std::unique_ptr<Item>(item));
        std::cout << "[Level] Spawned Item: " << data.type << " at ("
                  << data.position.x << ", " << data.position.y << ")"
                  << std::endl;
      }
    }
  }

  // Data-driven MovingPlatform configuration for Level 2 (based on 000
  // positions in 1-2.txt)
  if (levelId == 2) {
    struct PlatformConfig {
      float x, y, width;
      float bound1, bound2;
      float speed;
      MovingPlatform::Mode mode;
    };

    // Shaft 1 (Col 151, x=2416px): 2 platforms moving DOWN (LoopDown)
    // Shaft 2 (Col 166, x=2656px): 2 platforms moving UP (LoopUp)
    const PlatformConfig level2Platforms[] = {
        {2416.f, 304.f, 48.f, 304.f, 496.f, 50.f,
         MovingPlatform::Mode::LoopDown},
        {2416.f, 400.f, 48.f, 304.f, 496.f, 50.f,
         MovingPlatform::Mode::LoopDown},
        {2656.f, 304.f, 48.f, 320.f, 496.f, 50.f, MovingPlatform::Mode::LoopUp},
        {2656.f, 416.f, 48.f, 320.f, 496.f, 50.f, MovingPlatform::Mode::LoopUp},
    };

    for (const auto &cfg : level2Platforms) {
      movingPlatforms.push_back(
          std::make_unique<MovingPlatform>(cfg.x, cfg.y, cfg.width, cfg.bound1,
                                           cfg.bound2, cfg.speed, cfg.mode));
      std::cout << "[Level] Spawned MovingPlatform at (" << cfg.x << ","
                << cfg.y << ") width=" << cfg.width
                << " Mode=" << static_cast<int>(cfg.mode) << std::endl;
    }
  }
  if (levelId == 3) {
    struct PlatformConfig {
      float x, y, width;
      float bound1, bound2;
      float speed;
      MovingPlatform::Mode mode;
    };
    const PlatformConfig level3Platforms[] = {
        // Platform 1 (Picture 1 gap: cols 55-67, x=976)
        {1008.f, 96.f, 48.f, 96.f, 240.f, 50.f,
         MovingPlatform::Mode::OscillateVertical},
        // Platform 2 (Picture 2 gap left: cols 99-109, x=1584)
        {1584.f, 112.f, 48.f, 1584.f, 1680.f, 40.f,
         MovingPlatform::Mode::OscillateHorizontal},
        // Platform 3 (Picture 2 gap right: cols 110-119, x=1760)
        {1824.f, 144.f, 48.f, 1728.f, 1824.f, 40.f,
         MovingPlatform::Mode::OscillateHorizontal},
        // Platform 4 (Picture 3 gap: cols 135-149, x=2160)
        {2432.f, 144.f, 48.f, 2432.f, 2640.f, 50.f,
         MovingPlatform::Mode::OscillateHorizontal},
    };
    for (const auto &cfg : level3Platforms) {
      movingPlatforms.push_back(
          std::make_unique<MovingPlatform>(cfg.x, cfg.y, cfg.width, cfg.bound1,
                                           cfg.bound2, cfg.speed, cfg.mode));
      std::cout << "[Level] Spawned MovingPlatform at (" << cfg.x << ","
                << cfg.y << ") Axis=" << static_cast<int>(cfg.mode)
                << std::endl;
    }
  }
}

bool Level::loadInternal(const std::string &filename, bool isUndergroundFlag) {
  isUnderground = isUndergroundFlag;
  isInBonusRoom = false;
  enemies.clear();
  items.clear();
  movingPlatforms.clear();

  EntityFactory::getInstance().registerDefaultEntities();

  std::filesystem::path p(filename);
  std::string rawFilename = p.filename().string();

  const std::string candidates[] = {filename,
                                    "assets/maps/" + filename,
                                    "../assets/maps/" + filename,
                                    "../../assets/maps/" + filename,
                                    "assets/maps/1.1/" + rawFilename,
                                    "assets/maps/1.2/" + rawFilename,
                                    "assets/maps/1.3/" + rawFilename,
                                    "../assets/maps/1.1/" + rawFilename,
                                    "../assets/maps/1.2/" + rawFilename,
                                    "../assets/maps/1.3/" + rawFilename,
                                    "../../assets/maps/1.1/" + rawFilename,
                                    "../../assets/maps/1.2/" + rawFilename,
                                    "../../assets/maps/1.3/" + rawFilename,
                                    "../" + filename,
                                    "../../" + filename};

  for (const auto &path : candidates) {
    if (std::filesystem::exists(path)) {
      if (map.readFromFile(path)) {
        // Tự động phát hiện và cập nhật levelId dựa trên tên đường dẫn file
        // map
        if (path.find("1-1") != std::string::npos ||
            path.find("1.1") != std::string::npos) {
          levelId = 1;
        } else if (path.find("1-2") != std::string::npos ||
                   path.find("1.2") != std::string::npos) {
          levelId = 2;
        } else if (path.find("1-3") != std::string::npos ||
                   path.find("1.3") != std::string::npos) {
          levelId = 3;
        }

        std::filesystem::path bgPath =
            std::filesystem::path(path).parent_path() / "background.txt";
        if (std::filesystem::exists(bgPath)) {
          bgMap.readFromFile(bgPath.string());
        }
        spawnEntitiesFromMap();
        return true;
      }
    }
  }
  std::cerr << "[Level] Failed to find level map file: " << filename
            << std::endl;
  return false;
}

bool Level::loadLevel(const std::string &levelFile) {
  return loadInternal(levelFile, false);
}

bool Level::loadHiddenMap(const std::string &hiddenFile) {
  return loadInternal(hiddenFile, true);
}

bool Level::loadMap(const std::string &mapFile) {
  if (mapFile.find("underground") != std::string::npos ||
      mapFile.find("hidden") != std::string::npos) {
    return loadHiddenMap(mapFile);
  }
  return loadLevel(mapFile);
}

void Level::update(float dt) {
  // Update tile animations (question block shimmer, bump effects)
  map.update(dt);
  map.updateDebris(dt);

  sf::FloatRect camBounds = camera.getViewBounds();
  const float spawnMargin = 80.f;

  for (auto &enemy : enemies) {
    if (!enemy || !enemy->isActive())
      continue;

    if (!enemy->isActivated()) {
      if (enemy->getPosition().x <=
          camBounds.left + camBounds.width + spawnMargin) {
        enemy->setActivated(true);
      }
    }

    if (enemy->isActivated()) {
      enemy->update(dt);
      CollisionManager::resolveTileCollisions(*enemy, map);
    }
  }

  for (auto &item : items) {
    if (item && item->isActive()) {
      item->update(dt);

      bool isEthereal = false;
      if (auto *coin = dynamic_cast<Coin *>(item.get())) {
        if (coin->isPopping())
          isEthereal = true;
      } else if (auto *shroom = dynamic_cast<Mushroom *>(item.get())) {
        if (shroom->isEmerging())
          isEthereal = true;
      } else if (auto *flower = dynamic_cast<FireFlower *>(item.get())) {
        if (flower->isEmerging())
          isEthereal = true;
      } else if (auto *star = dynamic_cast<StarItem *>(item.get())) {
        if (star->isEmerging())
          isEthereal = true;
      }

      if (!isEthereal) {
        CollisionManager::resolveTileCollisions(*item, map);
      }
    }
  }

  for (auto &platform : movingPlatforms) {
    if (platform)
      platform->update(dt);
  }

  enemies.erase(
      std::remove_if(enemies.begin(), enemies.end(),
                     [](const auto &e) { return !e || !e->isActive(); }),
      enemies.end());

  items.erase(
      std::remove_if(items.begin(), items.end(),
                     [](const auto &i) { return !i || !i->isActive(); }),
      items.end());
}

void Level::render(sf::RenderWindow &window) {
  bgMap.render(window, camera);

  for (const auto &item : items) {
    if (item && item->isActive()) {
      item->render(window);
    }
  }

  // Render moving platforms
  for (const auto &platform : movingPlatforms) {
    if (platform && platform->isActive()) {
      platform->render(window);
    }
  }

  for (const auto &enemy : enemies) {
    if (enemy && enemy->isActive()) {
      enemy->render(window);
    }
  }

  map.render(window, camera);

  // Render brick debris on top of everything
  map.renderDebris(window);
}

void Level::spawnItemFromBlock(float x, float y) {
  spawnItemFromBlock(x, y, nullptr);
}

void Level::spawnItemFromBlock(float x, float y, Character *character) {
  std::string itemType = "Coin";

  bool starSpot = (levelId == 1 && std::abs(x - 1616.f) < 2.f);
  bool oneUpSpot = (levelId == 1 && std::abs(x - 1024.f) < 2.f);
  bool powerupSpot = (std::abs(x - 336.f) < 1.f || std::abs(x - 1248.f) < 1.f ||
                      std::abs(x - 1744.f) < 1.f);
  if (levelId == 2 && isUnderground && std::abs(x - 256.f) < 1.f) {
    powerupSpot = true;
  }

  if (starSpot) {
    itemType = "StarItem";
  } else if (oneUpSpot) {
    itemType = "1UpMushroom";
  } else if (powerupSpot) {
    if (character) {
      const std::string_view form = character->getCurrentFormName();
      if (form == "Super" || form == "Fire") {
        itemType = "FireFlower";
      } else {
        itemType = "Mushroom";
      }
    } else {
      itemType = "Mushroom";
    }
  }

  sf::Vector2f spawnPos =
      (itemType == "Coin") ? sf::Vector2f{x, y - 16.f} : sf::Vector2f{x, y};
  if (auto entity = EntityFactory::getInstance().create(itemType, spawnPos)) {
    if (itemType == "Coin") {
      if (auto *coin = dynamic_cast<Coin *>(entity.get())) {
        entity.release();
        coin->startPop();
        items.push_back(std::unique_ptr<Item>(coin));
      }
    } else {
      if (auto *item = dynamic_cast<Item *>(entity.get())) {
        if (auto *shroom = dynamic_cast<Mushroom *>(item)) {
          shroom->startEmerge();
        } else if (auto *flower = dynamic_cast<FireFlower *>(item)) {
          flower->startEmerge();
        } else if (auto *star = dynamic_cast<StarItem *>(item)) {
          star->startEmerge();
        }
        entity.release();
        items.push_back(std::unique_ptr<Item>(item));
      }
    }
  }
}

void Level::warpToUnderground(Character *character) {
  std::cout << "[Level] Teleporting to hidden underground map area..."
            << std::endl;
  if (character) {
    character->setPosition(3736.f, 32.f);
    character->setVelocity(sf::Vector2f(0.f, 0.f));
  }
  camera.setCenter(3736.f, 120.f);
}

void Level::warpToUnderground1_2(Character *character) {
  std::cout << "[Level] Teleporting to hidden underground coin room in 1-2..."
            << std::endl;
  if (character) {
    character->setPosition(48.f, 528.f);
    character->setVelocity(sf::Vector2f(0.f, 0.f));
  }
  camera.setCenter(200.f, 608.f);
}

// ── World 1-2 Warp Methods ─────────────────────────────────────────────────

// Pipe A: Automatic horizontal contact → underground main corridor
void Level::warpPipeA_Entry(Character *character) {
  std::cout << "[Level][1-2] Pipe A entered — warping to underground."
            << std::endl;
  isUnderground = true;
  isInBonusRoom = false;
  if (character) {
    // Underground floor tile tops are at y=464 (lines 30-31, row 29-30).
    // For a 16px-tall Small Mario, spawn with feet at y=464 → top at y=448.
    // Update per step 3: spawn at y=432.f so he safely lands on the floor.
    character->setPosition(256.f, 432.f);
    character->setVelocity(sf::Vector2f(30.f, 0.f)); // carry rightward momentum
  }
  // Underground corridor: ceiling y=304, floor y=480, midpoint=400
  camera.setCenter(300.f, 400.f);
}

// Pipe B: Down key on underground pipe → bonus room (hidden vault)
void Level::warpPipeB_Entry(Character *character) {
  std::cout << "[Level][1-2] Pipe B entered — warping to bonus room."
            << std::endl;
  isInBonusRoom = true;
  if (character) {
    // Bonus room entry: left side, just below the ceiling pipe (rows 32-34,
    // x=48, y=528)
    character->setPosition(48.f, 528.f);
    character->setVelocity(sf::Vector2f(0.f, 0.f));
  }
  // Camera: center on bonus room (rows 31-45 span y=480-720, center ≈ 600)
  camera.setCenter(200.f, 600.f);
}

// Pipe C1: Automatic horizontal exit from bonus room → resurface at Pipe C2
// in underground
void Level::warpPipeC1_Exit(Character *character) {
  std::cout
      << "[Level][1-2] Pipe C1 exit — returning to underground at Pipe C2."
      << std::endl;
  isInBonusRoom = false;
  if (character) {
    // Pipe C2 destination: back in the underground corridor, slightly to the
    // right of Pipe B so Mario exits moving right. y=400 is above the
    // underground floor.
    character->setPosition(672.f, 400.f);
    character->setVelocity(sf::Vector2f(0.f, -80.f)); // pop upward out of pipe
  }
  camera.setCenter(672.f, 352.f);
}

void Level::warpToOverworldExit(Character *character) {
  std::cout << "[Level] Teleporting back to Overworld (5th pipe)..."
            << std::endl;
  if (character) {
    character->setPosition(2608.f, 160.f);
    character->setVelocity(sf::Vector2f(0.f, -100.f)); // pop out of pipe
  }

  camera.setCenter(2608.f, 120.f);
}
