#pragma once

#include "Entities/Entity.h"
#include <memory>
#include <string>

enum class EntityType {
    GOOMBA,
    KOOPA,
    PIRANHA_PLANT,
    MUSHROOM,
    FIRE_FLOWER,
    COIN
};

class EntityFactory {
public:
    static std::unique_ptr<Entity> createEntity(EntityType type, float x, float y);
    static EntityType stringToType(const std::string& typeStr);
};
