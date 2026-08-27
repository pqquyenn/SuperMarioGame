#pragma once

#include <SFML/Graphics/Texture.hpp>

#include <string>

class AssetManager;

class EntityAssetProvider {
public:
    virtual ~EntityAssetProvider() = default;

    virtual const sf::Texture& getTexture(
        const std::string& name) const = 0;
};

class AssetManagerEntityAssetProvider final : public EntityAssetProvider {
public:
    explicit AssetManagerEntityAssetProvider(AssetManager& assets);

    const sf::Texture& getTexture(
        const std::string& name) const override;

private:
    AssetManager& assets;
};
