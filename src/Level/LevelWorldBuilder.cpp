#include "Level/LevelWorldBuilder.h"

#include "Entities/Enemies/Enemy.h"
#include "Entities/Enemies/RedKoopa.h"
#include "Entities/Items/Item.h"
#include "Entities/MovingPlatform.h"
#include "Factories/EntityFactory.h"
#include "Level/TileMap.h"

namespace {

sf::Vector2f toPixels(const sf::Vector2f& tile, float tileSize) {
    return {tile.x * tileSize, tile.y * tileSize};
}

MovingPlatform::Mode toPlatformMode(PlatformMotion motion) {
    switch (motion) {
        case PlatformMotion::OscillateHorizontal:
            return MovingPlatform::Mode::OscillateHorizontal;
        case PlatformMotion::LoopDown:
            return MovingPlatform::Mode::LoopDown;
        case PlatformMotion::LoopUp:
            return MovingPlatform::Mode::LoopUp;
        case PlatformMotion::OscillateVertical:
        default:
            return MovingPlatform::Mode::OscillateVertical;
    }
}

} // namespace

LevelWorldBuilder::LevelWorldBuilder(EntityFactory& factory)
    : factory{factory} {}

bool LevelWorldBuilder::build(
    const LevelDefinition& definition,
    TileMap& map,
    std::vector<std::unique_ptr<Enemy>>& enemies,
    std::vector<std::unique_ptr<Item>>& items,
    std::vector<std::unique_ptr<MovingPlatform>>& platforms,
    std::vector<std::string>& errors) const {
    errors.clear();

    for (const auto& specification : definition.entities) {
        const sf::Vector2f position =
            toPixels(specification.tilePosition, definition.tileSize);
        auto entity = factory.create(specification.resolvedType, position);
        if (!entity) {
            errors.push_back(
                "unknown enemy factory type '" + specification.resolvedType +
                "' for " + specification.id + " (symbol " +
                specification.symbol + ")");
            continue;
        }

        Enemy* enemy = dynamic_cast<Enemy*>(entity.get());
        if (!enemy) {
            errors.push_back(
                "factory type is not an enemy for " + specification.id +
                ": " + specification.resolvedType);
            continue;
        }

        enemy->setDirection(specification.direction);
        if (specification.speed >= 0.f) {
            enemy->setSpeed(specification.speed);
        }
        if (auto* redKoopa = dynamic_cast<RedKoopa*>(enemy)) {
            redKoopa->setTileMap(&map);
        }

        entity.release();
        enemies.emplace_back(enemy);
    }

    for (const auto& specification : definition.items) {
        const sf::Vector2f position =
            toPixels(specification.tilePosition, definition.tileSize);
        auto entity = factory.create(specification.resolvedType, position);
        if (!entity) {
            errors.push_back(
                "unknown item factory type '" + specification.resolvedType +
                "' for " + specification.id + " (symbol " +
                specification.symbol + ")");
            continue;
        }

        Item* item = dynamic_cast<Item*>(entity.get());
        if (!item) {
            errors.push_back(
                "factory type is not an item for " + specification.id +
                ": " + specification.resolvedType);
            continue;
        }

        entity.release();
        items.emplace_back(item);
    }

    for (const auto& specification : definition.platforms) {
        const float tileSize = definition.tileSize;
        platforms.push_back(std::make_unique<MovingPlatform>(
            specification.tilePosition.x * tileSize,
            specification.tilePosition.y * tileSize,
            specification.widthTiles * tileSize,
            specification.minimumTile * tileSize,
            specification.maximumTile * tileSize,
            specification.speed,
            toPlatformMode(specification.motion)));
    }

    return errors.empty();
}

