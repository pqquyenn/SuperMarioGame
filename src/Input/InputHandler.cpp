#include "Input/InputHandler.h"
#include "Commands/JumpCommand.h"
#include "Commands/MoveCommand.h"
#include "Commands/FireCommand.h"
#include <SFML/Window/Keyboard.hpp>

InputHandler::InputHandler() {
    buttonUp = std::make_unique<JumpCommand>();
    buttonLeft = std::make_unique<MoveLeftCommand>();
    buttonRight = std::make_unique<MoveRightCommand>();
    buttonAction = std::make_unique<FireCommand>();
}

void InputHandler::handleInput(Character& character, float dt) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        buttonLeft->execute(character, dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        buttonRight->execute(character, dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        buttonUp->execute(character, dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) || sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
        buttonAction->execute(character, dt);
    }
}
