#pragma once

#include "Entities/Character.h"
#include <memory>

class PlayerState; // Forward declaration (State Pattern)

class Mario : public Character {
private:
    std::unique_ptr<PlayerState> currentState;

public:
    Mario(float x = 0.f, float y = 0.f);
    void update(float dt) override;
    void changeState(std::unique_ptr<PlayerState> state);
};
