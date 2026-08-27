#pragma once

class EntityAssetProvider;
class EntityFactory;

// The provider must outlive every creator stored in the target factory.
void registerDefaultEntityTypes(
    EntityFactory& factory,
    EntityAssetProvider& assets);
