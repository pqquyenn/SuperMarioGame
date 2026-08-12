#include "Level/LevelCameraController.h"

#include "Level/Camera.h"

void LevelCameraController::update(
    Camera& camera,
    const LevelDefinition& definition,
    const std::string& area,
    const sf::Vector2f& playerPosition) const {
    const float tileSize = definition.tileSize;
    const sf::Vector2f playerTile = playerPosition / tileSize;
    const CameraZoneDefinition* selected = nullptr;
    for (const auto& zone : definition.cameraZones) {
        if (zone.area != area) continue;
        if (!selected) selected = &zone;
        if (zone.boundsTiles.contains(playerTile)) {
            selected = &zone;
            break;
        }
    }
    if (!selected) {
        camera.update(playerPosition);
        return;
    }
    const float targetX = selected->followX
        ? playerPosition.x
        : (selected->boundsTiles.left + selected->boundsTiles.width * 0.5f) * tileSize;
    const float targetY = selected->followY
        ? playerPosition.y
        : selected->centerYTiles * tileSize;
    camera.setCenter(targetX, targetY);
}

bool LevelCameraController::usesDarkBackground(
    const LevelDefinition& definition,
    const std::string& area) const {
    for (const auto& zone : definition.cameraZones) {
        if (zone.area == area) return zone.darkBackground;
    }
    return false;
}
