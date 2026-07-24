#include "Input/InputHandler.h"
#include "Commands/JumpCommand.h"
#include "Commands/MoveCommand.h"
#include "Commands/FireCommand.h"
#include "Entities/Character.h"
#include <SFML/Window/Keyboard.hpp>

namespace {
bool isHeld(const KeyBinding& binding) {
    const bool primaryHeld =
        binding.primary != sf::Keyboard::Unknown &&
        sf::Keyboard::isKeyPressed(binding.primary);

    const bool secondaryHeld =
        binding.secondary != sf::Keyboard::Unknown &&
        sf::Keyboard::isKeyPressed(binding.secondary);

    const bool tertiaryHeld =
        binding.tertiary != sf::Keyboard::Unknown &&
        sf::Keyboard::isKeyPressed(binding.tertiary);

    return primaryHeld || secondaryHeld || tertiaryHeld;
}
}

InputHandler::InputHandler(const InputBindings& inputBindings)
    : jumpCommand{std::make_unique<JumpCommand>()},
      moveLeftCommand{std::make_unique<MoveLeftCommand>()},
      moveRightCommand{std::make_unique<MoveRightCommand>()},
      actionCommand{std::make_unique<FireCommand>()},
      bindings{inputBindings} {}

void InputHandler::handleInput(Character& character, float dt) {
    if (!character.isActive()) {
        character.setRunning(false);
        actionWasHeld = false;
        return;
    }

    const bool moveLeftHeld = isHeld(bindings.moveLeft);
    const bool moveRightHeld = isHeld(bindings.moveRight);
    const bool jumpHeld = isHeld(bindings.jump);
    const bool actionHeld = isHeld(bindings.action);
    const bool separateRunHeld = isHeld(bindings.run);

    // Opposing horizontal inputs cancel one another.
    if (moveLeftHeld != moveRightHeld) {
        if (moveLeftHeld) {
            moveLeftCommand->execute(character, dt);
        } else {
            moveRightCommand->execute(character, dt);
        }
    }

    // Jump is evaluated every frame so holding the key produces a higher jump.
    if (jumpHeld) {
        jumpCommand->execute(character, dt);
    }

    // The action key doubles as run while held, but fires only on a new press.
    character.setRunning(actionHeld || separateRunHeld);

    if (actionHeld && !actionWasHeld) {
        actionCommand->execute(character, dt);
    }

    actionWasHeld = actionHeld;
}

void InputHandler::setBindings(const InputBindings& inputBindings) {
    bindings = inputBindings;
    actionWasHeld = false;
}

const InputBindings& InputHandler::getBindings() const {
    return bindings;
}
