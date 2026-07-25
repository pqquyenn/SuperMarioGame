#include "Level/Tile.h"
#include "Level/TileType.h"

Tile::Tile(const TileType* type, const sf::Vector2f& position)
    : m_type(type), m_position(position) {
}

void Tile::render(sf::RenderTarget& target) const {
    if (m_type) {
        // Delegate rendering to the shared Flyweight, passing our specific extrinsic position
        m_type->render(target, m_position);
    }
}

sf::FloatRect Tile::getBounds() const {
    if (!m_type) return sf::FloatRect(m_position.x, m_position.y, 0, 0);
    
    // Assuming the bounding box size matches the texture rect size
    return sf::FloatRect(
        m_position.x, 
        m_position.y, 
        static_cast<float>(m_type->textureRect.width), 
        static_cast<float>(m_type->textureRect.height)
    );
}

bool Tile::isSolid() const {
    // The boolean logic lives strictly in the shared Flyweight
    return m_type ? m_type->isSolid : false;
}

bool Tile::isWarpPipe() const {
    return m_type ? m_type->isWarpPipe : false;
}

int Tile::getWarpDirection() const {
    return m_type ? m_type->warpDirection : 0;
}
