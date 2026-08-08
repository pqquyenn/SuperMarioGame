#pragma once

#include "Commands/Command.h"
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
    KeyBinding action{sf::Keyboard::Key::Z, sf::Keyboard::Key::J, sf::Keyboard::Key::Q};
    KeyBinding run{sf::Keyboard::Key::LShift, sf::Keyboard::Key::RShift};
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
