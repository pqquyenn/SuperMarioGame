#include "PlayerStates/PlayerState.h"

void PlayerState::onExit(Character&) const {
    // Most states have no cleanup. Specialized states may override this hook.
}

void PlayerState::useSpecialAbility(Character&) const {
    // Most forms have no special action.
}
