#include "Entities/Character.h"
#include "Entities/Entity.h"
#include "Entities/MovingPlatform.h"
#include "Level/Tile.h"
#include "Level/TileMap.h"
#include "Physics/CollisionManager.h"
#include "Physics/TileCollisionHandler.h"
#include "PlayerStates/SuperState.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <string>

namespace {

struct TestRunner {
    int total{0};
    int failed{0};

    void expect(bool condition, const std::string& message) {
        ++total;
        if (!condition) {
            ++failed;
            std::cerr << "FAILED: " << message << '\n';
        }
    }
};

bool nearlyEqual(float actual, float expected) {
    return std::abs(actual - expected) < 0.001f;
}

class TestEntity final : public Entity {
public:
    TestEntity(float x, float y, sf::Vector2f entitySize = {16.f, 16.f})
        : Entity{x, y}, size{entitySize} {}

    void update(float) override {}

    sf::FloatRect getBounds() const override {
        return {position.x, position.y, size.x, size.y};
    }

    void onCollision(Entity&, const sf::FloatRect& overlap) override {
        ++entityCollisionCount;
        lastEntityOverlap = overlap;
    }

    void beginTileCollision() override {
        ++beginTileCollisionCount;
    }

    void onLanded() override {
        ++landedCount;
        if (landingVelocityEnabled) {
            velocity.y = landingVelocity;
        }
    }

    void onWallCollision() override {
        ++wallCount;
    }

    bool shouldSkipTileCollision() const override {
        return skipTileCollision;
    }

    sf::Vector2f size;
    sf::FloatRect lastEntityOverlap{};
    int entityCollisionCount{0};
    int beginTileCollisionCount{0};
    int landedCount{0};
    int wallCount{0};
    bool skipTileCollision{false};
    bool landingVelocityEnabled{false};
    float landingVelocity{-120.f};
};

class TestCharacter final : public Character {
public:
    TestCharacter(float x, float y)
        : Character{x, y} {}
};

class RecordingTileHandler final : public TileCollisionHandler {
public:
    void onTileCeilingContact(
        Entity&,
        TileMap&,
        Tile&,
        const TileHandle&,
        const CollisionContact& contact) override {
        ++ceilingCount;
        lastCeilingSide = contact.side;
    }

    void onTileOverlap(
        Entity&,
        TileMap&,
        Tile& tile,
        const TileHandle&,
        const CollisionContact&) override {
        ++overlapCount;
        coinOverlap = coinOverlap || tile.isCoinTile();
    }

    int ceilingCount{0};
    int overlapCount{0};
    bool coinOverlap{false};
    CollisionSide lastCeilingSide{CollisionSide::None};
};

void testEntityDoubleDispatch(TestRunner& runner) {
    TestEntity first{0.f, 0.f};
    TestEntity second{8.f, 0.f};

    CollisionManager::resolveEntityCollisions(first, second);

    runner.expect(first.entityCollisionCount == 1,
                  "first entity receives collision callback");
    runner.expect(second.entityCollisionCount == 1,
                  "second entity receives collision callback");
    runner.expect(nearlyEqual(first.lastEntityOverlap.width, 8.f),
                  "double dispatch receives the calculated overlap");
}

void testInactiveEntityPair(TestRunner& runner) {
    TestEntity inactive{0.f, 0.f};
    TestEntity active{8.f, 0.f};
    inactive.setActive(false);

    CollisionManager::resolveEntityCollisions(inactive, active);

    runner.expect(inactive.entityCollisionCount == 0,
                  "inactive entity does not receive collision callback");
    runner.expect(active.entityCollisionCount == 0,
                  "active entity is not notified about an inactive pair");
}

void testLanding(TileMap& map, TestRunner& runner) {
    TestEntity entity{32.f, 24.f};
    entity.setVelocity(25.f, 100.f);

    CollisionManager::resolveTileCollisions(entity, map);

    runner.expect(nearlyEqual(entity.getPosition().y, 16.f),
                  "landing separates entity to tile top");
    runner.expect(nearlyEqual(entity.getVelocity().y, 0.f),
                  "landing clears downward velocity");
    runner.expect(entity.beginTileCollisionCount == 1,
                  "tile pass begins through polymorphic hook");
    runner.expect(entity.landedCount == 1,
                  "landing is delegated to entity hook");
}

void testLandingHookVelocity(TileMap& map, TestRunner& runner) {
    TestEntity entity{32.f, 24.f};
    entity.setVelocity(0.f, 100.f);
    entity.landingVelocityEnabled = true;

    CollisionManager::resolveTileCollisions(entity, map);

    runner.expect(nearlyEqual(entity.getVelocity().y, entity.landingVelocity),
                  "landing hook may replace constrained velocity");
}

void testCeilingAndHandler(TileMap& map, TestRunner& runner) {
    TestEntity entity{32.f, 40.f};
    entity.setVelocity(0.f, -100.f);
    RecordingTileHandler handler;

    CollisionManager::resolveTileCollisions(entity, map, &handler);

    runner.expect(nearlyEqual(entity.getPosition().y, 48.f),
                  "ceiling contact separates entity below tile");
    runner.expect(nearlyEqual(entity.getVelocity().y, 0.f),
                  "ceiling contact clears upward velocity");
    runner.expect(handler.ceilingCount == 1,
                  "ceiling gameplay is delegated to tile handler");
    runner.expect(handler.lastCeilingSide == CollisionSide::Bottom,
                  "ceiling handler receives bottom contact side");
}

void testWallHooks(TileMap& map, TestRunner& runner) {
    TestEntity leftEntity{24.f, 32.f};
    leftEntity.setVelocity(100.f, 20.f);
    CollisionManager::resolveTileCollisions(leftEntity, map);

    runner.expect(nearlyEqual(leftEntity.getPosition().x, 16.f),
                  "left wall separates entity to tile left");
    runner.expect(nearlyEqual(leftEntity.getVelocity().x, 0.f),
                  "wall contact clears horizontal velocity");
    runner.expect(leftEntity.wallCount == 1,
                  "left wall reaction is delegated to entity hook");

    TestEntity rightEntity{40.f, 32.f};
    rightEntity.setVelocity(-100.f, 20.f);
    CollisionManager::resolveTileCollisions(rightEntity, map);

    runner.expect(nearlyEqual(rightEntity.getPosition().x, 48.f),
                  "right wall separates entity to tile right");
    runner.expect(rightEntity.wallCount == 1,
                  "right wall reaction is delegated to entity hook");
}

void testNonSolidOverlap(TileMap& map, TestRunner& runner) {
    TestEntity entity{32.f, 48.f};
    RecordingTileHandler handler;

    CollisionManager::resolveTileCollisions(entity, map, &handler);

    runner.expect(handler.overlapCount == 1,
                  "non-solid overlap is reported to tile handler");
    runner.expect(handler.coinOverlap,
                  "tile handler can identify coin gameplay tile");
    runner.expect(nearlyEqual(entity.getPosition().y, 48.f),
                  "non-solid tile does not separate entity");
}

void testSkippedCollision(TileMap& map, TestRunner& runner) {
    TestEntity entity{32.f, 24.f};
    entity.skipTileCollision = true;

    CollisionManager::resolveTileCollisions(entity, map);

    runner.expect(nearlyEqual(entity.getPosition().y, 24.f),
                  "skip hook bypasses tile separation");
    runner.expect(entity.beginTileCollisionCount == 0,
                  "skipped collision does not begin a tile pass");
}

void makePoweredAndCrouched(TestCharacter& character) {
    character.receivePowerUp(std::make_unique<SuperState>());
    character.setPosition(character.getPosition().x, 32.f);
    character.setGrounded(true);
    character.setCrouchRequested(true);
    character.setCrouchRequested(false);
}

void testCrawlHeadroom(TileMap& map, TestRunner& runner) {
    TestCharacter blocked{32.f, 32.f};
    makePoweredAndCrouched(blocked);

    runner.expect(blocked.isCrouching(),
                  "powered character enters crouch before headroom test");
    runner.expect(!CollisionManager::tryStandUp(blocked, map),
                  "solid tile blocks standing headroom");
    runner.expect(blocked.isCrouching(),
                  "blocked character remains crouched");

    TestCharacter clear{0.f, 32.f};
    makePoweredAndCrouched(clear);

    runner.expect(CollisionManager::tryStandUp(clear, map),
                  "clear headroom permits standing");
    runner.expect(!clear.isCrouching(),
                  "character leaves crouch when headroom is clear");
    runner.expect(nearlyEqual(clear.getBounds().height, 32.f),
                  "standing restores powered collision height");
}

void testMovingPlatformLanding(TestRunner& runner) {
    TestCharacter character{40.f, 56.f};
    character.setVelocity(20.f, 100.f);
    MovingPlatform platform{
        32.f,
        64.f,
        48.f,
        64.f,
        96.f,
        50.f,
        MovingPlatform::Mode::OscillateVertical};

    CollisionManager::resolveMovingPlatform(character, platform);

    runner.expect(nearlyEqual(character.getPosition().y, 48.f),
                  "moving platform places character on its top");
    runner.expect(nearlyEqual(character.getVelocity().y, 0.f),
                  "moving platform clears downward velocity");
    runner.expect(character.isGrounded(),
                  "moving platform contributes grounded contact");
}

} // namespace

int main() {
    TestRunner runner;
    TileMap map;
    runner.expect(map.readFromFile("tests/fixtures/collision-map.txt"),
                  "collision fixture map loads");

    testEntityDoubleDispatch(runner);
    testInactiveEntityPair(runner);
    testLanding(map, runner);
    testLandingHookVelocity(map, runner);
    testCeilingAndHandler(map, runner);
    testWallHooks(map, runner);
    testNonSolidOverlap(map, runner);
    testSkippedCollision(map, runner);
    testCrawlHeadroom(map, runner);
    testMovingPlatformLanding(runner);

    if (runner.failed == 0) {
        std::cout << "SOLID-04 tile collision tests passed ("
                  << runner.total << " checks)\n";
    }
    return runner.failed == 0 ? 0 : 1;
}
