#include "Core/SoundManager.h"
#include "Core/AssetManager.h"
#include <iostream>

SoundManager* SoundManager::instance = nullptr;

SoundManager* SoundManager::getInstance() {
    if (!instance) {
        instance = new SoundManager();
    }
    return instance;
}

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

void SoundManager::playSound(const std::string& name) {
    try {
        sf::SoundBuffer& buf = AssetManager::getInstance()->getSoundBuffer(name);
        sounds[name].setBuffer(buf);
        sounds[name].play();
    } catch (const std::exception& e) {
        std::cerr << "[SoundManager] Sound not found: " << name << " - " << e.what() << std::endl;
    }
}
