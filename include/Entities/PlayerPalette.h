#pragma once

#include <SFML/Graphics.hpp>

enum class PlayerPalette {
    Primary,
    Secondary
};

// Produces the alternate PvP costume directly from the canonical atlas.
// Normal forms use orange/blue clothing and Fire uses white/cyan. Skin,
// outlines, transparent pixels, and atlas backgrounds remain unchanged.
sf::Image makeSecondaryPlayerPalette(const sf::Image& source);

// Uploads the alternate palette once so characters can use it like any other
// atlas. Keeping this outside Character avoids palette rules leaking into
// animation or player-state code.
bool loadSecondaryPlayerTexture(
    const sf::Texture& source,
    sf::Texture& destination
);
