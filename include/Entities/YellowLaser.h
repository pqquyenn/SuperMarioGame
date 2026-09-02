#pragma once

#include "Entities/Entity.h"
#include <SFML/Graphics.hpp>

class YellowLaser : public Entity {
private:
    static constexpr float LaserSpeed{450.f};
    bool movingRight{true};
    bool exploding{false};
    float lifetime{0.f};
    sf::Vector2f laserSize{26.f, 10.f};

public:
    YellowLaser(
        float x,
        float y,
        bool movingRight,
        const sf::Texture& texture
    );

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    sf::FloatRect getBounds() const override;

    bool isMovingRight() const { return movingRight; }
    void explode();
    bool isExploding() const { return exploding; }
};
