#include "PlayerStates/SmallState.h"
#include "Entities/Character.h"

void SmallState::onEnter(Character& character) const {
    character.applyForm(getName(), getHeightMultiplier());
}

std::string_view SmallState::getName() const {
    return "Small";
}

FormTier SmallState::getFormTier() const {
    return FormTier::Small;
}

float SmallState::getHeightMultiplier() const {
    return 1.f;
}

bool SmallState::hasAbility(PlayerAbility) const {
    return false;
}

std::unique_ptr<PlayerState> SmallState::takeDamage() const {
    return nullptr;
}
