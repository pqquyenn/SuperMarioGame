#pragma once

#include "Entities/Character.h"

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(Character& character, float dt) = 0;
};
