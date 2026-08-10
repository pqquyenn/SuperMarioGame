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
    if (!character.isActive() || character.isDying()) {
        character.setRunning(false);
        character.setJumpHeld(false);
        // Preserve physical key history so held buttons through death or
        // respawn are not mistaken for new presses on the first active frame.
        jumpWasHeld = isHeld(bindings.jump);
        actionWasHeld = isHeld(bindings.action);
        return;
    }

    const bool moveLeftHeld = isHeld(bindings.moveLeft);
    const bool moveRightHeld = isHeld(bindings.moveRight);
    const bool jumpHeld = isHeld(bindings.jump);
    const bool actionHeld = isHeld(bindings.action);
    const bool separateRunHeld = isHeld(bindings.run);

    // Movement commands must observe the current frame's running state.
    // The action key follows classic controls: hold to run, press to act.
    character.setRunning(actionHeld || separateRunHeld);

    // Opposing horizontal inputs cancel one another.
    if (moveLeftHeld != moveRightHeld) {
        if (moveLeftHeld) {
            moveLeftCommand->execute(character, dt);
        } else {
            moveRightCommand->execute(character, dt);
        }
    }

    // A fresh press starts a jump. Holding the button after takeoff continues
    // to produce a higher jump, but holding it through landing does not start
    // another jump automatically.
    character.setJumpHeld(jumpHeld);
    if (jumpHeld && !jumpWasHeld) {
        jumpCommand->execute(character, dt);
    }

    // The action key fires only on a new press.
    if (actionHeld && !actionWasHeld) {
        actionCommand->execute(character, dt);
    }

    jumpWasHeld = jumpHeld;
    actionWasHeld = actionHeld;
}

void InputHandler::setBindings(const InputBindings& inputBindings) {
    bindings = inputBindings;
    jumpWasHeld = false;
    actionWasHeld = false;
}

const InputBindings& InputHandler::getBindings() const {
    return bindings;
}
