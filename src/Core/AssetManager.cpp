#include "Core/AssetManager.h"
#include <iostream>
#include <filesystem>

// === Meyers' Singleton ===

AssetManager& AssetManager::getInstance() {
    // Static local variable: chi tao 1 lan duy nhat khi goi lan dau
    // Thread-safe tu C++11 (compiler dam bao)
    // Tu dong huy khi chuong trinh ket thuc (khong memory leak)
    static AssetManager instance;
    return instance;
}

// === Texture ===

bool AssetManager::loadTexture(const std::string& name, const std::string& filename) {
    if (textures[name].loadFromFile(filename)) {
        textures[name].setSmooth(false);  // Pixel art khong can lam min
        return true;
    }
    std::cerr << "AssetManager: Failed to load texture " << filename << std::endl;
    textures.erase(name);
    return false;
}

sf::Texture& AssetManager::getTexture(const std::string& name) {
    return textures[name];
}