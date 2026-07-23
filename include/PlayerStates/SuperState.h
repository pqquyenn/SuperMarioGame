#pragma once

#include "PlayerStates/PlayerState.h"

class SuperState : public PlayerState {
public:
    void handleState(Character& character) override;
};
