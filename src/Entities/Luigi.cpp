#include "Entities/Luigi.h"

#include <algorithm>

namespace {
CharacterProfile makeLuigiProfile() {
    CharacterProfile profile;
    profile.moveAcceleration = 850.f;
    profile.walkSpeed = 150.f;
    profile.runSpeed = 220.f;
    profile.jumpForce = 400.f;
    profile.jumpHoldGravityMultiplier = 0.4f;
    profile.jumpReleaseGravityMultiplier = 2.35f;
    profile.maxJumpHoldTime = 0.22f;
    return profile;
}
}

Luigi::Luigi(float x, float y)
    : Character{
          x,
          y,
          makeLuigiProfile(),
          makeLuigiAnimationProfile()
      } {}

std::string_view Luigi::getCharacterType() const {
    return "Luigi";
}

void Luigi::setTexture(const sf::Texture& texture, bool resetRect) {
    constexpr unsigned LuigiBandTop = 62;
    constexpr unsigned LuigiBandBottom = 121;
    const sf::Color darkBackground{27, 89, 153};
    const sf::Color lightBackground{147, 187, 236};

    sf::Image image = texture.copyToImage();
    const sf::Vector2u size = image.getSize();

    if (size.x == 0 || size.y <= LuigiBandTop) {
        Entity::setTexture(texture, resetRect);
        return;
    }

    const unsigned lastRow = std::min(LuigiBandBottom, size.y - 1);
    for (unsigned y = LuigiBandTop; y <= lastRow; ++y) {
        for (unsigned x = 0; x < size.x; ++x) {
            const sf::Color pixel = image.getPixel(x, y);
            const bool isDarkBackground =
                pixel.r == darkBackground.r &&
                pixel.g == darkBackground.g &&
                pixel.b == darkBackground.b;
            const bool isLightBackground =
                pixel.r == lightBackground.r &&
                pixel.g == lightBackground.g &&
                pixel.b == lightBackground.b;

            if (isDarkBackground || isLightBackground) {
                image.setPixel(x, y, sf::Color::Transparent);
            }
        }
    }

    if (!animationTexture.loadFromImage(image)) {
        Entity::setTexture(texture, resetRect);
        return;
    }

    animationTexture.setSmooth(false);
    Entity::setTexture(animationTexture, resetRect);
}
