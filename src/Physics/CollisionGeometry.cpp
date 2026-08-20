#include "Physics/CollisionGeometry.h"

namespace CollisionGeometry {

bool intersects(
    const sf::FloatRect& moving,
    const sf::FloatRect& obstacle,
    sf::FloatRect& overlap) {
    return moving.intersects(obstacle, overlap);
}

std::optional<CollisionContact> findContact(
    const sf::FloatRect& moving,
    const sf::FloatRect& obstacle) {
    sf::FloatRect overlap;
    if (!intersects(moving, obstacle, overlap)) {
        return std::nullopt;
    }

    CollisionContact contact;
    contact.overlap = overlap;

    if (overlap.height <= overlap.width) {
        if (moving.top < obstacle.top) {
            contact.side = CollisionSide::Top;
            contact.normal = {0.f, -1.f};
        } else {
            contact.side = CollisionSide::Bottom;
            contact.normal = {0.f, 1.f};
        }
    } else if (moving.left < obstacle.left) {
        contact.side = CollisionSide::Left;
        contact.normal = {-1.f, 0.f};
    } else {
        contact.side = CollisionSide::Right;
        contact.normal = {1.f, 0.f};
    }

    return contact;
}

} // namespace CollisionGeometry
