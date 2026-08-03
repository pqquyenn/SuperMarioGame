#include "Commands/JumpCommand.h"
#include "Entities/Character.h"

void JumpCommand::execute(Character& character, float) const {
    character.jump();
}
