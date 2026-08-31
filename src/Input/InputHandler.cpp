#include "Input/InputHandler.h"
#include "Commands/JumpCommand.h"
#include "Commands/MoveCommand.h"
#include "Commands/FireCommand.h"
#include "Commands/CrawlCommand.h"
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

InputHandler::InputHandler(
    const InputBindings& inputBindings,
    bool actionCanRun
)
    : jumpCommand{std::make_unique<JumpCommand>()},
      moveLeftCommand{std::make_unique<MoveLeftCommand>()},
      moveRightCommand{std::make_unique<MoveRightCommand>()},
      actionCommand{std::make_unique<FireCommand>()},
      crawlCommand{std::make_unique<CrawlCommand>()},
      actionAlsoRuns{actionCanRun},
      bindings{inputBindings} {}

void InputHandler::handleInput(
    Character& character,
    float dt,
    const InputPermissions& permissions
) {
    if (!character.isActive() || character.isDying()) {
        character.setRunning(false);
        character.setJumpHeld(false);
        crawlCommand->release(character);
        // Preserve physical key history so held buttons through death or
        // respawn are not mistaken for new presses on the first active frame.
        jumpWasHeld = isHeld(bindings.jump);
        actionWasHeld = isHeld(bindings.action);
        return;
    }

    const bool rawMoveLeftHeld = isHeld(bindings.moveLeft);
    const bool rawMoveRightHeld = isHeld(bindings.moveRight);
    const bool rawJumpHeld = isHeld(bindings.jump);
    const bool rawCrouchHeld = isHeld(bindings.crouch);
    const bool rawActionHeld = isHeld(bindings.action);
    const bool rawRunHeld = isHeld(bindings.run);

    const bool moveLeftHeld =
        permissions.allowMoveLeft && rawMoveLeftHeld;
    const bool moveRightHeld =
        permissions.allowMoveRight && rawMoveRightHeld;
    const bool jumpHeld = permissions.allowJump && rawJumpHeld;
    const bool crouchHeld = permissions.allowCrouch && rawCrouchHeld;
    const bool actionHeld = permissions.allowAction && rawActionHeld;
    const bool separateRunHeld = permissions.allowRun && rawRunHeld;

    if (crouchHeld) {
        crawlCommand->execute(character, dt);
    } else {
        crawlCommand->release(character);
    }

    // Movement commands must observe the current frame's running state.
    // The action key follows classic controls: hold to run, press to act.
    character.setRunning(!character.isCrouching() &&
                         (separateRunHeld ||
                          (actionAlsoRuns && actionHeld)));

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
    character.setJumpHeld(!character.isCrouching() && jumpHeld);
    if (!character.isCrouching() && jumpHeld && !jumpWasHeld) {
        jumpCommand->execute(character, dt);
    }

    // The action key fires only on a new press.
    if (!character.isCrouching() && actionHeld && !actionWasHeld) {
        actionCommand->execute(character, dt);
    }

    // Track the physical keys rather than the gated values. Releasing a
    // gameplay restriction while a button is held must not synthesize a new
    // jump or action press.
    jumpWasHeld = rawJumpHeld;
    actionWasHeld = rawActionHeld;
}

void InputHandler::setBindings(const InputBindings& inputBindings) {
    bindings = inputBindings;
    jumpWasHeld = false;
    actionWasHeld = false;
}

const InputBindings& InputHandler::getBindings() const {
    return bindings;
}
