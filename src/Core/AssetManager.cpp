#include "Core/AssetManager.h"
#include <iostream>

AssetManager* AssetManager::instance = nullptr;

AssetManager* AssetManager::getInstance() {
    if (!instance) {
        instance = new AssetManager();
    }
    return instance;
}

bool AssetManager::loadTexture(const std::string& name, const std::string& filename) {
    sf::Texture tex;
    if (tex.loadFromFile(filename)) {
        textures[name] = tex;
        return true;
    }
    std::cerr << "[AssetManager] Failed to load texture: " << filename << std::endl;
    return false;
}

sf::Texture& AssetManager::getTexture(const std::string& name) {
    return textures.at(name);
}

bool AssetManager::loadFont(const std::string& name, const std::string& filename) {
    sf::Font font;
    if (font.loadFromFile(filename)) {
        fonts[name] = font;
        return true;
    }
    std::cerr << "[AssetManager] Failed to load font: " << filename << std::endl;
    return false;
}

sf::Font& AssetManager::getFont(const std::string& name) {
    return fonts.at(name);
}

bool AssetManager::loadSoundBuffer(const std::string& name, const std::string& filename) {
    sf::SoundBuffer buf;
    if (buf.loadFromFile(filename)) {
        soundBuffers[name] = buf;
        return true;
    }
    std::cerr << "[AssetManager] Failed to load sound buffer: " << filename << std::endl;
    return false;
}

sf::SoundBuffer& AssetManager::getSoundBuffer(const std::string& name) {
    return soundBuffers.at(name);
}
