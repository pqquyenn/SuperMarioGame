#include "Core/AssetManager.h"
#include <iostream>
#include <filesystem>

AssetManager* AssetManager::instance = nullptr;

AssetManager* AssetManager::getInstance() {
    if (!instance) {
        instance = new AssetManager();
    }
    return instance;
}

bool AssetManager::loadTexture(const std::string& name, const std::string& filename) {
    if (textures[name].loadFromFile(filename)) {
        textures[name].setSmooth(false);
        return true;
    }
    std::cerr << "AssetManager: Failed to load texture " << filename << std::endl;
    textures.erase(name);
    return false;
}

sf::Texture& AssetManager::getTexture(const std::string& name) {
    return textures[name];
}

void AssetManager::loadLevelAssets() {
    auto tryLoad = [&](const std::string& name, const std::string& rel) {
        if (std::filesystem::exists(rel)) { loadTexture(name, rel); return; }
        if (std::filesystem::exists("../" + rel)) { loadTexture(name, "../" + rel); return; }
        if (std::filesystem::exists("../../" + rel)) { loadTexture(name, "../../" + rel); return; }
        if (std::filesystem::exists("../../../" + rel)) { loadTexture(name, "../../../" + rel); return; }
    };
    tryLoad("BlockTileSheet", "assets/textures/blocks/BlockTileSheet.png");
    tryLoad("DecorSheet", "assets/textures/blocks/DecorSheet.png");
}

bool AssetManager::loadFont(const std::string& name, const std::string& filename) {
    sf::Font font;
    if (font.loadFromFile(filename)) {
        fonts[name] = font;
        return true;
    }
    return false;
}

sf::Font& AssetManager::getFont(const std::string& name) {
    return fonts[name];
}

bool AssetManager::loadSoundBuffer(const std::string& name, const std::string& filename) {
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(filename)) {
        soundBuffers[name] = buffer;
        return true;
    }
    return false;
}

sf::SoundBuffer& AssetManager::getSoundBuffer(const std::string& name) {
    return soundBuffers[name];
}
