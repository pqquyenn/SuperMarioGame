#pragma once

#include "Level/LevelDefinition.h"
#include <string>

class Camera;

class LevelCameraController {
public:
    void update(
        Camera& camera,
        const LevelDefinition& definition,
        const std::string& area,
        const sf::Vector2f& playerPosition) const;
    bool usesDarkBackground(
        const LevelDefinition& definition,
        const std::string& area) const;
};
