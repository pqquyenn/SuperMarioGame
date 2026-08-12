#pragma once

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

    void renderWorldAnnotations(
        sf::RenderWindow& window,
        const Character& character,
        const Level& level
    ) const;

public:
    AdminDebugView();

    void toggle();
    bool isVisible() const;
    void render(
        sf::RenderWindow& window,
        const Character& character,
        const Level& level
    );
};
