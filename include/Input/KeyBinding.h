#pragma once

#include <SFML/Window/Keyboard.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <vector>

enum class InputAction {
    MoveLeft,
    MoveRight,
    Jump,
    Crouch,
    Action,
    Run,
    Interact,
    Count
};

enum class BindingTarget {
    Solo,
    DuoPlayerOne,
    DuoPlayerTwo,
    PvPPlayerOne,
    PvPPlayerTwo,
    Count
};

constexpr std::size_t InputActionCount =
    static_cast<std::size_t>(InputAction::Count);
constexpr std::size_t BindingTargetCount =
    static_cast<std::size_t>(BindingTarget::Count);

struct BindingProfile {
    std::array<sf::Keyboard::Key, InputActionCount> keys{};

    sf::Keyboard::Key get(InputAction action) const;
    void set(InputAction action, sf::Keyboard::Key key);
};

using BindingProfiles =
    std::array<BindingProfile, BindingTargetCount>;

struct BindingChange {
    BindingTarget target{BindingTarget::Solo};
    InputAction action{InputAction::MoveLeft};
    sf::Keyboard::Key key{sf::Keyboard::Unknown};
};

enum class BindingIssueCode {
    InvalidTarget,
    InvalidAction,
    InvalidKey,
    ReservedKey,
    ActionConflict,
    CrossPlayerConflict
};

struct BindingIssue {
    BindingIssueCode code{BindingIssueCode::InvalidKey};
    std::string message;
};

struct BindingValidationResult {
    std::vector<BindingIssue> issues;

    bool valid() const { return issues.empty(); }
};

enum class BindingUpdateStatus {
    Applied,
    Unchanged,
    Rejected,
    SaveFailed
};

struct BindingUpdateResult {
    BindingUpdateStatus status{BindingUpdateStatus::Rejected};
    std::vector<BindingIssue> issues;

    bool applied() const {
        return status == BindingUpdateStatus::Applied;
    }
};

class IKeyBindingProvider {
public:
    virtual ~IKeyBindingProvider() = default;

    virtual sf::Keyboard::Key getKey(
        BindingTarget target,
        InputAction action) const = 0;
    virtual const BindingProfile& getProfile(
        BindingTarget target) const = 0;
};

class IKeyBindingEditor {
public:
    virtual ~IKeyBindingEditor() = default;

    virtual BindingUpdateResult tryUpdate(
        const BindingChange& change) = 0;
    virtual BindingUpdateResult resetProfile(
        BindingTarget target) = 0;
    virtual BindingUpdateResult resetAll() = 0;
};

const std::array<InputAction, InputActionCount>& allInputActions();
const std::array<BindingTarget, BindingTargetCount>& allBindingTargets();

BindingProfiles makeDefaultBindingProfiles();
BindingValidationResult validateBindingChange(
    const BindingProfiles& current,
    const BindingChange& change);
BindingValidationResult validateBindingProfiles(
    const BindingProfiles& profiles);

const char* inputActionName(InputAction action);
const char* bindingTargetName(BindingTarget target);
std::string keyDisplayName(sf::Keyboard::Key key);
