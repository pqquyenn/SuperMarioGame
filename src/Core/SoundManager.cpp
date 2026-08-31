#include "Core/SoundManager.h"
#include "Core/AssetManager.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>

// === Meyers' Singleton ===

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() {
    loadVolume();
}

// === Background Music ===

void SoundManager::playBGM(const std::string& filename, bool loop) {
    const std::string prefixes[] = {"", "../", "../../", "../../../"};
    std::string resolvedPath = filename;
    bool found = false;

    for (const auto& p : prefixes) {
        std::string testPath = p + filename;
        if (std::filesystem::exists(testPath)) {
            resolvedPath = testPath;
            found = true;
            break;
        }
    }

    if (backgroundMusic.openFromFile(resolvedPath)) {
        currentBgmPath = filename;
        backgroundMusic.setLoop(loop);
        backgroundMusic.setVolume(masterVolume);
        backgroundMusic.play();
    } else {
        std::cerr << "[SoundManager] Failed to load BGM: " << filename << std::endl;
    }
}

void SoundManager::stopBGM() {
    backgroundMusic.stop();
    currentBgmPath.clear();
}

// === Sound Effects ===

void SoundManager::playSound(const std::string& name) {
    try {
        // Chu y: dung dau cham (.) thay vi mui ten (->) vi getInstance() tra ve reference
        sf::SoundBuffer& buf = AssetManager::getInstance().getSoundBuffer(name);
        sounds[name].setBuffer(buf);
        sounds[name].setVolume(masterVolume);
        sounds[name].play();
    } catch (const std::exception& e) {
        std::cerr << "[SoundManager] Sound not found: " << name << " - " << e.what() << std::endl;
    }
}

void SoundManager::setMasterVolume(float volume) {
    masterVolume = std::clamp(volume, 0.f, 100.f);
    backgroundMusic.setVolume(masterVolume);
    for (auto& [name, sound] : sounds) {
        (void)name;
        sound.setVolume(masterVolume);
    }
    saveVolume();
}

// === Persistence ===

void SoundManager::saveVolume() {
    // Tao thu muc assets/state/ neu chua ton tai
    std::filesystem::path configPath(VOLUME_CONFIG_PATH);
    if (configPath.has_parent_path()) {
        std::filesystem::create_directories(configPath.parent_path());
    }

    std::ofstream file(VOLUME_CONFIG_PATH);
    if (file.is_open()) {
        file << masterVolume;
    }
}

void SoundManager::loadVolume() {
    std::ifstream file(VOLUME_CONFIG_PATH);
    if (file.is_open()) {
        float savedVolume = 100.f;
        if (file >> savedVolume) {
            masterVolume = std::clamp(savedVolume, 0.f, 100.f);
        }
    }
    // Neu file khong ton tai, giu mac dinh masterVolume = 100.f
}
