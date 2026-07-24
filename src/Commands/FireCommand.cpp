#include "Commands/FireCommand.h"
#include "Entities/Character.h"

void FireCommand::execute(Character& character, float) const {
    character.useSpecialAbility();
}
