#include "Physics/CollisionManager.h"
#include "Entities/Character.h"
#include "Entities/Enemies/Enemy.h"
#include "Entities/Entity.h"
#include "Entities/Fireball.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Mario.h"
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

  Mario *mario = dynamic_cast<Mario *>(&a);
  Enemy *enemy = dynamic_cast<Enemy *>(&b);
  if (!mario) {
    mario = dynamic_cast<Mario *>(&b);
    enemy = dynamic_cast<Enemy *>(&a);
  }

  if (mario && !mario->isDying() && enemy && enemy->isActive()) {
    sf::FloatRect marioBounds = mario->getBounds();
    sf::FloatRect enemyBounds = enemy->getBounds();

    // Stomp condition: Mario is moving downward and feet are above enemy top
    if (mario->getVelocity().y > 0.f &&
        (marioBounds.top + marioBounds.height - overlap.height <=
         enemyBounds.top + 8.f)) {
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

  sf::FloatRect bounds = entity.getBounds();

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

          if (auto *character = dynamic_cast<Character *>(&entity)) {
            character->setGrounded(true);
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
            if (mario->hasAbility(PlayerAbility::BreakBricks)) {
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

    sf::FloatRect mb = mario.getBounds();
    sf::FloatRect pb = platform.getBounds();

    // Quick broad-phase: no horizontal overlap → skip
    if (mb.left + mb.width <= pb.left || mb.left >= pb.left + pb.width) return;
    // Quick broad-phase: no vertical overlap → skip
    if (mb.top + mb.height <= pb.top || mb.top >= pb.top + pb.height) return;

    // Compute penetration on each axis
    float overlapLeft   = (mb.left + mb.width) - pb.left;   // Mario right past platform left
    float overlapRight  = (pb.left + pb.width) - mb.left;   // Mario left past platform right
    float overlapTop    = (mb.top + mb.height) - pb.top;    // Mario bottom past platform top
    float overlapBottom = (pb.top + pb.height) - mb.top;    // Mario top past platform bottom

    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    sf::Vector2f marioPos = mario.getPosition();
    sf::Vector2f marioVel = mario.getVelocity();

    if (minOverlapY <= minOverlapX) {
        // ── Vertical resolution ──────────────────────────────────────
        if (overlapTop < overlapBottom) {
            // Mario landed on TOP of the platform
            marioPos.y = pb.top - mb.height;
            mario.setPosition(marioPos.x, marioPos.y);
            mario.setVelocity(sf::Vector2f(marioVel.x, 0.f));
            mario.setGrounded(true);
            // Carry Mario along with the platform's movement
            sf::Vector2f delta = platform.getDelta();
            mario.setPosition(marioPos.x + delta.x, marioPos.y + delta.y);
        } else {
            // Mario hit the UNDERSIDE (ceiling collision when jumping up)
            marioPos.y = pb.top + pb.height;
            mario.setPosition(marioPos.x, marioPos.y);
            if (marioVel.y < 0.f) {
                mario.setVelocity(sf::Vector2f(marioVel.x, 0.f));
            }
        }
    } else {
        // ── Horizontal resolution (side push) ────────────────────────
        if (overlapLeft < overlapRight) {
            // Mario is to the LEFT of platform → push left
            marioPos.x = pb.left - mb.width;
        } else {
            // Mario is to the RIGHT of platform → push right
            marioPos.x = pb.left + pb.width;
        }
        mario.setPosition(marioPos.x, marioPos.y);
        mario.setVelocity(sf::Vector2f(0.f, marioVel.y));
    }
}

