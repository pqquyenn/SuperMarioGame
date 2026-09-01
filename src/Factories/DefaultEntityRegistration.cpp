#include "Factories/DefaultEntityRegistration.h"

#include "Entities/Enemies/DragonLugia.h"
#include "Entities/Enemies/Goomba.h"
#include "Entities/Enemies/GreenParatroopa.h"
#include "Entities/Enemies/Koopa.h"
#include "Entities/Enemies/PiranhaPlant.h"
#include "Entities/Enemies/RedKoopa.h"
#include "Entities/Enemies/RedParatroopa.h"
#include "Entities/Items/Coin.h"
#include "Entities/Items/FireFlower.h"
#include "Entities/Items/Mushroom.h"
#include "Entities/Items/OneUpMushroom.h"
#include "Entities/Items/PlaneItem.h"
#include "Entities/Items/StarItem.h"
#include "Factories/EntityAssetProvider.h"
#include "Factories/EntityFactory.h"

#include <memory>

void registerDefaultEntityTypes(
    EntityFactory& factory,
    EntityAssetProvider& assets) {
    factory.registerType("Goomba", [&assets](const sf::Vector2f& position) {
        auto entity = std::make_unique<Goomba>(position.x, position.y);
        entity->setTexture(assets.getTexture("Goomba"));
        return entity;
    });
    factory.registerType(
        "UndergroundGoomba",
        [&assets](const sf::Vector2f& position) {
            auto entity = std::make_unique<Goomba>(position.x, position.y);
            entity->setTexture(assets.getTexture("Goomba_Underground"));
            return entity;
        });
    factory.registerType("Koopa", [&assets](const sf::Vector2f& position) {
        auto entity = std::make_unique<Koopa>(position.x, position.y);
        entity->setTexture(assets.getTexture("Koopa"));
        return entity;
    });
    factory.registerType("RedKoopa", [&assets](const sf::Vector2f& position) {
        auto entity = std::make_unique<RedKoopa>(position.x, position.y);
        entity->setTexture(assets.getTexture("RedKoopa_Walk1"));
        return entity;
    });
    factory.registerType(
        "GreenParatroopa",
        [&assets](const sf::Vector2f& position) {
            auto entity =
                std::make_unique<GreenParatroopa>(position.x, position.y);
            entity->setTexture(assets.getTexture("GreenParatroopa_Walk1"));
            return entity;
        });
    factory.registerType(
        "RedParatroopa",
        [&assets](const sf::Vector2f& position) {
            auto entity =
                std::make_unique<RedParatroopa>(position.x, position.y);
            entity->setTexture(assets.getTexture("RedParatroopa_Walk1"));
            return entity;
        });
    factory.registerType(
        "PiranhaPlant",
        [&assets](const sf::Vector2f& position) {
            auto entity =
                std::make_unique<PiranhaPlant>(position.x, position.y);
            entity->setTexture(assets.getTexture("PiranhaPlant_1"));
            return entity;
        });
    factory.registerType(
        "DragonLugia",
        [&assets](const sf::Vector2f& position) {
            auto entity =
                std::make_unique<DragonLugia>(position.x, position.y);
            entity->setTexture(assets.getTexture("DragonLugia"));
            return entity;
        });
    factory.registerType("Coin", [&assets](const sf::Vector2f& position) {
        auto entity = std::make_unique<Coin>(position.x, position.y);
        entity->setTexture(assets.getTexture("Coin"));
        return entity;
    });
    factory.registerType("Mushroom", [&assets](const sf::Vector2f& position) {
        auto entity = std::make_unique<Mushroom>(position.x, position.y);
        entity->setTexture(assets.getTexture("Mushroom"));
        return entity;
    });
    factory.registerType(
        "1UpMushroom",
        [&assets](const sf::Vector2f& position) {
            auto entity =
                std::make_unique<OneUpMushroom>(position.x, position.y);
            entity->setTexture(assets.getTexture("1UpMushroom"));
            return entity;
        });
    factory.registerType(
        "FireFlower",
        [&assets](const sf::Vector2f& position) {
            auto entity =
                std::make_unique<FireFlower>(position.x, position.y);
            const sf::Texture& sheet = assets.getTexture("BlockTileSheet");
            entity->setTexture(
                sheet.getSize().x > 0
                    ? sheet
                    : assets.getTexture("FireFlower"));
            return entity;
        });

    auto starCreator = [&assets](const sf::Vector2f& position) {
        auto entity = std::make_unique<StarItem>(position.x, position.y);
        const sf::Texture& sheet = assets.getTexture("BlockTileSheet");
        entity->setTexture(
            sheet.getSize().x > 0 ? sheet : assets.getTexture("Starman"));
        return entity;
    };
    factory.registerType("StarItem", starCreator);
    factory.registerType("Star", starCreator);

    auto planeCreator = [&assets](const sf::Vector2f& position) {
        auto entity = std::make_unique<PlaneItem>(position.x, position.y);
        entity->setTexture(assets.getTexture("PlaneRed"));
        return entity;
    };
    factory.registerType("PlaneItem", planeCreator);
    factory.registerType("Plane", planeCreator);
}
