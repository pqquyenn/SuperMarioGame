#include "Core/SaveSystem.h"
#include <fstream>
#include <iostream>

bool SaveSystem::saveGame(const std::string& filename, const SaveData& data) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "[SaveSystem] Failed to open save file: " << filename << std::endl;
        return false;
    }
    outFile << data.score << "\n"
            << data.coins << "\n"
            << data.lives << "\n"
            << data.currentLevel << "\n";
    outFile.close();
    return true;
}

bool SaveSystem::loadGame(const std::string& filename, SaveData& data) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "[SaveSystem] Save file not found: " << filename << std::endl;
        return false;
    }
    inFile >> data.score >> data.coins >> data.lives >> data.currentLevel;
    inFile.close();
    return true;
}
