#include "Level/EntitySymbolCatalog.h"
#include "Level/LevelDefinitionLoader.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::abort();
    }
}

void testCatalog() {
    EntitySymbolCatalog catalog;
    std::vector<std::string> errors;
    expect(catalog.load("assets/config/entities.catalog", errors),
           "entity catalog loads");
    expect(errors.empty(), "entity catalog has no errors");

    const EntitySymbolDefinition* goomba = catalog.resolve("E1");
    expect(goomba != nullptr, "E1 resolves");
    expect(goomba->kind == EntitySymbolKind::Enemy,
           "E1 is an enemy symbol");
    expect(goomba->factoryType == "Goomba", "E1 resolves to Goomba");

    const EntitySymbolDefinition* star = catalog.resolve("I5");
    expect(star != nullptr, "I5 resolves");
    expect(star->kind == EntitySymbolKind::Item,
           "I5 is an item symbol");
    expect(star->factoryType == "StarItem", "I5 resolves to StarItem");
    expect(catalog.resolve("E999") == nullptr, "unknown symbol fails");
}

void testStage(
    const std::string& path,
    std::size_t expectedEnemies,
    std::size_t expectedPlatforms,
    std::size_t expectedBlocks,
    const std::string& expectedNext) {
    LevelDefinitionLoader loader;
    LevelDefinition definition;
    std::vector<std::string> errors;
    const bool loaded = loader.load(path, definition, errors);
    if (!loaded) {
        for (const std::string& error : errors) {
            std::cerr << "  " << error << '\n';
        }
    }
    expect(loaded, path + " loads");
    expect(errors.empty(), path + " has no validation errors");
    expect(definition.playerStartTile.has_value(),
           path + " has a player start marker");
    expect(definition.entities.size() == expectedEnemies,
           path + " enemy count is preserved");
    expect(definition.platforms.size() == expectedPlatforms,
           path + " platform count is preserved");
    expect(definition.blockContents.size() == expectedBlocks,
           path + " block content count is preserved");
    expect(definition.nextStage == expectedNext,
           path + " next stage is loaded from the manifest");
    if (!expectedNext.empty()) {
        expect(expectedNext.find(".level") != std::string::npos,
               path + " transitions to a level manifest");
        expect(!LevelDefinitionLoader::findManifest(expectedNext).empty(),
               path + " next-stage manifest exists");
    }
    expect(definition.terrainPath.find(".txt") != std::string::npos,
           path + " resolves a TXT terrain file");
    expect(definition.terrainPath.find("assets/maps") != std::string::npos,
           path + " resolves terrain relative to the manifest");

    if (path == "1.2/1-2.level") {
        bool foundPiranha = false;
        for (const auto& entity : definition.entities) {
            foundPiranha = foundPiranha || entity.resolvedType == "PiranhaPlant";
        }
        expect(foundPiranha, "E7 resolves to PiranhaPlant");

        bool foundStarBrick = false;
        for (const auto& block : definition.blockContents) {
            if (block.area == "underground" &&
                block.tilePosition.x == 40 &&
                block.tilePosition.y == 19 &&
                block.content == "StarItem") {
                foundStarBrick = true;
                break;
            }
        }
        expect(foundStarBrick, "1-2 star brick resolves to StarItem");
    }

    if (path == "1.1/1-1.level") {
        const CameraZoneDefinition* bonusCamera = nullptr;
        for (const auto& camera : definition.cameraZones) {
            if (camera.id == "bonus_camera") {
                bonusCamera = &camera;
                break;
            }
        }
        expect(bonusCamera != nullptr, "1-1 bonus camera exists");
        expect(!bonusCamera->followX, "1-1 bonus camera does not follow X");
        expect(!bonusCamera->followY, "1-1 bonus camera does not follow Y");
        expect(std::abs(bonusCamera->centerYTiles - 8.f) < 0.01f,
               "1-1 bonus camera is vertically centered");
    }
}

void testOneTwoTerrainAndBackground() {
    std::ifstream terrain("assets/maps/1.2/1-2.txt");
    expect(terrain.is_open(), "1-2 terrain opens");

    std::string line;
    std::string targetRow;
    for (int row = 0; row <= 19 && std::getline(terrain, line); ++row) {
        if (row == 19) {
            targetRow = line;
        }
    }
    expect(targetRow.size() > 40 && targetRow[40] == 'r',
           "1-2 tile (40,19) is an underground brick");

    std::ifstream background("assets/maps/1.2/background.txt");
    expect(background.is_open(), "1-2 background opens");

    bool hasCloud = false;
    bool hasHill = false;
    bool hasDarkRow = false;
    int row = 0;
    while (std::getline(background, line)) {
        std::istringstream tokens(line);
        std::string token;
        while (tokens >> token) {
            hasCloud = hasCloud || token == "cl1" || token == "Cl1" ||
                       token == "cL1";
            hasHill = hasHill || token == "h1" || token == "H1";
            hasDarkRow = hasDarkRow || (row >= 15 && token == "*");
        }
        ++row;
    }
    expect(hasCloud, "1-2 background contains clouds");
    expect(hasHill, "1-2 background contains hills");
    expect(hasDarkRow, "1-2 background keeps dark lower rows");
}

void testInvalidManifest() {
    LevelDefinition definition;
    std::vector<std::string> errors;
    LevelDefinitionLoader loader;
    expect(!loader.load("tests/fixtures/invalid.level", definition, errors),
           "invalid manifest fails");
    expect(!errors.empty(), "invalid manifest reports errors");

    bool mentionsLine = false;
    bool mentionsSymbol = false;
    for (const std::string& error : errors) {
        mentionsLine = mentionsLine || error.find(":") != std::string::npos;
        mentionsSymbol = mentionsSymbol || error.find("E999") != std::string::npos;
    }
    expect(mentionsLine, "invalid manifest error contains source location");
    expect(mentionsSymbol, "invalid manifest error names unknown symbol");
}

void testMalformedManifest() {
    LevelDefinition definition;
    std::vector<std::string> errors;
    LevelDefinitionLoader loader;
    expect(!loader.load("tests/fixtures/malformed.level", definition, errors),
           "malformed manifest fails");

    const auto contains = [&errors](const std::string& needle) {
        for (const std::string& error : errors) {
            if (error.find(needle) != std::string::npos) {
                return true;
            }
        }
        return false;
    };
    expect(contains("missing-background.txt"),
           "missing background is reported");
    expect(contains("duplicate entity id"),
           "duplicate IDs are reported");
    expect(contains("unknown entity symbol 'E999'"),
           "unknown entity alias is reported");
    expect(contains("unknown block content symbol 'I999'"),
           "unknown block alias is reported");
    expect(contains("platform movement bounds are outside terrain"),
           "platform bounds are validated");
    expect(contains("portal target anchor does not exist"),
           "portal target is validated");
    expect(contains("camera follow_x must be boolean"),
           "camera booleans are validated");
}

void testMissingFiles() {
    LevelDefinition definition;
    std::vector<std::string> errors;
    LevelDefinitionLoader loader;
    expect(!loader.load("tests/fixtures/missing-files.level", definition, errors),
           "missing files manifest fails");
    expect(!errors.empty(), "missing files reports an error");
    expect(errors.front().find("missing-terrain.txt") != std::string::npos,
           "missing terrain is reported");
}

void testTxtCompatibilityResolution() {
    const std::string manifest =
        LevelDefinitionLoader::findManifestForLegacyTerrain("1.1/1-1.txt");
    expect(!manifest.empty(),
           "explicit legacy adapter resolves a terrain request");
    expect(manifest.find("1-1.level") != std::string::npos,
           "legacy adapter resolves the related level manifest");

    LevelDefinition definition;
    std::vector<std::string> errors;
    LevelDefinitionLoader loader;
    expect(loader.load(manifest, definition, errors),
           "resolved legacy manifest remains loadable");
    expect(definition.sourcePath.find("1-1.level") != std::string::npos,
           "legacy adapter never loads raw terrain as a stage");

    errors.clear();
    expect(!loader.load("1.1/1-1.txt", definition, errors),
           "normal manifest loader rejects txt stage entry");
}

void testNavigationValidation() {
    LevelDefinition definition;
    std::vector<std::string> errors;
    LevelDefinitionLoader loader;
    expect(!loader.load(
               "tests/fixtures/invalid-navigation.level",
               definition,
               errors),
           "invalid navigation manifest fails");

    const auto contains = [&errors](const std::string& needle) {
        for (const std::string& error : errors) {
            if (error.find(needle) != std::string::npos) {
                return true;
            }
        }
        return false;
    };
    expect(contains("next_stage manifest does not exist"),
           "missing next-stage manifest is reported");
    expect(contains("initial_area references an area without a camera zone"),
           "initial area without camera is reported");
    expect(contains("anchor orphan_anchor references an area without a camera zone"),
           "anchor area without camera is reported");
}

} // namespace

int main() {
    testCatalog();
    testStage("1.1/1-1.level", 17, 0, 6, "1.2/1-2.level");
    testStage("1.2/1-2.level", 18, 4, 2, "1.3/1-3.level");
    testStage("1.3/1-3.level", 8, 4, 0, "1.4/1-4.level");
    testStage("1.4/1-4.level", 1, 0, 1, "");
    testInvalidManifest();
    testMalformedManifest();
    testMissingFiles();
    testTxtCompatibilityResolution();
    testNavigationValidation();
    std::cout << "SOLID-03 level definition tests passed\n";
    return 0;
}
