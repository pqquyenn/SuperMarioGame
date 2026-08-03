#pragma once

#include "PlayerStates/PlayerState.h"

class FireState : public PlayerState {
public:
    void onEnter(Character& character) const override;

    std::string_view getName() const override;
    FormTier getFormTier() const override;
    float getHeightMultiplier() const override;
    bool hasAbility(PlayerAbility ability) const override;
    void useSpecialAbility(Character& character) const override;

    std::unique_ptr<PlayerState> takeDamage() const override;
};
