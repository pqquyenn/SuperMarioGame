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