#pragma once

#include "Level/LevelDefinition.h"

#include <memory>
#include <string>
#include <vector>

class Enemy;
class Item;
class MovingPlatform;
class TileMap;

class LevelWorldBuilder {
public:
    bool build(
        const LevelDefinition& definition,
        TileMap& map,
        std::vector<std::unique_ptr<Enemy>>& enemies,
        std::vector<std::unique_ptr<Item>>& items,
        std::vector<std::unique_ptr<MovingPlatform>>& platforms,
        std::vector<std::string>& errors) const;
};

