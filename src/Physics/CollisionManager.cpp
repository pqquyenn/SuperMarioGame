#include "Physics/CollisionManager.h"
#include "Entities/Entity.h"
#include "Entities/Character.h"
#include "Entities/Mario.h"
#include "Entities/Enemies/Enemy.h"
#include "Level/Level.h"
#include "Level/TileMap.h"
#include "Level/Tile.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/Mushroom.h"
#include "Level/Tile.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>

bool CollisionManager::checkAABB(const sf::FloatRect& a, const sf::FloatRect& b, sf::FloatRect& overlap) {
    return a.intersects(b, overlap);
}

void CollisionManager::resolveEntityCollisions(Entity& a, Entity& b) {
    sf::FloatRect overlap;
    if (!checkAABB(a.getBounds(), b.getBounds(), overlap)) {
        return;
    }

    Mario* mario = dynamic_cast<Mario*>(&a);
    Enemy* enemy = dynamic_cast<Enemy*>(&b);
    if (!mario) {
        mario = dynamic_cast<Mario*>(&b);
        enemy = dynamic_cast<Enemy*>(&a);
    }

    if (mario && enemy && enemy->isActive()) {
        sf::FloatRect marioBounds = mario->getBounds();
        sf::FloatRect enemyBounds = enemy->getBounds();

        // Stomp condition: Mario is moving downward and feet are above enemy top
        if (mario->getVelocity().y > 0.f && (marioBounds.top + marioBounds.height - overlap.height <= enemyBounds.top + 8.f)) {
            enemy->onStomped();
            enemy->setActive(false);
            mario->setVelocity(sf::Vector2f(mario->getVelocity().x, -250.f));
            return;
        } else {
            mario->takeDamage();
            if (!mario->isActive()) {
                mario->respawn(40.f, 160.f);
            }
            return;
        }
    }

    // Pure OOP: no type-checking. Entities resolve their own state via double dispatch.
    a.onCollision(b, overlap);
    b.onCollision(a, overlap);
}

void CollisionManager::resolveTileCollisions(Entity& entity, TileMap& map, Level* level) {
    if (Coin* coin = dynamic_cast<Coin*>(&entity)) {
        if (coin->isPopping()) return;
    }
    if (Mushroom* shroom = dynamic_cast<Mushroom*>(&entity)) {
        if (shroom->isEmerging()) return;
    }

    sf::FloatRect bounds = entity.getBounds();

    // Retrieve only tiles physically near the entity to minimize comparisons
    const std::vector<Tile*> nearbyTiles = map.getTilesInBounds(bounds);

    sf::Vector2f pos = entity.getPosition();
    sf::Vector2f vel = entity.getVelocity();
    Mario* mario = dynamic_cast<Mario*>(&entity);
    Enemy* enemy = dynamic_cast<Enemy*>(&entity);

    // ──────────────────────────────────────────────────────────────
    // PASS 1 — Y-axis resolution (Ground & Ceiling landing)
    // ──────────────────────────────────────────────────────────────
    for (const Tile* tile : nearbyTiles) {
        if (!tile || !tile->isSolid()) {
            continue;
        }

        const sf::FloatRect tileBounds = tile->getBounds();

        if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
            if (overlap.height <= overlap.width) {
                if (bounds.top < tileBounds.top) {
                    pos.y -= overlap.height;  // Landed on top of the tile (ground)

                    if (auto* character = dynamic_cast<Character*>(&entity)) {
                        character->setGrounded(true);
                    }

                    // Check 4th pipe warp (Warp down to Underground when pressing Down/S on pipe)
                    if (mario && tile->isWarpPipe() && level) {
                        bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ||
                                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
                        if (downPressed && pos.x >= 880.f && pos.x <= 960.f && !level->getIsUnderground()) {
                            level->warpToUnderground(mario);
                            return;
                        }
                    }
                } else {
                    pos.y += overlap.height;  // Hit the underside (ceiling)

                    // Question block bump logic
                    if (mario && tile->isQuestionBlock()) {
                        map.hitTile(const_cast<Tile*>(tile));
                        if (level) {
                            level->spawnItemFromBlock(tileBounds.left, tileBounds.top);
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
    for (const Tile* tile : nearbyTiles) {
        if (!tile || !tile->isSolid()) {
            continue;
        }

        const sf::FloatRect tileBounds = tile->getBounds();

        if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
            if (overlap.width < overlap.height) {
                if (bounds.left < tileBounds.left) {
                    pos.x -= overlap.width;   // Entity is to the left of the tile
                } else {
                    pos.x += overlap.width;   // Entity is to the right of the tile
                }
                vel.x = 0.f;  // Kill horizontal momentum on wall contact

                if (enemy) {
                    enemy->reverseDirection();
                } else if (Mushroom* shroom = dynamic_cast<Mushroom*>(&entity)) {
                    shroom->reverseDirection();
                }

                // Check horizontal pipe warp (Exit from underground)
                if (mario && tile->isWarpPipe() && level && pos.x >= 3720.f) {
                    bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
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
        for (Tile* tile : nearbyTiles) {
            if (!tile) continue;
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
}
