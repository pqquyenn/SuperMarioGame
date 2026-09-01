#pragma once

#include "PlayerStates/PlayerState.h"
#include <string>
#include <string_view>

class PlaneState : public PlayerState {
private:
    std::string baseFormName{"Small"};

public:
    PlaneState(std::string_view baseForm = "Small");

    void onEnter(Character& character) const override;
    void onExit(Character& character) const override;

    std::string_view getName() const override;
    std::string_view getBaseFormName() const { return baseFormName; }
    FormTier getFormTier() const override;
    float getHeightMultiplier() const override;
    bool hasAbility(PlayerAbility ability) const override;
    void useSpecialAbility(Character& character) const override;

    std::unique_ptr<PlayerState> takeDamage() const override;
};
