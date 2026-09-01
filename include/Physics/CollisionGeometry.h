#pragma once

#include "Physics/CollisionContact.h"

#include <optional>

namespace CollisionGeometry {

bool intersects(
    const sf::FloatRect& moving,
    const sf::FloatRect& obstacle,
    sf::FloatRect& overlap);

std::optional<CollisionContact> findContact(
    const sf::FloatRect& moving,
    const sf::FloatRect& obstacle);

} // namespace CollisionGeometry
