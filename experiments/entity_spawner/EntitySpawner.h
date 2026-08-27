#pragma once

#include <cstddef>
#include <optional>
#include <random>
#include <string>
#include <unordered_set>

namespace entity_spawner_experiment {

struct SpawnPosition {
    float x{0.f};
    float y{0.f};
};

enum class SpawnCondition {
    AfterTime,
    AfterEntityDisappeared,
    AfterEnemyDefeated,
    External
};

enum class EntityEndReason {
    Disappeared,
    Defeated,
    Despawned
};

struct EntitySpawnerConfig {
    std::string id;
    std::string entityIdentity;
    SpawnPosition spawnPosition;
    SpawnCondition condition{SpawnCondition::AfterTime};
    float initialDelay{0.f};
    float respawnInterval{5.f};
    float intervalVariation{0.f};
    std::size_t maxAlive{1};
    std::size_t maxTotalSpawns{0}; // Zero means unlimited.
    bool spawnInitially{true};
    bool enabled{true};
    unsigned int randomSeed{0};
};

struct SpawnRequest {
    std::string instanceId;
    std::string spawnerId;
    std::string entityIdentity;
    SpawnPosition position;
};

class EntitySpawner {
public:
    explicit EntitySpawner(EntitySpawnerConfig config);

    std::optional<SpawnRequest> update(float deltaSeconds);
    bool notifyEntityEnded(
        const std::string& instanceId,
        EntityEndReason reason);
    void requestExternalSpawn();
    bool cancelSpawn(const std::string& instanceId);

    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    std::size_t getAliveCount() const { return activeInstances_.size(); }
    std::size_t getTotalSpawned() const { return totalSpawned_; }
    float getSecondsUntilNextSpawn() const { return countdown_; }
    const EntitySpawnerConfig& getConfig() const { return config_; }

private:
    EntitySpawnerConfig config_;
    std::mt19937 randomEngine_;
    std::unordered_set<std::string> activeInstances_;
    std::size_t totalSpawned_{0};
    std::size_t nextInstanceSerial_{1};
    float countdown_{0.f};
    bool spawnArmed_{false};
    bool enabled_{true};

    bool canSpawn() const;
    bool endReasonMeetsCondition(EntityEndReason reason) const;
    float nextRespawnInterval();
    void arm(float delay);
};

} // namespace entity_spawner_experiment
