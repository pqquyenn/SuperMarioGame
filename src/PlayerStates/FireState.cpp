#include "PlayerStates/FireState.h"
#include "Entities/Character.h"
#include "PlayerStates/SmallState.h"

void FireState::onEnter(Character& character) const {
    character.applyForm(getName(), getHeightMultiplier());
}

std::string_view FireState::getName() const {
    return "Fire";
}

FormTier FireState::getFormTier() const {
    return FormTier::Powered;
}

float FireState::getHeightMultiplier() const {
    return 2.f;
}

bool FireState::hasAbility(PlayerAbility ability) const {
    return ability == PlayerAbility::BreakBricks ||
           ability == PlayerAbility::ShootFireballs;
}

void FireState::useSpecialAbility(Character& character) const {
    character.shootFireball();
}

std::unique_ptr<PlayerState> FireState::takeDamage() const {
    return std::make_unique<SmallState>();
}
