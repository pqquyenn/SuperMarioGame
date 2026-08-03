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
    tryLoad("Castle", "assets/maps/Mario Game Assets/Castle.png");
    tryLoad("FlagPole", "assets/maps/Mario Game Assets/FlagPole.png");
    tryLoad("Flag", "assets/maps/Mario Game Assets/Flag.png");
    tryLoad("GroundBlock", "assets/maps/Mario Game Assets/GroundBlock.png");
    tryLoad("HardBlock", "assets/maps/Mario Game Assets/HardBlock.png");
    tryLoad("Brick", "assets/maps/Mario Game Assets/Brick.png");
    tryLoad("MysteryBlock", "assets/maps/Mario Game Assets/MysteryBlock.png");
    tryLoad("PipeTop", "assets/maps/Mario Game Assets/PipeTop.png");
    tryLoad("PipeBottom", "assets/maps/Mario Game Assets/PipeBottom.png");
    tryLoad("PipeConnection", "assets/maps/Mario Game Assets/PipeConnection.png");
    tryLoad("Cloud2", "assets/maps/Mario Game Assets/Cloud2.png");
    tryLoad("UndergroundBlock", "assets/maps/Mario Game Assets/UndergroundBlock.png");
    tryLoad("UndergroundBrick", "assets/maps/Mario Game Assets/UndergroundBrick.png");
    tryLoad("Coin_Underground", "assets/maps/Mario Game Assets/Coin_Underground.png");
    tryLoad("PlayerSpriteSheet", "assets/textures/characters/PlayerSpriteSheet.png");
    tryLoad("Mario_Small_Idle", "assets/maps/Mario Game Assets/Mario_Small_Idle.png");
    tryLoad("Mario_Small_Run1", "assets/maps/Mario Game Assets/Mario_Small_Run1.png");
    tryLoad("Mario_Small_Run2", "assets/maps/Mario Game Assets/Mario_Small_Run2.png");
    tryLoad("Mario_Small_Run3", "assets/maps/Mario Game Assets/Mario_Small_Run3.png");
    tryLoad("Mario_Small_Jump", "assets/maps/Mario Game Assets/Mario_Small_Jump.png");
    tryLoad("Mario_Small_Slide", "assets/maps/Mario Game Assets/Mario_Small_Slide.png");
    tryLoad("Mario_Big_Idle", "assets/maps/Mario Game Assets/Mario_Big_Idle.png");
    tryLoad("Mario_Big_Run1", "assets/maps/Mario Game Assets/Mario_Big_Run1.png");
    tryLoad("Mario_Big_Run2", "assets/maps/Mario Game Assets/Mario_Big_Run2.png");
    tryLoad("Mario_Big_Run3", "assets/maps/Mario Game Assets/Mario_Big_Run3.png");
    tryLoad("Mario_Big_Jump", "assets/maps/Mario Game Assets/Mario_Big_Jump.png");
    tryLoad("Mario_Big_Slide", "assets/maps/Mario Game Assets/Mario_Big_Slide.png");
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