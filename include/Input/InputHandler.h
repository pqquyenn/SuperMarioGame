#pragma once

#include "Commands/Command.h"
#include <SFML/Window/Keyboard.hpp>
#include <memory>

struct KeyBinding {
    sf::Keyboard::Key primary{sf::Keyboard::Unknown};
    sf::Keyboard::Key secondary{sf::Keyboard::Unknown};
    sf::Keyboard::Key tertiary{sf::Keyboard::Unknown};
};

struct InputBindings {
    KeyBinding moveLeft{sf::Keyboard::Left, sf::Keyboard::A};
    KeyBinding moveRight{sf::Keyboard::Right, sf::Keyboard::D};
    KeyBinding jump{
        sf::Keyboard::Space,
        sf::Keyboard::W,
        sf::Keyboard::Up
    };
    KeyBinding action{sf::Keyboard::Z, sf::Keyboard::J};
    KeyBinding run{sf::Keyboard::LShift, sf::Keyboard::RShift};
};

class InputHandler {
private:
    std::unique_ptr<Command> jumpCommand;
    std::unique_ptr<Command> moveLeftCommand;
    std::unique_ptr<Command> moveRightCommand;
    std::unique_ptr<Command> actionCommand;

    bool actionWasHeld{false};
    InputBindings bindings;

public:
    explicit InputHandler(
        const InputBindings& inputBindings = InputBindings{}
    );

    void handleInput(Character& character, float dt);
    void setBindings(const InputBindings& inputBindings);
    const InputBindings& getBindings() const;
};
