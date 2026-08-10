#include "Entities/MovingPlatform.h"
#include "Core/AssetManager.h"
#include <SFML/Graphics.hpp>

MovingPlatform::MovingPlatform(float x, float y, float w,
                               float minY, float maxY,
                               float speed, Axis axis)
    : Entity(x, y)
    , m_width(w)
    , m_minBound(minY)
    , m_maxBound(maxY)
    , m_speed(speed)
    , m_axis(axis)
    , m_prevPosition(x, y)
    , m_delta(0.f, 0.f)
{
    // Grab platform texture from asset manager (same sprite sheet used by static tiles)
    m_texture = &AssetManager::getInstance().getTexture("Platform");
}

void MovingPlatform::update(float dt) {
    m_prevPosition = position;

    if (m_axis == Axis::Vertical) {
        position.y += m_speed * m_direction * dt;
        if (position.y >= m_maxBound) {
            position.y = m_maxBound;
            m_direction = -1.f;
        } else if (position.y <= m_minBound) {
            position.y = m_minBound;
            m_direction = 1.f;
        }
    } else {
        position.x += m_speed * m_direction * dt;
        if (position.x >= m_maxBound) {
            position.x = m_maxBound;
            m_direction = -1.f;
        } else if (position.x <= m_minBound) {
            position.x = m_minBound;
            m_direction = 1.f;
        }
    }

    m_delta = position - m_prevPosition;
    syncSpritePosition();
}

void MovingPlatform::render(sf::RenderWindow& window) const {
    if (!m_texture) return;

    // Draw left segment
    sf::Sprite spr(*m_texture, sf::IntRect(0, 0, 16, 8));
    spr.setPosition(position.x, position.y);
    window.draw(spr);

    // Draw middle segments
    int numMiddle = static_cast<int>(m_width / 16.f) - 2;
    for (int i = 0; i < numMiddle; ++i) {
        spr.setTextureRect(sf::IntRect(16, 0, 16, 8));
        spr.setPosition(position.x + 16.f + i * 16.f, position.y);
        window.draw(spr);
    }

    // Draw right segment
    spr.setTextureRect(sf::IntRect(32, 0, 16, 8));
    spr.setPosition(position.x + m_width - 16.f, position.y);
    window.draw(spr);
}

sf::FloatRect MovingPlatform::getBounds() const {
    return sf::FloatRect(position.x, position.y, m_width, 8.f);
}
