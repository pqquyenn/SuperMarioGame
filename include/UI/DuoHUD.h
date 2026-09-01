#pragma once

#include "Duo/DuoTypes.h"

#include <SFML/Graphics.hpp>

#include <string>

class Character;

struct DuoHudPlayerData {
    std::string label;
    std::string characterName;
    std::string formName;
    int score{0};
    int coins{0};
    int lives{0};
    DuoLifeState lifeState{DuoLifeState::Active};
};

class DuoHUD {
private:
    sf::Font font;
    bool fontLoaded{false};

    static const char* lifeStateName(DuoLifeState state);

public:
    DuoHUD();

    void render(
        sf::RenderWindow& window,
        const DuoHudPlayerData& playerOne,
        const DuoHudPlayerData& playerTwo,
        const std::string& worldName,
        float timeRemaining) const;

    void renderPlayerMarker(
        sf::RenderWindow& window,
        const Character& character,
        const std::string& label) const;
};
