#include "Commands/JumpCommand.h"

void JumpCommand::execute(Character& character, float dt) {
    character.jump();
}
