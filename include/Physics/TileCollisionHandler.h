#pragma once

#include "Physics/CollisionContact.h"

class Entity;
class Tile;
class TileMap;
struct TileHandle;

class TileCollisionHandler {
public:
    virtual ~TileCollisionHandler() = default;

    virtual void onTileCeilingContact(
        Entity& entity,
        TileMap& map,
        Tile& tile,
        const TileHandle& handle,
        const CollisionContact& contact) = 0;

    virtual void onTileOverlap(
        Entity& entity,
        TileMap& map,
        Tile& tile,
        const TileHandle& handle,
        const CollisionContact& contact) = 0;
};
