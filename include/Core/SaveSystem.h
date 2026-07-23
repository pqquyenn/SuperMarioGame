#pragma once

#include <string>

struct SaveData {
    int score = 0;
    int coins = 0;
    int lives = 3;
    int currentLevel = 1;
};

class SaveSystem {
public:
    static bool saveGame(const std::string& filename, const SaveData& data);
    static bool loadGame(const std::string& filename, SaveData& data);
};
