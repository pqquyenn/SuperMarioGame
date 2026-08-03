#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <vector>

// Immutable frame data that can be shared by multiple SpriteAnimator objects.
struct AnimationClip {
    std::vector<sf::IntRect> frames;
    float frameDuration{0.1f};
    bool looping{true};

    bool isValid() const {
        return !frames.empty() && frameDuration > 0.f;
    }
};
