#include "Physics/CollisionManager.h"

#include "Entities/Character.h"
#include "Entities/Entity.h"
#include "Entities/MovingPlatform.h"
#include "Level/Tile.h"
#include "Level/TileMap.h"
#include "Physics/CollisionGeometry.h"
#include "Physics/TileCollisionHandler.h"

#include <algorithm>
#include <vector>

bool CollisionManager::checkAABB(
    const sf::FloatRect& a,
    const sf::FloatRect& b,
    sf::FloatRect& overlap) {
    return CollisionGeometry::intersects(a, b, overlap);
}

void CollisionManager::resolveEntityCollisions(Entity& a, Entity& b) {
    if (!a.isActive() || !b.isActive()) {
        return;
    }

    const auto contact = CollisionGeometry::findContact(
        a.getBounds(), b.getBounds());
    if (!contact) {
        return;
    }

    // Both entities receive the same contact. Each concrete gameplay object
    // owns its reaction instead of being selected here through RTTI.
    a.onCollision(b, contact->overlap);
    b.onCollision(a, contact->overlap);
}

void CollisionManager::resolveTileCollisions(
    Entity& entity,
    TileMap& map,
    TileCollisionHandler* tileHandler) {
    if (!entity.isActive() || entity.shouldSkipTileCollision()) {
        return;
    }

    entity.beginTileCollision();

    sf::FloatRect bounds = entity.getBounds();
    const std::vector<TileHandle> nearbyTiles = map.getTilesInBounds(bounds);
    sf::Vector2f position = entity.getPosition();
    sf::Vector2f velocity = entity.getVelocity();

    // Pass 1: resolve vertical penetration before horizontal penetration so
    // landing and ceiling contacts receive deterministic normals.
    for (const TileHandle& handle : nearbyTiles) {
        Tile* tile = map.getTile(handle);
        if (!tile || !tile->isSolid()) {
            continue;
        }

        const auto contact = CollisionGeometry::findContact(
            bounds, tile->getBounds());
        if (!contact ||
            (contact->side != CollisionSide::Top &&
             contact->side != CollisionSide::Bottom)) {
            continue;
        }

        if (contact->side == CollisionSide::Top) {
            position.y -= contact->overlap.height;
        } else {
            position.y += contact->overlap.height;
        }

        velocity.y = 0.f;
        entity.setPosition(position);
        entity.setVelocity(velocity);

        if (contact->side == CollisionSide::Top) {
            entity.onLanded();
        } else if (tileHandler) {
            tileHandler->onTileCeilingContact(
                entity, map, *tile, handle, *contact);
        }

        // Hooks may intentionally alter motion, such as a fireball bounce.
        position = entity.getPosition();
        velocity = entity.getVelocity();
        bounds = entity.getBounds();
    }

    // Pass 2: resolve horizontal penetration and delegate the wall reaction.
    for (const TileHandle& handle : nearbyTiles) {
        Tile* tile = map.getTile(handle);
        if (!tile || !tile->isSolid()) {
            continue;
        }

        const auto contact = CollisionGeometry::findContact(
            bounds, tile->getBounds());
        if (!contact ||
            (contact->side != CollisionSide::Left &&
             contact->side != CollisionSide::Right)) {
            continue;
        }

        if (contact->side == CollisionSide::Left) {
            position.x -= contact->overlap.width;
        } else {
            position.x += contact->overlap.width;
        }

        velocity.x = 0.f;
        entity.setPosition(position);
        entity.setVelocity(velocity);
        entity.onWallCollision();

        position = entity.getPosition();
        velocity = entity.getVelocity();
        bounds = entity.getBounds();
        if (!entity.isActive() || bounds.width <= 0.f || bounds.height <= 0.f) {
            return;
        }
    }

    // Pass 3: report non-solid and residual overlaps to the gameplay adapter.
    if (tileHandler) {
        for (const TileHandle& handle : nearbyTiles) {
            Tile* tile = map.getTile(handle);
            if (!tile) {
                continue;
            }

            const auto contact = CollisionGeometry::findContact(
                bounds, tile->getBounds());
            if (!contact) {
                continue;
            }

            tileHandler->onTileOverlap(
                entity, map, *tile, handle, *contact);
            position = entity.getPosition();
            velocity = entity.getVelocity();
            bounds = entity.getBounds();
        }
    }

    entity.setVelocity(velocity);
}

bool CollisionManager::tryStandUp(
    Character& character,
    const TileMap& map) {
    if (!character.isCrouching()) {
        return true;
    }

    // Check only the height added above the crouched body. Including the full
    // standing body would misclassify small floor penetration as headroom.
    const sf::FloatRect headroom = character.getStandingHeadroomBounds();
    bool headroomBlocked = false;
    for (const TileHandle& handle : map.getTilesInBounds(headroom)) {
        const Tile* tile = map.getTile(handle);
        if (!tile || !tile->isSolid()) {
            continue;
        }

        sf::FloatRect overlap;
        if (checkAABB(headroom, tile->getBounds(), overlap)) {
            headroomBlocked = true;
            break;
        }
    }

    character.resolveCrouchState(headroomBlocked);
    return !character.isCrouching();
}

void CollisionManager::resolveMovingPlatform(
    Character& character,
    MovingPlatform& platform) {
    if (!character.isActive() || !platform.isActive()) {
        return;
    }

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
            character.setPosition(
                position.x + platformDelta.x,
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
