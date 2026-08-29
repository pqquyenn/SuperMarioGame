#include "Core/AssetManager.h"
#include <filesystem>
#include <iostream>


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
    if (name == "PlayerSpriteSheet") {
      // A reloaded source atlas invalidates its lazily generated variant.
      textures.erase("PlayerSpriteSheetSecondary");
    }
    return true;
  }
  std::cerr << "AssetManager: Failed to load texture " << filename << std::endl;
  textures.erase(name);
  return false;
}

sf::Texture &AssetManager::getTexture(const std::string &name) {
  return textures[name];
}

sf::Texture &AssetManager::getPlayerTexture(PlayerPalette palette) {
  sf::Texture &primary = textures["PlayerSpriteSheet"];
  if (palette == PlayerPalette::Primary) {
    return primary;
  }

  auto secondary = textures.find("PlayerSpriteSheetSecondary");
  if (secondary != textures.end() && secondary->second.getSize().x > 0) {
    return secondary->second;
  }

  sf::Texture &generated = textures["PlayerSpriteSheetSecondary"];
  if (!loadSecondaryPlayerTexture(primary, generated)) {
    textures.erase("PlayerSpriteSheetSecondary");
    return primary;
  }
  return generated;
}

void AssetManager::loadLevelAssets() {
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
  tryLoad("Goomba_Underground",
          "assets/maps/Mario Game Assets/GoombaUndergroundWalk1.png");
  tryLoad("Goomba_Underground_Walk2",
          "assets/maps/Mario Game Assets/GoombaUndergroundWalk2.png");
  tryLoad("Goomba_Underground_Flat",
          "assets/maps/Mario Game Assets/GoombaUndergroundFlat.png");
  tryLoad("Koopa", "assets/maps/Mario Game Assets/Koopa_Walk1.png");
  tryLoad("Koopa_Walk2", "assets/maps/Mario Game Assets/Koopa_Walk2.png");
  tryLoad("PiranhaPlant_1",
          "assets/maps/Mario Game Assets/PirannhaPlantUnder1.png");
  tryLoad("PiranhaPlant_2",
          "assets/maps/Mario Game Assets/PirannhaPlantUnder2.png");
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
  tryLoad("DragonLugia",
          "assets/sprites/characters/dragon_lugia.png");
  tryLoad("DragonFlameProjectile",
          "assets/sprites/effects/flame_green_glowing.png");
  tryLoad("DragonFlameBurst",
          "assets/sprites/effects/burst_green_bright.png");
  tryLoad("DragonFlameBurstSheet",
          "assets/sprites/effects/dragon_flame_burst_green.png");
  // Generate solid black tile
  if (textures.find("BlackTile") == textures.end()) {
    sf::Image blackImg;
    blackImg.create(16, 16, sf::Color::Black);
    textures["BlackTile"].loadFromImage(blackImg);
  }
  // Load all sound effect buffers
  loadSoundAssets();
}

void AssetManager::loadSoundAssets() {
  auto tryLoadSound = [this](const std::string &name, const std::string &rel) {
    if (soundBuffers.find(name) != soundBuffers.end() &&
        soundBuffers[name].getSampleCount() > 0) {
      return;
    }
    const std::string prefixes[] = {"", "../", "../../", "../../../"};
    for (const auto &p : prefixes) {
      std::string path = p + rel;
      if (std::filesystem::exists(path)) {
        loadSoundBuffer(name, path);
        return;
      }
    }
  };

  tryLoadSound("blockbreak",     "assets/audio/effects/blockbreak.wav");
  tryLoadSound("blockhit",       "assets/audio/effects/blockhit.wav");
  tryLoadSound("bowserfall",     "assets/audio/effects/bowserfall.wav");
  tryLoadSound("bowserfire",     "assets/audio/effects/bowserfire.wav");
  tryLoadSound("cannonfire",     "assets/audio/effects/cannonfire.wav");
  tryLoadSound("castleclear",    "assets/audio/effects/castleclear.wav");
  tryLoadSound("coin",           "assets/audio/effects/coin.wav");
  tryLoadSound("death",          "assets/audio/effects/death.wav");
  tryLoadSound("fireball",       "assets/audio/effects/fireball.wav");
  tryLoadSound("flagraise",      "assets/audio/effects/flagraise.wav");
  tryLoadSound("gameover",       "assets/audio/effects/gameover.wav");
  tryLoadSound("jump",           "assets/audio/effects/jump.wav");
  tryLoadSound("kick",           "assets/audio/effects/kick.wav");
  tryLoadSound("oneup",          "assets/audio/effects/oneup.wav");
  tryLoadSound("pause",          "assets/audio/effects/pause.wav");
  tryLoadSound("pipe",           "assets/audio/effects/pipe.wav");
  tryLoadSound("powerupappear",  "assets/audio/effects/powerupappear.wav");
  tryLoadSound("powerupcollect", "assets/audio/effects/powerupcollect.wav");
  tryLoadSound("shrink",         "assets/audio/effects/shrink.wav");
  tryLoadSound("stomp",          "assets/audio/effects/stomp.wav");
  tryLoadSound("timertick",      "assets/audio/effects/timertick.wav");
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
