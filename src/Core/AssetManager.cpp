#include "Core/AssetManager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>


// === Meyers' Singleton ===

AssetManager &AssetManager::getInstance() {
  // Static local variable: chi tao 1 lan duy nhat khi goi lan dau
  // Thread-safe tu C++11 (compiler dam bao)
  // Tu dong huy khi chuong trinh ket thuc (khong memory leak)
  static AssetManager instance;
  return instance;
}

bool AssetManager::loadTexture(const std::string &name,
                               const std::string &filename) {
  if (textures[name].loadFromFile(filename)) {
    textures[name].setSmooth(false);
    return true;
  }
  std::cerr << "AssetManager: Failed to load texture " << filename << std::endl;
  textures.erase(name);
  return false;
}

sf::Texture &AssetManager::getTexture(const std::string &name) {
  return textures[name];
}

bool AssetManager::hasTexture(const std::string &name) const {
  const auto found = textures.find(name);
  return found != textures.end() && found->second.getSize().x > 0 &&
         found->second.getSize().y > 0;
}

void AssetManager::loadLevelAssets() {
  if (levelAssetsLoaded) return;
  auto tryLoad = [&](const std::string &name, const std::string &rel) {
    if (std::filesystem::exists(rel)) {
      loadTexture(name, rel);
      return;
    }
    if (std::filesystem::exists("../" + rel)) {
      loadTexture(name, "../" + rel);
      return;
    }
    if (std::filesystem::exists("../../" + rel)) {
      loadTexture(name, "../../" + rel);
      return;
    }
    if (std::filesystem::exists("../../../" + rel)) {
      loadTexture(name, "../../../" + rel);
      return;
    }
  };

  const std::filesystem::path catalogCandidates[] = {
      "assets/config/assets.catalog", "../assets/config/assets.catalog",
      "../../assets/config/assets.catalog", "../../../assets/config/assets.catalog"};
  for (const auto &catalogPath : catalogCandidates) {
    if (!std::filesystem::is_regular_file(catalogPath)) continue;
    std::ifstream catalog(catalogPath);
    std::string line;
    while (std::getline(catalog, line)) {
      const auto first = line.find_first_not_of(" \t\r");
      if (first == std::string::npos || line[first] == '#') continue;
      std::istringstream stream(line.substr(first));
      std::string name;
      stream >> name;
      std::string alternatives;
      std::getline(stream, alternatives);
      alternatives.erase(0, alternatives.find_first_not_of(" \t"));
      std::istringstream paths(alternatives);
      std::string path;
      while (std::getline(paths, path, '|')) {
        const auto pathFirst = path.find_first_not_of(" \t");
        const auto pathLast = path.find_last_not_of(" \t\r");
        if (pathFirst == std::string::npos) continue;
        path = path.substr(pathFirst, pathLast - pathFirst + 1);
        tryLoad(name, path);
        if (textures.find(name) != textures.end() &&
            textures[name].getSize().x > 0) break;
      }
    }
    if (textures.find("BlackTile") == textures.end()) {
      sf::Image blackImg;
      blackImg.create(16, 16, sf::Color::Black);
      textures["BlackTile"].loadFromImage(blackImg);
    }
    levelAssetsLoaded = true;
    return;
  }
  throw std::runtime_error("assets/config/assets.catalog was not found");
#if 0 // Removed compiled catalog retained temporarily for merge archaeology.
  // tryLoad("BlockTileSheet", "assets/sprites/blocks/BlockTileSheet.png");
  tryLoad("BlockTileSheet", "assets/sprites/blocks/BlockTileSheet.png");
  tryLoad("DecorSheet", "assets/textures/blocks/DecorSheet.png");
  tryLoad("Overworld", "assets/maps/Mario Game Assets/Overworld.png");
  tryLoad("Underground", "assets/maps/Mario Game Assets/Underground.png");
  tryLoad("Coin_Underground",
          "assets/maps/Mario Game Assets/Coin_Underground.png");
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
  tryLoad("UndergroundBlock",
          "assets/maps/Mario Game Assets/UndergroundBlock.png");
  tryLoad("UndergroundBrick",
          "assets/maps/Mario Game Assets/UndergroundBrick.png");
  tryLoad("SpriteIsland", "assets/maps/Mario Game Assets/SpriteIsland.png");
  if (textures.find("SpriteIsland") == textures.end() ||
      textures["SpriteIsland"].getSize().x == 0) {
    tryLoad("SpriteIsland", "assets/maps/Mario Game Assets/SpriteIsland.webp");
  }

  tryLoad("UndergroundHardBlock",
          "assets/maps/Mario Game Assets/UndergroundHardBlock.png");
  if (textures.find("UndergroundHardBlock") == textures.end() ||
      textures["UndergroundHardBlock"].getSize().x == 0) {
    tryLoad("UndergroundHardBlock",
            "assets/maps/Mario Game Assets/UndergroundHardBlock.webp");
  }
  tryLoad("Platform", "assets/maps/Mario Game Assets/Platform.png");
  tryLoad("LargeCastle", "assets/maps/Mario Game Assets/LargeCastle.png");

  // Load Entity Textures
  tryLoad("Goomba", "assets/maps/Mario Game Assets/Goomba_Walk1.png");
  tryLoad("Koopa", "assets/maps/Mario Game Assets/Koopa_Walk1.png");
  tryLoad("Koopa_Walk2", "assets/maps/Mario Game Assets/Koopa_Walk2.png");
  tryLoad("PiranhaPlant",
          "assets/maps/Mario Game Assets/PipeTop.png"); // Fallback if no
                                                        // PiranhaPlant.png
  tryLoad("Coin", "assets/maps/Mario Game Assets/Coin.png");
  tryLoad("Mushroom", "assets/maps/Mario Game Assets/MagicMushroom.png");
  tryLoad("1UpMushroom", "assets/maps/Mario Game Assets/1upMushroom.png");
  tryLoad(
      "FireFlower",
      "assets/maps/Mario Game Assets/Starman.png"); // Fallback if no FireFlower
  tryLoad("Starman", "assets/maps/Mario Game Assets/Starman.png");
  tryLoad("Koopa_Shell", "assets/maps/Mario Game Assets/Koopa_Shell.png");

  // Red Koopa & Paratroopa textures
  tryLoad("RedKoopa_Walk1",  "assets/maps/Mario Game Assets/Redkoopa_walk1.png");
  tryLoad("RedKoopa_Walk2",  "assets/maps/Mario Game Assets/Redkoopa_walk2.png");
  tryLoad("RedKoopa_Shell",  "assets/maps/Mario Game Assets/Redkoopa_shell.png");
  tryLoad("RedKoopa_Shell1", "assets/maps/Mario Game Assets/Redkoopa_shell1.png");
  tryLoad("RedKoopa_Shell2", "assets/maps/Mario Game Assets/Redkoopa_shell2.png");

  // Green Paratroopa
  tryLoad("GreenParatroopa_Walk1", "assets/maps/Mario Game Assets/Patrakoopa_walk1.png");
  tryLoad("GreenParatroopa_Walk2", "assets/maps/Mario Game Assets/Patrakoopa_walk2.png");

  // Red Paratroopa
  tryLoad("RedParatroopa_Walk1", "assets/maps/Mario Game Assets/Redpatrakoopa_walk1.png");
  tryLoad("RedParatroopa_Walk2", "assets/maps/Mario Game Assets/Redpatrakoopa_walk2.png");

  tryLoad("PlayerSpriteSheet",
          "assets/sprites/characters/PlayerSpriteSheet.png");
  // Generate solid black tile
  if (textures.find("BlackTile") == textures.end()) {
    sf::Image blackImg;
    blackImg.create(16, 16, sf::Color::Black);
    textures["BlackTile"].loadFromImage(blackImg);
  }
  levelAssetsLoaded = true;
#endif
}

bool AssetManager::loadFont(const std::string &name,
                            const std::string &filename) {
  sf::Font font;
  if (font.loadFromFile(filename)) {
    fonts[name] = font;
    return true;
  }
  return false;
}

sf::Font &AssetManager::getFont(const std::string &name) { return fonts[name]; }

bool AssetManager::loadSoundBuffer(const std::string &name,
                                   const std::string &filename) {
  sf::SoundBuffer buffer;
  if (buffer.loadFromFile(filename)) {
    soundBuffers[name] = buffer;
    return true;
  }
  return false;
}

sf::SoundBuffer &AssetManager::getSoundBuffer(const std::string &name) {
  return soundBuffers[name];
}
