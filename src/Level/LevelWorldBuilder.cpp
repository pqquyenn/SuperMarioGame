#include "Level/LevelWorldBuilder.h"

#include "Entities/Enemies/Enemy.h"
#include "Entities/Items/Item.h"
#include "Entities/MovingPlatform.h"
#include "Factories/EntityFactory.h"
#include "Level/TileMap.h"

bool LevelWorldBuilder::build(
    const LevelDefinition& definition,
    TileMap& map,
    std::vector<std::unique_ptr<Enemy>>& enemies,
    std::vector<std::unique_ptr<Item>>& items,
    std::vector<std::unique_ptr<MovingPlatform>>& platforms,
    std::vector<std::string>& errors) const {
    errors.clear();
    auto& factory = EntityFactory::getInstance();
    factory.registerDefaultEntities();
    const float tileSize = definition.tileSize;

    for (const auto& specification : definition.entities) {
        auto entity = factory.create(
            specification.type, specification.tilePosition * tileSize);
        if (!entity) {
            errors.push_back("unknown entity type '" + specification.type +
                             "' for " + specification.id);
            continue;
        }
        auto* enemy = dynamic_cast<Enemy*>(entity.get());
        if (!enemy) {
            errors.push_back("object is not an enemy: " + specification.id);
            continue;
        }
        enemy->setDirection(specification.direction);
        if (specification.speed >= 0.f) enemy->setSpeed(specification.speed);
        enemy->setNavigationMap(&map);
        entity.release();
        enemies.emplace_back(enemy);
    }

    for (const auto& specification : definition.items) {
        auto entity = factory.create(
            specification.type, specification.tilePosition * tileSize);
        if (!entity) {
            errors.push_back("unknown item type '" + specification.type +
                             "' for " + specification.id);
            continue;
        }
        auto* item = dynamic_cast<Item*>(entity.get());
        if (!item) {
            errors.push_back("object is not an item: " + specification.id);
            continue;
        }
        entity.release();
        items.emplace_back(item);
    }

    for (const auto& specification : definition.platforms) {
        MovingPlatform::Mode mode = MovingPlatform::Mode::OscillateVertical;
        switch (specification.motion) {
            case PlatformMotion::OscillateHorizontal:
                mode = MovingPlatform::Mode::OscillateHorizontal; break;
            case PlatformMotion::LoopDown:
                mode = MovingPlatform::Mode::LoopDown; break;
            case PlatformMotion::LoopUp:
                mode = MovingPlatform::Mode::LoopUp; break;
            case PlatformMotion::OscillateVertical:
                break;
        }
        platforms.push_back(std::make_unique<MovingPlatform>(
            specification.tilePosition.x * tileSize,
            specification.tilePosition.y * tileSize,
            specification.widthTiles * tileSize,
            specification.minimumTile * tileSize,
            specification.maximumTile * tileSize,
            specification.speed,
            mode));
    }
    return errors.empty();
}
