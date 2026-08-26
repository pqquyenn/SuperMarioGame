#include "EntitySpawner.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace entity_spawner_experiment {

namespace {
constexpr float TimerEpsilon = 0.0001f;
}

EntitySpawner::EntitySpawner(EntitySpawnerConfig config)
    : config_{std::move(config)},
      randomEngine_{config_.randomSeed},
      countdown_{config_.initialDelay},
      spawnArmed_{config_.spawnInitially},
      enabled_{config_.enabled} {
    if (config_.id.empty()) {
        throw std::invalid_argument("spawner id cannot be empty");
    }
    if (config_.entityIdentity.empty()) {
        throw std::invalid_argument("entity identity cannot be empty");
    }
    if (config_.initialDelay < 0.f || config_.respawnInterval < 0.f ||
        config_.intervalVariation < 0.f) {
        throw std::invalid_argument("spawner durations cannot be negative");
    }
    if (config_.maxAlive == 0) {
        throw std::invalid_argument("maxAlive must be at least one");
    }
}

std::optional<SpawnRequest> EntitySpawner::update(float deltaSeconds) {
    if (deltaSeconds < 0.f) {
        throw std::invalid_argument("deltaSeconds cannot be negative");
    }
    if (!enabled_ || !spawnArmed_) {
        return std::nullopt;
    }

    countdown_ = std::max(0.f, countdown_ - deltaSeconds);
    if (countdown_ > TimerEpsilon || !canSpawn()) {
        return std::nullopt;
    }

    const std::string instanceId =
        config_.id + "#" + std::to_string(nextInstanceSerial_++);
    activeInstances_.insert(instanceId);
    ++totalSpawned_;

    if (config_.condition == SpawnCondition::AfterTime) {
        arm(nextRespawnInterval());
    } else {
        spawnArmed_ = false;
    }

    return SpawnRequest{instanceId, config_.id, config_.entityIdentity,
                        config_.spawnPosition};
}

bool EntitySpawner::notifyEntityEnded(
    const std::string& instanceId,
    EntityEndReason reason) {
    if (activeInstances_.erase(instanceId) == 0) {
        return false;
    }

    if (endReasonMeetsCondition(reason)) {
        arm(nextRespawnInterval());
    }
    return true;
}

void EntitySpawner::requestExternalSpawn() {
    if (config_.condition == SpawnCondition::External) {
        arm(nextRespawnInterval());
    }
}

bool EntitySpawner::cancelSpawn(const std::string& instanceId) {
    if (activeInstances_.erase(instanceId) == 0) {
        return false;
    }
    if (totalSpawned_ > 0) {
        --totalSpawned_;
    }
    arm(0.f);
    return true;
}

void EntitySpawner::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool EntitySpawner::canSpawn() const {
    const bool belowAliveLimit = activeInstances_.size() < config_.maxAlive;
    const bool belowTotalLimit = config_.maxTotalSpawns == 0 ||
        totalSpawned_ < config_.maxTotalSpawns;
    return belowAliveLimit && belowTotalLimit;
}

bool EntitySpawner::endReasonMeetsCondition(EntityEndReason reason) const {
    if (config_.condition == SpawnCondition::AfterEntityDisappeared) {
        return reason == EntityEndReason::Disappeared;
    }
    if (config_.condition == SpawnCondition::AfterEnemyDefeated) {
        return reason == EntityEndReason::Defeated;
    }
    return false;
}

float EntitySpawner::nextRespawnInterval() {
    if (config_.intervalVariation == 0.f) {
        return config_.respawnInterval;
    }
    const float minimum = std::max(
        0.f, config_.respawnInterval - config_.intervalVariation);
    const float maximum = config_.respawnInterval + config_.intervalVariation;
    return std::uniform_real_distribution<float>{minimum, maximum}(
        randomEngine_);
}

void EntitySpawner::arm(float delay) {
    spawnArmed_ = true;
    countdown_ = std::max(0.f, delay);
}

} // namespace entity_spawner_experiment
