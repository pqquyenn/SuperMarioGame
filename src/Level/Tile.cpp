#include "Level/Tile.h"
#include "Level/TileType.h"
#include <cmath>

Tile::Tile(const TileType* type, const sf::Vector2f& position)
    : m_type(type), m_position(position) {
}

void Tile::update(float dt) {
    // Question block shimmer animation
    if (m_type && m_type->isAnimated) {
        m_animTimer += dt;
        if (m_animTimer >= 0.15f) {
            m_animTimer -= 0.15f;
            m_animFrame = (m_animFrame + 1) % 4;
        }
    }

    // Bump animation (block bounces up and back)
    if (m_bumping) {
        m_animTimer += dt; // reuse timer won't conflict since bump is quick
        // Quick up-down bounce over ~0.15s
        float bumpDuration = 0.15f;
        float bumpHeight = 4.f;
        float progress = m_bumpOffset;
        m_bumpOffset += dt / bumpDuration;
        if (m_bumpOffset >= 1.f) {
            m_bumpOffset = 0.f;
            m_bumping = false;
        } else {
            // Parabolic: goes up then comes back
        }
    }
}

void Tile::render(sf::RenderTarget& target) const {
    if (!m_type || !m_type->texture) return;

    sf::Sprite sprite(*m_type->texture, m_type->textureRect);
    
    // Apply bump offset
    float yOffset = 0.f;
    if (m_bumping) {
        // Parabolic bump: max at 0.5, returns to 0 at 1.0
        float t = m_bumpOffset;
        yOffset = -4.f * std::sin(t * 3.14159f);
    }
    
    sprite.setPosition(m_position.x, m_position.y + yOffset);

    // Shimmer color tint for animated tiles (question blocks)
    if (m_type->isAnimated) {
        switch (m_animFrame) {
            case 0: sprite.setColor(sf::Color(255, 255, 255)); break;
            case 1: sprite.setColor(sf::Color(255, 255, 180)); break;
            case 2: sprite.setColor(sf::Color(220, 200, 150)); break;
            case 3: sprite.setColor(sf::Color(255, 240, 200)); break;
        }
    }

    target.draw(sprite);
}

sf::FloatRect Tile::getBounds() const {
    if (!m_type) return sf::FloatRect(m_position, {0.f, 0.f});
    sf::Vector2f size(static_cast<float>(m_type->textureRect.width), static_cast<float>(m_type->textureRect.height));
    return sf::FloatRect(m_position, size);
}

bool Tile::isSolid() const {
    return m_type ? m_type->isSolid : false;
}

bool Tile::isWarpPipe() const {
    return m_type ? m_type->isWarpPipe : false;
}

bool Tile::isHorizontalWarpPipe() const {
    return m_type ? m_type->isHorizontalWarpPipe : false;
}

int Tile::getWarpDirection() const {
    return m_type ? m_type->warpDirection : 0;
}

bool Tile::isQuestionBlock() const {
    return m_type ? m_type->isQuestionBlock : false;
}

bool Tile::isCoinTile() const {
    return m_type ? m_type->isCoinTile : false;
}

bool Tile::isBrick() const {
    return m_type ? m_type->isBrick : false;
}

bool Tile::isAnimated() const {
    return m_type ? m_type->isAnimated : false;
}

bool Tile::isFlagpole() const {
    return m_type ? m_type->isFlagpole : false;
}

bool Tile::isCastle() const {
    return m_type ? m_type->isCastle : false;
}

void Tile::startBump() {
    m_bumping = true;
    m_bumpOffset = 0.f;
}