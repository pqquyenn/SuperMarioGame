#pragma once

#include "Level/LevelDefinition.h"
#include <string>
#include <vector>

class LevelDefinitionLoader {
public:
    bool load(
        const std::string& requestedPath,
        LevelDefinition& definition,
        std::vector<std::string>& errors) const;

    static std::string findManifest(const std::string& requestedPath);
};

class LevelValidator {
public:
    static std::vector<std::string> validate(
        const LevelDefinition& definition);
};
