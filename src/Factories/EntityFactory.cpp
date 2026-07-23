#include "Factories/EntityFactory.h"
#include "Entities/Enemies/Goomba.h"
#include "Entities/Enemies/Koopa.h"
#include "Entities/Enemies/PiranhaPlant.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Coin.h"

std::unique_ptr<Entity> EntityFactory::createEntity(EntityType type, float x, float y) {
    switch (type) {
        case EntityType::GOOMBA: return std::make_unique<Goomba>(x, y);
        case EntityType::KOOPA: return std::make_unique<Koopa>(x, y);
        case EntityType::PIRANHA_PLANT: return std::make_unique<PiranhaPlant>(x, y);
        case EntityType::MUSHROOM: return std::make_unique<Mushroom>(x, y);
        case EntityType::FIRE_FLOWER: return std::make_unique<FireFlower>(x, y);
        case EntityType::COIN: return std::make_unique<Coin>(x, y);
        default: return nullptr;
    }
}

EntityType EntityFactory::stringToType(const std::string& typeStr) {
    if (typeStr == "goomba") return EntityType::GOOMBA;
    if (typeStr == "koopa") return EntityType::KOOPA;
    if (typeStr == "piranha") return EntityType::PIRANHA_PLANT;
    if (typeStr == "mushroom") return EntityType::MUSHROOM;
    if (typeStr == "fireflower") return EntityType::FIRE_FLOWER;
    return EntityType::COIN;
}
