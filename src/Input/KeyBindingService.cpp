#include "Input/KeyBindingService.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <system_error>
#include <utility>

namespace {
constexpr int ConfigVersion = 1;

std::size_t targetIndex(BindingTarget target) {
    return static_cast<std::size_t>(target);
}
}

KeyBindingService& KeyBindingService::getInstance() {
    static KeyBindingService service;
    return service;
}

KeyBindingService::KeyBindingService(
    std::filesystem::path path)
    : profiles{makeDefaultBindingProfiles()},
      configPath{std::move(path)} {
    load();
}

sf::Keyboard::Key KeyBindingService::getKey(
    BindingTarget target,
    InputAction action) const {
    if (static_cast<std::size_t>(target) >= BindingTargetCount) {
        return sf::Keyboard::Unknown;
    }
    return profiles[targetIndex(target)].get(action);
}

const BindingProfile& KeyBindingService::getProfile(
    BindingTarget target) const {
    if (static_cast<std::size_t>(target) >= BindingTargetCount) {
        return profiles.front();
    }
    return profiles[targetIndex(target)];
}

BindingUpdateResult KeyBindingService::tryUpdate(
    const BindingChange& change) {
    const BindingValidationResult validation =
        validateBindingChange(profiles, change);
    if (!validation.valid()) {
        return {BindingUpdateStatus::Rejected, validation.issues};
    }
    if (getKey(change.target, change.action) == change.key) {
        return {BindingUpdateStatus::Unchanged, {}};
    }

    BindingProfiles candidate = profiles;
    candidate[targetIndex(change.target)].set(
        change.action,
        change.key);
    if (!save(candidate)) {
        return {
            BindingUpdateStatus::SaveFailed,
            {{BindingIssueCode::InvalidKey,
              "Could not save key binding settings"}}};
    }
    profiles = candidate;
    return {BindingUpdateStatus::Applied, {}};
}

BindingUpdateResult KeyBindingService::resetProfile(
    BindingTarget target) {
    if (static_cast<std::size_t>(target) >= BindingTargetCount) {
        return {
            BindingUpdateStatus::Rejected,
            {{BindingIssueCode::InvalidTarget,
              "Invalid binding target"}}};
    }
    BindingProfiles candidate = profiles;
    candidate[targetIndex(target)] =
        makeDefaultBindingProfiles()[targetIndex(target)];
    const BindingValidationResult validation =
        validateBindingProfiles(candidate);
    if (!validation.valid()) {
        return {BindingUpdateStatus::Rejected, validation.issues};
    }
    if (!save(candidate)) {
        return {
            BindingUpdateStatus::SaveFailed,
            {{BindingIssueCode::InvalidKey,
              "Could not save default bindings"}}};
    }
    profiles = candidate;
    return {BindingUpdateStatus::Applied, {}};
}

BindingUpdateResult KeyBindingService::resetAll() {
    BindingProfiles candidate = makeDefaultBindingProfiles();
    if (!save(candidate)) {
        return {
            BindingUpdateStatus::SaveFailed,
            {{BindingIssueCode::InvalidKey,
              "Could not save default bindings"}}};
    }
    profiles = candidate;
    return {BindingUpdateStatus::Applied, {}};
}

bool KeyBindingService::reload() {
    profiles = makeDefaultBindingProfiles();
    return load();
}

bool KeyBindingService::load() {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    std::string marker;
    int version = 0;
    if (!(file >> marker >> version) || marker != "version" ||
        version != ConfigVersion) {
        return false;
    }

    BindingProfiles candidate = makeDefaultBindingProfiles();
    std::array<std::array<bool, InputActionCount>, BindingTargetCount>
        seen{};
    int target = 0;
    int action = 0;
    int key = 0;
    while (file >> target >> action >> key) {
        if (target < 0 ||
            target >= static_cast<int>(BindingTargetCount) ||
            action < 0 ||
            action >= static_cast<int>(InputActionCount) ||
            key < static_cast<int>(sf::Keyboard::A) ||
            key >= static_cast<int>(sf::Keyboard::KeyCount)) {
            profiles = makeDefaultBindingProfiles();
            return false;
        }
        candidate[static_cast<std::size_t>(target)].set(
            static_cast<InputAction>(action),
            static_cast<sf::Keyboard::Key>(key));
        bool& wasSeen = seen[static_cast<std::size_t>(target)]
                            [static_cast<std::size_t>(action)];
        if (wasSeen) {
            profiles = makeDefaultBindingProfiles();
            return false;
        }
        wasSeen = true;
    }

    const bool complete = std::all_of(
        seen.begin(), seen.end(), [](const auto& profile) {
            return std::all_of(profile.begin(), profile.end(),
                               [](bool value) { return value; });
        });
    if (!file.eof() || !complete ||
        !validateBindingProfiles(candidate).valid()) {
        profiles = makeDefaultBindingProfiles();
        return false;
    }
    profiles = candidate;
    return true;
}

bool KeyBindingService::save(
    const BindingProfiles& candidate) const {
    std::error_code error;
    if (configPath.has_parent_path()) {
        std::filesystem::create_directories(
            configPath.parent_path(),
            error);
        if (error) {
            return false;
        }
    }

    const std::filesystem::path temporary =
        configPath.string() + ".tmp";
    std::ofstream file(temporary, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file << "version " << ConfigVersion << '\n';
    for (BindingTarget target : allBindingTargets()) {
        for (InputAction action : allInputActions()) {
            file << static_cast<int>(target) << ' '
                 << static_cast<int>(action) << ' '
                 << static_cast<int>(
                        candidate[targetIndex(target)].get(action))
                 << '\n';
        }
    }
    file.close();
    if (!file) {
        return false;
    }

    std::filesystem::remove(configPath, error);
    error.clear();
    std::filesystem::rename(temporary, configPath, error);
    return !error;
}
