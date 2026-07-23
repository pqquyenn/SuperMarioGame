#pragma once

#include <SFML/Graphics.hpp>

class Entity {
protected:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Sprite sprite;
    bool active = true;

public:
    Entity(float x = 0.f, float y = 0.f) : position(x, y), velocity(0.f, 0.f) {}
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return position; }
    void setPosition(float x, float y);

    bool isActive() const { return active; }
    void setActive(bool status) { active = status; }
};
