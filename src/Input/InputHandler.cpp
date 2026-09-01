#include "Input/InputHandler.h"
#include "Commands/JumpCommand.h"
#include "Commands/MoveCommand.h"
#include "Commands/FireCommand.h"
#include "Commands/CrawlCommand.h"
#include "Entities/Character.h"
#include "Input/KeyBindingService.h"
#include <SFML/Window/Keyboard.hpp>

namespace {
bool isKeyHeld(sf::Keyboard::Key key) {
    return key != sf::Keyboard::Unknown &&
           sf::Keyboard::isKeyPressed(key);
}
}

InputHandler::InputHandler(
    BindingTarget target,
    bool actionCanRun
)
    : InputHandler(
          target,
          KeyBindingService::getInstance(),
          actionCanRun) {}

InputHandler::InputHandler(
    BindingTarget target,
    const IKeyBindingProvider& provider,
    bool actionCanRun
)
    : jumpCommand{std::make_unique<JumpCommand>()},
      moveLeftCommand{std::make_unique<MoveLeftCommand>()},
      moveRightCommand{std::make_unique<MoveRightCommand>()},
      actionCommand{std::make_unique<FireCommand>()},
      crawlCommand{std::make_unique<CrawlCommand>()},
      actionAlsoRuns{actionCanRun},
      bindingTarget{target},
      bindingProvider{&provider},
      observedJumpKey{keyFor(InputAction::Jump)},
      observedActionKey{keyFor(InputAction::Action)} {}

sf::Keyboard::Key InputHandler::keyFor(InputAction action) const {
    return bindingProvider
        ? bindingProvider->getKey(bindingTarget, action)
        : sf::Keyboard::Unknown;
}

bool InputHandler::held(InputAction action) const {
    return isKeyHeld(keyFor(action));
}

void InputHandler::synchronizeEdgeBindings() {
    const sf::Keyboard::Key jumpKey = keyFor(InputAction::Jump);
    if (jumpKey != observedJumpKey) {
        observedJumpKey = jumpKey;
        jumpWasHeld = isKeyHeld(jumpKey);
    }

    const sf::Keyboard::Key actionKey = keyFor(InputAction::Action);
    if (actionKey != observedActionKey) {
        observedActionKey = actionKey;
        actionWasHeld = isKeyHeld(actionKey);
    }
}

void InputHandler::handleInput(
    Character& character,
    float dt,
    const InputPermissions& permissions
) {
    synchronizeEdgeBindings();

    if (!character.isActive() || character.isDying()) {
        character.setRunning(false);
        character.setJumpHeld(false);
        crawlCommand->release(character);
        // Preserve physical key history so held buttons through death or
        // respawn are not mistaken for new presses on the first active frame.
        jumpWasHeld = held(InputAction::Jump);
        actionWasHeld = held(InputAction::Action);
        return;
    }

    const bool rawMoveLeftHeld = held(InputAction::MoveLeft);
    const bool rawMoveRightHeld = held(InputAction::MoveRight);
    const bool rawJumpHeld = held(InputAction::Jump);
    const bool rawCrouchHeld = held(InputAction::Crouch);
    const bool rawActionHeld = held(InputAction::Action);
    const bool rawRunHeld = held(InputAction::Run);

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

bool InputHandler::isHeld(InputAction action) const {
    return held(action);
}

bool InputHandler::matches(
    InputAction action,
    sf::Keyboard::Key key) const {
    return keyFor(action) == key;
}
