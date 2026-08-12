#pragma once

#include "Level/LevelDefinition.h"
#include <string>
#include <optional>
#include <vector>

class StageCatalog {
public:
    static std::vector<StageCatalogEntry> discover();
    static std::optional<StageCatalogEntry> findById(const std::string& id);
    static bool validateAll(std::vector<std::string>& errors);
};
