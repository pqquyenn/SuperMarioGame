#pragma once

#include "PlayerStates/PlayerState.h"

class FireState : public PlayerState {
public:
    void handleState(Character& character) override;
};
