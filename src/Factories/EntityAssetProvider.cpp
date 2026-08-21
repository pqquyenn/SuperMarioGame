#include "Factories/EntityAssetProvider.h"

#include "Core/AssetManager.h"

AssetManagerEntityAssetProvider::AssetManagerEntityAssetProvider(
    AssetManager& assets)
    : assets{assets} {}

const sf::Texture& AssetManagerEntityAssetProvider::getTexture(
    const std::string& name) const {
    return assets.getTexture(name);
}
