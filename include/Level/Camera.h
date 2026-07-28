#pragma once

#include <SFML/Graphics.hpp>

class Camera {
private:
    sf::View view;
    float levelWidth = 3000.f;
    float levelHeight = 600.f;

public:
    Camera(float width = 800.f, float height = 600.f);
    
    void setLevelBounds(float width, float height);
    void setCenter(float x, float y);
    void move(float offsetX, float offsetY);
    
    void update(const sf::Vector2f& playerPos);
    void applyTo(sf::RenderWindow& window);

    const sf::View& getView() const { return view; }
};
