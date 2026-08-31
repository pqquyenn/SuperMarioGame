#include "PvP/PvPCameraPolicy.h"

#include <algorithm>

PvPCameraLayout PvPCameraPolicy::layout(
    sf::Vector2f minimumView,
    sf::Vector2f worldSize,
    sf::Vector2u windowSize) {
    PvPCameraLayout result;
    result.viewSize = {
        std::max(minimumView.x, worldSize.x),
        std::max(minimumView.y, worldSize.y)};
    result.viewport = {0.f, 0.f, 1.f, 1.f};

    if (windowSize.x == 0 || windowSize.y == 0 ||
        result.viewSize.x <= 0.f || result.viewSize.y <= 0.f) {
        return result;
    }

    const float viewAspect = result.viewSize.x / result.viewSize.y;
    const float windowAspect =
        static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    if (windowAspect > viewAspect) {
        const float width = viewAspect / windowAspect;
        result.viewport = {(1.f - width) * 0.5f, 0.f, width, 1.f};
    } else if (windowAspect < viewAspect) {
        const float height = windowAspect / viewAspect;
        result.viewport = {0.f, (1.f - height) * 0.5f, 1.f, height};
    }
    return result;
}
