#pragma once

#include <SFML/Audio.hpp>
#include <map>
#include <string>

class SoundManager {
private:
    static SoundManager* instance;
    sf::Music backgroundMusic;
    std::map<std::string, sf::Sound> sounds;

    SoundManager() = default;

public:
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    static SoundManager* getInstance();

    void playBGM(const std::string& filename, bool loop = true);
    void stopBGM();
    void playSound(const std::string& name);
};
