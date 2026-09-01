#include "PlayerStates/PlaneState.h"
#include "Entities/Character.h"
#include "PlayerStates/FireState.h"
#include "PlayerStates/SmallState.h"
#include "PlayerStates/SuperState.h"

PlaneState::PlaneState(std::string_view baseForm)
    : baseFormName(baseForm.empty() || baseForm == "Plane" ? "Small" : std::string(baseForm)) {}

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
    if (baseFormName == "Fire") {
        return std::make_unique<FireState>();
    }
    if (baseFormName == "Super") {
        return std::make_unique<SuperState>();
    }
    return std::make_unique<SmallState>();
}
