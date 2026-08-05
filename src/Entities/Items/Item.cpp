#include "Entities/Items/Item.h"
#include "Entities/Character.h"

Item::Item(float x, float y) : Entity(x, y) {}

sf::FloatRect Item::getBounds() const {
    return sf::FloatRect(position.x, position.y, size.x, size.y);
}

bool Item::isCollected() const {
    return collected;
}

bool Item::tryCollect(Character& character) {
    onCollect();
    return true;
}

void Item::onCollision(Entity& other, const sf::FloatRect& overlap) {
    if (Character* character = dynamic_cast<Character*>(&other)) {
        tryCollect(*character);
    }
}
