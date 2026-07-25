#include "Physics/CollisionManager.h"
#include "Entities/Entity.h" 
#include "Level/TileMap.h"
#include "Level/Tile.h"
#include <algorithm>
#include <cmath>

bool CollisionManager::checkAABB(const sf::FloatRect& a, const sf::FloatRect& b, sf::FloatRect& overlap) {
    return a.intersects(b, overlap);
}

void CollisionManager::resolveEntityCollisions(Entity& a, Entity& b) {
    sf::FloatRect overlap;
    
    // Pure OOP implementation: No `if (a.type == Mario)` logic!
    if (checkAABB(a.getBounds(), b.getBounds(), overlap)) {
        // Double Dispatch: We allow the entities themselves to resolve state logic
        // Examples: Mario loses health, Goomba dies, Coin is collected.
        
        // TODO: Uncomment these lines once your teammates add the virtual `onCollision` method to Entity.h!
        // a.onCollision(b, overlap);
        // b.onCollision(a, overlap);
    }
}

void CollisionManager::resolveTileCollisions(Entity& entity, const TileMap& map) {
    sf::FloatRect bounds = entity.getBounds();
    
    // Retrieve only tiles physically near the entity to optimize physics calculations
    std::vector<Tile*> nearbyTiles = map.getTilesInBounds(bounds);

    for (Tile* tile : nearbyTiles) {
        if (!tile || !tile->isSolid()) continue; // Ignore air and background items

        sf::FloatRect tileBounds = tile->getBounds();
        sf::FloatRect overlap;

        if (checkAABB(bounds, tileBounds, overlap)) {
            // Found a physical collision! 
            // In a complete implementation, you'd calculate velocity vectors 
            // and determine the shortest axis to push the entity out.
            
            sf::Vector2f newPos = entity.getPosition();

            // Very basic separation: Push out along the smallest overlap axis
            if (overlap.width < overlap.height) {
                // Horizontal Collision
                if (bounds.left < tileBounds.left) {
                    newPos.x -= overlap.width; // Left side
                } else {
                    newPos.x += overlap.width; // Right side
                }
            } else {
                // Vertical Collision
                if (bounds.top < tileBounds.top) {
                    newPos.y -= overlap.height; // Top side (Landed on ground)
                } else {
                    newPos.y += overlap.height; // Bottom side (Hit block from below)
                }
            }

            // Apply corrected position back to the entity
            entity.setPosition(newPos);
            
            // Re-update bounds for subsequent tile checks in the same frame
            bounds = entity.getBounds(); 
        }
    }
}
