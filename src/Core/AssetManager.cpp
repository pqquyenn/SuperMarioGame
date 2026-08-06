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
    // tryLoad("BlockTileSheet", "assets/sprites/blocks/BlockTileSheet.png");
    tryLoad("DecorSheet", "assets/textures/blocks/DecorSheet.png");
    tryLoad("Overworld", "assets/maps/Mario Game Assets/Overworld.png");
    tryLoad("Underground", "assets/maps/Mario Game Assets/Underground.png");
    tryLoad("Coin_Underground", "assets/maps/Mario Game Assets/Coin_Underground.png");
    tryLoad("EmptyBlock", "assets/maps/Mario Game Assets/EmptyBlock.png");
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
    tryLoad("Cloud1", "assets/maps/Mario Game Assets/Cloud1.png");
    tryLoad("Cloud2", "assets/maps/Mario Game Assets/Cloud2.png");
    tryLoad("Cloud3", "assets/maps/Mario Game Assets/Cloud3.png");
    tryLoad("Bush1", "assets/maps/Mario Game Assets/Bush1.png");
    tryLoad("Bush2", "assets/maps/Mario Game Assets/Bush2.png");
    tryLoad("Bush3", "assets/maps/Mario Game Assets/Bush3.png");
    tryLoad("Hill1", "assets/maps/Mario Game Assets/Hill1.png");
    tryLoad("Hill2", "assets/maps/Mario Game Assets/Hill2.png");
    tryLoad("UndergroundBlock", "assets/maps/Mario Game Assets/UndergroundBlock.png");
    tryLoad("UndergroundBrick", "assets/maps/Mario Game Assets/UndergroundBrick.png");  
    tryLoad("SpriteIsland", "assets/maps/Mario Game Assets/SpriteIsland.png");
    if (textures.find("SpriteIsland") == textures.end() || textures["SpriteIsland"].getSize().x == 0) {
        tryLoad("SpriteIsland", "assets/maps/Mario Game Assets/SpriteIsland.webp");
    }
    
    tryLoad("UndergroundHardBlock", "assets/maps/Mario Game Assets/UndergroundHardBlock.png");
    if (textures.find("UndergroundHardBlock") == textures.end() || textures["UndergroundHardBlock"].getSize().x == 0) {
        tryLoad("UndergroundHardBlock", "assets/maps/Mario Game Assets/UndergroundHardBlock.webp");
    }
    tryLoad("Platform", "assets/maps/Mario Game Assets/Platform.png");
    tryLoad("LargeCastle", "assets/maps/Mario Game Assets/LargeCastle.png");
    
    // Load Entity Textures
    tryLoad("Goomba", "assets/maps/Mario Game Assets/Goomba_Walk1.png");
    tryLoad("Koopa", "assets/maps/Mario Game Assets/Koopa_Walk1.png");
    tryLoad("PiranhaPlant", "assets/maps/Mario Game Assets/PipeTop.png"); // Fallback if no PiranhaPlant.png
    tryLoad("Coin", "assets/maps/Mario Game Assets/Coin.png");
    tryLoad("Mushroom", "assets/maps/Mario Game Assets/MagicMushroom.png");
    tryLoad("FireFlower", "assets/maps/Mario Game Assets/Starman.png"); // Fallback if no FireFlower
    tryLoad("PlayerSpriteSheet", "assets/textures/characters/PlayerSpriteSheet.png");

    if (textures.find("PlayerSpriteSheet") == textures.end() || textures["PlayerSpriteSheet"].getSize().x == 0) {
        sf::Image sheetImg;
        sheetImg.create(403, 266, sf::Color::Transparent);

        auto copyToSheet = [&](const std::string& relPath, unsigned int destX, unsigned int destY) {
            std::string actualPath;
            if (std::filesystem::exists(relPath)) actualPath = relPath;
            else if (std::filesystem::exists("../" + relPath)) actualPath = "../" + relPath;
            else if (std::filesystem::exists("../../" + relPath)) actualPath = "../../" + relPath;
            else if (std::filesystem::exists("../../../" + relPath)) actualPath = "../../../" + relPath;

            if (!actualPath.empty()) {
                sf::Image img;
                if (img.loadFromFile(actualPath)) {
                    sheetImg.copy(img, destX, destY, sf::IntRect(0, 0, 0, 0), true);
                }
            }
        };

        // Small Mario frames (Row Y = 9, 16x16)
        copyToSheet("assets/maps/Mario Game Assets/Mario_Small_Idle.png", 1, 9);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Small_Death.png", 17, 9);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Small_Run1.png", 33, 9);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Small_Run2.png", 49, 9);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Small_Run3.png", 65, 9);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Small_Slide.png", 81, 9);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Small_Jump.png", 97, 9);

        // Big Mario frames (Row Y = 25, 16x32)
        copyToSheet("assets/maps/Mario Game Assets/Mario_Big_Idle.png", 1, 25);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Big_Run1.png", 33, 25);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Big_Run2.png", 49, 25);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Big_Run3.png", 65, 25);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Big_Slide.png", 81, 25);
        copyToSheet("assets/maps/Mario Game Assets/Mario_Big_Jump.png", 97, 25);

        textures["PlayerSpriteSheet"].loadFromImage(sheetImg);
        textures["PlayerSpriteSheet"].setSmooth(false);
    }
    
    // Generate solid black tile
    if (textures.find("BlackTile") == textures.end()) {
        sf::Image blackImg;
        blackImg.create(16, 16, sf::Color::Black);
        textures["BlackTile"].loadFromImage(blackImg);
    }
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