#pragma once

#include "Entities/Character.h"

class Luigi : public Character {
private:
    // Luigi's source rows have an opaque blue background. Keep a private,
    // cleaned copy so the shared player atlas remains unchanged for Mario.
    sf::Texture animationTexture;

public:
    Luigi(float x = 0.f, float y = 0.f);

    void setTexture(
        const sf::Texture& texture,
        bool resetRect = false
    ) override;
};
