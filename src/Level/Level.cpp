#include "Level/Level.h"
#include "Core/SoundManager.h"
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

Level::Level() = default;
Level::~Level() = default;

bool Level::spawnEntitiesFromMap() {
  std::vector<std::string> errors;
  const bool success = LevelWorldBuilder{EntityFactory::getInstance()}.build(
      definition, map, enemies, items, movingPlatforms, errors);
  for (const auto& error : errors) {
    std::cerr << "[Level] " << error << std::endl;
  }
  return success;
}

void Level::setCurrentArea(const std::string& area) {
  currentArea = area.empty() ? "overworld" : area;
}

bool Level::loadManifest(const std::string &filename) {
  enemies.clear();
  items.clear();
  movingPlatforms.clear();
  removedEnemyCount = 0;
  hasDefinition = false;
  definition = {};
  currentArea = "overworld";

  LevelDefinition loaded;
  std::vector<std::string> errors;
  if (!LevelDefinitionLoader{}.load(filename, loaded, errors)) {
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

  return spawnEntitiesFromMap();
}

bool Level::loadLevel(const std::string &levelFile) {
  const std::filesystem::path requested(levelFile);
  if (requested.extension() != ".level") {
    std::cerr << "[Level] Stage entry must be a .level manifest: "
              << levelFile << std::endl;
    return false;
  }
  return loadManifest(levelFile);
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
        portal.activation == PortalActivation::Down
            ? "DOWN"
            : (portal.activation == PortalActivation::Right
                   ? "RIGHT"
                   : "INTERACT");
    zones.push_back({
        {portal.triggerTiles.left * tileSize,
         portal.triggerTiles.top * tileSize,
         portal.triggerTiles.width * tileSize,
         portal.triggerTiles.height * tileSize},
        portal.id + " " + activation + " -> " + portal.targetAnchor});
  }
  return zones;
}

const AnchorDefinition* Level::findAnchor(
    const std::string& anchorId) const {
  const auto anchor = std::find_if(
      definition.anchors.begin(),
      definition.anchors.end(),
      [&anchorId](const AnchorDefinition& candidate) {
        return candidate.id == anchorId;
      });
  return anchor == definition.anchors.end() ? nullptr : &*anchor;
}

bool Level::activatePortal(
    Character& character,
    const PortalDefinition& portal) {
  const AnchorDefinition* anchor = findAnchor(portal.targetAnchor);
  if (!anchor) {
    std::cerr << "[Level] Portal target anchor not found: "
              << portal.targetAnchor << std::endl;
    return false;
  }

  const float tileSize = definition.tileSize;
  const PortalTransition transition{
      portal.id,
      anchor->id,
      anchor->area,
      {anchor->tilePosition.x * tileSize,
       anchor->tilePosition.y * tileSize},
      anchor->exitVelocity};
  return activatePortalForParty(transition, {&character});
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

    return activatePortal(character, portal);
  }

  return false;
}

bool Level::tryActivatePortalForInput(
    Character& character,
    PortalActivation activation) {
  const auto transition = queryPortalForInput(character, activation);
  return transition && activatePortalForParty(*transition, {&character});
}

std::optional<PortalTransition> Level::queryPortalForInput(
    const Character& character,
    PortalActivation activation) const {
  if (!character.isActive() || character.isDying() || !hasDefinition) {
    return std::nullopt;
  }

  const sf::FloatRect bounds = character.getBounds();
  const float centerX = bounds.left + bounds.width * 0.5f;
  const float feetY = bounds.top + bounds.height;
  const float tileSize = definition.tileSize;
  constexpr float contactTolerance = 6.f;

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
      const bool centered = centerX >= (trigger.left - 4.f) &&
                            centerX <= (trigger.left + trigger.width + 4.f);
      const bool touchingTop =
          std::abs(feetY - trigger.top) <= contactTolerance;
      canActivate = centered && touchingTop;
    } else if (activation == PortalActivation::Right) {
      const bool touchingLeft =
          std::abs((bounds.left + bounds.width) - trigger.left) <=
              contactTolerance ||
          ((bounds.left + bounds.width >= trigger.left - contactTolerance) &&
           (bounds.left <= trigger.left + trigger.width));
      const bool verticalOverlap =
          (feetY > trigger.top) &&
          (bounds.top < trigger.top + trigger.height + 4.f);
      canActivate =
          (touchingLeft && verticalOverlap) || trigger.intersects(bounds);
    } else {
      canActivate = trigger.intersects(bounds);
    }

    if (!canActivate) {
      continue;
    }

    const AnchorDefinition* anchor = findAnchor(portal.targetAnchor);
    if (!anchor) {
      return std::nullopt;
    }
    return PortalTransition{
        portal.id,
        anchor->id,
        anchor->area,
        {anchor->tilePosition.x * tileSize,
         anchor->tilePosition.y * tileSize},
        anchor->exitVelocity};
  }
  return std::nullopt;
}

bool Level::activatePortalForParty(
    const PortalTransition& transition,
    const std::vector<Character*>& characters) {
  const auto first = std::find_if(
      characters.begin(), characters.end(),
      [](const Character* character) { return character != nullptr; });
  if (!hasDefinition || first == characters.end() ||
      transition.targetArea.empty()) {
    return false;
  }

  const std::string oldArea = currentArea;
  setCurrentArea(transition.targetArea);

  std::size_t placed = 0;
  Character* cameraTarget = nullptr;
  for (Character* character : characters) {
    if (!character) {
      continue;
    }
    sf::Vector2f target = transition.targetPosition;
    if (placed > 0) {
      const sf::FloatRect bounds = character->getBounds();
      target = findSafeSpawnNear(
          transition.targetPosition,
          {std::max(1.f, bounds.width), std::max(1.f, bounds.height)});
    }
    character->setPosition(target);
    character->setVelocity(transition.exitVelocity);
    if (!cameraTarget) {
      cameraTarget = character;
    }
    ++placed;
  }

  if (cameraTarget) {
    updateCameraFor(cameraTarget->getPosition());
  }
  SoundManager::getInstance().playSound("pipe");

  if (oldArea != currentArea && cameraTarget &&
      !cameraTarget->isStarInvincible()) {
    if (usesDarkBackground() || transition.targetArea == "bonus" ||
        transition.targetArea == "underground") {
      SoundManager::getInstance().playBGM(
          "assets/audio/music/underground.wav");
    } else {
      SoundManager::getInstance().playBGM(
          "assets/audio/music/overworld.wav");
    }
  }

  std::cout << "[Level] Activated portal " << transition.portalId << " -> "
            << transition.targetAnchor << " for " << placed
            << " player(s)" << std::endl;
  return placed > 0;
}

bool Level::tryActivateFirstPortalFromCurrentArea(Character& character) {
  if (!hasDefinition || !character.isActive() || character.isDying()) {
    return false;
  }

  const auto portal = std::find_if(
      definition.portals.begin(),
      definition.portals.end(),
      [this](const PortalDefinition& candidate) {
        return candidate.sourceArea == currentArea;
      });
  return portal != definition.portals.end() &&
         activatePortal(character, *portal);
}

void Level::resetToInitialArea() {
  if (hasDefinition) {
    setCurrentArea(definition.initialArea);
  }
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

  float targetX = selected->followX
      ? playerPosition.x
      : bounds.left + bounds.width * 0.5f;
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
    return false;
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
  return false;
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

sf::Vector2f Level::findSafeSpawnNear(
    const sf::Vector2f& requestedPosition,
    const sf::Vector2f& characterSize) const {
  const float width = std::max(1.f, characterSize.x);
  const float height = std::max(1.f, characterSize.y);
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const float worldWidth = map.getMapWidth() * tileSize;
  const float worldHeight = map.getMapHeight() * tileSize;
  const float offsets[] = {24.f, -24.f, 40.f, -40.f, 56.f, -56.f};

  for (float offset : offsets) {
    const sf::Vector2f candidate{
        std::clamp(requestedPosition.x + offset,
                   getLeftBoundaryX(),
                   std::max(getLeftBoundaryX(),
                            getRightBoundaryX(width))),
        std::clamp(requestedPosition.y, 0.f,
                   std::max(0.f, worldHeight - height))};
    const sf::FloatRect candidateBounds{
        candidate.x, candidate.y, width, height};
    bool blocked = false;
    for (const TileHandle& handle : map.getTilesInBounds(candidateBounds)) {
      const Tile* tile = map.getTile(handle);
      if (tile && tile->isSolid() &&
          tile->getBounds().intersects(candidateBounds)) {
        blocked = true;
        break;
      }
    }
    if (!blocked && candidate.x + width <= worldWidth) {
      return candidate;
    }
  }

  return findGroundedSpawn(requestedPosition, {width, height});
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

  // Terrain is the world backdrop. Draw gameplay entities afterward so they
  // cannot disappear behind ordinary solid tiles.
  map.render(window, camera);

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

  // Render brick debris on top of everything
  map.renderDebris(window);
}

void Level::spawnItemFromBlock(float x, float y) {
  spawnItemFromBlock(x, y, nullptr);
}

void Level::spawnItemFromBlock(float x, float y, Character *character) {
  spawnItemFromBlock(x, y, character, 1u);
}

void Level::spawnItemFromBlock(
    float x,
    float y,
    Character *character,
    unsigned requestedCopies) {
  const std::string itemType = getBlockItemType(x, y, character);
  const float tileSize = hasDefinition ? definition.tileSize : 16.f;
  const bool canDuplicate =
      itemType == "Mushroom" || itemType == "FireFlower";
  const unsigned copies = canDuplicate
      ? std::clamp(requestedCopies, 1u, 4u)
      : 1u;
  bool spawnedPowerup = false;

  for (unsigned index = 0; index < copies; ++index) {
    float horizontalOffset = 0.f;
    if (copies > 1) {
      const float center = (static_cast<float>(copies) - 1.f) * 0.5f;
      horizontalOffset = (static_cast<float>(index) - center) * 8.f;
    }
    sf::Vector2f spawnPos =
        (itemType == "Coin")
            ? sf::Vector2f{x, y - tileSize}
            : sf::Vector2f{x + horizontalOffset, y};
    auto entity = EntityFactory::getInstance().create(itemType, spawnPos);
    if (!entity) {
      continue;
    }
    if (itemType == "Coin") {
      if (auto *coin = dynamic_cast<Coin *>(entity.get())) {
        entity.release();
        coin->startPop();
        items.push_back(std::unique_ptr<Item>(coin));
        SoundManager::getInstance().playSound("coin");
      }
    } else {
      if (auto *item = dynamic_cast<Item *>(entity.get())) {
        if (auto *shroom = dynamic_cast<Mushroom *>(item)) {
          if (copies > 1 && index == 0) {
            shroom->reverseDirection();
          }
          shroom->startEmerge();
        } else if (auto *flower = dynamic_cast<FireFlower *>(item)) {
          flower->startEmerge();
        } else if (auto *star = dynamic_cast<StarItem *>(item)) {
          star->startEmerge();
        }
        entity.release();
        items.push_back(std::unique_ptr<Item>(item));
        spawnedPowerup = true;
      }
    }
  }
  if (spawnedPowerup) {
    SoundManager::getInstance().playSound("powerupappear");
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
    const std::string itemType = getBlockItemType(
        tileBounds.left, tileBounds.top, character);
    tile.startBump();
    tileMap.hitTile(handle);
    spawnItemFromBlock(
        tileBounds.left,
        tileBounds.top,
        character,
        powerupSpawnMultiplier);
    if (itemType == "Coin") {
      character->notify(
          GameEvent::coinCollected(Coin::defaultScoreValue()));
    }
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
    SoundManager::getInstance().playSound("blockbreak");
  } else {
    tile.startBump();
    SoundManager::getInstance().playSound("blockhit");
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
  SoundManager::getInstance().playSound("coin");
  character->notify(GameEvent::coinCollected(Coin::defaultScoreValue()));
}
