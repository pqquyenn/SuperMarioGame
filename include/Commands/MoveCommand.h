#pragma once

#include "Commands/Command.h"

class MoveLeftCommand : public Command {
public:
    void execute(Character& character, float dt) override;
};

class MoveRightCommand : public Command {
public:
    void execute(Character& character, float dt) override;
};
