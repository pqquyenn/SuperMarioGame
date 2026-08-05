#include "Level/Tile.h"
#include "Level/TileType.h"

Tile::Tile(const TileType* type, const sf::Vector2f& position)
    : m_type(type), m_position(position) {
}

void Tile::render(sf::RenderTarget& target) const {
    if (m_type) {
        m_type->render(target, m_position);
    }
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