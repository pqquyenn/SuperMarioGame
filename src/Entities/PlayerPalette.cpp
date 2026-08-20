#include "Entities/PlayerPalette.h"

namespace {
constexpr unsigned MarioBandBottom = 61;
constexpr unsigned LuigiBandBottom = 121;
constexpr unsigned FireBandBottom = 216;

bool matches(const sf::Color& left, const sf::Color& right) {
    return left.r == right.r && left.g == right.g &&
           left.b == right.b && left.a == right.a;
}
}

sf::Image makeSecondaryPlayerPalette(const sf::Image& source) {
    sf::Image result = source;
    const sf::Vector2u size = result.getSize();

    const sf::Color marioRed{216, 40, 0};
    const sf::Color marioBrown{136, 112, 0};
    const sf::Color luigiGreen{0, 148, 0};
    const sf::Color luigiBrightGreen{0, 168, 0};
    const sf::Color luigiWhite{252, 252, 252};
    const sf::Color fireOrange{252, 152, 56};

    const sf::Color alternateOrange{252, 112, 16};
    const sf::Color alternateBlue{36, 84, 204};
    const sf::Color alternateCyan{0, 128, 136};
    const sf::Color alternateWhite{252, 252, 252};

    for (unsigned y = 0; y < size.y; ++y) {
        for (unsigned x = 0; x < size.x; ++x) {
            const sf::Color pixel = result.getPixel(x, y);
            sf::Color replacement = pixel;

            if (y <= MarioBandBottom) {
                if (matches(pixel, marioRed)) {
                    replacement = alternateOrange;
                } else if (matches(pixel, marioBrown)) {
                    replacement = alternateBlue;
                }
            } else if (y <= LuigiBandBottom) {
                if (matches(pixel, luigiGreen) ||
                    matches(pixel, luigiBrightGreen)) {
                    replacement = alternateOrange;
                } else if (matches(pixel, luigiWhite)) {
                    replacement = alternateBlue;
                }
            } else if (y <= FireBandBottom) {
                if (matches(pixel, marioRed)) {
                    replacement = alternateCyan;
                } else if (matches(pixel, fireOrange)) {
                    replacement = alternateWhite;
                }
            }

            if (!matches(pixel, replacement)) {
                replacement.a = pixel.a;
                result.setPixel(x, y, replacement);
            }
        }
    }

    return result;
}

bool loadSecondaryPlayerTexture(
    const sf::Texture& source,
    sf::Texture& destination
) {
    const sf::Image secondary =
        makeSecondaryPlayerPalette(source.copyToImage());
    if (!destination.loadFromImage(secondary)) {
        return false;
    }
    destination.setSmooth(false);
    return true;
}
