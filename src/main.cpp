#include "Core/Game.h"
#include "Level/StageCatalog.h"
#include "Level/Level.h"
#include "Entities/Mario.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--validate-levels") {
        std::vector<std::string> errors;
        if (!StageCatalog::validateAll(errors)) {
            for (const auto& error : errors) std::cerr << error << '\n';
            return 1;
        }
        std::cout << "All stage manifests are valid.\n";
        return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--smoke-levels") {
        std::vector<std::string> errors;
        if (!StageCatalog::validateAll(errors)) {
            for (const auto& error : errors) std::cerr << error << '\n';
            return 1;
        }
        for (const auto& stage : StageCatalog::discover()) {
            Level level;
            if (!level.loadLevel(stage.manifestPath) || !level.getEndPosition()) {
                std::cerr << "Could not build stage: " << stage.id << '\n';
                return 1;
            }
            if (level.getEnemies().size() !=
                    level.getDefinition().entities.size() ||
                level.getMovingPlatforms().size() !=
                    level.getDefinition().platforms.size()) {
                std::cerr << "Built object count mismatch: " << stage.id << '\n';
                return 1;
            }
            for (const sf::Vector2f size : {
                     sf::Vector2f{16.f, 16.f}, sf::Vector2f{16.f, 32.f}}) {
                const sf::Vector2f spawn = level.getStartPosition(size);
                if (!level.getTileMap().isSolidAt(
                        spawn.x + size.x * 0.5f, spawn.y + size.y + 1.f)) {
                    std::cerr << "Unsafe player start for " << stage.id << '\n';
                    return 1;
                }
            }
            if (!level.getDefinition().portals.empty()) {
                Mario testPlayer;
                const float tileSize = level.getDefinition().tileSize;
                for (const auto& portal : level.getDefinition().portals) {
                    const sf::FloatRect contact{
                        portal.triggerTiles.left * tileSize,
                        portal.triggerTiles.top * tileSize,
                        tileSize,
                        tileSize};
                    if (!level.tryActivatePortal(
                            testPlayer, contact, portal.activation)) {
                        std::cerr << "Portal chain failed at " << portal.id
                                  << " in " << stage.id << '\n';
                        return 1;
                    }
                }
            }
            std::cout << "Built " << stage.id << " ("
                      << level.getEnemies().size() << " enemies, "
                      << level.getMovingPlatforms().size() << " platforms)\n";
        }
        return 0;
    }
    Game game;
    game.run();
    return 0;
}

