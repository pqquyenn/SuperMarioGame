#pragma once

#include "Commands/Command.h"

class FireCommand : public Command {
public:
    void execute(Character& character, float dt) const override;
};
