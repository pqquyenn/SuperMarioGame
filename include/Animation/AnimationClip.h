#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <vector>

// A frame may select a rectangle from the sprite's current texture or inject
// another externally owned texture. The optional pointer lets one animator
// support both atlases and collections of individual frame images.
struct AnimationFrame {
    sf::IntRect textureRect;
    const sf::Texture* texture{nullptr};

    AnimationFrame(
        const sf::IntRect& rect,
        const sf::Texture* frameTexture = nullptr
    )
        : textureRect{rect}, texture{frameTexture} {}
};

// Immutable frame data that can be shared by multiple SpriteAnimator objects.
struct AnimationClip {
    std::vector<AnimationFrame> frames;
    float frameDuration{0.1f};
    bool looping{true};

    bool isValid() const {
        return !frames.empty() && frameDuration > 0.f;
    }
};
