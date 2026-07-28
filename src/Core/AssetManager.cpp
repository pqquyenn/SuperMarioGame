// ============================================================
//  AssetManager.cpp – STUB tạm thời (placeholder)
//  TODO: File này sẽ được implement đầy đủ bởi Phan Quỳnh Quyền
//        (Core Engine / Singleton Pattern).
//  Mục đích hiện tại: Cung cấp symbol định nghĩa để link thành công
//  trong quá trình test riêng từng module.
// ============================================================
#include "Core/AssetManager.h"
#include <stdexcept>
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
    if (!tex.loadFromFile(filename)) {
        std::cerr << "[AssetManager] Failed to load texture: " << filename << "\n";
        return false;
    }
    textures[name] = std::move(tex);
    return true;
}

sf::Texture& AssetManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end())
        throw std::runtime_error("[AssetManager] Texture not found: " + name);
    return it->second;
}

void AssetManager::loadLevelAssets() {
    // TODO: implement by Phan Quỳnh Quyền
}

bool AssetManager::loadFont(const std::string& name, const std::string& filename) {
    sf::Font f;
    if (!f.loadFromFile(filename)) {
        std::cerr << "[AssetManager] Failed to load font: " << filename << "\n";
        return false;
    }
    fonts[name] = std::move(f);
    return true;
}

sf::Font& AssetManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end())
        throw std::runtime_error("[AssetManager] Font not found: " + name);
    return it->second;
}

bool AssetManager::loadSoundBuffer(const std::string& name, const std::string& filename) {
    sf::SoundBuffer buf;
    if (!buf.loadFromFile(filename)) {
        std::cerr << "[AssetManager] Failed to load sound: " << filename << "\n";
        return false;
    }
    soundBuffers[name] = std::move(buf);
    return true;
}

sf::SoundBuffer& AssetManager::getSoundBuffer(const std::string& name) {
    auto it = soundBuffers.find(name);
    if (it == soundBuffers.end())
        throw std::runtime_error("[AssetManager] SoundBuffer not found: " + name);
    return it->second;
}
