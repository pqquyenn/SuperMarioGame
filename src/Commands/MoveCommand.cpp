#include "Commands/MoveCommand.h"
#include "Entities/Character.h"

void MoveLeftCommand::execute(Character& character, float dt) const {
    character.moveLeft(dt);
}

void MoveRightCommand::execute(Character& character, float dt) const {
    character.moveRight(dt);
}
