#pragma once

#include "Entities/Character.h"

// Textures remain owned by AssetManager (or another asset store) and must
// outlive Mario. The death texture is optional; Small Idle is its fallback.
struct MarioAnimationTextures {
    const sf::Texture* smallIdle{nullptr};
    const sf::Texture* smallRun1{nullptr};
    const sf::Texture* smallRun2{nullptr};
    const sf::Texture* smallRun3{nullptr};
    const sf::Texture* smallJump{nullptr};
    const sf::Texture* smallSlide{nullptr};
    const sf::Texture* smallDeath{nullptr};

    const sf::Texture* superIdle{nullptr};
    const sf::Texture* superRun1{nullptr};
    const sf::Texture* superRun2{nullptr};
    const sf::Texture* superRun3{nullptr};
    const sf::Texture* superJump{nullptr};
    const sf::Texture* superSlide{nullptr};

    bool isValid() const;
};

class Mario : public Character {
public:
    Mario(float x = 0.f, float y = 0.f);

    // Configures the currently available individual Small/Big Mario images.
    // Fire temporarily reuses Super visuals until Fire images are provided.
    bool setAnimationTextures(const MarioAnimationTextures& textures);
};
