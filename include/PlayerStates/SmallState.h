#pragma once

#include "PlayerStates/PlayerState.h"

class SmallState : public PlayerState {
public:
    void handleState(Character& character) override;
};
