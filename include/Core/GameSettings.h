#pragma once

enum class CharacterChoice {
    Mario,
    Luigi
};

class GameSettings final {
public:
    static GameSettings& getInstance() {
        static GameSettings settings;
        return settings;
    }

    GameSettings(const GameSettings&) = delete;
    GameSettings& operator=(const GameSettings&) = delete;

    CharacterChoice getCharacterChoice() const { return characterChoice; }
    void setCharacterChoice(CharacterChoice choice) { characterChoice = choice; }

private:
    GameSettings() = default;

    CharacterChoice characterChoice{CharacterChoice::Mario};
};
