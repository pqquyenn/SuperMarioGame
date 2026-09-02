#pragma once

#include "Observer/Observer.h"
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

enum class AchievementId : std::size_t {
    ClearWorld11,
    ClearWorld12,
    ClearWorld13,
    SmallIsEnough,
    Hardcore,
    Friendly,
    Count
};

class AchievementSystem final : public Observer {
private:
    struct LevelAttempt {
        bool active{false};
        bool changedForm{false};
        bool lostLife{false};
        bool defeatedEnemy{false};
        std::string initialForm{"Small"};
    };

    std::array<bool, static_cast<std::size_t>(AchievementId::Count)> unlocked{};
    int highestScore{0};
    LevelAttempt attempt;

    AchievementSystem();
    void load();
    void save() const;
    bool unlock(AchievementId id);

public:
    static AchievementSystem& getInstance();

    AchievementSystem(const AchievementSystem&) = delete;
    AchievementSystem& operator=(const AchievementSystem&) = delete;

    void beginLevel(std::string_view initialForm);
    void observeForm(std::string_view currentForm);
    void completeLevel(int levelId, int score);
    void recordScore(int score);
    void onNotify(const GameEvent& event) override;

    int getHighestScore() const { return highestScore; }
    bool isUnlocked(AchievementId id) const;
};
