#include "Level/PortalSystem.h"

#include "Entities/Character.h"
#include <algorithm>
#include <iostream>

bool PortalSystem::tryActivate(
    const LevelDefinition& definition,
    std::string& currentArea,
    Character& character,
    const sf::FloatRect& contactedTile,
    PortalActivation activation) const {
    const float tileSize = definition.tileSize;
    const sf::FloatRect contactedTiles{
        contactedTile.left / tileSize, contactedTile.top / tileSize,
        contactedTile.width / tileSize, contactedTile.height / tileSize};
    for (const auto& portal : definition.portals) {
        if (portal.sourceArea != currentArea || portal.activation != activation ||
            !portal.triggerTiles.intersects(contactedTiles)) continue;
        const auto target = std::find_if(
            definition.anchors.begin(), definition.anchors.end(),
            [&portal](const AnchorDefinition& anchor) {
                return anchor.id == portal.targetAnchor;
            });
        if (target == definition.anchors.end()) return false;
        currentArea = target->area;
        character.setPosition(target->tilePosition * tileSize);
        character.setVelocity(target->exitVelocity);
        std::cout << "[PortalSystem] " << portal.id << " -> "
                  << target->id << std::endl;
        return true;
    }
    return false;
}
