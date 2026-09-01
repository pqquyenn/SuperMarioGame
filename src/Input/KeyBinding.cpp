#include "Input/KeyBinding.h"

#include <algorithm>
#include <sstream>

namespace {
std::size_t actionIndex(InputAction action) {
    return static_cast<std::size_t>(action);
}

std::size_t targetIndex(BindingTarget target) {
    return static_cast<std::size_t>(target);
}

bool validAction(InputAction action) {
    return actionIndex(action) < InputActionCount;
}

bool validTarget(BindingTarget target) {
    return targetIndex(target) < BindingTargetCount;
}

bool validKey(sf::Keyboard::Key key) {
    return key >= sf::Keyboard::A &&
           key < sf::Keyboard::KeyCount;
}

bool reservedKey(sf::Keyboard::Key key) {
    return key == sf::Keyboard::Escape ||
           key == sf::Keyboard::F11;
}

bool pairedTargets(
    BindingTarget first,
    BindingTarget second) {
    return (first == BindingTarget::DuoPlayerOne &&
            second == BindingTarget::DuoPlayerTwo) ||
           (first == BindingTarget::DuoPlayerTwo &&
            second == BindingTarget::DuoPlayerOne) ||
           (first == BindingTarget::PvPPlayerOne &&
            second == BindingTarget::PvPPlayerTwo) ||
           (first == BindingTarget::PvPPlayerTwo &&
            second == BindingTarget::PvPPlayerOne);
}

BindingValidationResult validateCandidate(
    const BindingProfiles& profiles) {
    BindingValidationResult result;

    for (BindingTarget target : allBindingTargets()) {
        const BindingProfile& profile = profiles[targetIndex(target)];
        for (InputAction action : allInputActions()) {
            const sf::Keyboard::Key key = profile.get(action);
            if (!validKey(key)) {
                result.issues.push_back({
                    BindingIssueCode::InvalidKey,
                    std::string{inputActionName(action)} +
                        " must have a valid key"});
                continue;
            }
            if (reservedKey(key)) {
                result.issues.push_back({
                    BindingIssueCode::ReservedKey,
                    keyDisplayName(key) + " is reserved"});
            }

            for (InputAction other : allInputActions()) {
                if (actionIndex(other) <= actionIndex(action)) {
                    continue;
                }
                if (profile.get(other) == key) {
                    result.issues.push_back({
                        BindingIssueCode::ActionConflict,
                        keyDisplayName(key) + " conflicts between " +
                            inputActionName(action) + " and " +
                            inputActionName(other) + " in " +
                            bindingTargetName(target)});
                }
            }
        }
    }

    for (BindingTarget target : allBindingTargets()) {
        for (BindingTarget otherTarget : allBindingTargets()) {
            if (targetIndex(otherTarget) <= targetIndex(target) ||
                !pairedTargets(target, otherTarget)) {
                continue;
            }
            const BindingProfile& profile = profiles[targetIndex(target)];
            const BindingProfile& other =
                profiles[targetIndex(otherTarget)];
            for (InputAction action : allInputActions()) {
                const sf::Keyboard::Key key = profile.get(action);
                for (InputAction otherAction : allInputActions()) {
                    if (other.get(otherAction) == key) {
                        result.issues.push_back({
                            BindingIssueCode::CrossPlayerConflict,
                            keyDisplayName(key) + " is used by both " +
                                bindingTargetName(target) + " and " +
                                bindingTargetName(otherTarget)});
                    }
                }
            }
        }
    }

    return result;
}
}

sf::Keyboard::Key BindingProfile::get(InputAction action) const {
    if (!validAction(action)) {
        return sf::Keyboard::Unknown;
    }
    return keys[actionIndex(action)];
}

void BindingProfile::set(
    InputAction action,
    sf::Keyboard::Key key) {
    if (validAction(action)) {
        keys[actionIndex(action)] = key;
    }
}

const std::array<InputAction, InputActionCount>& allInputActions() {
    static const std::array<InputAction, InputActionCount> actions{
        InputAction::MoveLeft,
        InputAction::MoveRight,
        InputAction::Jump,
        InputAction::Crouch,
        InputAction::Action,
        InputAction::Run,
        InputAction::Interact};
    return actions;
}

const std::array<BindingTarget, BindingTargetCount>& allBindingTargets() {
    static const std::array<BindingTarget, BindingTargetCount> targets{
        BindingTarget::Solo,
        BindingTarget::DuoPlayerOne,
        BindingTarget::DuoPlayerTwo,
        BindingTarget::PvPPlayerOne,
        BindingTarget::PvPPlayerTwo};
    return targets;
}

BindingProfiles makeDefaultBindingProfiles() {
    BindingProfiles defaults;

    auto set = [&defaults](
        BindingTarget target,
        InputAction action,
        sf::Keyboard::Key key) {
        defaults[targetIndex(target)].set(action, key);
    };

    set(BindingTarget::Solo, InputAction::MoveLeft, sf::Keyboard::Left);
    set(BindingTarget::Solo, InputAction::MoveRight, sf::Keyboard::Right);
    set(BindingTarget::Solo, InputAction::Jump, sf::Keyboard::Up);
    set(BindingTarget::Solo, InputAction::Crouch, sf::Keyboard::Down);
    set(BindingTarget::Solo, InputAction::Action, sf::Keyboard::Z);
    set(BindingTarget::Solo, InputAction::Run, sf::Keyboard::LShift);
    set(BindingTarget::Solo, InputAction::Interact, sf::Keyboard::E);

    set(BindingTarget::DuoPlayerOne, InputAction::MoveLeft, sf::Keyboard::A);
    set(BindingTarget::DuoPlayerOne, InputAction::MoveRight, sf::Keyboard::D);
    set(BindingTarget::DuoPlayerOne, InputAction::Jump, sf::Keyboard::W);
    set(BindingTarget::DuoPlayerOne, InputAction::Crouch, sf::Keyboard::S);
    set(BindingTarget::DuoPlayerOne, InputAction::Action, sf::Keyboard::Z);
    set(BindingTarget::DuoPlayerOne, InputAction::Run, sf::Keyboard::LShift);
    set(BindingTarget::DuoPlayerOne, InputAction::Interact, sf::Keyboard::E);

    set(BindingTarget::DuoPlayerTwo, InputAction::MoveLeft, sf::Keyboard::Left);
    set(BindingTarget::DuoPlayerTwo, InputAction::MoveRight, sf::Keyboard::Right);
    set(BindingTarget::DuoPlayerTwo, InputAction::Jump, sf::Keyboard::Up);
    set(BindingTarget::DuoPlayerTwo, InputAction::Crouch, sf::Keyboard::Down);
    set(BindingTarget::DuoPlayerTwo, InputAction::Action, sf::Keyboard::Numpad1);
    set(BindingTarget::DuoPlayerTwo, InputAction::Run, sf::Keyboard::Numpad0);
    set(BindingTarget::DuoPlayerTwo, InputAction::Interact, sf::Keyboard::Enter);

    set(BindingTarget::PvPPlayerOne, InputAction::MoveLeft, sf::Keyboard::A);
    set(BindingTarget::PvPPlayerOne, InputAction::MoveRight, sf::Keyboard::D);
    set(BindingTarget::PvPPlayerOne, InputAction::Jump, sf::Keyboard::W);
    set(BindingTarget::PvPPlayerOne, InputAction::Crouch, sf::Keyboard::S);
    set(BindingTarget::PvPPlayerOne, InputAction::Action, sf::Keyboard::Z);
    set(BindingTarget::PvPPlayerOne, InputAction::Run, sf::Keyboard::LShift);
    set(BindingTarget::PvPPlayerOne, InputAction::Interact, sf::Keyboard::E);

    set(BindingTarget::PvPPlayerTwo, InputAction::MoveLeft, sf::Keyboard::Left);
    set(BindingTarget::PvPPlayerTwo, InputAction::MoveRight, sf::Keyboard::Right);
    set(BindingTarget::PvPPlayerTwo, InputAction::Jump, sf::Keyboard::Up);
    set(BindingTarget::PvPPlayerTwo, InputAction::Crouch, sf::Keyboard::Down);
    set(BindingTarget::PvPPlayerTwo, InputAction::Action, sf::Keyboard::J);
    set(BindingTarget::PvPPlayerTwo, InputAction::Run, sf::Keyboard::RShift);
    set(BindingTarget::PvPPlayerTwo, InputAction::Interact, sf::Keyboard::Enter);

    return defaults;
}

BindingValidationResult validateBindingChange(
    const BindingProfiles& current,
    const BindingChange& change) {
    BindingValidationResult result;
    if (!validTarget(change.target)) {
        result.issues.push_back({
            BindingIssueCode::InvalidTarget,
            "Invalid binding target"});
        return result;
    }
    if (!validAction(change.action)) {
        result.issues.push_back({
            BindingIssueCode::InvalidAction,
            "Invalid input action"});
        return result;
    }
    if (!validKey(change.key)) {
        result.issues.push_back({
            BindingIssueCode::InvalidKey,
            "A valid keyboard key is required"});
        return result;
    }

    BindingProfiles candidate = current;
    candidate[targetIndex(change.target)].set(
        change.action,
        change.key);
    return validateCandidate(candidate);
}

BindingValidationResult validateBindingProfiles(
    const BindingProfiles& profiles) {
    return validateCandidate(profiles);
}

const char* inputActionName(InputAction action) {
    switch (action) {
        case InputAction::MoveLeft: return "MOVE LEFT";
        case InputAction::MoveRight: return "MOVE RIGHT";
        case InputAction::Jump: return "JUMP";
        case InputAction::Crouch: return "CROUCH";
        case InputAction::Action: return "ACTION";
        case InputAction::Run: return "RUN";
        case InputAction::Interact: return "INTERACT";
        case InputAction::Count: break;
    }
    return "UNKNOWN ACTION";
}

const char* bindingTargetName(BindingTarget target) {
    switch (target) {
        case BindingTarget::Solo: return "SOLO";
        case BindingTarget::DuoPlayerOne: return "DUO PLAYER 1";
        case BindingTarget::DuoPlayerTwo: return "DUO PLAYER 2";
        case BindingTarget::PvPPlayerOne: return "PVP PLAYER 1";
        case BindingTarget::PvPPlayerTwo: return "PVP PLAYER 2";
        case BindingTarget::Count: break;
    }
    return "UNKNOWN PROFILE";
}

std::string keyDisplayName(sf::Keyboard::Key key) {
    if (key >= sf::Keyboard::A && key <= sf::Keyboard::Z) {
        return std::string(
            1,
            static_cast<char>('A' + key - sf::Keyboard::A));
    }
    if (key >= sf::Keyboard::Num0 && key <= sf::Keyboard::Num9) {
        return "NUM " + std::to_string(key - sf::Keyboard::Num0);
    }
    if (key >= sf::Keyboard::Numpad0 && key <= sf::Keyboard::Numpad9) {
        return "NUMPAD " + std::to_string(key - sf::Keyboard::Numpad0);
    }
    if (key >= sf::Keyboard::F1 && key <= sf::Keyboard::F15) {
        return "F" + std::to_string(key - sf::Keyboard::F1 + 1);
    }

    switch (key) {
        case sf::Keyboard::Unknown: return "UNBOUND";
        case sf::Keyboard::Escape: return "ESCAPE";
        case sf::Keyboard::LControl: return "LEFT CTRL";
        case sf::Keyboard::LShift: return "LEFT SHIFT";
        case sf::Keyboard::LAlt: return "LEFT ALT";
        case sf::Keyboard::LSystem: return "LEFT SYSTEM";
        case sf::Keyboard::RControl: return "RIGHT CTRL";
        case sf::Keyboard::RShift: return "RIGHT SHIFT";
        case sf::Keyboard::RAlt: return "RIGHT ALT";
        case sf::Keyboard::RSystem: return "RIGHT SYSTEM";
        case sf::Keyboard::Menu: return "MENU";
        case sf::Keyboard::LBracket: return "[";
        case sf::Keyboard::RBracket: return "]";
        case sf::Keyboard::Semicolon: return ";";
        case sf::Keyboard::Comma: return ",";
        case sf::Keyboard::Period: return ".";
        case sf::Keyboard::Quote: return "QUOTE";
        case sf::Keyboard::Slash: return "/";
        case sf::Keyboard::Backslash: return "BACKSLASH";
        case sf::Keyboard::Tilde: return "TILDE";
        case sf::Keyboard::Equal: return "=";
        case sf::Keyboard::Hyphen: return "-";
        case sf::Keyboard::Space: return "SPACE";
        case sf::Keyboard::Enter: return "ENTER";
        case sf::Keyboard::Backspace: return "BACKSPACE";
        case sf::Keyboard::Tab: return "TAB";
        case sf::Keyboard::PageUp: return "PAGE UP";
        case sf::Keyboard::PageDown: return "PAGE DOWN";
        case sf::Keyboard::End: return "END";
        case sf::Keyboard::Home: return "HOME";
        case sf::Keyboard::Insert: return "INSERT";
        case sf::Keyboard::Delete: return "DELETE";
        case sf::Keyboard::Add: return "NUMPAD +";
        case sf::Keyboard::Subtract: return "NUMPAD -";
        case sf::Keyboard::Multiply: return "NUMPAD *";
        case sf::Keyboard::Divide: return "NUMPAD /";
        case sf::Keyboard::Left: return "LEFT";
        case sf::Keyboard::Right: return "RIGHT";
        case sf::Keyboard::Up: return "UP";
        case sf::Keyboard::Down: return "DOWN";
        case sf::Keyboard::Pause: return "PAUSE";
        default: break;
    }

    std::ostringstream label;
    label << "KEY " << static_cast<int>(key);
    return label.str();
}
