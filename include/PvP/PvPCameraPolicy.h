#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

struct PvPCameraLayout {
    sf::Vector2f viewSize;
    sf::FloatRect viewport;
};

class PvPCameraPolicy {
public:
    static PvPCameraLayout layout(
        sf::Vector2f minimumView,
        sf::Vector2f worldSize,
        sf::Vector2u windowSize);
};
