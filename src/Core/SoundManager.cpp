#include "Core/SoundManager.h"
#include "Core/AssetManager.h"
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
        backgroundMusic.play();
    } else {
        std::cerr << "[SoundManager] Failed to load BGM: " << filename << std::endl;
    }
}

void SoundManager::stopBGM() {
    backgroundMusic.stop();
}
