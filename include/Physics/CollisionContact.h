#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

enum class CollisionSide {
    None,
    Top,
    Bottom,
    Left,
    Right
};

struct CollisionContact {
    sf::FloatRect overlap{};
    CollisionSide side{CollisionSide::None};
    sf::Vector2f normal{0.f, 0.f};
};
