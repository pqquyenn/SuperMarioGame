#pragma once
#include "Entities/Entity.h"
#include <SFML/Graphics.hpp>

/**
 * @brief A platform tile promoted to a dynamic entity that oscillates
 *        along one axis and carries Mario when he stands on its surface.
 *
 * Platforms are created from 'O' tile positions in the TileMap.
 * A 3-tile-wide platform uses three consecutive O positions; they are
 * merged into a single MovingPlatform spanning their total width.
 */
class MovingPlatform : public Entity {
public:
    enum class Axis { Vertical, Horizontal };

    /**
     * @param x, y   Spawn position (top-left corner)
     * @param w      Platform width in pixels
     * @param minY   Upper Y bound (smallest y the platform reaches)
     * @param maxY   Lower Y bound (largest y the platform reaches)
     * @param speed  Oscillation speed in pixels/sec (default 50)
     * @param axis   Axis of movement (default Vertical)
     */
    MovingPlatform(float x, float y, float w,
                   float minY, float maxY,
                   float speed = 50.f,
                   Axis axis = Axis::Vertical);

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    sf::FloatRect getBounds() const override;

    // Returns the per-frame displacement so CollisionManager can carry Mario.
    sf::Vector2f getDelta() const { return m_delta; }

    float getWidth() const { return m_width; }

private:
    float m_width;
    float m_minBound;
    float m_maxBound;
    float m_speed;
    float m_direction = 1.f;
    Axis  m_axis;
    sf::Vector2f m_prevPosition;
    sf::Vector2f m_delta;

    const sf::Texture* m_texture = nullptr;
};
