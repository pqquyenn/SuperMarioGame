#pragma once

#include "Commands/Command.h"
#include "Commands/CrawlCommand.h"
#include "Input/KeyBinding.h"
#include <SFML/Window/Keyboard.hpp>
#include <memory>

// A gameplay ruleset can temporarily suppress individual actions without
// changing the configured keys. Duo mode uses the horizontal flags for its
// shared-camera tether; existing Solo and PvP callers receive full access.
struct InputPermissions {
    bool allowMoveLeft{true};
    bool allowMoveRight{true};
    bool allowJump{true};
    bool allowCrouch{true};
    bool allowAction{true};
    bool allowRun{true};
};

class InputHandler {
private:
    std::unique_ptr<Command> jumpCommand;
    std::unique_ptr<Command> moveLeftCommand;
    std::unique_ptr<Command> moveRightCommand;
    std::unique_ptr<Command> actionCommand;
    std::unique_ptr<CrawlCommand> crawlCommand;

    bool jumpWasHeld{false};
    bool actionWasHeld{false};
    bool actionAlsoRuns{true};
    BindingTarget bindingTarget{BindingTarget::Solo};
    const IKeyBindingProvider* bindingProvider{nullptr};
    sf::Keyboard::Key observedJumpKey{sf::Keyboard::Unknown};
    sf::Keyboard::Key observedActionKey{sf::Keyboard::Unknown};

    sf::Keyboard::Key keyFor(InputAction action) const;
    bool held(InputAction action) const;
    void synchronizeEdgeBindings();

public:
    explicit InputHandler(
        BindingTarget target = BindingTarget::Solo,
        bool actionCanRun = true
    );
    InputHandler(
        BindingTarget target,
        const IKeyBindingProvider& provider,
        bool actionCanRun = true
    );

    void handleInput(
        Character& character,
        float dt,
        const InputPermissions& permissions = InputPermissions{}
    );
    bool isHeld(InputAction action) const;
    bool matches(InputAction action, sf::Keyboard::Key key) const;
    BindingTarget getBindingTarget() const { return bindingTarget; }
};
