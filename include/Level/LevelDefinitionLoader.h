#pragma once

#include "Level/LevelDefinition.h"

#include <string>
#include <vector>

class EntitySymbolCatalog;

class LevelDefinitionLoader {
public:
    explicit LevelDefinitionLoader(std::string entityCatalogPath = {});

    bool load(
        const std::string& requestedPath,
        LevelDefinition& definition,
        std::vector<std::string>& errors) const;

    static std::string findManifest(const std::string& requestedPath);
    static std::string findManifestForLegacyTerrain(
        const std::string& terrainPath);

private:
    std::string entityCatalogPath;
};

class LevelValidator {
public:
    static std::vector<std::string> validate(
        const LevelDefinition& definition);
};

