#pragma once

#include "Commands/Command.h"
#include <memory>
#include <vector>

class InputHandler {
private:
    std::unique_ptr<Command> buttonUp;
    std::unique_ptr<Command> buttonLeft;
    std::unique_ptr<Command> buttonRight;
    std::unique_ptr<Command> buttonAction;

public:
    InputHandler();
    void handleInput(Character& character, float dt);
};
