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
#include "Entities/Mario.h"
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

  Mario *mario = dynamic_cast<Mario *>(&a);
  Enemy *enemy = dynamic_cast<Enemy *>(&b);
  if (!mario) {
    mario = dynamic_cast<Mario *>(&b);
    enemy = dynamic_cast<Enemy *>(&a);
  }

  if (mario && !mario->isDying() && enemy && enemy->isActive()) {
    // A flattened enemy remains active briefly so its defeat animation can be
    // rendered. It must not damage the player again during that interval.
    if (enemy->isSquished()) {
      return;
    }

    // Star invincibility: Mario defeats any enemy on contact (not just stomp)
    if (mario->defeatsEnemiesOnContact()) {
      enemy->onStomped();
      mario->notify(GameEvent{GameEventType::ENEMY_DEFEATED, 100});
      return;
    }

    sf::FloatRect marioBounds = mario->getBounds();
    sf::FloatRect enemyBounds = enemy->getBounds();

    // Stomp condition: Mario is moving downward and feet are above enemy top
    if (mario->getVelocity().y > 0.f &&
        (marioBounds.top + marioBounds.height - overlap.height <=
         enemyBounds.top + 8.f)) {
      // Resolve the overlap before bouncing. Without this separation Mario
      // can still overlap the active defeat/shell animation on the next frame,
      // where his new upward velocity would misclassify it as side damage.
      mario->setPosition(
          mario->getPosition().x,
          enemyBounds.top - marioBounds.height);
      enemy->onStomped();
      // enemy->setActive(false);
      mario->notify(GameEvent{GameEventType::ENEMY_DEFEATED, 100});
      mario->setVelocity(sf::Vector2f(mario->getVelocity().x, -250.f));
      return;
    } else {
      mario->takeDamage();
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
  const std::vector<Tile *> nearbyTiles = map.getTilesInBounds(bounds);

  sf::Vector2f pos = entity.getPosition();
  sf::Vector2f vel = entity.getVelocity();
  Mario *mario = dynamic_cast<Mario *>(&entity);
  Enemy *enemy = dynamic_cast<Enemy *>(&entity);
  Fireball *fireball = dynamic_cast<Fireball *>(&entity);
  bool landedThisFrame = false;

  // ──────────────────────────────────────────────────────────────
  // PASS 1 — Y-axis resolution (Ground & Ceiling landing)
  // ──────────────────────────────────────────────────────────────
  for (const Tile *tile : nearbyTiles) {
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

          if (mario && tile->isWarpPipe() && level) {
            bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
            if (downPressed) {
                if (!level->getIsUnderground() && level->getLevelId() == 1 && pos.x >= 880.f && pos.x <= 960.f) {
                    level->warpToUnderground(mario);
                    return;
                }
                if (level->getIsUnderground() && level->getLevelId() == 2 && !level->getIsInBonusRoom() && pos.x >= 1850.f && pos.x <= 2050.f) {
                    level->warpPipeB_Entry(mario);
                    return;
                }
            }
          }
          if (mario && tile->isHorizontalWarpPipe() && level) {
            bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
            if (rightPressed && mario->getBounds().left + mario->getBounds().width <= tileBounds.left + 5.f) {
                if (!level->getIsUnderground() && level->getLevelId() == 2) {
                    level->warpPipeA_Entry(mario);
                    return;
                }
                if (level->getIsUnderground() && level->getLevelId() == 1) {
                    level->warpToOverworldExit(mario);
                    return;
                }
            }
          }
        } else {
          pos.y += overlap.height; // Hit the underside (ceiling)

          // Question block bump logic
          if (mario && tile->isQuestionBlock()) {
            Tile* mutableTile = const_cast<Tile *>(tile);
            mutableTile->startBump();
            map.hitTile(mutableTile);
            if (level) {
              level->spawnItemFromBlock(tileBounds.left, tileBounds.top, mario);
            }
            mario->notify(GameEvent{GameEventType::COIN_COLLECTED, 200});
          }

          // Brick block logic
          if (mario && tile->isBrick()) {
            Tile* mutableTile = const_cast<Tile *>(tile);
            
            // Check if this brick contains an item (e.g. Star in 1-1 at x=1616)
            bool isItemBrick = false;
            if (level && level->getLevelId() == 1 && std::abs(tileBounds.left - 1616.f) < 2.f) {
              isItemBrick = true;
            }

            if (isItemBrick) {
              mutableTile->startBump();
              map.hitTile(mutableTile);
              if (level) {
                level->spawnItemFromBlock(tileBounds.left, tileBounds.top, mario);
              }
            } else if (mario->hasAbility(PlayerAbility::BreakBricks)) {
              // Super/Fire Mario breaks the brick
              map.breakBrick(mutableTile);
            } else {
              // Small Mario just bumps the brick
              mutableTile->startBump();
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
  for (const Tile *tile : nearbyTiles) {
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
        if (mario && tile->isWarpPipe() && level && pos.x >= 3720.f) {
          bool rightPressed =
              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
          if (rightPressed) {
            level->warpToOverworldExit(mario);
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
  if (mario) {
    for (Tile *tile : nearbyTiles) {
      if (!tile)
        continue;
      sf::FloatRect tileBounds = tile->getBounds();
      if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
        if (tile->isCoinTile()) {
          map.removeTile(tile);
          mario->notify(GameEvent{GameEventType::COIN_COLLECTED, 200});
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
void CollisionManager::resolveMovingPlatform(Mario& mario, MovingPlatform& platform) {
    if (!mario.isActive() || !platform.isActive()) return;
    sf::FloatRect marioBounds = mario.getBounds();
    sf::FloatRect platBounds = platform.getBounds();
    if (marioBounds.left + marioBounds.width > platBounds.left && marioBounds.left < platBounds.left + platBounds.width) {
        if (mario.getVelocity().y >= 0.f && marioBounds.top + marioBounds.height >= platBounds.top && marioBounds.top + marioBounds.height <= platBounds.top + 8.f) {
            mario.setPosition(mario.getPosition().x, platBounds.top - marioBounds.height);
            mario.setVelocity(sf::Vector2f(mario.getVelocity().x, 0.f));
            mario.setGrounded(true);
            mario.move(platform.getVelocity() * (1.f/60.f));
        }
    }
}
