#include "Level/BlockContentResolver.h"

#include "Entities/Character.h"
#include <string_view>
#include <utility>

BlockContentResolver::BlockContentResolver() {
    registerRule("PowerupByForm", [](const Character* character) {
        if (!character) return std::string("Mushroom");
        const std::string_view form = character->getCurrentFormName();
        return (form == "Super" || form == "Fire")
            ? std::string("FireFlower") : std::string("Mushroom");
    });
}

BlockContentResolver& BlockContentResolver::getInstance() {
    static BlockContentResolver resolver;
    return resolver;
}

void BlockContentResolver::registerRule(const std::string& id, Rule rule) {
    rules[id] = std::move(rule);
}

std::string BlockContentResolver::resolve(
    const std::string& contentOrRule,
    const Character* character) const {
    const auto found = rules.find(contentOrRule);
    return found == rules.end()
        ? contentOrRule : found->second(character);
}

bool BlockContentResolver::hasRule(const std::string& id) const {
    return rules.find(id) != rules.end();
}
