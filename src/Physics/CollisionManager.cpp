#include "Physics/CollisionManager.h"
#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/GreenParatroopa.h"
#include "Entities/Entity.h"
#include "Entities/Fireball.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/StarItem.h"
#include "Entities/MovingPlatform.h"
#include "Level/Level.h"
#include "Level/Tile.h"
#include "Level/TileMap.h"
#include "PlayerStates/PlayerState.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>


bool CollisionManager::checkAABB(const sf::FloatRect &a, const sf::FloatRect &b,
                                 sf::FloatRect &overlap) {
  return a.intersects(b, overlap);
}

void CollisionManager::resolveEntityCollisions(Entity &a, Entity &b) {
  sf::FloatRect overlap;
  if (!checkAABB(a.getBounds(), b.getBounds(), overlap)) {
    return;
  }

  Character *character = dynamic_cast<Character *>(&a);
  Enemy *enemy = dynamic_cast<Enemy *>(&b);
  if (!character) {
    character = dynamic_cast<Character *>(&b);
    enemy = dynamic_cast<Enemy *>(&a);
  }

  if (character && !character->isDying() && enemy && enemy->isActive()) {
    // A flattened enemy remains active briefly so its defeat animation can be
    // rendered. It must not damage the player again during that interval.
    if (enemy->isSquished()) {
      return;
    }

    // Star invincibility: Mario defeats any enemy on contact (not just stomp)
    if (character->defeatsEnemiesOnContact()) {
      enemy->onStomped();
      character->notify(GameEvent{GameEventType::ENEMY_DEFEATED, 100});
      return;
    }

    sf::FloatRect characterBounds = character->getBounds();
    sf::FloatRect enemyBounds = enemy->getBounds();

    // Stomp condition: Mario is moving downward and feet are above enemy top
    if (character->getVelocity().y > 0.f &&
        (characterBounds.top + characterBounds.height - overlap.height <=
         enemyBounds.top + 8.f)) {
      // Resolve the overlap before bouncing. Without this separation Mario
      // can still overlap the active defeat/shell animation on the next frame,
      // where his new upward velocity would misclassify it as side damage.
      character->setPosition(
          character->getPosition().x,
          enemyBounds.top - characterBounds.height);
      enemy->onStomped();
      // enemy->setActive(false);
      character->notify(GameEvent{GameEventType::ENEMY_DEFEATED, 100});
      character->setVelocity(
          sf::Vector2f(character->getVelocity().x, -250.f));
      return;
    } else {
      character->takeDamage();
      return;
    }
  }

  // Pure OOP: no type-checking. Entities resolve their own state via double
  // dispatch.
  a.onCollision(b, overlap);
  b.onCollision(a, overlap);
}

void CollisionManager::resolveTileCollisions(Entity &entity, TileMap &map,
                                             Level *level) {
  if (Coin *coin = dynamic_cast<Coin *>(&entity)) {
    if (coin->isPopping())
      return;
  }
  if (Mushroom *shroom = dynamic_cast<Mushroom *>(&entity)) {
    if (shroom->isEmerging())
      return;
  }
  if (FireFlower *flower = dynamic_cast<FireFlower *>(&entity)) {
    if (flower->isEmerging())
      return;
  }
  if (StarItem *star = dynamic_cast<StarItem *>(&entity)) {
    if (star->isEmerging())
      return;
  }

  sf::FloatRect bounds = entity.getBounds();

  // Grounded is contact state, not a persistent movement state. Clear the
  // previous frame's result before testing the current position; landing on a
  // tile (or the later moving-platform pass) will set it back to true.
  Character *character = dynamic_cast<Character *>(&entity);
  if (character) {
    character->setGrounded(false);
  }

  // Retrieve only tiles physically near the entity to minimize comparisons
  const std::vector<TileHandle> nearbyTiles = map.getTilesInBounds(bounds);

  sf::Vector2f pos = entity.getPosition();
  sf::Vector2f vel = entity.getVelocity();
  Enemy *enemy = dynamic_cast<Enemy *>(&entity);
  Fireball *fireball = dynamic_cast<Fireball *>(&entity);
  bool landedThisFrame = false;

  // ──────────────────────────────────────────────────────────────
  // PASS 1 — Y-axis resolution (Ground & Ceiling landing)
  // ──────────────────────────────────────────────────────────────
  for (const TileHandle& handle : nearbyTiles) {
    Tile *tile = map.getTile(handle);
    if (!tile || !tile->isSolid()) {
      continue;
    }

    const sf::FloatRect tileBounds = tile->getBounds();

    if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
      if (overlap.height <= overlap.width) {
        if (bounds.top < tileBounds.top) {
          pos.y -= overlap.height; // Landed on top of the tile (ground)
          landedThisFrame = true;

          if (character) {
            character->setGrounded(true);
          } else if (auto *star = dynamic_cast<StarItem *>(&entity)) {
            star->notifyGrounded();
          } else if (auto *gp = dynamic_cast<GreenParatroopa *>(&entity)) {
            gp->notifyLanded();
          }

          if (character && tile->isWarpPipe() && level) {
            bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
            if (downPressed && level->tryActivatePortal(
                    *character, tileBounds, PortalActivation::Down)) {
              return;
            }
            if (downPressed && !level->isDataDriven()) {
                // Pipe 2: Underground → Hidden Room (cols 116-118, x = 1856-1904)
                if (level->getIsUnderground() && !level->getIsInBonusRoom() && level->getLevelId() == 2
                    && pos.x >= 1840.f && pos.x <= 1920.f) {
                    return;
                }
            }
          }
        } else {
          pos.y += overlap.height; // Hit the underside (ceiling)

          // Question block bump logic
          if (character && tile->isQuestionBlock()) {
            tile->startBump();
            map.hitTile(handle);
            if (level) {
              level->spawnItemFromBlock(tileBounds.left, tileBounds.top,
                                        character);
            }
            character->notify(GameEvent{GameEventType::COIN_COLLECTED, 200});
          }

          // Brick block logic
          if (character && tile->isBrick()) {
            // Check if this brick contains an item (e.g. 1UP Mushroom at x=1024, Star at x=1616 in 1-1)
            bool isItemBrick = level && level->hasBlockContent(
                tileBounds.left, tileBounds.top);

            if (isItemBrick) {
              tile->startBump();
              map.hitTile(handle);
              if (level) {
                level->spawnItemFromBlock(tileBounds.left, tileBounds.top,
                                          character);
              }
            } else if (character->hasAbility(PlayerAbility::BreakBricks)) {
              // Super/Fire Mario breaks the brick
              map.breakBrick(handle);
            } else {
              // Small Mario just bumps the brick
              tile->startBump();
            }
          }
        }
        vel.y = 0.f;

        entity.setPosition(pos);
        bounds = entity.getBounds();
      }
    }
  }

  // ──────────────────────────────────────────────────────────────
  // PASS 2 — X-axis resolution (Wall contact & Direction reversal)
  // ──────────────────────────────────────────────────────────────
  for (const TileHandle& handle : nearbyTiles) {
    Tile *tile = map.getTile(handle);
    if (!tile || !tile->isSolid()) {
      continue;
    }

    const sf::FloatRect tileBounds = tile->getBounds();

    if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
      if (overlap.width < overlap.height) {
        if (bounds.left < tileBounds.left) {
          pos.x -= overlap.width; // Entity is to the left of the tile
        } else {
          pos.x += overlap.width; // Entity is to the right of the tile
        }
        vel.x = 0.f; // Kill horizontal momentum on wall contact

        if (enemy) {
          enemy->reverseDirection();
        } else if (Mushroom *shroom = dynamic_cast<Mushroom *>(&entity)) {
          shroom->reverseDirection();
        } else if (StarItem *star = dynamic_cast<StarItem *>(&entity)) {
          star->reverseDirection();
        } else if (fireball) {
          // Fireball chạm tường thì nổ / biến mất
          fireball->explode();
          entity.setPosition(pos);
          entity.setVelocity(vel);
          return;
        }

        // Horizontal Warp Pipes (Right-key triggers)
        if (character && tile->isWarpPipe() && level) {
          bool rightPressed =
              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
          if (rightPressed && level->tryActivatePortal(
                  *character, tileBounds, PortalActivation::Right)) {
            return;
          }
          if (rightPressed && !level->isDataDriven()) {
            // Pipe 1 Entry: Overworld -> Underground (horizontal pipe at start)
            if (!level->getIsUnderground() && !level->getIsInBonusRoom() && level->getLevelId() == 2
                && pos.x < 1000.f) {
                return;
            }
            // Pipe 4: Hidden room exit -> back to Underground
            if (level->getIsInBonusRoom() && level->getLevelId() == 2) {
                return;
            }
            // Last pipe: Underground -> Overworld (horizontal pipe at end)
            if (level->getIsUnderground() && !level->getIsInBonusRoom() && level->getLevelId() == 2
                && pos.x >= 2900.f) {
                return;
            }
          }
        }

        entity.setPosition(pos);
        bounds = entity.getBounds();
      }
    }
  }

  // ──────────────────────────────────────────────────────────────
  // PASS 3 — Collectibles & Warp Exits (non-solid tile checks)
  // ──────────────────────────────────────────────────────────────
  if (character) {
    for (const TileHandle& handle : nearbyTiles) {
      Tile *tile = map.getTile(handle);
      if (!tile)
        continue;
      sf::FloatRect tileBounds = tile->getBounds();
      if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
        if (tile->isCoinTile()) {
          map.removeTile(handle);
          character->notify(GameEvent{GameEventType::COIN_COLLECTED, 200});
        }
      }
    }
  }

  // Write the constrained velocity back to the entity
  entity.setVelocity(vel);

  // Fireball bounces back up when it touches ground.
  if (fireball && landedThisFrame) {
    fireball->bounce();
  }
}
void CollisionManager::resolveMovingPlatform(Character& character,
                                              MovingPlatform& platform) {
    if (!character.isActive() || !platform.isActive()) return;

    const sf::FloatRect characterBounds = character.getBounds();
    const sf::FloatRect platformBounds = platform.getBounds();
    if (characterBounds.left + characterBounds.width <= platformBounds.left ||
        characterBounds.left >= platformBounds.left + platformBounds.width ||
        characterBounds.top + characterBounds.height <= platformBounds.top ||
        characterBounds.top >= platformBounds.top + platformBounds.height) {
        return;
    }

    const float overlapLeft =
        characterBounds.left + characterBounds.width - platformBounds.left;
    const float overlapRight =
        platformBounds.left + platformBounds.width - characterBounds.left;
    const float overlapTop =
        characterBounds.top + characterBounds.height - platformBounds.top;
    const float overlapBottom =
        platformBounds.top + platformBounds.height - characterBounds.top;

    const float minOverlapX = std::min(overlapLeft, overlapRight);
    const float minOverlapY = std::min(overlapTop, overlapBottom);
    sf::Vector2f position = character.getPosition();
    const sf::Vector2f velocity = character.getVelocity();

    if (minOverlapY <= minOverlapX) {
        if (overlapTop < overlapBottom) {
            position.y = platformBounds.top - characterBounds.height;
            const sf::Vector2f platformDelta = platform.getDelta();
            character.setPosition(position.x + platformDelta.x,
                                  position.y + platformDelta.y);
            character.setVelocity(sf::Vector2f(velocity.x, 0.f));
            character.setGrounded(true);
        } else {
            position.y = platformBounds.top + platformBounds.height;
            character.setPosition(position);
            if (velocity.y < 0.f) {
                character.setVelocity(sf::Vector2f(velocity.x, 0.f));
            }
        }
    } else {
        if (overlapLeft < overlapRight) {
            position.x = platformBounds.left - characterBounds.width;
        } else {
            position.x = platformBounds.left + platformBounds.width;
        }
        character.setPosition(position);
        character.setVelocity(sf::Vector2f(0.f, velocity.y));
    }
}
