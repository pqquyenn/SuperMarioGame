#include "Level/Level.h"
#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Item.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/OneUpMushroom.h"
#include "Entities/Items/StarItem.h"
#include "Entities/MovingPlatform.h"
#include "Factories/EntityFactory.h"
#include "Level/LevelDefinitionLoader.h"
#include "Level/LevelWorldBuilder.h"
#include "Level/BlockContentResolver.h"
#include "Level/LevelCameraController.h"
#include "Level/PortalSystem.h"
#include "Physics/CollisionManager.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <vector>

Level::Level(int id) : levelId(id) {}
Level::~Level() = default;

bool Level::spawnEntitiesFromMap() {
  std::vector<std::string> errors;
  const bool success = LevelWorldBuilder{}.build(
      definition, map, enemies, items, movingPlatforms, errors);
  for (const auto &error : errors) {
    std::cerr << "[Level] " << error << std::endl;
  }
  return success;
}

bool Level::loadInternal(const std::string &filename, bool isUndergroundFlag) {
  if (!isUndergroundFlag) {
    LevelDefinition loaded;
    std::vector<std::string> errors;
    if (!LevelDefinitionLoader{}.load(filename, loaded, errors)) {
      std::cerr << "[Level] Invalid stage definition: " << filename
                << std::endl;
      for (const auto &error : errors) {
        std::cerr << "  - " << error << std::endl;
      }
      return false;
    }

    enemies.clear();
    items.clear();
    movingPlatforms.clear();
    activatedCheckpoints.clear();
    removedEnemyCount = 0;
    EntityFactory::getInstance().registerDefaultEntities();

    definition = std::move(loaded);
    hasDefinition = true;
    currentArea = definition.initialArea;
    isUnderground = currentArea == "underground";
    isInBonusRoom = currentArea == "bonus";

    for (const auto &block : definition.blockContents) {
      if (!BlockContentResolver::getInstance().hasRule(block.content) &&
          !EntityFactory::getInstance().contains(block.content)) {
        std::cerr << "[Level] Unknown block content or rule: "
                  << block.content << std::endl;
        return false;
      }
    }

    if (!map.readFromFile(definition.terrainPath)) return false;
    if (const auto &start = map.getStartMarker()) {
      levelStartHint = *start;
    } else {
      std::cerr << "[Level] Missing '@' start marker in "
                << definition.terrainPath << std::endl;
      return false;
    }
    levelEndHint = map.getEndMarker();
    if (!levelEndHint) {
      std::cerr << "[Level] Missing '!' end marker in "
                << definition.terrainPath << std::endl;
      return false;
    }
    if (!definition.backgroundPath.empty()) {
      bgMap.readFromFile(definition.backgroundPath);
    }

    // Legacy numeric ID remains available only to old diagnostics.
    if (definition.id == "world-1-1") levelId = 1;
    else if (definition.id == "world-1-2") levelId = 2;
    else if (definition.id == "world-1-3") levelId = 3;

    return spawnEntitiesFromMap();
  }

  std::cerr << "[Level] Legacy standalone hidden maps are disabled; "
               "define an area and portals in the stage manifest instead"
            << std::endl;
  return false;
#if 0 // Removed legacy loader retained temporarily for merge archaeology.

  // Compatibility path for the old debug-only hidden-map hotkey.
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
            std::cerr << "[Level] Map has no '@' player start marker: "
                      << path << std::endl;
          }

          levelEndHint = map.getEndMarker();
          if (!levelEndHint) {
            std::cerr << "[Level] Map has no '!' stage end marker: "
                      << path << std::endl;
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
#endif
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

sf::Vector2f Level::findGroundedSpawn(
    const sf::Vector2f &requestedPosition,
    const sf::Vector2f &characterSize) const {
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const float safeWidth = std::max(1.f, characterSize.x);
  const float safeHeight = std::max(1.f, characterSize.y);
  const float worldWidth = map.getMapWidth() * tileSize;
  const float worldHeight = map.getMapHeight() * tileSize;
  const float spawnX = std::clamp(
      requestedPosition.x, 0.f, std::max(0.f, worldWidth - safeWidth));
  const float searchTop = std::clamp(requestedPosition.y, 0.f, worldHeight);
  const sf::FloatRect searchBounds{
      spawnX, searchTop, safeWidth, std::max(0.f, worldHeight - searchTop)};

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

sf::Vector2f Level::getStartPosition(
    const sf::Vector2f &characterSize) const {
  return findGroundedSpawn(levelStartHint, characterSize);
}

bool Level::hasReachedEnd(const sf::FloatRect &characterBounds) const {
  if ((hasDefinition && currentArea != definition.initialArea) ||
      !levelEndHint) {
    return false;
  }

  const float markerSize = hasDefinition ? definition.tileSize : 16.f;
  const sf::FloatRect endBounds{
      levelEndHint->x, levelEndHint->y, markerSize, markerSize};
  return characterBounds.intersects(endBounds);
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

void Level::update(float dt) {
  // Update tile animations (question block shimmer, bump effects)
  map.update(dt);
  map.updateDebris(dt);

  sf::FloatRect camBounds = camera.getViewBounds();
  const float spawnMargin = 80.f;
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const float voidMarginTiles = hasDefinition
      ? definition.rules.enemyVoidMarginTiles : 4.f;
  const float enemyVoidY =
      map.getMapHeight() * tileSize + voidMarginTiles * tileSize;

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
  const std::string itemType = getBlockItemType(x, y, character);

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

std::string Level::getBlockItemType(
    float x,
    float y,
    const Character *character) const {
  const int tileX = static_cast<int>(std::lround(x / definition.tileSize));
  const int tileY = static_cast<int>(std::lround(y / definition.tileSize));
  std::string itemType = "Coin";
  for (const auto &block : definition.blockContents) {
    if (block.tilePosition.x == tileX && block.tilePosition.y == tileY) {
      itemType = block.content;
      break;
    }
  }

  return BlockContentResolver::getInstance().resolve(itemType, character);
}

bool Level::hasBlockContent(float x, float y) const {
  const int tileX = static_cast<int>(std::lround(x / definition.tileSize));
  const int tileY = static_cast<int>(std::lround(y / definition.tileSize));
  return std::any_of(
      definition.blockContents.begin(), definition.blockContents.end(),
      [tileX, tileY](const BlockContentDefinition &block) {
        return block.tilePosition.x == tileX && block.tilePosition.y == tileY;
      });
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

// Pipe A: Down-key on overworld Pipe 1 → underground main corridor
void Level::warpPipeA_Entry(Character *character) {
  std::cout << "[Level][1-2] Pipe A entered — warping to underground."
            << std::endl;
  isUnderground = true;
  isInBonusRoom = false;
  if (character) {
    // Underground floor tile tops are at y=464 (rows 29-30).
    // Spawn with feet safely above floor: y=432 lands him on row 27.
    character->setPosition(256.f, 432.f);
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
  std::cout
      << "[Level][1-2] Pipe C1 exit — returning to underground corridor."
      << std::endl;
  isInBonusRoom = false;
  if (character) {
    // Return to underground corridor past Pipe 2 so player continues rightward.
    // y=400 is safely above the underground floor (y≈464).
    character->setPosition(2000.f, 400.f);
    character->setVelocity(sf::Vector2f(30.f, -80.f)); // pop upward and drift right
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
    character->setPosition(2816.f, 144.f);
    character->setVelocity(sf::Vector2f(0.f, -100.f)); // pop out of pipe
  }
  // Lock camera to overworld band (Y=112 keeps view at y=0..225, above underground)
  camera.setCenter(2816.f, 112.f);
}

bool Level::tryActivatePortal(
    Character &character,
    const sf::FloatRect &contactedTile,
    PortalActivation activation) {
  if (!hasDefinition || !PortalSystem{}.tryActivate(
          definition, currentArea, character, contactedTile, activation)) {
    return false;
  }
  isUnderground = currentArea == "underground";
  isInBonusRoom = currentArea == "bonus";
  updateCameraFor(character.getPosition());
  return true;
}

void Level::updateCameraFor(const sf::Vector2f &playerPosition) {
  if (!hasDefinition) camera.update(playerPosition);
  else LevelCameraController{}.update(
      camera, definition, currentArea, playerPosition);
}

bool Level::usesDarkBackground() const {
  return hasDefinition
      ? LevelCameraController{}.usesDarkBackground(definition, currentArea)
      : isUnderground || isInBonusRoom;
}

float Level::getKillPlaneY() const {
  if (hasDefinition && definition.rules.killPlaneTile >= 0.f) {
    return definition.rules.killPlaneTile * definition.tileSize;
  }
  return (map.getMapHeight() + 4.f) * 16.f;
}

float Level::getLeftBoundaryX() const {
  return hasDefinition
      ? definition.rules.leftBoundaryTile * definition.tileSize
      : 0.f;
}

float Level::getRightBoundaryX(float entityWidth) const {
  const float mapRight = map.getMapWidth() *
      (hasDefinition ? definition.tileSize : 16.f);
  const float configured = hasDefinition && definition.rules.rightBoundaryTile >= 0.f
      ? definition.rules.rightBoundaryTile * definition.tileSize
      : mapRight;
  return std::max(getLeftBoundaryX(), configured - entityWidth);
}

std::optional<sf::Vector2f> Level::activateCheckpoint(
    const sf::FloatRect &characterBounds,
    const sf::Vector2f &characterSize) {
  if (!hasDefinition) return std::nullopt;
  const float tileSize = definition.tileSize;
  for (const auto &checkpoint : definition.checkpoints) {
    if (checkpoint.area != currentArea ||
        activatedCheckpoints.find(checkpoint.id) != activatedCheckpoints.end()) {
      continue;
    }
    const sf::FloatRect trigger{
        checkpoint.triggerTiles.left * tileSize,
        checkpoint.triggerTiles.top * tileSize,
        checkpoint.triggerTiles.width * tileSize,
        checkpoint.triggerTiles.height * tileSize};
    if (!trigger.intersects(characterBounds)) continue;
    activatedCheckpoints.insert(checkpoint.id);
    return findGroundedSpawn(checkpoint.spawnTile * tileSize, characterSize);
  }
  return std::nullopt;
}
