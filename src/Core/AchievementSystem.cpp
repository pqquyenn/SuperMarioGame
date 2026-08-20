#include "Core/AchievementSystem.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
constexpr std::size_t indexOf(AchievementId id) {
    return static_cast<std::size_t>(id);
}

std::filesystem::path achievementFilePath() {
#ifdef _WIN32
    if (const char* localAppData = std::getenv("LOCALAPPDATA")) {
        return std::filesystem::path(localAppData) / "SFML-Mario" /
               "achievements.dat";
    }
#endif
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / ".sfml-mario" /
               "achievements.dat";
    }
    return std::filesystem::current_path() / "save" / "achievements.dat";
}

const char* keyFor(AchievementId id) {
    switch (id) {
        case AchievementId::ClearWorld11: return "clear_world_1_1";
        case AchievementId::ClearWorld12: return "clear_world_1_2";
        case AchievementId::ClearWorld13: return "clear_world_1_3";
        case AchievementId::SmallIsEnough: return "small_is_enough";
        case AchievementId::Hardcore: return "hardcore";
        case AchievementId::Friendly: return "friendly";
        case AchievementId::IHateMystery: return "i_hate_mystery";
        case AchievementId::Count: break;
    }
    return "unknown";
}
}

AchievementSystem::AchievementSystem() {
    load();
}

AchievementSystem& AchievementSystem::getInstance() {
    static AchievementSystem instance;
    return instance;
}

void AchievementSystem::beginLevel(std::string_view initialForm) {
    attempt = {};
    attempt.active = true;
    attempt.initialForm = initialForm;
}

void AchievementSystem::observeForm(std::string_view currentForm) {
    if (attempt.active && currentForm != attempt.initialForm) {
        attempt.changedForm = true;
    }
}

void AchievementSystem::completeLevel(int levelId, int score) {
    if (!attempt.active) {
        return;
    }

    recordScore(score);
    switch (levelId) {
        case 1: unlock(AchievementId::ClearWorld11); break;
        case 2: unlock(AchievementId::ClearWorld12); break;
        case 3: unlock(AchievementId::ClearWorld13); break;
        default: break;
    }

    bool changed = false;
    if (!attempt.changedForm) {
        changed |= unlock(AchievementId::SmallIsEnough);
    }
    if (!attempt.lostLife) {
        changed |= unlock(AchievementId::Hardcore);
    }
    if (!attempt.defeatedEnemy) {
        changed |= unlock(AchievementId::Friendly);
    }
    if (!attempt.touchedMystery) {
        changed |= unlock(AchievementId::IHateMystery);
    }

    attempt.active = false;
    if (changed) {
        save();
    }
}

void AchievementSystem::recordScore(int score) {
    if (score > highestScore) {
        highestScore = score;
        save();
    }
}

void AchievementSystem::onNotify(const GameEvent& event) {
    if (!attempt.active) {
        return;
    }

    switch (event.type) {
        case GameEventType::PLAYER_DIED:
            attempt.lostLife = true;
            break;
        case GameEventType::ENEMY_DEFEATED:
            attempt.defeatedEnemy = true;
            break;
        case GameEventType::MYSTERY_BLOCK_TOUCHED:
            attempt.touchedMystery = true;
            break;
        default:
            break;
    }
}

bool AchievementSystem::isUnlocked(AchievementId id) const {
    return id != AchievementId::Count && unlocked[indexOf(id)];
}

bool AchievementSystem::unlock(AchievementId id) {
    if (id == AchievementId::Count || unlocked[indexOf(id)]) {
        return false;
    }
    unlocked[indexOf(id)] = true;
    save();
    return true;
}

void AchievementSystem::load() {
    std::ifstream input(achievementFilePath());
    if (!input) {
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);
        try {
            if (key == "highest_score") {
                highestScore = std::max(0, std::stoi(value));
                continue;
            }
            for (std::size_t i = 0; i < indexOf(AchievementId::Count); ++i) {
                const auto id = static_cast<AchievementId>(i);
                if (key == keyFor(id)) {
                    unlocked[i] = value == "1";
                    break;
                }
            }
        } catch (const std::exception&) {
            std::cerr << "[AchievementSystem] Ignoring invalid record: "
                      << line << std::endl;
        }
    }
}

void AchievementSystem::save() const {
    const std::filesystem::path path = achievementFilePath();
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        std::cerr << "[AchievementSystem] Cannot create save directory: "
                  << error.message() << std::endl;
        return;
    }

    std::ofstream output(path, std::ios::trunc);
    if (!output) {
        std::cerr << "[AchievementSystem] Cannot save achievements to "
                  << path << std::endl;
        return;
    }

    output << "highest_score=" << highestScore << '\n';
    for (std::size_t i = 0; i < indexOf(AchievementId::Count); ++i) {
        const auto id = static_cast<AchievementId>(i);
        output << keyFor(id) << '=' << (unlocked[i] ? 1 : 0) << '\n';
    }
}
