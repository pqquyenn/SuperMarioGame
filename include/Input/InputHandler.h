#pragma once

#include "Commands/Command.h"
#include "Commands/CrawlCommand.h"
#include <SFML/Window/Keyboard.hpp>
#include <memory>

struct KeyBinding {
    sf::Keyboard::Key primary{sf::Keyboard::Key::Unknown};
    sf::Keyboard::Key secondary{sf::Keyboard::Key::Unknown};
    sf::Keyboard::Key tertiary{sf::Keyboard::Key::Unknown};
};

struct InputBindings {
    KeyBinding moveLeft{sf::Keyboard::Key::Left, sf::Keyboard::Key::A};
    KeyBinding moveRight{sf::Keyboard::Key::Right, sf::Keyboard::Key::D};
    KeyBinding jump{
        sf::Keyboard::Key::Space,
        sf::Keyboard::Key::W,
        sf::Keyboard::Key::Up
    };
    KeyBinding crouch{sf::Keyboard::Key::Down, sf::Keyboard::Key::S};
    KeyBinding action{sf::Keyboard::Key::Z, sf::Keyboard::Key::J, sf::Keyboard::Key::Q};
    KeyBinding run{sf::Keyboard::Key::LShift, sf::Keyboard::Key::RShift};
};

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
    InputBindings bindings;

public:
    explicit InputHandler(
        const InputBindings& inputBindings = InputBindings{},
        bool actionCanRun = true
    );

    void handleInput(
        Character& character,
        float dt,
        const InputPermissions& permissions = InputPermissions{}
    );
    void setBindings(const InputBindings& inputBindings);
    const InputBindings& getBindings() const;
};
