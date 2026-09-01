#include "EntitySpawner.h"

#include <cassert>
#include <iostream>

using namespace entity_spawner_experiment;

namespace {

void testTimedSpawner() {
    EntitySpawnerConfig config;
    config.id = "timed_goomba";
    config.entityIdentity = "Goomba";
    config.spawnPosition = {80.f, 192.f};
    config.condition = SpawnCondition::AfterTime;
    config.initialDelay = 1.f;
    config.respawnInterval = 2.f;
    config.maxAlive = 2;

    EntitySpawner spawner{config};
    assert(!spawner.update(0.9f));
    const auto first = spawner.update(0.1f);
    assert(first && first->entityIdentity == "Goomba");
    assert(first->position.x == 80.f && first->position.y == 192.f);
    assert(!spawner.update(1.9f));
    assert(spawner.update(0.1f));
    assert(spawner.getAliveCount() == 2);
    assert(!spawner.update(10.f));
}

void testDisappearanceCondition() {
    EntitySpawnerConfig config;
    config.id = "coin_respawn";
    config.entityIdentity = "Coin";
    config.condition = SpawnCondition::AfterEntityDisappeared;
    config.respawnInterval = 3.f;

    EntitySpawner spawner{config};
    const auto coin = spawner.update(0.f);
    assert(coin);
    assert(spawner.notifyEntityEnded(
        coin->instanceId, EntityEndReason::Disappeared));
    assert(!spawner.update(2.9f));
    assert(spawner.update(0.1f));
}

void testDefeatConditionRejectsOtherEndReasons() {
    EntitySpawnerConfig config;
    config.id = "defeated_enemy";
    config.entityIdentity = "Koopa";
    config.condition = SpawnCondition::AfterEnemyDefeated;
    config.respawnInterval = 4.f;

    EntitySpawner spawner{config};
    const auto first = spawner.update(0.f);
    assert(first);
    assert(spawner.notifyEntityEnded(
        first->instanceId, EntityEndReason::Despawned));
    assert(!spawner.update(10.f));

    EntitySpawner defeatedSpawner{config};
    const auto enemy = defeatedSpawner.update(0.f);
    assert(enemy);
    defeatedSpawner.notifyEntityEnded(
        enemy->instanceId, EntityEndReason::Defeated);
    assert(!defeatedSpawner.update(3.9f));
    assert(defeatedSpawner.update(0.1f));
}

void testExternalConditionAndDisable() {
    EntitySpawnerConfig config;
    config.id = "boss_trigger";
    config.entityIdentity = "DragonLugia";
    config.condition = SpawnCondition::External;
    config.spawnInitially = false;
    config.respawnInterval = 0.f;

    EntitySpawner spawner{config};
    assert(!spawner.update(100.f));
    spawner.requestExternalSpawn();
    spawner.setEnabled(false);
    assert(!spawner.update(0.f));
    spawner.setEnabled(true);
    assert(spawner.update(0.f));
}

void testLimitsVariationAndRollback() {
    EntitySpawnerConfig config;
    config.id = "limited_wave";
    config.entityIdentity = "Goomba";
    config.condition = SpawnCondition::AfterTime;
    config.respawnInterval = 5.f;
    config.intervalVariation = 2.f;
    config.maxAlive = 1;
    config.maxTotalSpawns = 2;
    config.randomSeed = 7;

    EntitySpawner spawner{config};
    const auto first = spawner.update(0.f);
    assert(first);
    assert(spawner.getSecondsUntilNextSpawn() >= 3.f);
    assert(spawner.getSecondsUntilNextSpawn() <= 7.f);

    assert(spawner.cancelSpawn(first->instanceId));
    const auto replacement = spawner.update(0.f);
    assert(replacement);
    assert(spawner.getTotalSpawned() == 1);

    spawner.notifyEntityEnded(
        replacement->instanceId, EntityEndReason::Defeated);
    assert(spawner.update(7.f));
    assert(spawner.getTotalSpawned() == 2);
    assert(!spawner.update(100.f));
}

} // namespace

int main() {
    testTimedSpawner();
    testDisappearanceCondition();
    testDefeatConditionRejectsOtherEndReasons();
    testExternalConditionAndDisable();
    testLimitsVariationAndRollback();
    std::cout << "EntitySpawner experiment tests passed\n";
    return 0;
}
