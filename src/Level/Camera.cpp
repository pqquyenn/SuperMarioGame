#include "Level/Camera.h"
#include <algorithm>

Camera::Camera(float width, float height) {
    view.setSize(width, height);
    view.setCenter(width / 2.f, height / 2.f);
}

void Camera::update(const sf::Vector2f& playerPos) {
    float targetX = std::max(view.getSize().x / 2.f, playerPos.x);
    targetX = std::min(targetX, levelWidth - view.getSize().x / 2.f);
    view.setCenter(targetX, view.getCenter().y);
}

void Camera::applyTo(sf::RenderWindow& window) {
    window.setView(view);
}
