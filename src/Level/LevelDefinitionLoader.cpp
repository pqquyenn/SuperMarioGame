#include "Level/LevelDefinitionLoader.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace {
std::string trim(std::string value) {
    const auto first = std::find_if_not(value.begin(), value.end(),
        [](unsigned char c) { return std::isspace(c) != 0; });
    const auto last = std::find_if_not(value.rbegin(), value.rend(),
        [](unsigned char c) { return std::isspace(c) != 0; }).base();
    if (first >= last) return {};
    return std::string(first, last);
}

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::unordered_map<std::string, std::string> attributes(
    const std::string& line) {
    std::unordered_map<std::string, std::string> result;
    std::istringstream stream(line);
    std::string token;
    stream >> token; // record kind
    while (stream >> token) {
        const std::size_t separator = token.find('=');
        if (separator == std::string::npos) continue;
        result[lower(token.substr(0, separator))] =
            token.substr(separator + 1);
    }
    return result;
}

bool parseFloat(const std::string& value, float& result) {
    try {
        std::size_t used = 0;
        result = std::stof(value, &used);
        return used == value.size();
    } catch (...) {
        return false;
    }
}

bool parseInt(const std::string& value, int& result) {
    try {
        std::size_t used = 0;
        result = std::stoi(value, &used);
        return used == value.size();
    } catch (...) {
        return false;
    }
}

bool parsePair(const std::string& value, sf::Vector2f& result) {
    const std::size_t comma = value.find(',');
    if (comma == std::string::npos) return false;
    return parseFloat(value.substr(0, comma), result.x) &&
           parseFloat(value.substr(comma + 1), result.y);
}

bool parseIntPair(const std::string& value, sf::Vector2i& result) {
    const std::size_t comma = value.find(',');
    if (comma == std::string::npos) return false;
    return parseInt(value.substr(0, comma), result.x) &&
           parseInt(value.substr(comma + 1), result.y);
}

bool parseRect(const std::string& value, sf::FloatRect& result) {
    std::istringstream stream(value);
    std::string part;
    float values[4]{};
    for (float& component : values) {
        if (!std::getline(stream, part, ',') || !parseFloat(part, component)) {
            return false;
        }
    }
    if (std::getline(stream, part, ',')) return false;
    result = {values[0], values[1], values[2], values[3]};
    return true;
}

bool parseBool(const std::string& value, bool fallback) {
    const std::string normalized = lower(value);
    if (normalized == "true" || normalized == "yes" || normalized == "1") {
        return true;
    }
    if (normalized == "false" || normalized == "no" || normalized == "0") {
        return false;
    }
    return fallback;
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
    std::size_t line,
    const std::string& message) {
    errors.push_back("line " + std::to_string(line) + ": " + message);
}

std::filesystem::path resolveRelative(
    const std::filesystem::path& manifest,
    const std::string& referencedPath) {
    const std::filesystem::path path(referencedPath);
    if (path.is_absolute()) return path;
    return manifest.parent_path() / path;
}
}

std::string LevelDefinitionLoader::findManifest(
    const std::string& requestedPath) {
    std::filesystem::path requested(requestedPath);
    if (requested.extension() == ".txt") requested.replace_extension(".level");

    const std::string relative = requested.generic_string();
    const std::filesystem::path candidates[] = {
        requested,
        std::filesystem::path("assets/maps") / requested,
        std::filesystem::path("../assets/maps") / requested,
        std::filesystem::path("../../assets/maps") / requested,
        std::filesystem::path("../../../assets/maps") / requested
    };
    for (const auto& candidate : candidates) {
        std::error_code error;
        if (std::filesystem::is_regular_file(candidate, error)) {
            return candidate.lexically_normal().generic_string();
        }
    }
    (void)relative;
    return {};
}

bool LevelDefinitionLoader::load(
    const std::string& requestedPath,
    LevelDefinition& definition,
    std::vector<std::string>& errors) const {
    errors.clear();
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

    definition = {};
    definition.sourcePath = manifestPath;
    std::string section;
    std::string raw;
    std::size_t lineNumber = 0;
    while (std::getline(input, raw)) {
        ++lineNumber;
        std::string line = trim(raw);
        if (line.empty() || line.front() == '#') continue;
        if (line.front() == '[' && line.back() == ']') {
            section = lower(trim(line.substr(1, line.size() - 2)));
            continue;
        }

        if (section == "stage" || section == "rules") {
            const std::size_t separator = line.find('=');
            if (separator == std::string::npos) {
                addParseError(errors, lineNumber, "expected key=value");
                continue;
            }
            const std::string key = lower(trim(line.substr(0, separator)));
            const std::string value = trim(line.substr(separator + 1));
            if (section == "stage") {
                if (key == "version") {
                    if (!parseInt(value, definition.version))
                        addParseError(errors, lineNumber, "version must be an integer");
                }
                else if (key == "id") definition.id = value;
                else if (key == "name") definition.name = value;
                else if (key == "terrain") definition.terrainPath = value;
                else if (key == "background") definition.backgroundPath = value;
                else if (key == "next_stage") definition.nextStage = value;
                else if (key == "initial_area") definition.initialArea = value;
                else if (key == "time_limit") {
                    if (!parseInt(value, definition.timeLimit))
                        addParseError(errors, lineNumber, "time_limit must be an integer");
                } else if (key == "tile_size") {
                    if (!parseFloat(value, definition.tileSize))
                        addParseError(errors, lineNumber, "tile_size must be numeric");
                }
                else addParseError(errors, lineNumber, "unknown stage key: " + key);
            } else {
                bool parsed = true;
                if (key == "kill_plane_tile") parsed = parseFloat(value, definition.rules.killPlaneTile);
                else if (key == "left_boundary_tile") parsed = parseFloat(value, definition.rules.leftBoundaryTile);
                else if (key == "right_boundary_tile") parsed = parseFloat(value, definition.rules.rightBoundaryTile);
                else if (key == "enemy_void_margin_tiles") parsed = parseFloat(value, definition.rules.enemyVoidMarginTiles);
                else addParseError(errors, lineNumber, "unknown rules key: " + key);
                if (!parsed) addParseError(errors, lineNumber, key + " must be numeric");
            }
            continue;
        }

        const auto values = attributes(line);
        const std::string kind = lower(line.substr(0, line.find_first_of(" \t")));
        if (section == "entities" && kind == "entity") {
            EntitySpawnDefinition entity;
            entity.id = valueOr(values, "id");
            entity.type = valueOr(values, "type");
            if (!parsePair(valueOr(values, "tile"), entity.tilePosition)) {
                addParseError(errors, lineNumber, "entity requires tile=x,y");
            }
            parseInt(valueOr(values, "direction", "-1"), entity.direction);
            parseFloat(valueOr(values, "speed", "-1"), entity.speed);
            definition.entities.push_back(entity);
        } else if (section == "items" && kind == "item") {
            EntitySpawnDefinition item;
            item.id = valueOr(values, "id");
            item.type = valueOr(values, "type");
            if (!parsePair(valueOr(values, "tile"), item.tilePosition)) {
                addParseError(errors, lineNumber, "item requires tile=x,y");
            }
            definition.items.push_back(item);
        } else if (section == "platforms" && kind == "platform") {
            PlatformDefinition platform;
            platform.id = valueOr(values, "id");
            if (!parsePair(valueOr(values, "tile"), platform.tilePosition)) {
                addParseError(errors, lineNumber, "platform requires tile=x,y");
            }
            parseFloat(valueOr(values, "width", "3"), platform.widthTiles);
            sf::Vector2f bounds;
            if (!parsePair(valueOr(values, "bounds"), bounds)) {
                addParseError(errors, lineNumber, "platform requires bounds=min,max");
            } else {
                platform.minimumTile = bounds.x;
                platform.maximumTile = bounds.y;
            }
            parseFloat(valueOr(values, "speed", "50"), platform.speed);
            const std::string motion = lower(valueOr(values, "motion"));
            if (motion == "horizontal") platform.motion = PlatformMotion::OscillateHorizontal;
            else if (motion == "loop_down") platform.motion = PlatformMotion::LoopDown;
            else if (motion == "loop_up") platform.motion = PlatformMotion::LoopUp;
            else platform.motion = PlatformMotion::OscillateVertical;
            definition.platforms.push_back(platform);
        } else if (section == "blocks" && kind == "block") {
            BlockContentDefinition block;
            if (!parseIntPair(valueOr(values, "tile"), block.tilePosition)) {
                addParseError(errors, lineNumber, "block requires tile=x,y");
            }
            block.content = valueOr(values, "content", "Coin");
            definition.blockContents.push_back(block);
        } else if (section == "anchors" && kind == "anchor") {
            AnchorDefinition anchor;
            anchor.id = valueOr(values, "id");
            anchor.area = valueOr(values, "area", "overworld");
            if (!parsePair(valueOr(values, "tile"), anchor.tilePosition)) {
                addParseError(errors, lineNumber, "anchor requires tile=x,y");
            }
            parsePair(valueOr(values, "velocity", "0,0"), anchor.exitVelocity);
            definition.anchors.push_back(anchor);
        } else if (section == "portals" && kind == "portal") {
            PortalDefinition portal;
            portal.id = valueOr(values, "id");
            portal.sourceArea = valueOr(values, "area", "overworld");
            portal.targetAnchor = valueOr(values, "target");
            portal.activation = lower(valueOr(values, "activation")) == "right"
                ? PortalActivation::Right : PortalActivation::Down;
            if (!parseRect(valueOr(values, "trigger"), portal.triggerTiles)) {
                addParseError(errors, lineNumber, "portal requires trigger=x,y,w,h");
            }
            definition.portals.push_back(portal);
        } else if (section == "camera_zones" && kind == "camera") {
            CameraZoneDefinition camera;
            camera.id = valueOr(values, "id");
            camera.area = valueOr(values, "area", "overworld");
            if (!parseRect(valueOr(values, "bounds"), camera.boundsTiles)) {
                addParseError(errors, lineNumber, "camera requires bounds=x,y,w,h");
            }
            camera.followX = parseBool(valueOr(values, "follow_x", "true"), true);
            camera.followY = parseBool(valueOr(values, "follow_y", "false"), false);
            parseFloat(valueOr(values, "center_y", "7"), camera.centerYTiles);
            camera.darkBackground = parseBool(valueOr(values, "dark", "false"), false);
            definition.cameraZones.push_back(camera);
        } else if (section == "checkpoints" && kind == "checkpoint") {
            CheckpointDefinition checkpoint;
            checkpoint.id = valueOr(values, "id");
            checkpoint.area = valueOr(values, "area", "overworld");
            if (!parseRect(valueOr(values, "trigger"), checkpoint.triggerTiles)) {
                addParseError(errors, lineNumber, "checkpoint requires trigger=x,y,w,h");
            }
            if (!parsePair(valueOr(values, "spawn"), checkpoint.spawnTile)) {
                addParseError(errors, lineNumber, "checkpoint requires spawn=x,y");
            }
            definition.checkpoints.push_back(checkpoint);
        } else {
            addParseError(errors, lineNumber,
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

    const auto validationErrors = LevelValidator::validate(definition);
    errors.insert(errors.end(), validationErrors.begin(), validationErrors.end());
    return errors.empty();
}

std::vector<std::string> LevelValidator::validate(
    const LevelDefinition& definition) {
    std::vector<std::string> errors;
    if (definition.version != 1) errors.push_back("unsupported version");
    if (definition.id.empty()) errors.push_back("stage id is required");
    if (definition.name.empty()) errors.push_back("stage name is required");
    if (definition.tileSize <= 0.f) errors.push_back("tile_size must be positive");
    if (definition.timeLimit <= 0) errors.push_back("time_limit must be positive");
    if (definition.rules.enemyVoidMarginTiles < 0.f) {
        errors.push_back("enemy_void_margin_tiles cannot be negative");
    }
    if (definition.terrainPath.empty() ||
        !std::filesystem::is_regular_file(definition.terrainPath)) {
        errors.push_back("terrain file does not exist: " + definition.terrainPath);
        return errors;
    }

    std::ifstream terrain(definition.terrainPath);
    std::vector<std::string> rows;
    std::string row;
    int startMarkers = 0;
    int endMarkers = 0;
    std::size_t mapWidth = 0;
    while (std::getline(terrain, row)) {
        if (row.empty() || row.front() == '#') continue;
        startMarkers += static_cast<int>(std::count(row.begin(), row.end(), '@'));
        endMarkers += static_cast<int>(std::count(row.begin(), row.end(), '!'));
        mapWidth = std::max(mapWidth, row.size());
        rows.push_back(row);
    }
    if (rows.empty()) errors.push_back("terrain contains no rows");
    if (startMarkers != 1) errors.push_back("terrain must contain exactly one '@' start marker");
    if (endMarkers != 1) errors.push_back("terrain must contain exactly one '!' end marker");
    const float mapWidthTiles = static_cast<float>(mapWidth);
    const float mapHeightTiles = static_cast<float>(rows.size());
    auto insideMap = [mapWidthTiles, mapHeightTiles](const sf::Vector2f& point) {
        return point.x >= 0.f && point.y >= 0.f &&
               point.x < mapWidthTiles && point.y < mapHeightTiles;
    };
    auto rectInsideMap = [mapWidthTiles, mapHeightTiles](const sf::FloatRect& rect) {
        return rect.left >= 0.f && rect.top >= 0.f &&
               rect.width > 0.f && rect.height > 0.f &&
               rect.left + rect.width <= mapWidthTiles &&
               rect.top + rect.height <= mapHeightTiles;
    };
    if (definition.rules.killPlaneTile <= 0.f) {
        errors.push_back("kill_plane_tile must be positive");
    }
    if (definition.rules.rightBoundaryTile >= 0.f &&
        definition.rules.rightBoundaryTile <= definition.rules.leftBoundaryTile) {
        errors.push_back("right boundary must be greater than left boundary");
    }

    std::unordered_set<std::string> ids;
    auto requireUnique = [&errors, &ids](const std::string& kind,
                                         const std::string& id) {
        if (id.empty()) {
            errors.push_back(kind + " id is required");
        } else if (!ids.insert(id).second) {
            errors.push_back("duplicate object id: " + id);
        }
    };
    for (const auto& entity : definition.entities) {
        requireUnique("entity", entity.id);
        if (entity.type.empty()) errors.push_back("entity type is required");
        if (!insideMap(entity.tilePosition)) {
            errors.push_back("entity is outside terrain: " + entity.id);
        }
    }
    for (const auto& item : definition.items) {
        requireUnique("item", item.id);
        if (item.type.empty()) errors.push_back("item type is required");
        if (!insideMap(item.tilePosition)) {
            errors.push_back("item is outside terrain: " + item.id);
        }
    }
    for (const auto& platform : definition.platforms) {
        requireUnique("platform", platform.id);
        if (platform.widthTiles <= 0.f) errors.push_back("platform width must be positive");
        if (platform.maximumTile < platform.minimumTile) errors.push_back("platform bounds are reversed");
        if (!insideMap(platform.tilePosition)) {
            errors.push_back("platform is outside terrain: " + platform.id);
        }
    }
    std::unordered_set<std::string> anchors;
    for (const auto& anchor : definition.anchors) {
        requireUnique("anchor", anchor.id);
        anchors.insert(anchor.id);
        if (!insideMap(anchor.tilePosition)) {
            errors.push_back("anchor is outside terrain: " + anchor.id);
        }
    }
    for (const auto& portal : definition.portals) {
        requireUnique("portal", portal.id);
        if (portal.triggerTiles.width <= 0.f || portal.triggerTiles.height <= 0.f) {
            errors.push_back("portal trigger must have positive size: " + portal.id);
        }
        if (!rectInsideMap(portal.triggerTiles)) {
            errors.push_back("portal trigger is outside terrain: " + portal.id);
        }
        if (anchors.find(portal.targetAnchor) == anchors.end()) {
            errors.push_back("portal target does not exist: " + portal.targetAnchor);
        }
    }
    const bool initialCamera = std::any_of(
        definition.cameraZones.begin(), definition.cameraZones.end(),
        [&definition](const CameraZoneDefinition& zone) {
            return zone.area == definition.initialArea;
        });
    if (!initialCamera) {
        errors.push_back("initial area has no camera zone: " +
                         definition.initialArea);
    }
    for (const auto& anchor : definition.anchors) {
        const bool areaHasCamera = std::any_of(
            definition.cameraZones.begin(), definition.cameraZones.end(),
            [&anchor](const CameraZoneDefinition& zone) {
                return zone.area == anchor.area;
            });
        if (!areaHasCamera) {
            errors.push_back("anchor area has no camera zone: " + anchor.area);
        }
    }
    for (const auto& camera : definition.cameraZones) {
        requireUnique("camera", camera.id);
        if (camera.boundsTiles.width <= 0.f || camera.boundsTiles.height <= 0.f) {
            errors.push_back("camera bounds must have positive size: " + camera.id);
        }
        if (!rectInsideMap(camera.boundsTiles)) {
            errors.push_back("camera bounds are outside terrain: " + camera.id);
        }
    }
    for (const auto& checkpoint : definition.checkpoints) {
        requireUnique("checkpoint", checkpoint.id);
        if (checkpoint.triggerTiles.width <= 0.f ||
            checkpoint.triggerTiles.height <= 0.f) {
            errors.push_back("checkpoint trigger must have positive size: " +
                             checkpoint.id);
        }
        if (!rectInsideMap(checkpoint.triggerTiles)) {
            errors.push_back("checkpoint trigger is outside terrain: " +
                             checkpoint.id);
        }
        if (!insideMap(checkpoint.spawnTile)) {
            errors.push_back("checkpoint spawn is outside terrain: " +
                             checkpoint.id);
        }
    }
    for (const auto& block : definition.blockContents) {
        const int x = block.tilePosition.x;
        const int y = block.tilePosition.y;
        if (y < 0 || y >= static_cast<int>(rows.size()) || x < 0 ||
            x >= static_cast<int>(rows[static_cast<std::size_t>(y)].size())) {
            errors.push_back("block content is outside terrain");
            continue;
        }
        const char symbol = rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
        if (symbol != '?' && symbol != 'Q' && symbol != 'S' && symbol != 'r') {
            errors.push_back("block content must target an interactive block at " +
                std::to_string(x) + "," + std::to_string(y));
        }
    }
    return errors;
}
