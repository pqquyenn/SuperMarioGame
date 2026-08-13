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
#include <limits>
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
    enemySpawns = {
        // Underground Goombas
        {"UndergroundGoomba", {304.f, 432.f}}, // Row 28, Col 19
        {"UndergroundGoomba", {320.f, 432.f}}, // Row 28, Col 20
        {"UndergroundGoomba", {656.f, 432.f}},
        {"UndergroundGoomba", {1280.f, 432.f}},
        {"UndergroundGoomba", {1296.f, 432.f}},
        {"UndergroundGoomba", {1328.f, 320.f}},
        {"UndergroundGoomba", {1408.f, 384.f}},
        {"UndergroundGoomba", {1424.f, 384.f}},
        {"UndergroundGoomba", {1728.f, 432.f}},
        {"UndergroundGoomba", {1744.f, 432.f}},
        {"UndergroundGoomba", {1760.f, 432.f}},
        {"UndergroundGoomba", {2352.f, 368.f}},
        {"UndergroundGoomba", {2368.f, 368.f}},

        // Koopas
        {"Koopa", {960.f, 416.f}},
        {"Koopa", {976.f, 416.f}},

        // Red Koopa
        {"RedKoopa", {2624.f, 416.f}},

        // Piranha Plants
        {"PiranhaPlant", {1864.f, 400.f}},
        {"PiranhaPlant", {1944.f, 384.f}},
    };
  } else if (levelId == 3) {
    enemySpawns = {
        {"RedKoopa", {496.f, 64.f}},        {"Goomba", {800.f, 64.f}},
        {"Goomba", {816.f, 64.f}},          {"RedParatroopa", {1456.f, 112.f}},
        {"Goomba", {1552.f, 96.f}},         {"RedKoopa", {2160.f, 96.f}},
        {"RedParatroopa", {2208.f, 128.f}}, {"RedKoopa", {2676.f, 192.f}},
    };
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
        {2432.f, 304.f, 48.f, 304.f, 496.f, 50.f,
         MovingPlatform::Mode::LoopDown},
        {2432.f, 400.f, 48.f, 304.f, 496.f, 50.f,
         MovingPlatform::Mode::LoopDown},
        {2720.f, 304.f, 48.f, 320.f, 496.f, 50.f, MovingPlatform::Mode::LoopUp},
        {2720.f, 416.f, 48.f, 320.f, 496.f, 50.f, MovingPlatform::Mode::LoopUp},
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
        {1024.f, 96.f, 48.f, 96.f, 240.f, 50.f,
         MovingPlatform::Mode::OscillateVertical},
        {1600.f, 112.f, 48.f, 1600.f, 1696.f, 40.f,
         MovingPlatform::Mode::OscillateHorizontal},
        {1840.f, 144.f, 48.f, 1744.f, 1840.f, 40.f,
         MovingPlatform::Mode::OscillateHorizontal},
        {2432.f, 144.f, 48.f, 2432.f, 2656.f, 50.f,
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
  removedEnemyCount = 0;

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

        // Hidden/debug maps must not replace the selected level's respawn
        // point. Returning from them or dying in them still uses the main
        // level's start marker.
        if (!isUndergroundFlag) {
          if (const auto &startMarker = map.getStartMarker()) {
            levelStartHint = *startMarker;
          } else {
            levelStartHint = {0.f, 0.f};
            std::cerr << "[Level] Map has no '@' player start marker: " << path
                      << std::endl;
          }
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

sf::Vector2f Level::findGroundedSpawn(const sf::Vector2f &requestedPosition,
                                      const sf::Vector2f &characterSize) const {
  constexpr float TileSize = 16.f;
  const float safeWidth = std::max(1.f, characterSize.x);
  const float safeHeight = std::max(1.f, characterSize.y);
  const float worldWidth = map.getMapWidth() * TileSize;
  const float worldHeight = map.getMapHeight() * TileSize;
  const float spawnX = std::clamp(requestedPosition.x, 0.f,
                                  std::max(0.f, worldWidth - safeWidth));
  const float searchTop = std::clamp(requestedPosition.y, 0.f, worldHeight);
  const sf::FloatRect searchBounds{spawnX, searchTop, safeWidth,
                                   std::max(0.f, worldHeight - searchTop)};

  float nearestSurface = std::numeric_limits<float>::max();
  for (const TileHandle &handle : map.getTilesInBounds(searchBounds)) {
    const Tile *tile = map.getTile(handle);
    if (!tile || !tile->isSolid()) {
      continue;
    }

    const sf::FloatRect tileBounds = tile->getBounds();
    const bool overlapsHorizontally =
        spawnX + safeWidth > tileBounds.left &&
        spawnX < tileBounds.left + tileBounds.width;
    const bool isBelowMarker =
        tileBounds.top >= requestedPosition.y + safeHeight;
    if (overlapsHorizontally && isBelowMarker &&
        tileBounds.top < nearestSurface) {
      nearestSurface = tileBounds.top;
    }
  }

  if (nearestSurface != std::numeric_limits<float>::max()) {
    return {spawnX, nearestSurface - safeHeight};
  }

  // A malformed or intentionally airborne map remains loadable. The marker
  // position is safer than inventing another level-specific hardcoded point.
  return {spawnX, requestedPosition.y};
}

sf::Vector2f Level::getStartPosition(const sf::Vector2f &characterSize) const {
  return findGroundedSpawn(levelStartHint, characterSize);
}

EnemyRuntimeStats Level::getEnemyRuntimeStats() const {
  EnemyRuntimeStats stats;
  stats.removed = removedEnemyCount;

  for (const auto &enemy : enemies) {
    if (!enemy || !enemy->isActive()) {
      continue;
    }

    if (enemy->isActivated()) {
      ++stats.active;
    } else {
      ++stats.inactive;
    }
  }

  return stats;
}

std::vector<WarpZoneInfo> Level::getWarpZones() const {
  if (levelId == 1) {
    return {
        {{912.f, 144.f, 32.f, 16.f}, "TUNNEL DOWN -> BONUS"},
        {{3920.f, 176.f, 48.f, 32.f}, "TUNNEL RIGHT -> OVERWORLD"},
    };
  }

  if (levelId == 2) {
    return {
        {{240.f, 160.f, 64.f, 48.f}, "WARP A RIGHT -> UNDERGROUND"},
        {{1856.f, 400.f, 32.f, 16.f}, "WARP B DOWN -> BONUS"},
        {{608.f, 656.f, 48.f, 32.f}, "WARP C1 RIGHT -> UNDERGROUND"},
        {{3008.f, 288.f, 64.f, 128.f}, "WARP EXIT RIGHT -> OVERWORLD"},
    };
  }

  return {};
}

void Level::update(float dt) {
  // Update tile animations (question block shimmer, bump effects)
  map.update(dt);
  map.updateDebris(dt);

  sf::FloatRect camBounds = camera.getViewBounds();
  const float spawnMargin = 80.f;
  constexpr float TileSize = 16.f;
  constexpr float EnemyVoidMargin = 64.f;
  const float enemyVoidY = map.getMapHeight() * TileSize + EnemyVoidMargin;

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

      // The camera is not a gameplay boundary. Remove an enemy only after its
      // logical position is safely below the complete map, so temporary jumps
      // and normal movement near the bottom row cannot trigger false deaths.
      if (enemy->getPosition().y > enemyVoidY) {
        enemy->onFellIntoVoid();
      }
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

  const std::size_t enemyCountBeforeCleanup = enemies.size();
  enemies.erase(
      std::remove_if(enemies.begin(), enemies.end(),
                     [](const auto &e) { return !e || !e->isActive(); }),
      enemies.end());
  removedEnemyCount += enemyCountBeforeCleanup - enemies.size();

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
  const std::string itemType = getBlockItemType(x, character);

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

std::string Level::getBlockItemType(float x, const Character *character) const {
  const std::string brickItemType = getBrickItemType(x);
  if (!brickItemType.empty()) {
    return brickItemType;
  }

  std::string itemType = "Coin";

  bool powerupSpot = (std::abs(x - 336.f) < 1.f || std::abs(x - 1248.f) < 1.f ||
                      std::abs(x - 1744.f) < 1.f);
  if (levelId == 2 && isUnderground && std::abs(x - 256.f) < 1.f) {
    powerupSpot = true;
  }

  if (powerupSpot) {
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
  return itemType;
}

std::string Level::getBrickItemType(float x) const {
  if (levelId != 1) {
    return {};
  }

  if (std::abs(x - 1616.f) < 2.f) {
    return "StarItem";
  }
  if (std::abs(x - 1024.f) < 2.f) {
    return "1UpMushroom";
  }
  return {};
}

void Level::warpToUnderground(Character *character) {
  std::cout << "[Level] Teleporting to hidden underground map area..."
            << std::endl;
  isUnderground = true;
  isInBonusRoom = false;
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

// Pipe A: Down-key on overworld Pipe 1 → underground main corridor
void Level::warpPipeA_Entry(Character *character) {
  std::cout << "[Level][1-2] Pipe A entered — warping to underground."
            << std::endl;
  isUnderground = true;
  isInBonusRoom = false;
  if (character) {
    // Underground floor tile tops are at y=464 (rows 29-30).
    // Spawn freely in air
    character->setPosition(48.f, 256.f);
    character->setVelocity(sf::Vector2f(30.f, 0.f)); // carry rightward momentum
  }
  // Underground corridor: ceiling y=304, floor y=480, midpoint=400
  camera.setCenter(300.f, 400.f);
}

// Pipe B: Down-key on underground Pipe 2 → bonus / hidden room (rows 35-43)
void Level::warpPipeB_Entry(Character *character) {
  std::cout << "[Level][1-2] Pipe B entered — warping to bonus room."
            << std::endl;
  isInBonusRoom = true;
  if (character) {
    // Hidden room occupies rows 35-43 (y=544-704).
    // Spawn Mario just inside the top-left opening (col 1-3 are empty).
    character->setPosition(32.f, 560.f);
    character->setVelocity(sf::Vector2f(0.f, 0.f));
  }
  // Camera: initial center matches Mario's Y so there is no abrupt snap
  camera.setCenter(200.f, 560.f);
}

// Pipe C1: Right-contact exit from bonus room → resurface in underground
void Level::warpPipeC1_Exit(Character *character) {
  std::cout << "[Level][1-2] Pipe C1 exit — returning to underground corridor."
            << std::endl;
  isInBonusRoom = false;
  if (character) {
    // Return to underground corridor past Pipe 2 so player continues rightward.
    // y=400 is safely above the underground floor (y≈464).
    character->setPosition(2000.f, 400.f);
    character->setVelocity(
        sf::Vector2f(30.f, -80.f)); // pop upward and drift right
  }
  camera.setCenter(2000.f, 400.f);
}

void Level::warpToOverworldExit(Character *character) {
  std::cout << "[Level] Teleporting back to Overworld — near-last pipe..."
            << std::endl;
  isUnderground = false;
  isInBonusRoom = false;
  if (character) {
    // Near-last overworld pipe is at col ~176-177 (x≈2816).
    // Pop Mario out of the pipe top (pipe top row 10, y=160 → spawn at y=144).
    character->setPosition(2872.f, 144.f);
    character->setVelocity(sf::Vector2f(0.f, -100.f)); // pop out of pipe
  }
  // Lock camera to overworld band (Y=112 keeps view at y=0..225, above
  // underground)
  camera.setCenter(2816.f, 112.f);
}
