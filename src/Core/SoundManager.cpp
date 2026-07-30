#include "Core/SoundManager.h"
#include "Core/AssetManager.h"
#include <iostream>

// === Meyers' Singleton ===

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}
