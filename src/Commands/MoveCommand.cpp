#include "Commands/MoveCommand.h"

void MoveLeftCommand::execute(Character& character, float dt) {
    character.moveLeft(dt);
}

void MoveRightCommand::execute(Character& character, float dt) {
    character.moveRight(dt);
}
