#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <string>

class AssetManager {
private:
    // === Meyers' Singleton: constructor private ===
    AssetManager() = default;
    ~AssetManager() = default;

    // === Luu tru tai nguyen ===
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font> fonts;
    std::map<std::string, sf::SoundBuffer> soundBuffers;

public:
    // === Xoa copy & move de dam bao chi co 1 instance duy nhat ===
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    // === Meyers' Singleton: tra ve tham chieu (khong dung raw pointer) ===
    // Thread-safe tu C++11, tu dong huy khi chuong trinh ket thuc
    static AssetManager& getInstance();

    // === Texture ===
    bool loadTexture(const std::string& name, const std::string& filename);
    sf::Texture& getTexture(const std::string& name);
    void loadLevelAssets();

    // === Font ===
    bool loadFont(const std::string& name, const std::string& filename);
    sf::Font& getFont(const std::string& name);

    // === Sound Buffer ===
    bool loadSoundBuffer(const std::string& name, const std::string& filename);
    sf::SoundBuffer& getSoundBuffer(const std::string& name);
};
