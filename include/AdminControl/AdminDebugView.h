#pragma once

#include "AdminControl/DebugMovementTrail.h"
#include <SFML/Graphics.hpp>

class Character;
class Level;

class AdminDebugView {
private:
    bool visible{false};
    bool fontLoaded{false};
    sf::Font font;
    sf::Text informationText;
    sf::RectangleShape panel;
    DebugMovementTrail movementTrail;

    void renderWorldAnnotations(
        sf::RenderWindow& window,
        const Character& character,
        const Level& level
    ) const;

public:
    AdminDebugView();

    void toggle();
    bool isVisible() const;
    void startMovementTrail(const Character& character);
    void updateMovementTrail(const Character& character, float dt);
    void render(
        sf::RenderWindow& window,
        const Character& character,
        const Level& level
    );
};
