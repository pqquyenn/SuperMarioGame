#pragma once

#include <functional>
#include <string>
#include <unordered_map>

class Character;

class BlockContentResolver {
public:
    using Rule = std::function<std::string(const Character*)>;

    static BlockContentResolver& getInstance();
    void registerRule(const std::string& id, Rule rule);
    std::string resolve(
        const std::string& contentOrRule,
        const Character* character) const;
    bool hasRule(const std::string& id) const;

private:
    BlockContentResolver();
    std::unordered_map<std::string, Rule> rules;
};
