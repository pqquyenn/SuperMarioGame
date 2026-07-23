#pragma once

#include <SFML/Graphics.hpp>

class Camera {
private:
    sf::View view;
    float levelWidth = 3000.f;

public:
    Camera(float width = 800.f, float height = 600.f);
    void update(const sf::Vector2f& playerPos);
    void applyTo(sf::RenderWindow& window);

    const sf::View& getView() const { return view; }
};
