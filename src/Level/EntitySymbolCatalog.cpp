#include "Level/EntitySymbolCatalog.h"

#include <cctype>
#include <fstream>
#include <sstream>

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

} // namespace

bool EntitySymbolCatalog::load(
    const std::string& filepath,
    std::vector<std::string>& errors) {
    definitions.clear();

    std::ifstream input(filepath);
    if (!input) {
        errors.push_back("could not open entity symbol catalog: " + filepath);
        return false;
    }

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty() || line.front() == '#') {
            continue;
        }

        std::istringstream stream(line);
        std::string kind;
        std::string symbol;
        std::string factoryType;
        std::string extra;
        if (!(stream >> kind >> symbol >> factoryType) || (stream >> extra)) {
            errors.push_back(
                filepath + ":" + std::to_string(lineNumber) +
                ": expected '<enemy|item> <symbol> <factory type>'");
            continue;
        }

        EntitySymbolKind symbolKind;
        const std::string normalizedKind = lower(kind);
        if (normalizedKind == "enemy") {
            symbolKind = EntitySymbolKind::Enemy;
        } else if (normalizedKind == "item") {
            symbolKind = EntitySymbolKind::Item;
        } else {
            errors.push_back(
                filepath + ":" + std::to_string(lineNumber) +
                ": unknown symbol kind '" + kind + "'");
            continue;
        }

        if (symbol.empty() || factoryType.empty()) {
            errors.push_back(
                filepath + ":" + std::to_string(lineNumber) +
                ": symbol and factory type must not be empty");
            continue;
        }

        if (definitions.find(symbol) != definitions.end()) {
            errors.push_back(
                filepath + ":" + std::to_string(lineNumber) +
                ": duplicate entity symbol '" + symbol + "'");
            continue;
        }

        definitions.emplace(
            symbol,
            EntitySymbolDefinition{symbol, symbolKind, factoryType});
    }

    return errors.empty();
}

const EntitySymbolDefinition* EntitySymbolCatalog::resolve(
    const std::string& symbol) const {
    const auto found = definitions.find(symbol);
    return found == definitions.end() ? nullptr : &found->second;
}

