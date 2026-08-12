#include "Level/TileCatalogLoader.h"

#include "Core/AssetManager.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::unordered_map<std::string, std::string> attributes(std::istringstream& stream) {
    std::unordered_map<std::string, std::string> values;
    std::string token;
    while (stream >> token) {
        const auto separator = token.find('=');
        if (separator != std::string::npos) {
            values[lower(token.substr(0, separator))] = token.substr(separator + 1);
        }
    }
    return values;
}

bool boolean(const std::unordered_map<std::string, std::string>& values,
             const std::string& key) {
    const auto found = values.find(key);
    if (found == values.end()) return false;
    const std::string value = lower(found->second);
    return value == "true" || value == "yes" || value == "1";
}

int integer(const std::unordered_map<std::string, std::string>& values,
            const std::string& key, int fallback = 0) {
    const auto found = values.find(key);
    if (found == values.end()) return fallback;
    try { return std::stoi(found->second); } catch (...) { return fallback; }
}

sf::Vector2f pair(const std::unordered_map<std::string, std::string>& values,
                  const std::string& key) {
    const auto found = values.find(key);
    if (found == values.end()) return {};
    const auto comma = found->second.find(',');
    if (comma == std::string::npos) return {};
    try {
        return {std::stof(found->second.substr(0, comma)),
                std::stof(found->second.substr(comma + 1))};
    } catch (...) { return {}; }
}

std::filesystem::path findCatalog() {
    const std::filesystem::path candidates[] = {
        "assets/config/tiles.catalog", "../assets/config/tiles.catalog",
        "../../assets/config/tiles.catalog", "../../../assets/config/tiles.catalog"
    };
    for (const auto& candidate : candidates) {
        std::error_code error;
        if (std::filesystem::is_regular_file(candidate, error)) return candidate;
    }
    return {};
}

void applyFlags(TileType& type,
                const std::unordered_map<std::string, std::string>& values) {
    type.isSolid = boolean(values, "solid");
    type.isWarpPipe = boolean(values, "warp");
    type.warpDirection = integer(values, "warp_direction");
    type.isQuestionBlock = boolean(values, "question");
    type.isCoinTile = boolean(values, "coin");
    type.isBrick = boolean(values, "brick");
    type.isAnimated = boolean(values, "animated");
    type.isHorizontalWarpPipe = boolean(values, "horizontal_warp");
    type.placementOffset = pair(values, "offset");
}
}

bool TileCatalogLoader::load(
    Registry& registry, std::vector<std::string>& errors) const {
    errors.clear();
    const auto path = findCatalog();
    if (path.empty()) {
        errors.push_back("assets/config/tiles.catalog was not found");
        return false;
    }
    std::ifstream input(path);
    AssetManager& assets = AssetManager::getInstance();
    std::string line;
    std::size_t lineNumber = 0;
    Registry loaded;
    while (std::getline(input, line)) {
        ++lineNumber;
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos || line[first] == '#') continue;
        std::istringstream stream(line.substr(first));
        std::string kind;
        std::string id;
        stream >> kind >> id;
        const auto values = attributes(stream);
        const auto textureName = values.find("texture");
        if (id.empty() || textureName == values.end()) {
            errors.push_back("line " + std::to_string(lineNumber) +
                             ": id and texture are required");
            continue;
        }
        if (!assets.hasTexture(textureName->second)) {
            errors.push_back("line " + std::to_string(lineNumber) +
                             ": texture is not loaded: " +
                             textureName->second);
            continue;
        }
        const int cellWidth = integer(values, "cell_width", 16);
        const int cellHeight = integer(values, "cell_height", 16);
        const int columns = kind == "grid" ? integer(values, "columns", 1) : 1;
        const int rows = kind == "grid" ? integer(values, "rows", 1) : 1;
        const int left = integer(values, "left");
        const int top = integer(values, "top");
        const int width = integer(values, "width", cellWidth);
        const int height = integer(values, "height", cellHeight);
        const int lastRowHeight = integer(values, "last_row_height", cellHeight);
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                const std::string key = kind == "grid"
                    ? id + std::to_string(row * columns + column + 1) : id;
                if (loaded.find(key) != loaded.end()) {
                    errors.push_back("duplicate tile id: " + key);
                    continue;
                }
                auto type = std::make_shared<TileType>();
                type->texture = &assets.getTexture(textureName->second);
                type->textureRect = kind == "grid"
                    ? sf::IntRect(left + column * cellWidth,
                                  top + row * cellHeight,
                                  cellWidth,
                                  row == rows - 1 ? lastRowHeight : cellHeight)
                    : sf::IntRect(left, top, width, height);
                applyFlags(*type, values);
                loaded.emplace(key, std::move(type));
            }
        }
    }
    if (!errors.empty()) return false;
    registry = std::move(loaded);
    return !registry.empty();
}
