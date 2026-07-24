#pragma once

#include <SFML/Graphics.hpp>

class Entity {
protected:
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f velocity{0.f, 0.f};
    sf::Sprite sprite;
    bool active{true};

    void syncSpritePosition();
    void integrateVelocity(float dt);

public:
    // constructor and destructor
    explicit Entity(float x = 0.f, float y = 0.f);
    virtual ~Entity() = default;

    // prevent copy constructor, assignment operator, move constructor, move assignment operator
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    // facade update and rendering
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) const;

    // getters, setters and movement handling
    virtual sf::FloatRect getBounds() const;

    void setTexture(const sf::Texture& texture, bool resetRect = false);
    void setTextureRect(const sf::IntRect& textureRect);

    const sf::Vector2f& getPosition() const;
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& newPosition);
    void move(float offsetX, float offsetY);
    void move(const sf::Vector2f& offset);

    const sf::Vector2f& getVelocity() const;
    void setVelocity(float x, float y);
    void setVelocity(const sf::Vector2f& newVelocity);
    void addVelocity(float x, float y);

    bool isActive() const;
    void setActive(bool status);
};
