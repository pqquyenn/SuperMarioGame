#include "Level/StageCatalog.h"

#include "Level/LevelDefinitionLoader.h"
#include <algorithm>
#include <filesystem>
#include <unordered_set>

std::vector<StageCatalogEntry> StageCatalog::discover() {
    const std::filesystem::path roots[] = {
        "assets/maps", "../assets/maps", "../../assets/maps", "../../../assets/maps"
    };
    std::vector<StageCatalogEntry> stages;
    std::unordered_set<std::string> seen;
    LevelDefinitionLoader loader;
    for (const auto& root : roots) {
        std::error_code error;
        if (!std::filesystem::is_directory(root, error)) continue;
        for (std::filesystem::recursive_directory_iterator iterator(root, error), end;
             iterator != end && !error; iterator.increment(error)) {
            if (!iterator->is_regular_file() || iterator->path().extension() != ".level") {
                continue;
            }
            LevelDefinition definition;
            std::vector<std::string> errors;
            if (!loader.load(iterator->path().generic_string(), definition, errors) ||
                !seen.insert(definition.id).second) {
                continue;
            }
            stages.push_back({definition.id, definition.name,
                              iterator->path().generic_string()});
        }
        if (!stages.empty()) break;
    }
    std::sort(stages.begin(), stages.end(),
        [](const StageCatalogEntry& a, const StageCatalogEntry& b) {
            return a.id < b.id;
        });
    return stages;
}

std::optional<StageCatalogEntry> StageCatalog::findById(const std::string& id) {
    const auto stages = discover();
    const auto found = std::find_if(
        stages.begin(), stages.end(), [&id](const StageCatalogEntry& stage) {
            return stage.id == id;
        });
    return found == stages.end()
        ? std::optional<StageCatalogEntry>{} : *found;
}

bool StageCatalog::validateAll(std::vector<std::string>& errors) {
    errors.clear();
    const std::filesystem::path roots[] = {
        "assets/maps", "../assets/maps", "../../assets/maps", "../../../assets/maps"
    };
    std::filesystem::path selectedRoot;
    for (const auto& root : roots) {
        std::error_code error;
        if (std::filesystem::is_directory(root, error)) {
            selectedRoot = root;
            break;
        }
    }
    if (selectedRoot.empty()) {
        errors.push_back("assets/maps directory was not found");
        return false;
    }

    std::size_t manifestCount = 0;
    std::vector<LevelDefinition> definitions;
    LevelDefinitionLoader loader;
    std::error_code iterationError;
    for (std::filesystem::recursive_directory_iterator iterator(
             selectedRoot, iterationError), end;
         iterator != end && !iterationError; iterator.increment(iterationError)) {
        if (!iterator->is_regular_file() || iterator->path().extension() != ".level") {
            continue;
        }
        ++manifestCount;
        LevelDefinition definition;
        std::vector<std::string> stageErrors;
        if (!loader.load(iterator->path().generic_string(), definition, stageErrors)) {
            for (const auto& error : stageErrors) {
                errors.push_back(iterator->path().generic_string() + ": " + error);
            }
        } else {
            definitions.push_back(std::move(definition));
        }
    }
    if (iterationError) errors.push_back("could not scan stage directory");
    if (manifestCount == 0) errors.push_back("no .level manifests were found");
    std::unordered_set<std::string> stageIds;
    for (const auto& definition : definitions) {
        if (!stageIds.insert(definition.id).second) {
            errors.push_back("duplicate stage id: " + definition.id);
        }
    }
    for (const auto& definition : definitions) {
        if (!definition.nextStage.empty() &&
            stageIds.find(definition.nextStage) == stageIds.end()) {
            errors.push_back("stage " + definition.id +
                             " references missing next_stage " +
                             definition.nextStage);
        }
    }
    return errors.empty();
}
