#include "Physics/CollisionManager.h"
#include "Entities/Entity.h"
#include "Entities/Character.h"
#include "Level/TileMap.h"
#include "Level/Tile.h"
#include <algorithm>
#include <cmath>
#include <vector>

bool CollisionManager::checkAABB(const sf::FloatRect& a, const sf::FloatRect& b, sf::FloatRect& overlap) {
    return a.intersects(b, overlap);
}

void CollisionManager::resolveEntityCollisions(Entity& a, Entity& b) {
    // Pure OOP: no type-checking. Entities resolve their own state via double dispatch.
    if (sf::FloatRect overlap; checkAABB(a.getBounds(), b.getBounds(), overlap)) {
        a.onCollision(b, overlap);
        b.onCollision(a, overlap);
    }
}

void CollisionManager::resolveTileCollisions(Entity& entity, const TileMap& map) {
    sf::FloatRect bounds = entity.getBounds();

    // Retrieve only tiles physically near the entity to minimize comparisons
    const std::vector<Tile*> nearbyTiles = map.getTilesInBounds(bounds);

    sf::Vector2f pos = entity.getPosition();
    sf::Vector2f vel = entity.getVelocity();

    // ──────────────────────────────────────────────────────────────
    // PASS 1 — X-axis resolution
    // We resolve horizontal collisions first so that the corrected
    // X position is available when we check vertical overlaps.
    // This prevents the classic "corner-sticking" glitch where an
    // entity moving diagonally into a corner gets pushed along the
    // wrong axis.
    // ──────────────────────────────────────────────────────────────
    for (const Tile* tile : nearbyTiles) {
        if (!tile || !tile->isSolid()) {
            continue;
        }

        const sf::FloatRect tileBounds = tile->getBounds();

        if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
            // Only handle horizontal penetration in this pass
            if (overlap.width < overlap.height) {
                if (bounds.left < tileBounds.left) {
                    pos.x -= overlap.width;   // Entity is to the left of the tile
                } else {
                    pos.x += overlap.width;   // Entity is to the right of the tile
                }
                vel.x = 0.f;  // Kill horizontal momentum on wall contact

                // Sync immediately so Pass 2 sees the corrected X position
                entity.setPosition(pos);
                bounds = entity.getBounds();
            }
        }
    }

    // ──────────────────────────────────────────────────────────────
    // PASS 2 — Y-axis resolution
    // With the X position already corrected, any remaining overlap
    // is a genuine vertical collision (floor or ceiling).
    // ──────────────────────────────────────────────────────────────
    for (const Tile* tile : nearbyTiles) {
        if (!tile || !tile->isSolid()) {
            continue;
        }

        const sf::FloatRect tileBounds = tile->getBounds();

        if (sf::FloatRect overlap; checkAABB(bounds, tileBounds, overlap)) {
            // Only handle vertical penetration in this pass
            if (overlap.height <= overlap.width) {
                if (bounds.top < tileBounds.top) {
                    pos.y -= overlap.height;  // Landed on top of the tile (ground)

                    // ★ Task 4: Set grounded for Characters (Mario/Luigi) so jump() works.
                    // Enemies and Items skip this branch silently.
                    if (auto* character = dynamic_cast<Character*>(&entity)) {
                        character->setGrounded(true);
                    }
                } else {
                    pos.y += overlap.height;  // Hit the underside (ceiling)
                }
                vel.y = 0.f;  // Kill vertical momentum (stops gravity accumulation)

                entity.setPosition(pos);
                bounds = entity.getBounds();
            }
        }
    }

    // Write the constrained velocity back to the entity
    entity.setVelocity(vel);
}
