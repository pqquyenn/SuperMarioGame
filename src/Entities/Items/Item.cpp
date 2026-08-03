#include "Entities/Items/Item.h"

Item::Item(float x, float y) : Entity(x, y) {}

sf::FloatRect Item::getBounds() const {
    return sf::FloatRect(position.x, position.y, size.x, size.y);
}

bool Item::isCollected() const {
    return collected;
}
