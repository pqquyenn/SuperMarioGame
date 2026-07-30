#pragma once

#include <SFML/Audio.hpp>
#include <map>
#include <string>

class SoundManager {
private:
    // === Meyers' Singleton: constructor private ===
    SoundManager() = default;
    ~SoundManager() = default;

    // === Luu tru am thanh ===
    sf::Music backgroundMusic;
    std::map<std::string, sf::Sound> sounds;

public:
    // === Xoa copy & move de dam bao chi co 1 instance duy nhat ===
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
    SoundManager(SoundManager&&) = delete;
    SoundManager& operator=(SoundManager&&) = delete;

    // === Meyers' Singleton: tra ve tham chieu ===
    // Thread-safe tu C++11, tu dong huy khi chuong trinh ket thuc
    static SoundManager& getInstance();

    // === Background Music ===
    void playBGM(const std::string& filename, bool loop = true);
    void stopBGM();

    // === Sound Effects ===
    void playSound(const std::string& name);
};
