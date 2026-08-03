#pragma once

#include "Commands/Command.h"

class JumpCommand : public Command {
public:
    void execute(Character& character, float dt) const override;
};
