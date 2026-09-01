#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class EntitySymbolKind {
    Enemy,
    Item
};

struct EntitySymbolDefinition {
    std::string symbol;
    EntitySymbolKind kind{EntitySymbolKind::Enemy};
    std::string factoryType;
};

class EntitySymbolCatalog {
public:
    bool load(const std::string& filepath, std::vector<std::string>& errors);

    const EntitySymbolDefinition* resolve(const std::string& symbol) const;

private:
    std::unordered_map<std::string, EntitySymbolDefinition> definitions;
};

