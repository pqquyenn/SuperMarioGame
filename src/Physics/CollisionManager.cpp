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
            if (downPressed) {
                if (!level->getIsUnderground() && level->getLevelId() == 1 && pos.x >= 880.f && pos.x <= 960.f) {
                    level->warpToUnderground(character);
                    return;
                }
                if (level->getIsUnderground() && level->getLevelId() == 2 && !level->getIsInBonusRoom() && pos.x >= 1850.f && pos.x <= 2050.f) {
                    level->warpPipeB_Entry(character);
                    return;
                }
            }
          }
          if (character && tile->isHorizontalWarpPipe() && level) {
            bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
            if (rightPressed && character->getBounds().left +
                                    character->getBounds().width <=
                                    tileBounds.left + 5.f) {
                if (!level->getIsUnderground() && level->getLevelId() == 2) {
                    level->warpPipeA_Entry(character);
                    return;
                }
                if (level->getIsUnderground() && level->getLevelId() == 1) {
                    level->warpToOverworldExit(character);
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
            // Check if this brick contains an item (e.g. Star in 1-1 at x=1616)
            bool isItemBrick = false;
            if (level && level->getLevelId() == 1 && std::abs(tileBounds.left - 1616.f) < 2.f) {
              isItemBrick = true;
            }

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

        // Check horizontal pipe warp (Exit from underground)
        if (character && tile->isWarpPipe() && level && pos.x >= 3720.f) {
          bool rightPressed =
              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
          if (rightPressed) {
            level->warpToOverworldExit(character);
            return;
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
    sf::FloatRect characterBounds = character.getBounds();
    sf::FloatRect platBounds = platform.getBounds();
    if (characterBounds.left + characterBounds.width > platBounds.left &&
        characterBounds.left < platBounds.left + platBounds.width) {
        if (character.getVelocity().y >= 0.f &&
            characterBounds.top + characterBounds.height >= platBounds.top &&
            characterBounds.top + characterBounds.height <= platBounds.top + 8.f) {
            character.setPosition(character.getPosition().x,
                                  platBounds.top - characterBounds.height);
            character.setVelocity(sf::Vector2f(character.getVelocity().x, 0.f));
            character.setGrounded(true);
            character.move(platform.getVelocity() * (1.f/60.f));
        }
    }
}
