# Entity Rendering Responsibilities

## Question

> In the current structure, is there an issue because no one is responsible for
> rendering entities? What do you think?

## Answer

Yes, the current structure has an integration gap, but it is more accurate to
say that rendering responsibility is fragmented rather than completely
unassigned.

### Suggested ownership

| Rendering responsibility | Owner |
|---|---|
| Basic sprite drawing through `Entity::render()` | Mân |
| Player texture, form appearance, facing, and animation | Mân |
| Enemy and item appearance and animation | Minh |
| Tile-map rendering | Nhật |
| Loading and caching textures through `AssetManager` | Quyền |
| Calling everything in the correct render order through `PlayState` | Quyền |
| HUD rendering | Minh |

### Intended rendering flow

```cpp
void PlayState::render(sf::RenderWindow& window) {
    window.clear(backgroundColor);

    camera.applyTo(window);

    level.renderTiles(window);

    for (auto& item : level.getItems())
        item->render(window);

    for (auto& enemy : level.getEnemies())
        enemy->render(window);

    mario.render(window);

    for (auto& projectile : projectiles)
        projectile->render(window);

    window.setView(window.getDefaultView());
    hud.render(window);
}
```

### Current missing integration

- `PlayState::render()` calls only `level.render()`.
- `Level::render()` draws only the `TileMap`.
- No player is owned by `PlayState`.
- No enemy or item collection is rendered by `Level` or `PlayState`.
- Player textures are never assigned.
- `HUD::render()` is empty.

Player rendering is currently only partly complete. `Character::render()` can
draw a sprite, but nothing assigns its texture or animation frame. The project
plan says `PlayerState` changes the form's size, spritesheet, and abilities, so
player form visuals are partly Mân's responsibility.

### Recommended separation

```text
Player code:
Player form/action -> choose visual or animation identity

AssetManager:
Visual identity -> provide the corresponding texture

PlayState:
Create Mario, attach assets, and call update/render

Camera:
Choose the visible area of the world
```

`Character` should not load files directly because that would couple player
mechanics to the filesystem and asset-management implementation. Instead,
Character should accept textures or animation configuration supplied by
`PlayState` and `AssetManager`.

The team should explicitly assign world-rendering orchestration to Quyền. Mân
should complete only the player-facing visual API and player animation behavior,
without editing `PlayState`, `Level`, enemy/item rendering, or `AssetManager`
unless the responsible teammates coordinate those changes.
