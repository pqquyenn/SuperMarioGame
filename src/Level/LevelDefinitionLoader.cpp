#include "Level/LevelDefinitionLoader.h"

#include "Level/EntitySymbolCatalog.h"

#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace {

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string lower(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

bool parseFloat(const std::string& raw, float& result) {
    const std::string value = trim(raw);
    if (value.empty()) {
        return false;
    }

    try {
        std::size_t used = 0;
        result = std::stof(value, &used);
        return used == value.size();
    } catch (...) {
        return false;
    }
}

bool parseInt(const std::string& raw, int& result) {
    const std::string value = trim(raw);
    if (value.empty()) {
        return false;
    }

    try {
        std::size_t used = 0;
        result = std::stoi(value, &used);
        return used == value.size();
    } catch (...) {
        return false;
    }
}

bool parsePair(const std::string& raw, sf::Vector2f& result) {
    const std::string value = trim(raw);
    const std::size_t comma = value.find(',');
    if (comma == std::string::npos ||
        value.find(',', comma + 1) != std::string::npos) {
        return false;
    }

    return parseFloat(value.substr(0, comma), result.x) &&
           parseFloat(value.substr(comma + 1), result.y);
}

bool parseIntPair(const std::string& raw, sf::Vector2i& result) {
    const std::string value = trim(raw);
    const std::size_t comma = value.find(',');
    if (comma == std::string::npos ||
        value.find(',', comma + 1) != std::string::npos) {
        return false;
    }

    return parseInt(value.substr(0, comma), result.x) &&
           parseInt(value.substr(comma + 1), result.y);
}

bool parseRect(const std::string& raw, sf::FloatRect& result) {
    std::istringstream stream(trim(raw));
    std::string part;
    float values[4]{};
    for (float& value : values) {
        if (!std::getline(stream, part, ',') || !parseFloat(part, value)) {
            return false;
        }
    }

    if (std::getline(stream, part, ',')) {
        return false;
    }

    result = {values[0], values[1], values[2], values[3]};
    return true;
}

bool parseBool(const std::string& raw, bool& result) {
    const std::string value = lower(trim(raw));
    if (value == "true" || value == "yes" || value == "1") {
        result = true;
        return true;
    }
    if (value == "false" || value == "no" || value == "0") {
        result = false;
        return true;
    }
    return false;
}

std::unordered_map<std::string, std::string> attributes(
    const std::string& line) {
    std::unordered_map<std::string, std::string> result;
    std::istringstream stream(line);
    std::string token;
    stream >> token;
    while (stream >> token) {
        const std::size_t separator = token.find('=');
        if (separator == std::string::npos) {
            continue;
        }
        result[lower(token.substr(0, separator))] =
            token.substr(separator + 1);
    }
    return result;
}

std::string valueOr(
    const std::unordered_map<std::string, std::string>& values,
    const std::string& key,
    const std::string& fallback = {}) {
    const auto found = values.find(key);
    return found == values.end() ? fallback : found->second;
}

void addParseError(
    std::vector<std::string>& errors,
    const std::string& filepath,
    std::size_t line,
    const std::string& message) {
    errors.push_back(
        filepath + ":" + std::to_string(line) + ": " + message);
}

std::vector<std::filesystem::path> pathCandidates(
    const std::filesystem::path& requested,
    const std::string& assetFolder) {
    std::vector<std::filesystem::path> candidates;
    candidates.push_back(requested);
    candidates.push_back(std::filesystem::path(assetFolder) / requested);
    candidates.push_back(std::filesystem::path("../") / assetFolder / requested);
    candidates.push_back(std::filesystem::path("../../") / assetFolder / requested);
    candidates.push_back(std::filesystem::path("../../../") / assetFolder / requested);

    const std::filesystem::path filename = requested.filename();
    if (!filename.empty()) {
        for (const char* directory : {"1.1", "1.2", "1.3"}) {
            candidates.push_back(
                std::filesystem::path(assetFolder) / directory / filename);
            candidates.push_back(
                std::filesystem::path("../") / assetFolder / directory / filename);
            candidates.push_back(
                std::filesystem::path("../../") / assetFolder / directory / filename);
        }
    }

    return candidates;
}

std::string findExistingPath(
    const std::filesystem::path& requested,
    const std::string& assetFolder) {
    std::error_code error;
    for (const auto& candidate : pathCandidates(requested, assetFolder)) {
        if (std::filesystem::is_regular_file(candidate, error)) {
            return candidate.lexically_normal().generic_string();
        }
    }
    return {};
}

std::filesystem::path resolveRelative(
    const std::filesystem::path& manifest,
    const std::string& reference) {
    const std::filesystem::path path(reference);
    if (path.is_absolute()) {
        return path;
    }
    return manifest.parent_path() / path;
}

struct TerrainInfo {
    int width{0};
    int height{0};
    int startMarkers{0};
    sf::Vector2f firstStart{0.f, 0.f};
};

bool isDimensionHeader(const std::string& line) {
    std::istringstream stream(line);
    int rows = 0;
    int columns = 0;
    std::string extra;
    return (stream >> rows >> columns) && !(stream >> extra) &&
           rows > 0 && columns > 0;
}

bool readTerrainInfo(
    const std::string& filepath,
    TerrainInfo& info,
    std::vector<std::string>& errors) {
    std::ifstream input(filepath);
    if (!input) {
        errors.push_back("could not open terrain file: " + filepath);
        return false;
    }

    std::string line;
    bool firstDataLine = true;
    int row = 0;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            continue;
        }

        if (firstDataLine && isDimensionHeader(trimmed)) {
            firstDataLine = false;
            continue;
        }
        firstDataLine = false;

        int width = 0;
        if (line.find_first_of(" \t") != std::string::npos) {
            std::istringstream stream(line);
            std::string token;
            int column = 0;
            while (stream >> token) {
                if (token == "@") {
                    ++info.startMarkers;
                    if (info.startMarkers == 1) {
                        info.firstStart = {
                            static_cast<float>(column),
                            static_cast<float>(row)};
                    }
                }
                ++column;
            }
            width = column;
        } else {
            width = static_cast<int>(line.size());
            for (int column = 0; column < width; ++column) {
                if (line[static_cast<std::size_t>(column)] == '@') {
                    ++info.startMarkers;
                    if (info.startMarkers == 1) {
                        info.firstStart = {
                            static_cast<float>(column),
                            static_cast<float>(row)};
                    }
                }
            }
        }

        info.width = std::max(info.width, width);
        ++row;
    }

    info.height = row;
    if (info.width <= 0 || info.height <= 0) {
        errors.push_back("terrain file contains no map rows: " + filepath);
        return false;
    }
    return true;
}

bool insideMap(const sf::Vector2f& point, const TerrainInfo& terrain) {
    return point.x >= 0.f && point.y >= 0.f &&
           point.x < static_cast<float>(terrain.width) &&
           point.y < static_cast<float>(terrain.height);
}

bool insideMap(const sf::Vector2i& point, const TerrainInfo& terrain) {
    return point.x >= 0 && point.y >= 0 && point.x < terrain.width &&
           point.y < terrain.height;
}

bool insideMap(const sf::FloatRect& rect, const TerrainInfo& terrain) {
    return rect.left >= 0.f && rect.top >= 0.f && rect.width > 0.f &&
           rect.height > 0.f &&
           rect.left + rect.width <= static_cast<float>(terrain.width) &&
           rect.top + rect.height <= static_cast<float>(terrain.height);
}

std::string sourcePrefix(const LevelDefinition& definition) {
    return definition.sourcePath.empty() ? "level" : definition.sourcePath;
}

void addValidationError(
    std::vector<std::string>& errors,
    const LevelDefinition& definition,
    const std::string& message) {
    errors.push_back(sourcePrefix(definition) + ": " + message);
}

template <typename Definition>
void validateUniqueIds(
    const std::vector<Definition>& definitions,
    const LevelDefinition& level,
    const std::string& label,
    std::vector<std::string>& errors) {
    std::unordered_set<std::string> ids;
    for (const auto& definition : definitions) {
        if (definition.id.empty()) {
            addValidationError(errors, level, label + " id is required");
            continue;
        }
        if (!ids.insert(definition.id).second) {
            addValidationError(
                errors,
                level,
                "duplicate " + label + " id: " + definition.id);
        }
    }
}

} // namespace

LevelDefinitionLoader::LevelDefinitionLoader(std::string catalogPath)
    : entityCatalogPath(std::move(catalogPath)) {}

std::string LevelDefinitionLoader::findManifest(
    const std::string& requestedPath) {
    std::filesystem::path requested(requestedPath);
    if (lower(requested.extension().string()) != ".level") {
        return {};
    }
    return findExistingPath(requested, "assets/maps");
}

std::string LevelDefinitionLoader::findManifestForLegacyTerrain(
    const std::string& terrainPath) {
    std::filesystem::path requested(terrainPath);
    if (lower(requested.extension().string()) != ".txt") {
        return {};
    }
    requested.replace_extension(".level");
    return findExistingPath(requested, "assets/maps");
}

bool LevelDefinitionLoader::load(
    const std::string& requestedPath,
    LevelDefinition& definition,
    std::vector<std::string>& errors) const {
    errors.clear();
    definition = {};

    const std::string manifestPath = findManifest(requestedPath);
    if (manifestPath.empty()) {
        errors.push_back("stage manifest not found: " + requestedPath);
        return false;
    }

    std::ifstream input(manifestPath);
    if (!input) {
        errors.push_back("could not open stage manifest: " + manifestPath);
        return false;
    }

    std::string catalogPath = entityCatalogPath;
    if (catalogPath.empty()) {
        catalogPath = findExistingPath(
            std::filesystem::path("entities.catalog"), "assets/config");
    } else if (!std::filesystem::is_regular_file(catalogPath)) {
        catalogPath = findExistingPath(
            std::filesystem::path(catalogPath), "assets/config");
    }

    EntitySymbolCatalog catalog;
    if (catalogPath.empty() || !catalog.load(catalogPath, errors)) {
        if (catalogPath.empty()) {
            errors.push_back("entity symbol catalog not found");
        }
        return false;
    }

    definition.sourcePath = manifestPath;
    std::string section;
    std::string raw;
    std::size_t lineNumber = 0;
    while (std::getline(input, raw)) {
        ++lineNumber;
        std::string line = trim(raw);
        if (line.empty() || line.front() == '#') {
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            section = lower(trim(line.substr(1, line.size() - 2)));
            continue;
        }

        if (section == "stage" || section == "rules") {
            const std::size_t separator = line.find('=');
            if (separator == std::string::npos) {
                addParseError(errors, manifestPath, lineNumber,
                              "expected key=value");
                continue;
            }

            const std::string key = lower(trim(line.substr(0, separator)));
            const std::string value = trim(line.substr(separator + 1));
            if (section == "stage") {
                if (key == "version") {
                    if (!parseInt(value, definition.version)) {
                        addParseError(errors, manifestPath, lineNumber,
                                      "version must be an integer");
                    }
                } else if (key == "id") {
                    definition.id = value;
                } else if (key == "name") {
                    definition.name = value;
                } else if (key == "terrain") {
                    definition.terrainPath = value;
                } else if (key == "background") {
                    definition.backgroundPath = value;
                } else if (key == "next_stage") {
                    definition.nextStage = value;
                } else if (key == "initial_area") {
                    definition.initialArea = value;
                } else if (key == "time_limit") {
                    if (!parseInt(value, definition.timeLimit)) {
                        addParseError(errors, manifestPath, lineNumber,
                                      "time_limit must be an integer");
                    }
                } else if (key == "tile_size") {
                    if (!parseFloat(value, definition.tileSize)) {
                        addParseError(errors, manifestPath, lineNumber,
                                      "tile_size must be numeric");
                    }
                } else {
                    addParseError(errors, manifestPath, lineNumber,
                                  "unknown stage key: " + key);
                }
            } else {
                bool parsed = true;
                if (key == "kill_plane_tile") {
                    parsed = parseFloat(value, definition.rules.killPlaneTile);
                } else if (key == "left_boundary_tile") {
                    parsed = parseFloat(value, definition.rules.leftBoundaryTile);
                } else if (key == "right_boundary_tile") {
                    parsed = parseFloat(value, definition.rules.rightBoundaryTile);
                } else if (key == "enemy_void_margin_tiles") {
                    parsed = parseFloat(
                        value, definition.rules.enemyVoidMarginTiles);
                } else {
                    addParseError(errors, manifestPath, lineNumber,
                                  "unknown rules key: " + key);
                    parsed = true;
                }
                if (!parsed) {
                    addParseError(errors, manifestPath, lineNumber,
                                  key + " must be numeric");
                }
            }
            continue;
        }

        if (section.empty()) {
            addParseError(errors, manifestPath, lineNumber,
                          "record appears before a section");
            continue;
        }

        const std::size_t firstSpace = line.find_first_of(" \t");
        const std::string kind = lower(
            line.substr(0, firstSpace == std::string::npos
                              ? line.size()
                              : firstSpace));
        const auto values = attributes(line);

        auto parseEntity = [&](bool item) {
            EntitySpawnDefinition entity;
            entity.id = valueOr(values, "id");
            entity.area = valueOr(values, "area", definition.initialArea);
            entity.symbol = valueOr(values, "symbol");
            const std::string explicitType = valueOr(values, "type");

            if (entity.symbol.empty() && explicitType.empty()) {
                addParseError(errors, manifestPath, lineNumber,
                              "entity requires symbol=...");
            } else if (!entity.symbol.empty()) {
                const EntitySymbolDefinition* symbol =
                    catalog.resolve(entity.symbol);
                if (!symbol) {
                    addParseError(
                        errors,
                        manifestPath,
                        lineNumber,
                        "unknown entity symbol '" + entity.symbol + "'");
                } else if ((item && symbol->kind != EntitySymbolKind::Item) ||
                           (!item && symbol->kind != EntitySymbolKind::Enemy)) {
                    addParseError(
                        errors,
                        manifestPath,
                        lineNumber,
                        "symbol '" + entity.symbol +
                            "' has the wrong entity kind");
                } else {
                    entity.resolvedType = symbol->factoryType;
                }
            } else {
                entity.symbol = explicitType;
                entity.resolvedType = explicitType;
            }

            if (!parsePair(valueOr(values, "tile"), entity.tilePosition)) {
                addParseError(errors, manifestPath, lineNumber,
                              "entity requires tile=x,y");
            }
            if (!parseInt(valueOr(values, "direction", "-1"),
                          entity.direction)) {
                addParseError(errors, manifestPath, lineNumber,
                              "direction must be an integer");
            }
            if (values.find("speed") != values.end() &&
                !parseFloat(valueOr(values, "speed"), entity.speed)) {
                addParseError(errors, manifestPath, lineNumber,
                              "speed must be numeric");
            }
            const auto parseOptionalSeconds = [&](const char* key,
                                                   float& destination) {
                if (values.find(key) != values.end() &&
                    !parseFloat(valueOr(values, key), destination)) {
                    addParseError(errors, manifestPath, lineNumber,
                                  std::string{key} + " must be numeric");
                }
            };
            if (!item) {
                parseOptionalSeconds("visible_time", entity.visibleDuration);
                parseOptionalSeconds("hidden_time", entity.hiddenDuration);
                parseOptionalSeconds("initial_delay", entity.initialDelay);
            }

            (item ? definition.items : definition.entities).push_back(entity);
        };

        if (section == "entities" && kind == "entity") {
            parseEntity(false);
        } else if (section == "items" && kind == "item") {
            parseEntity(true);
        } else if (section == "platforms" && kind == "platform") {
            PlatformDefinition platform;
            platform.id = valueOr(values, "id");
            platform.area = valueOr(values, "area", definition.initialArea);
            if (!parsePair(valueOr(values, "tile"), platform.tilePosition)) {
                addParseError(errors, manifestPath, lineNumber,
                              "platform requires tile=x,y");
            }
            if (!parseFloat(valueOr(values, "width", "3"),
                            platform.widthTiles)) {
                addParseError(errors, manifestPath, lineNumber,
                              "platform width must be numeric");
            }
            sf::Vector2f bounds;
            if (!parsePair(valueOr(values, "bounds"), bounds)) {
                addParseError(errors, manifestPath, lineNumber,
                              "platform requires bounds=min,max");
            } else {
                platform.minimumTile = bounds.x;
                platform.maximumTile = bounds.y;
            }
            if (!parseFloat(valueOr(values, "speed", "50"),
                            platform.speed)) {
                addParseError(errors, manifestPath, lineNumber,
                              "platform speed must be numeric");
            }

            const std::string motion = lower(valueOr(values, "motion"));
            if (motion == "horizontal" || motion == "oscillate_horizontal") {
                platform.motion = PlatformMotion::OscillateHorizontal;
            } else if (motion == "loop_down") {
                platform.motion = PlatformMotion::LoopDown;
            } else if (motion == "loop_up") {
                platform.motion = PlatformMotion::LoopUp;
            } else if (motion.empty() || motion == "vertical" ||
                       motion == "oscillate_vertical") {
                platform.motion = PlatformMotion::OscillateVertical;
            } else {
                addParseError(errors, manifestPath, lineNumber,
                              "unknown platform motion: " + motion);
            }
            definition.platforms.push_back(platform);
        } else if (section == "blocks" && kind == "block") {
            BlockContentDefinition block;
            block.area = valueOr(values, "area", definition.initialArea);
            if (!parseIntPair(valueOr(values, "tile"), block.tilePosition)) {
                addParseError(errors, manifestPath, lineNumber,
                              "block requires tile=x,y");
            }
            block.content = valueOr(values, "content", "Coin");

            if (block.content != "PowerupByForm") {
                if (const EntitySymbolDefinition* symbol =
                        catalog.resolve(block.content)) {
                    if (symbol->kind != EntitySymbolKind::Item) {
                        addParseError(errors, manifestPath, lineNumber,
                                      "block content symbol is not an item: " +
                                          block.content);
                    } else {
                        block.content = symbol->factoryType;
                    }
                } else if (block.content != "Coin" &&
                           block.content != "Mushroom" &&
                           block.content != "1UpMushroom" &&
                           block.content != "FireFlower" &&
                           block.content != "StarItem") {
                    addParseError(errors, manifestPath, lineNumber,
                                  "unknown block content symbol '" +
                                      block.content + "'");
                }
            }
            definition.blockContents.push_back(block);
        } else if (section == "anchors" && kind == "anchor") {
            AnchorDefinition anchor;
            anchor.id = valueOr(values, "id");
            anchor.area = valueOr(values, "area", definition.initialArea);
            if (!parsePair(valueOr(values, "tile"), anchor.tilePosition)) {
                addParseError(errors, manifestPath, lineNumber,
                              "anchor requires tile=x,y");
            }
            if (!parsePair(valueOr(values, "velocity", "0,0"),
                           anchor.exitVelocity)) {
                addParseError(errors, manifestPath, lineNumber,
                              "anchor velocity must be x,y");
            }
            definition.anchors.push_back(anchor);
        } else if (section == "portals" && kind == "portal") {
            PortalDefinition portal;
            portal.id = valueOr(values, "id");
            portal.sourceArea =
                valueOr(values, "area", definition.initialArea);
            portal.targetAnchor = valueOr(values, "target");
            const std::string activation =
                lower(valueOr(values, "activation", "down"));
            if (activation == "right") {
                portal.activation = PortalActivation::Right;
            } else if (activation == "down") {
                portal.activation = PortalActivation::Down;
            } else {
                addParseError(errors, manifestPath, lineNumber,
                              "unknown portal activation: " + activation);
            }
            if (!parseRect(valueOr(values, "trigger"),
                           portal.triggerTiles)) {
                addParseError(errors, manifestPath, lineNumber,
                              "portal requires trigger=x,y,w,h");
            }
            definition.portals.push_back(portal);
        } else if (section == "camera_zones" && kind == "camera") {
            CameraZoneDefinition camera;
            camera.id = valueOr(values, "id");
            camera.area = valueOr(values, "area", definition.initialArea);
            if (!parseRect(valueOr(values, "bounds"), camera.boundsTiles)) {
                addParseError(errors, manifestPath, lineNumber,
                              "camera requires bounds=x,y,w,h");
            }
            const auto followX = values.find("follow_x");
            if (followX == values.end()) {
                camera.followX = true;
            } else if (!parseBool(followX->second, camera.followX)) {
                addParseError(errors, manifestPath, lineNumber,
                              "camera follow_x must be boolean");
            }
            const auto followY = values.find("follow_y");
            if (followY == values.end()) {
                camera.followY = false;
            } else if (!parseBool(followY->second, camera.followY)) {
                addParseError(errors, manifestPath, lineNumber,
                              "camera follow_y must be boolean");
            }
            if (!parseFloat(valueOr(values, "center_y", "7"),
                            camera.centerYTiles)) {
                addParseError(errors, manifestPath, lineNumber,
                              "camera center_y must be numeric");
            }
            const auto dark = values.find("dark");
            if (dark == values.end()) {
                camera.darkBackground = false;
            } else if (!parseBool(dark->second, camera.darkBackground)) {
                addParseError(errors, manifestPath, lineNumber,
                              "camera dark must be boolean");
            }
            definition.cameraZones.push_back(camera);
        } else {
            addParseError(errors, manifestPath, lineNumber,
                          "unsupported record in section [" + section + "]");
        }
    }

    const std::filesystem::path manifest(manifestPath);
    if (!definition.terrainPath.empty()) {
        definition.terrainPath = resolveRelative(
            manifest, definition.terrainPath).lexically_normal().generic_string();
    }
    if (!definition.backgroundPath.empty()) {
        definition.backgroundPath = resolveRelative(
            manifest, definition.backgroundPath).lexically_normal().generic_string();
    }

    TerrainInfo terrain;
    if (!definition.terrainPath.empty() &&
        readTerrainInfo(definition.terrainPath, terrain, errors)) {
        if (terrain.startMarkers != 1) {
            errors.push_back(
                definition.terrainPath + ": expected exactly one '@' player "
                "start marker, found " + std::to_string(terrain.startMarkers));
        } else {
            definition.playerStartTile = terrain.firstStart;
        }
    }

    const auto validationErrors = LevelValidator::validate(definition);
    errors.insert(errors.end(), validationErrors.begin(), validationErrors.end());
    return errors.empty();
}

std::vector<std::string> LevelValidator::validate(
    const LevelDefinition& definition) {
    std::vector<std::string> errors;
    if (definition.version != 1) {
        addValidationError(errors, definition, "unsupported version");
    }
    if (definition.id.empty()) {
        addValidationError(errors, definition, "stage id is required");
    }
    if (definition.name.empty()) {
        addValidationError(errors, definition, "stage name is required");
    }
    if (std::abs(definition.tileSize - 16.f) > 0.001f) {
        addValidationError(errors, definition,
                           "tile_size must be 16 for the current TileMap");
    }
    if (definition.timeLimit <= 0) {
        addValidationError(errors, definition, "time_limit must be positive");
    }
    if (definition.rules.enemyVoidMarginTiles < 0.f) {
        addValidationError(errors, definition,
                           "enemy_void_margin_tiles cannot be negative");
    }
    if (!definition.nextStage.empty()) {
        const std::filesystem::path nextStage(definition.nextStage);
        if (lower(nextStage.extension().string()) != ".level") {
            addValidationError(errors, definition,
                               "next_stage must reference a .level manifest");
        } else if (LevelDefinitionLoader::findManifest(
                       definition.nextStage).empty()) {
            addValidationError(errors, definition,
                               "next_stage manifest does not exist: " +
                                   definition.nextStage);
        }
    }
    if (definition.terrainPath.empty()) {
        addValidationError(errors, definition, "terrain file is required");
        return errors;
    }
    if (!std::filesystem::is_regular_file(definition.terrainPath)) {
        addValidationError(errors, definition,
                           "terrain file does not exist: " +
                               definition.terrainPath);
        return errors;
    }
    if (!definition.backgroundPath.empty() &&
        !std::filesystem::is_regular_file(definition.backgroundPath)) {
        addValidationError(errors, definition,
                           "background file does not exist: " +
                               definition.backgroundPath);
    }

    TerrainInfo terrain;
    std::vector<std::string> terrainErrors;
    if (!readTerrainInfo(definition.terrainPath, terrain, terrainErrors)) {
        errors.insert(errors.end(), terrainErrors.begin(), terrainErrors.end());
        return errors;
    }

    if (!definition.playerStartTile) {
        addValidationError(errors, definition,
                           "player start is missing from terrain");
    } else if (!insideMap(*definition.playerStartTile, terrain)) {
        addValidationError(errors, definition,
                           "player start is outside terrain");
    }

    validateUniqueIds(definition.entities, definition, "entity", errors);
    validateUniqueIds(definition.items, definition, "item", errors);
    validateUniqueIds(definition.platforms, definition, "platform", errors);
    validateUniqueIds(definition.anchors, definition, "anchor", errors);
    validateUniqueIds(definition.portals, definition, "portal", errors);
    validateUniqueIds(definition.cameraZones, definition, "camera", errors);

    std::unordered_set<std::string> cameraAreas;
    for (const auto& camera : definition.cameraZones) {
        if (!camera.area.empty()) {
            cameraAreas.insert(camera.area);
        }
    }
    const auto validateArea = [&](const std::string& area,
                                  const std::string& owner) {
        if (!area.empty() && cameraAreas.find(area) == cameraAreas.end()) {
            addValidationError(errors, definition,
                               owner + " references an area without a camera "
                               "zone: " + area);
        }
    };
    if (definition.initialArea.empty()) {
        addValidationError(errors, definition, "initial_area cannot be empty");
    } else {
        validateArea(definition.initialArea, "initial_area");
    }

    for (const auto& entity : definition.entities) {
        if (entity.resolvedType.empty()) {
            addValidationError(errors, definition,
                               "entity has no resolved factory type: " +
                                   entity.id);
        }
        if (!insideMap(entity.tilePosition, terrain)) {
            addValidationError(errors, definition,
                               "entity is outside terrain: " + entity.id);
        }
        if (entity.direction != -1 && entity.direction != 1 &&
            entity.direction != 0) {
            addValidationError(errors, definition,
                               "entity direction must be -1, 0, or 1: " +
                                   entity.id);
        }
        if (entity.area.empty()) {
            addValidationError(errors, definition,
                               "entity area cannot be empty: " + entity.id);
        }
        const auto validOptionalDuration = [](float duration) {
            return duration == -1.f || duration >= 0.f;
        };
        if (!validOptionalDuration(entity.visibleDuration) ||
            !validOptionalDuration(entity.hiddenDuration) ||
            !validOptionalDuration(entity.initialDelay)) {
            addValidationError(
                errors, definition,
                "entity cycle timing cannot be negative: " + entity.id);
        }
        validateArea(entity.area, "entity " + entity.id);
    }

    for (const auto& item : definition.items) {
        if (item.resolvedType.empty()) {
            addValidationError(errors, definition,
                               "item has no resolved factory type: " +
                                   item.id);
        }
        if (!insideMap(item.tilePosition, terrain)) {
            addValidationError(errors, definition,
                               "item is outside terrain: " + item.id);
        }
        if (item.area.empty()) {
            addValidationError(errors, definition,
                               "item area cannot be empty: " + item.id);
        }
        validateArea(item.area, "item " + item.id);
    }

    for (const auto& platform : definition.platforms) {
        if (!insideMap(platform.tilePosition, terrain)) {
            addValidationError(errors, definition,
                               "platform is outside terrain: " + platform.id);
        }
        if (platform.widthTiles <= 0.f) {
            addValidationError(errors, definition,
                               "platform width must be positive: " +
                                   platform.id);
        }
        if (platform.maximumTile < platform.minimumTile) {
            addValidationError(errors, definition,
                               "platform bounds are reversed: " + platform.id);
        }
        if (platform.speed < 0.f) {
            addValidationError(errors, definition,
                               "platform speed cannot be negative: " +
                                   platform.id);
        }
        if (platform.area.empty()) {
            addValidationError(errors, definition,
                               "platform area cannot be empty: " +
                                   platform.id);
        }
        validateArea(platform.area, "platform " + platform.id);
        if (platform.tilePosition.x + platform.widthTiles >
            static_cast<float>(terrain.width)) {
            addValidationError(errors, definition,
                               "platform extends beyond terrain: " +
                                   platform.id);
        }
        const float motionLimit =
            platform.motion == PlatformMotion::OscillateHorizontal
                ? static_cast<float>(terrain.width)
                : static_cast<float>(terrain.height);
        if (platform.minimumTile < 0.f ||
            platform.maximumTile > motionLimit) {
            addValidationError(errors, definition,
                               "platform movement bounds are outside terrain: " +
                                   platform.id);
        }
    }

    for (const auto& block : definition.blockContents) {
        if (!insideMap(block.tilePosition, terrain)) {
            addValidationError(errors, definition,
                               "block content is outside terrain at " +
                                   std::to_string(block.tilePosition.x) + "," +
                                   std::to_string(block.tilePosition.y));
        }
        if (block.content.empty()) {
            addValidationError(errors, definition,
                               "block content cannot be empty");
        }
        if (block.area.empty()) {
            addValidationError(errors, definition,
                               "block area cannot be empty");
        }
        validateArea(block.area, "block content");
    }

    std::unordered_set<std::string> anchorIds;
    for (const auto& anchor : definition.anchors) {
        anchorIds.insert(anchor.id);
        if (!insideMap(anchor.tilePosition, terrain)) {
            addValidationError(errors, definition,
                                   "anchor is outside terrain: " + anchor.id);
        }
        if (anchor.area.empty()) {
            addValidationError(errors, definition,
                               "anchor area cannot be empty: " + anchor.id);
        }
        validateArea(anchor.area, "anchor " + anchor.id);
    }

    for (const auto& portal : definition.portals) {
        if (portal.targetAnchor.empty() ||
            anchorIds.find(portal.targetAnchor) == anchorIds.end()) {
            addValidationError(errors, definition,
                               "portal target anchor does not exist: " +
                                   portal.id);
        }
        if (!insideMap(portal.triggerTiles, terrain)) {
            addValidationError(errors, definition,
                               "portal trigger is outside terrain: " +
                                   portal.id);
        }
        if (portal.sourceArea.empty()) {
            addValidationError(errors, definition,
                               "portal source area cannot be empty: " +
                                   portal.id);
        }
        validateArea(portal.sourceArea, "portal " + portal.id);
    }

    for (const auto& camera : definition.cameraZones) {
        if (!insideMap(camera.boundsTiles, terrain)) {
            addValidationError(errors, definition,
                               "camera bounds are outside terrain: " +
                                   camera.id);
        }
        if (camera.area.empty()) {
            addValidationError(errors, definition,
                               "camera area cannot be empty: " + camera.id);
        }
    }

    if (definition.rules.leftBoundaryTile < 0.f) {
        addValidationError(errors, definition,
                           "left_boundary_tile cannot be negative");
    }
    if (definition.rules.rightBoundaryTile >= 0.f &&
        definition.rules.rightBoundaryTile > static_cast<float>(terrain.width)) {
        addValidationError(errors, definition,
                           "right_boundary_tile is outside terrain");
    }

    return errors;
}
