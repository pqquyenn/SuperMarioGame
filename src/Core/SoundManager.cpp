#include "Core/SoundManager.h"
#include "Core/AssetManager.h"
#include <algorithm>
#include <iostream>

// === Meyers' Singleton ===

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

// === Background Music ===

void SoundManager::playBGM(const std::string& filename, bool loop) {
    if (backgroundMusic.openFromFile(filename)) {
        backgroundMusic.setLoop(loop);
        backgroundMusic.setVolume(masterVolume);
        backgroundMusic.play();
    } else {
        std::cerr << "[SoundManager] Failed to load BGM: " << filename << std::endl;
    }
}

void SoundManager::stopBGM() {
    backgroundMusic.stop();
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
}
