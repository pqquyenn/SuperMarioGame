#include "Commands/FireCommand.h"

void FireCommand::execute(Character& character, float dt) {
    character.shootFireball();
}
