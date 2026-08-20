#include "Level/EntitySymbolCatalog.h"
#include "Level/LevelDefinitionLoader.h"
#include "PvP/PvPCombatResolver.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
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
    }
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
    LevelDefinition definition;
    std::vector<std::string> errors;
    LevelDefinitionLoader loader;
    expect(loader.load("1.1/1-1.txt", definition, errors),
           "legacy txt request resolves its level manifest");
    expect(definition.sourcePath.find("1-1.level") != std::string::npos,
           "legacy txt request loads the level manifest");
}

void testPvPContactClassification() {
    const PvPBodyFrame target{
        {10.f, 20.f, 16.f, 16.f},
        {10.f, 20.f, 16.f, 16.f},
        {0.f, 0.f}};
    const PvPBodyFrame trueStomp{
        {10.f, 2.f, 16.f, 16.f},
        {10.f, 8.f, 16.f, 16.f},
        {0.f, 120.f}};
    expect(PvPCombatResolver::classifyPlayerContact(trueStomp, target) ==
               PvPContactOutcome::PlayerOneStomps,
           "a downward top crossing is a P1 stomp");

    const PvPBodyFrame sideContact{
        {0.f, 20.f, 16.f, 16.f},
        {4.f, 20.f, 16.f, 16.f},
        {120.f, 0.f}};
    expect(PvPCombatResolver::classifyPlayerContact(sideContact, target) ==
               PvPContactOutcome::PushApart,
           "side contact is pushback, not a stomp");

    const PvPBodyFrame merelyHigher{
        {10.f, 2.f, 16.f, 16.f},
        {10.f, 8.f, 16.f, 16.f},
        {0.f, -40.f}};
    expect(PvPCombatResolver::classifyPlayerContact(merelyHigher, target) ==
               PvPContactOutcome::PushApart,
           "being higher without falling is not a stomp");

    const PvPBodyFrame separate{
        {100.f, 0.f, 16.f, 16.f},
        {100.f, 0.f, 16.f, 16.f},
        {0.f, 0.f}};
    expect(PvPCombatResolver::classifyPlayerContact(separate, target) ==
               PvPContactOutcome::None,
           "separate players have no contact outcome");
}

void testPvPFireSpawnAnchorsAreOpen() {
    LevelDefinition definition;
    std::vector<std::string> errors;
    LevelDefinitionLoader loader;
    expect(loader.load("pvp/super-arena.level", definition, errors),
           "PvP Super arena loads for anchor validation");

    std::ifstream terrain{definition.terrainPath};
    std::vector<std::string> rows;
    for (std::string row; std::getline(terrain, row);) {
        rows.push_back(row);
    }

    std::size_t fireSpawnCount = 0;
    for (const auto& anchor : definition.anchors) {
        if (anchor.id.find("fire_spawn_") != 0) {
            continue;
        }
        ++fireSpawnCount;
        const auto column = static_cast<std::size_t>(anchor.tilePosition.x);
        const auto row = static_cast<std::size_t>(anchor.tilePosition.y);
        expect(row < rows.size() && column < rows[row].size() &&
                   rows[row][column] == '-',
               anchor.id + " occupies an open terrain cell");
    }
    expect(fireSpawnCount >= 3,
           "PvP Super arena has multiple Fire Flower spawn choices");
}

} // namespace

int main() {
    testCatalog();
    testStage("1.1/1-1.level", 17, 0, 6, "1.2/1-2.level");
    testStage("1.2/1-2.level", 18, 4, 1, "1.3/1-3.level");
    testStage("1.3/1-3.level", 8, 4, 0, "1.4/1-4.level");
    testStage("1.4/1-4.level", 1, 0, 1, "");
    testStage("pvp/small-arena.level", 0, 0, 0, "");
    testStage("pvp/super-arena.level", 3, 0, 0, "");
    testInvalidManifest();
    testMalformedManifest();
    testMissingFiles();
    testTxtCompatibilityResolution();
    testPvPContactClassification();
    testPvPFireSpawnAnchorsAreOpen();
    std::cout << "SOLID-03 level definition tests passed\n";
    return 0;
}
