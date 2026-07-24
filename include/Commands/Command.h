#pragma once

class Character;

class Command {
public:
    virtual ~Command() = default;

    virtual void execute(Character& character, float dt) const = 0;
};
