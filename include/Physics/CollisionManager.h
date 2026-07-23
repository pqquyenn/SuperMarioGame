#pragma once

#include "Entities/Character.h"
#include "Level/TileMap.h"
#include <vector>

class CollisionManager {
public:
    static bool checkAABB(const sf::FloatRect& rectA, const sf::FloatRect& rectB);
    static void resolveCharacterMapCollision(Character& character, const TileMap& tileMap);
};
