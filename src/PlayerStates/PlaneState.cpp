#include "PlayerStates/PlaneState.h"
#include "Entities/Character.h"
#include "PlayerStates/SmallState.h"

void PlaneState::onEnter(Character& character) const {
    character.applyForm(getName(), getHeightMultiplier());
}

void PlaneState::onExit(Character&) const {}

std::string_view PlaneState::getName() const {
    return "Plane";
}

FormTier PlaneState::getFormTier() const {
    return FormTier::Powered;
}

float PlaneState::getHeightMultiplier() const {
    return 1.0f;
}

bool PlaneState::hasAbility(PlayerAbility ability) const {
    return ability == PlayerAbility::Fly ||
           ability == PlayerAbility::ShootFireballs;
}

void PlaneState::useSpecialAbility(Character& character) const {
    character.requestProjectile(ProjectileType::YellowLaser);
}

std::unique_ptr<PlayerState> PlaneState::takeDamage() const {
    return std::make_unique<SmallState>();
}
