#include "Entities/Enemies/Goomba.h"
#include "Entities/Entity.h"
#include "Entities/Items/Coin.h"
#include "Factories/DefaultEntityRegistration.h"
#include "Factories/EntityAssetProvider.h"
#include "Factories/EntityFactory.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

class TestEntity final : public Entity {
public:
    explicit TestEntity(const sf::Vector2f& position)
        : Entity{position.x, position.y} {}

    void update(float) override {}
};

class FakeEntityAssetProvider final : public EntityAssetProvider {
public:
    const sf::Texture& getTexture(
        const std::string& name) const override {
        requests.push_back(name);
        return texture;
    }

    bool wasRequested(const std::string& name) const {
        return std::find(requests.begin(), requests.end(), name) !=
               requests.end();
    }

private:
    mutable std::vector<std::string> requests;
    sf::Texture texture;
};

bool nearlyEqual(float actual, float expected) {
    return std::abs(actual - expected) < 0.001f;
}

void testUnknownType(TestRunner& runner) {
    EntityFactory factory;
    runner.expect(factory.create("MissingType", {0.f, 0.f}) == nullptr,
                  "an unknown type returns nullptr");
}

void testCustomRegistration(TestRunner& runner) {
    EntityFactory factory;
    factory.registerType(
        "TestEntity",
        [](const sf::Vector2f& position) {
            return std::make_unique<TestEntity>(position);
        });

    auto entity = factory.create("TestEntity", {12.f, 34.f});
    runner.expect(entity != nullptr, "a registered type is created");
    runner.expect(dynamic_cast<TestEntity*>(entity.get()) != nullptr,
                  "the registered creator controls the concrete type");
    runner.expect(entity && nearlyEqual(entity->getPosition().x, 12.f) &&
                      nearlyEqual(entity->getPosition().y, 34.f),
                  "the creator receives the requested position");
}

void testDefaultRegistrationWithFakeAssets(TestRunner& runner) {
    FakeEntityAssetProvider assets;
    EntityFactory factory;
    registerDefaultEntityTypes(factory, assets);

    auto goomba = factory.create("Goomba", {16.f, 32.f});
    auto coin = factory.create("Coin", {48.f, 64.f});

    runner.expect(dynamic_cast<Goomba*>(goomba.get()) != nullptr,
                  "default registration creates a Goomba");
    runner.expect(dynamic_cast<Coin*>(coin.get()) != nullptr,
                  "default registration creates a Coin");
    runner.expect(assets.wasRequested("Goomba"),
                  "Goomba texture is requested through the provider");
    runner.expect(assets.wasRequested("Coin"),
                  "Coin texture is requested through the provider");
    runner.expect(factory.create("MissingDefault", {0.f, 0.f}) == nullptr,
                  "unknown default type still returns nullptr");
}

void testRegistryIsolationAndRepeatableDefaults(TestRunner& runner) {
    FakeEntityAssetProvider firstAssets;
    FakeEntityAssetProvider secondAssets;
    EntityFactory first;
    EntityFactory second;

    registerDefaultEntityTypes(first, firstAssets);
    runner.expect(first.create("Goomba", {0.f, 0.f}) != nullptr,
                  "the first local factory receives default registrations");
    runner.expect(second.create("Goomba", {0.f, 0.f}) == nullptr,
                  "a second local factory starts with an empty registry");

    registerDefaultEntityTypes(second, secondAssets);
    runner.expect(second.create("Coin", {0.f, 0.f}) != nullptr,
                  "defaults can be registered on another local factory");

    registerDefaultEntityTypes(first, firstAssets);
    runner.expect(first.create("Coin", {0.f, 0.f}) != nullptr,
                  "default registration is repeatable without static state");
}

void testDebugSpawnTypes(TestRunner& runner) {
    FakeEntityAssetProvider assets;
    EntityFactory factory;
    registerDefaultEntityTypes(factory, assets);

    runner.expect(factory.create("Mushroom", {0.f, 0.f}) != nullptr,
                  "debug Mushroom type is registered");
    runner.expect(factory.create("FireFlower", {0.f, 0.f}) != nullptr,
                  "debug FireFlower type is registered");
    runner.expect(factory.create("StarItem", {0.f, 0.f}) != nullptr,
                  "debug StarItem type is registered");
}

} // namespace

int main() {
    TestRunner runner;
    testUnknownType(runner);
    testCustomRegistration(runner);
    testDefaultRegistrationWithFakeAssets(runner);
    testRegistryIsolationAndRepeatableDefaults(runner);
    testDebugSpawnTypes(runner);

    if (runner.failed == 0) {
        std::cout << "SOLID-07 entity factory tests passed ("
                  << runner.total << " checks)\n";
    }
    return runner.failed == 0 ? 0 : 1;
}
