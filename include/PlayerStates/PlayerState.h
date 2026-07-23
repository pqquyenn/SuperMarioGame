#pragma once

class Character;

class PlayerState {
public:
    virtual ~PlayerState() = default;
    virtual void handleState(Character& character) = 0;
};
