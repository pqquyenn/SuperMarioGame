#include "Level/Camera.h"
#include <algorithm>

Camera::Camera(float width, float height) {
    view.setSize(width, height);
    view.setCenter(width / 2.f, height / 2.f);
}

void Camera::setLevelBounds(float width, float height) {
    levelWidth = width;
    levelHeight = height;
}

void Camera::setCenter(float x, float y) {
    // 2-Way Camera clamping in X and Y directions
    float targetX = std::max(view.getSize().x / 2.f, x);
    targetX = std::min(targetX, levelWidth - view.getSize().x / 2.f);

    // Compartmentalize Y bounds: Overworld (0-15 rows), Underworld (15+ rows)
    float splitY = 15.f * 16.f;
    float targetY = y;
    
    if (y < splitY) {
        // Overworld clamp
        targetY = std::max(view.getSize().y / 2.f, y);
        targetY = std::min(targetY, splitY - view.getSize().y / 2.f);
    } else {
        // Underworld clamp
        targetY = std::max(splitY + view.getSize().y / 2.f, y);
        targetY = std::min(targetY, levelHeight - view.getSize().y / 2.f);
    }

    view.setCenter(targetX, targetY);
}

void Camera::move(float offsetX, float offsetY) {
    sf::Vector2f newCenter = view.getCenter() + sf::Vector2f(offsetX, offsetY);
    setCenter(newCenter.x, newCenter.y);
}

void Camera::update(const sf::Vector2f& playerPos) {
    setCenter(playerPos.x, playerPos.y);
}

void Camera::applyTo(sf::RenderWindow& window) {
    window.setView(view);
}
