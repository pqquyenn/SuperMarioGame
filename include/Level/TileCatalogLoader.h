#pragma once

#include "Level/TileType.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TileCatalogLoader {
public:
    using Registry =
        std::unordered_map<std::string, std::shared_ptr<TileType>>;

    bool load(Registry& registry, std::vector<std::string>& errors) const;
};
