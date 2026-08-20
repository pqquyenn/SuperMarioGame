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
#include "Physics/CollisionManager.h"
#include "PlayerStates/PlayerState.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <utility>
#include <vector>

Level::Level(int id) : levelId(id) {}
Level::~Level() = default;

bool Level::spawnEntitiesFromMap() {
  std::vector<std::string> errors;
  const bool success = LevelWorldBuilder{}.build(
      definition, map, enemies, items, movingPlatforms, errors);
  for (const auto& error : errors) {
    std::cerr << "[Level] " << error << std::endl;
  }
  return success;
}

void Level::setCurrentArea(const std::string& area) {
  currentArea = area.empty() ? "overworld" : area;
  isInBonusRoom = currentArea == "bonus";
  isUnderground = isInBonusRoom || currentArea == "underground";
}

bool Level::loadInternal(const std::string &filename, bool isUndergroundFlag) {
  enemies.clear();
  items.clear();
  movingPlatforms.clear();
  removedEnemyCount = 0;
  hasDefinition = false;
  definition = {};
  currentArea = "overworld";
  isUnderground = false;
  isInBonusRoom = false;

  EntityFactory::getInstance().registerDefaultEntities();

  if (isUndergroundFlag) {
    return loadLegacyMap(filename, true);
  }

  LevelDefinition loaded;
  std::vector<std::string> errors;
  if (!LevelDefinitionLoader{}.load(filename, loaded, errors)) {
    const std::filesystem::path requested(filename);
    if (requested.extension() == ".txt" &&
        LevelDefinitionLoader::findManifest(filename).empty()) {
      std::cerr << "[Level] No manifest found for " << filename
                << "; using legacy terrain loader." << std::endl;
      return loadLegacyMap(filename, false);
    }
    std::cerr << "[Level] Invalid stage definition: " << filename << std::endl;
    for (const auto& error : errors) {
      std::cerr << "  - " << error << std::endl;
    }
    return false;
  }

  definition = std::move(loaded);
  hasDefinition = true;
  setCurrentArea(definition.initialArea);

  if (!map.readFromFile(definition.terrainPath)) {
    std::cerr << "[Level] Failed to load terrain: "
              << definition.terrainPath << std::endl;
    return false;
  }

  if (const auto& startMarker = map.getStartMarker()) {
    levelStartHint = *startMarker;
  } else {
    std::cerr << "[Level] Terrain has no '@' player start marker: "
              << definition.terrainPath << std::endl;
    return false;
  }

  if (!definition.backgroundPath.empty() &&
      !bgMap.readFromFile(definition.backgroundPath)) {
    std::cerr << "[Level] Failed to load background: "
              << definition.backgroundPath << std::endl;
  }

  // Keep the legacy numeric ID only for existing UI/debug consumers. The
  // level definition, rather than this ID, owns all gameplay content.
  if (definition.id == "world-1-1") {
    levelId = 1;
  } else if (definition.id == "world-1-2") {
    levelId = 2;
  } else if (definition.id == "world-1-3") {
    levelId = 3;
  }

  return spawnEntitiesFromMap();
}

bool Level::loadLegacyMap(
    const std::string& filename,
    bool isUndergroundFlag) {
  hasDefinition = false;
  definition = {};
  currentArea = "overworld";
  setCurrentArea(isUndergroundFlag ? "underground" : "overworld");

  const std::filesystem::path requested(filename);
  const std::string rawFilename = requested.filename().string();

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
        if (!isUndergroundFlag) {
          if (const auto& startMarker = map.getStartMarker()) {
            levelStartHint = *startMarker;
          } else {
          std::cerr << "[Level] Map has no '@' player start marker: "
                    << path << std::endl;
          }
        }

        std::filesystem::path bgPath =
            std::filesystem::path(path).parent_path() / "background.txt";
        if (std::filesystem::exists(bgPath)) {
          bgMap.readFromFile(bgPath.string());
        }
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
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const float safeWidth = std::max(1.f, characterSize.x);
  const float safeHeight = std::max(1.f, characterSize.y);
  const float worldWidth = map.getMapWidth() * tileSize;
  const float worldHeight = map.getMapHeight() * tileSize;
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
  std::vector<WarpZoneInfo> zones;
  if (!hasDefinition) {
    return zones;
  }

  const float tileSize = definition.tileSize;
  for (const auto& portal : definition.portals) {
    const std::string activation =
        portal.activation == PortalActivation::Down ? "DOWN" : "RIGHT";
    zones.push_back({
        {portal.triggerTiles.left * tileSize,
         portal.triggerTiles.top * tileSize,
         portal.triggerTiles.width * tileSize,
         portal.triggerTiles.height * tileSize},
        portal.id + " " + activation + " -> " + portal.targetAnchor});
  }
  return zones;
}

bool Level::tryActivatePortal(
    Character& character,
    const sf::FloatRect& contact,
    PortalActivation activation) {
  if (!hasDefinition || !character.isActive() || character.isDying()) {
    return false;
  }

  const float tileSize = definition.tileSize;
  const sf::FloatRect contactTiles{
      contact.left / tileSize,
      contact.top / tileSize,
      contact.width / tileSize,
      contact.height / tileSize};

  for (const auto& portal : definition.portals) {
    if (portal.sourceArea != currentArea ||
        portal.activation != activation ||
        !portal.triggerTiles.intersects(contactTiles)) {
      continue;
    }

    const auto anchor = std::find_if(
        definition.anchors.begin(),
        definition.anchors.end(),
        [&portal](const AnchorDefinition& candidate) {
          return candidate.id == portal.targetAnchor;
        });
    if (anchor == definition.anchors.end()) {
      std::cerr << "[Level] Portal target anchor not found: "
                << portal.targetAnchor << std::endl;
      return false;
    }

    setCurrentArea(anchor->area);
    character.setPosition(
        anchor->tilePosition.x * tileSize,
        anchor->tilePosition.y * tileSize);
    character.setVelocity(anchor->exitVelocity);
    updateCameraFor(character.getPosition());
    std::cout << "[Level] Activated portal " << portal.id << " -> "
              << anchor->id << std::endl;
    return true;
  }

  return false;
}

bool Level::tryActivatePortalForInput(
    Character& character,
    PortalActivation activation) {
  if (!character.isActive() || character.isDying()) {
    return false;
  }

  if (hasDefinition) {
    const sf::FloatRect bounds = character.getBounds();
    const float centerX = bounds.left + bounds.width * 0.5f;
    const float feetY = bounds.top + bounds.height;
    const float tileSize = definition.tileSize;
    constexpr float contactTolerance = 3.f;

    for (const auto& portal : definition.portals) {
      if (portal.sourceArea != currentArea ||
          portal.activation != activation) {
        continue;
      }

      const sf::FloatRect trigger{
          portal.triggerTiles.left * tileSize,
          portal.triggerTiles.top * tileSize,
          portal.triggerTiles.width * tileSize,
          portal.triggerTiles.height * tileSize};

      bool canActivate = false;
      if (activation == PortalActivation::Down) {
        const bool centered = centerX >= trigger.left &&
                              centerX <= trigger.left + trigger.width;
        const bool touchingTop =
            std::abs(feetY - trigger.top) <= contactTolerance;
        canActivate = centered && touchingTop;
      } else {
        canActivate = trigger.intersects(bounds);
      }

      if (canActivate &&
          tryActivatePortal(character, trigger, activation)) {
        return true;
      }
    }
    return false;
  }

  const sf::FloatRect characterBounds = character.getBounds();
  const float centerX =
      characterBounds.left + characterBounds.width * 0.5f;
  const float feetY = characterBounds.top + characterBounds.height;
  constexpr float contactTolerance = 3.f;
  const auto standsOnEntrance =
      [centerX, feetY, contactTolerance](const sf::FloatRect& entrance) {
        const bool centered = centerX >= entrance.left &&
                              centerX <= entrance.left + entrance.width;
        const bool touchingTop =
            std::abs(feetY - entrance.top) <= contactTolerance;
        return centered && touchingTop;
      };

  if (activation == PortalActivation::Down) {
    if (levelId == 1 && !isUnderground && !isInBonusRoom &&
        standsOnEntrance({912.f, 144.f, 32.f, 16.f})) {
      warpToUnderground(&character);
      return true;
    }

    if (levelId == 2 && isUnderground && !isInBonusRoom &&
        standsOnEntrance({1856.f, 400.f, 32.f, 16.f})) {
      warpPipeB_Entry(&character);
      return true;
    }
    return false;
  }

  bool touchingWarpPipe = false;
  for (const TileHandle& handle : map.getTilesInBounds(characterBounds)) {
    const Tile* tile = map.getTile(handle);
    if (!tile || !tile->isWarpPipe()) {
      continue;
    }

    sf::FloatRect overlap;
    if (characterBounds.intersects(tile->getBounds(), overlap)) {
      touchingWarpPipe = true;
      break;
    }
  }

  if (!touchingWarpPipe) {
    return false;
  }

  if (levelId == 1 && isUnderground && character.getPosition().x >= 3720.f) {
    warpToOverworldExit(&character);
    return true;
  }

  if (levelId == 2 && !isUnderground && !isInBonusRoom &&
      character.getPosition().x < 1000.f) {
    warpPipeA_Entry(&character);
    return true;
  }

  if (levelId == 2 && isInBonusRoom) {
    warpPipeC1_Exit(&character);
    return true;
  }

  if (levelId == 2 && isUnderground && !isInBonusRoom &&
      character.getPosition().x >= 2900.f) {
    warpToOverworldExit(&character);
    return true;
  }

  return false;
}

void Level::updateCameraFor(const sf::Vector2f& playerPosition) {
  if (!hasDefinition || definition.cameraZones.empty()) {
    camera.update(playerPosition);
    return;
  }

  const float tileSize = definition.tileSize;
  const sf::Vector2f playerTile{
      playerPosition.x / tileSize, playerPosition.y / tileSize};
  const CameraZoneDefinition* selected = nullptr;

  for (const auto& zone : definition.cameraZones) {
    if (zone.area != currentArea) {
      continue;
    }
    if (zone.boundsTiles.contains(playerTile)) {
      selected = &zone;
      break;
    }
    if (!selected) {
      selected = &zone;
    }
  }

  if (!selected) {
    camera.update(playerPosition);
    return;
  }

  const sf::Vector2f viewSize = camera.getView().getSize();
  const float halfWidth = viewSize.x * 0.5f;
  const float halfHeight = viewSize.y * 0.5f;
  const sf::FloatRect bounds{
      selected->boundsTiles.left * tileSize,
      selected->boundsTiles.top * tileSize,
      selected->boundsTiles.width * tileSize,
      selected->boundsTiles.height * tileSize};

  float targetX = camera.getView().getCenter().x;
  if (selected->followX) {
    targetX = playerPosition.x;
  }
  const float minX = bounds.left + halfWidth;
  const float maxX = bounds.left + bounds.width - halfWidth;
  if (maxX >= minX) {
    targetX = std::clamp(targetX, minX, maxX);
  } else if (!selected->followX) {
    targetX = bounds.left + bounds.width * 0.5f;
  }

  float targetY = selected->followY
      ? playerPosition.y
      : selected->centerYTiles * tileSize;
  const float minY = bounds.top + halfHeight;
  const float maxY = bounds.top + bounds.height - halfHeight;
  if (maxY >= minY) {
    targetY = std::clamp(targetY, minY, maxY);
  } else if (!selected->followY) {
    targetY = bounds.top + bounds.height * 0.5f;
  }

  camera.setCenter(targetX, targetY);
}

bool Level::usesDarkBackground() const {
  if (!hasDefinition) {
    return isUnderground || isInBonusRoom;
  }

  const sf::Vector2f center = camera.getView().getCenter();
  const float tileSize = definition.tileSize;
  for (const auto& zone : definition.cameraZones) {
    if (zone.area != currentArea) {
      continue;
    }
    if (zone.boundsTiles.contains({center.x / tileSize, center.y / tileSize})) {
      return zone.darkBackground;
    }
  }
  return isUnderground || isInBonusRoom;
}

float Level::getKillPlaneY() const {
  if (hasDefinition && definition.rules.killPlaneTile >= 0.f) {
    return definition.rules.killPlaneTile * definition.tileSize;
  }

  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const float margin = hasDefinition
      ? definition.rules.enemyVoidMarginTiles * tileSize
      : 64.f;
  return map.getMapHeight() * tileSize + margin;
}

float Level::getLeftBoundaryX() const {
  if (hasDefinition) {
    return definition.rules.leftBoundaryTile * definition.tileSize;
  }
  return 0.f;
}

float Level::getRightBoundaryX(float entityWidth) const {
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const float rightTile = hasDefinition &&
                                  definition.rules.rightBoundaryTile >= 0.f
      ? definition.rules.rightBoundaryTile
      : static_cast<float>(map.getMapWidth());
  return std::max(getLeftBoundaryX(), rightTile * tileSize - entityWidth);
}

void Level::setIsUnderground(bool value) {
  if (value) {
    setCurrentArea("underground");
  } else {
    isUnderground = false;
    if (!isInBonusRoom) {
      currentArea = "overworld";
    }
  }
}

void Level::setIsInBonusRoom(bool value) {
  if (value) {
    setCurrentArea("bonus");
  } else {
    isInBonusRoom = false;
    currentArea = isUnderground ? "underground" : "overworld";
  }
}

void Level::update(float dt) {
  // Update tile animations (question block shimmer, bump effects)
  map.update(dt);
  map.updateDebris(dt);

  sf::FloatRect camBounds = camera.getViewBounds();
  const float spawnMargin = 80.f;
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const float enemyVoidY = hasDefinition
      ? map.getMapHeight() * tileSize +
            definition.rules.enemyVoidMarginTiles * tileSize
      : getKillPlaneY();

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
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;

  sf::Vector2f spawnPos =
      (itemType == "Coin") ? sf::Vector2f{x, y - tileSize}
                            : sf::Vector2f{x, y};
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
  std::string itemType = "Coin";

  if (hasDefinition) {
    const float tileSize = definition.tileSize;
    for (const auto& block : definition.blockContents) {
      if (block.area == currentArea &&
          std::abs(static_cast<float>(block.tilePosition.x) - x / tileSize) <
              0.01f &&
          std::abs(static_cast<float>(block.tilePosition.y) - y / tileSize) <
              0.01f) {
        itemType = block.content;
        break;
      }
    }
  }

  if (itemType == "PowerupByForm") {
    if (character) {
      const std::string_view form = character->getCurrentFormName();
      itemType = (form == "Super" || form == "Fire")
          ? "FireFlower"
          : "Mushroom";
    } else {
      itemType = "Mushroom";
    }
  }

  return itemType;
}

std::string Level::getBrickItemType(float x, float y) const {
  if (!hasDefinition) {
    return {};
  }

  const float tileSize = definition.tileSize;
  for (const auto& block : definition.blockContents) {
    if (block.area == currentArea &&
        std::abs(static_cast<float>(block.tilePosition.x) - x / tileSize) <
            0.01f &&
        std::abs(static_cast<float>(block.tilePosition.y) - y / tileSize) <
            0.01f) {
      return block.content;
    }
  }
  return {};
}

void Level::onTileCeilingContact(
    Entity& entity,
    TileMap& tileMap,
    Tile& tile,
    const TileHandle& handle,
    const CollisionContact& contact) {
  (void)contact;
  auto* character = dynamic_cast<Character*>(&entity);
  if (!character || !character->isActive()) {
    return;
  }

  const sf::FloatRect tileBounds = tile.getBounds();
  if (tile.isQuestionBlock()) {
    tile.startBump();
    tileMap.hitTile(handle);
    spawnItemFromBlock(tileBounds.left, tileBounds.top, character);
    character->notify(GameEvent::coinCollected(Coin::defaultScoreValue()));
    return;
  }

  if (!tile.isBrick()) {
    return;
  }

  // A brick with an entry in the manifest is an item brick. Ordinary bricks
  // retain the classic break-or-bump behaviour based on player ability.
  const bool isItemBrick = !getBrickItemType(
      tileBounds.left, tileBounds.top).empty();
  if (isItemBrick) {
    tile.startBump();
    tileMap.hitTile(handle);
    spawnItemFromBlock(tileBounds.left, tileBounds.top, character);
  } else if (character->hasAbility(PlayerAbility::BreakBricks)) {
    tileMap.breakBrick(handle);
  } else {
    tile.startBump();
  }
}

void Level::onTileOverlap(
    Entity& entity,
    TileMap& tileMap,
    Tile& tile,
    const TileHandle& handle,
    const CollisionContact& contact) {
  (void)contact;
  auto* character = dynamic_cast<Character*>(&entity);
  if (!character || !character->isActive() || !tile.isCoinTile()) {
    return;
  }

  tileMap.removeTile(handle);
  character->notify(GameEvent::coinCollected(Coin::defaultScoreValue()));
}

void Level::warpToUnderground(Character *character) {
  std::cout << "[Level] Teleporting to hidden underground map area..."
            << std::endl;
  setCurrentArea("bonus");
  if (character) {
    character->setPosition(3736.f, 32.f);
    character->setVelocity(sf::Vector2f(0.f, 0.f));
  }
  if (hasDefinition) {
    updateCameraFor(character ? character->getPosition()
                             : camera.getView().getCenter());
  } else {
    camera.setCenter(3736.f, 120.f);
  }
}

void Level::warpToUnderground1_2(Character *character) {
  std::cout << "[Level] Teleporting to hidden underground coin room in 1-2..."
            << std::endl;
  setCurrentArea("bonus");
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
  setCurrentArea("underground");
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
  setCurrentArea("bonus");
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
  setCurrentArea("underground");
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
  setCurrentArea("overworld");
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
