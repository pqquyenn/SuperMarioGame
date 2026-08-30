#pragma once

#include "Core/GameSettings.h"

#include <string>

enum class DuoPlayerId {
    One,
    Two
};

enum class DuoLifeState {
    Active,
    Dying,
    Bubble,
    Out
};

struct DuoPlayerStats {
    int score{0};
    int coins{0};
    int enemiesDefeated{0};
    int deaths{0};
    int rescuesPerformed{0};
    int rescuesReceived{0};
    // Height above the base of the flagpole in world pixels. A negative value
    // means that this player did not touch a flagpole.
    float flagHeight{-1.f};
};

struct DuoSessionConfig {
    std::string mapPath{"1.1/1-1.level"};
    CharacterChoice playerOneChoice{CharacterChoice::Mario};
    CharacterChoice playerTwoChoice{CharacterChoice::Luigi};
    int startingLives{3};
};

struct DuoLevelResult {
    DuoSessionConfig session;
    std::string stageName;
    std::string nextStage;
    DuoPlayerStats playerOne;
    DuoPlayerStats playerTwo;
    int playerOneLives{0};
    int playerTwoLives{0};
    int timeBonus{0};
    bool completedByFlag{false};
};

enum class DuoMvpResult {
    PlayerOne,
    PlayerTwo,
    Tie
};
