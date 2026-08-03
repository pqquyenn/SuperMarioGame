#include "PlayerStates/SuperState.h"
#include "Entities/Character.h"
#include "PlayerStates/SmallState.h"

void SuperState::onEnter(Character& character) const {
    character.applyForm(getName(), getHeightMultiplier());
}

std::string_view SuperState::getName() const {
    return "Super";
}

FormTier SuperState::getFormTier() const {
    return FormTier::Powered;
}

float SuperState::getHeightMultiplier() const {
    return 2.f;
}

bool SuperState::hasAbility(PlayerAbility ability) const {
    return ability == PlayerAbility::BreakBricks;
}

std::unique_ptr<PlayerState> SuperState::takeDamage() const {
    return std::make_unique<SmallState>();
}
