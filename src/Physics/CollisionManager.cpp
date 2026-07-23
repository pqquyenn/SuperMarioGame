#include "Physics/CollisionManager.h"

bool CollisionManager::checkAABB(const sf::FloatRect& rectA, const sf::FloatRect& rectB) {
    return rectA.intersects(rectB);
}

void CollisionManager::resolveCharacterMapCollision(Character& character, const TileMap& tileMap) {
    // Collision resolution logic against solid tiles
}
