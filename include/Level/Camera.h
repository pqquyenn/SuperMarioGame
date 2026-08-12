#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>

class Camera {
private:
    sf::View view;
    float levelWidth = 3000.f;
    float levelHeight = 600.f;

public:
    Camera(float width = 320.f, float height = 240.f) {
        view.setSize(width, height);
        view.setCenter(width / 2.f, height / 2.f);
    }

    void setLevelBounds(float width, float height) {
        levelWidth = width;
        levelHeight = height;
    }

    void setSize(float width, float height) {
        view.setSize(width, height);
    }

    void zoom(float factor) {
        view.zoom(factor);
    }


    void setCenter(float x, float y) {
        float halfW = view.getSize().x / 2.f;
        float halfH = view.getSize().y / 2.f;

        float targetX = std::max(halfW, x);
        targetX = std::min(targetX, levelWidth - halfW);

        float targetY = std::max(halfH, y);
        targetY = std::min(targetY, levelHeight - halfH);

        if (view.getSize().x >= levelWidth)  targetX = levelWidth  / 2.f;
        if (view.getSize().y >= levelHeight) targetY = levelHeight / 2.f;

        view.setCenter(targetX, targetY);
    }

    void move(float offsetX, float offsetY) {
        sf::Vector2f newCenter = view.getCenter() + sf::Vector2f(offsetX, offsetY);
        setCenter(newCenter.x, newCenter.y);
    }

    void update(const sf::Vector2f& playerPos) {
        sf::Vector2f current = view.getCenter();
        const float lerpFactor = 0.15f;
        float targetX = current.x + (playerPos.x - current.x) * lerpFactor;
        float targetY = current.y + (playerPos.y - current.y) * lerpFactor;
        setCenter(targetX, targetY);
    }

    void applyTo(sf::RenderWindow& window) {
        sf::View fitted = view;
        const sf::Vector2u windowSize = window.getSize();
        if (windowSize.x > 0 && windowSize.y > 0) {
            const float viewAspect = view.getSize().x / view.getSize().y;
            const float windowAspect = static_cast<float>(windowSize.x) /
                                       static_cast<float>(windowSize.y);
            sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
            if (windowAspect > viewAspect) {
                viewport.width = viewAspect / windowAspect;
                viewport.left = (1.f - viewport.width) * 0.5f;
            } else if (windowAspect < viewAspect) {
                viewport.height = windowAspect / viewAspect;
                viewport.top = (1.f - viewport.height) * 0.5f;
            }
            fitted.setViewport(viewport);
        }
        window.setView(fitted);
    }

    const sf::View& getView() const {
        return view;
    }

    sf::FloatRect getViewBounds() const {
        sf::Vector2f c = view.getCenter();
        sf::Vector2f s = view.getSize();
        return sf::FloatRect({c.x - s.x * 0.5f, c.y - s.y * 0.5f}, s);
    }
};
