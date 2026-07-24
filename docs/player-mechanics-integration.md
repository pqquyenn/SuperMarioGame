# Player Mechanics Integration Guide

This file describes how other subsystems should integrate with the completed
player-mechanics APIs. The player branch does not modify teammate-owned source
files.

## PlayState

`PlayState` should own each player and one `InputHandler` per locally controlled
player. Because `GameState::handleInput()` does not receive delta time, process
real-time player controls at the beginning of `PlayState::update(dt)`.

```cpp
void PlayState::update(float dt) {
    marioInput.handleInput(mario, dt);
    mario.update(dt);

    CollisionManager::resolveCharacterMapCollision(
        mario,
        level.getTileMap()
    );

    camera.update(mario.getPosition());
}
```

The recommended interaction priority in each frame is:

1. Forced-death conditions such as falling below the level
2. Harmful enemy or hazard collisions
3. Collectible-item collisions
4. Terrain correction and grounded-state updates

The exact physics sequence may be adjusted by the collision owner, but it must
be deterministic and documented.

## CollisionManager

Character exposes the required terrain-collision API:

```cpp
character.getBounds();
character.getPosition();
character.setPosition(x, y);
character.getVelocity();
character.setVelocity(x, y);
character.setGrounded(true);
```

`Character::getBounds()` is independent of the sprite texture and changes with
Small/Super/Fire form height. CollisionManager should use it instead of reading
sprite data.

Normal enemy damage:

```cpp
character.takeDamage();
```

Star contact:

```cpp
if (character.defeatsEnemiesOnContact()) {
    enemy.setActive(false);
} else {
    character.takeDamage();
}
```

Void or forced death:

```cpp
character.die(DeathCause::Void);
```

Invincibility and shields intentionally do not block `die()`.

## Items

Items should apply their reward to a `Character` and become inactive only when
the Character accepts it. A suitable teammate-owned interface is:

```cpp
class Character;

class Item : public Entity {
public:
    virtual bool tryCollect(Character& character) = 0;
};
```

Mushroom:

```cpp
bool Mushroom::tryCollect(Character& character) {
    const bool accepted = character.receivePowerUp(
        std::make_unique<SuperState>()
    );

    if (accepted) {
        setActive(false);
    }

    return accepted;
}
```

Fire Flower:

```cpp
bool FireFlower::tryCollect(Character& character) {
    const bool accepted = character.receivePowerUp(
        std::make_unique<FireState>()
    );

    if (accepted) {
        setActive(false);
    }

    return accepted;
}
```

Character applies the classic tier rule automatically:

```text
Small + advanced form -> Super
Powered + advanced form -> requested form
```

Star:

```cpp
const bool accepted = character.addEffect(
    std::make_unique<StarEffect>(10.f)
);
```

Shield:

```cpp
const bool accepted = character.addEffect(
    std::make_unique<ShieldEffect>(3)
);
```

The Item should deactivate only if `accepted` is true.

## Projectiles

`PlayState` or another world-owning system should register a projectile
request handler:

```cpp
mario.setProjectileRequestHandler(
    [this](const ProjectileRequest& request) {
        // Create the requested projectile and add it to the world.
    }
);
```

The request contains:

- Projectile type
- Spawn position
- Facing direction

Character does not own the world entity container.

## Observer and HUD

Character is a `Subject` and emits:

```text
PLAYER_HIT
PLAYER_DIED
POWERUP_COLLECTED
```

PlayState should register HUD as an observer:

```cpp
mario.addObserver(&hud);
```

`PLAYER_DIED` stores the `DeathCause` numeric value in `GameEvent::value`.
The gameplay/session system should own the authoritative lives count, handle
respawning, and enter Game Over when no lives remain.

Respawn:

```cpp
character.respawn(spawnX, spawnY);
```

Respawn clears Effects, resets motion, reactivates the player, and restores
SmallState.

## Form Visuals

PlayerState already updates:

- Current form name
- Collision-body height
- Player feet position when growing or shrinking

The asset/animation integration can query:

```cpp
character.getCurrentFormName();
```

Entity provides:

```cpp
character.setTexture(texture, true);
character.setTextureRect(frame);
```

Use separate Small, Super, and Fire animation frames. Do not vertically stretch
the Small sprite to simulate Super form.

## Input Bindings

Create one `InputHandler` per player. Default controls preserve the original
combined keyboard mappings.

For simultaneous players, give each handler independent bindings:

```cpp
InputBindings marioKeys;
marioKeys.moveLeft = {sf::Keyboard::Left};
marioKeys.moveRight = {sf::Keyboard::Right};
marioKeys.jump = {sf::Keyboard::Up};
marioKeys.action = {sf::Keyboard::RControl};
marioKeys.run = {sf::Keyboard::RShift};

InputBindings luigiKeys;
luigiKeys.moveLeft = {sf::Keyboard::A};
luigiKeys.moveRight = {sf::Keyboard::D};
luigiKeys.jump = {sf::Keyboard::W};
luigiKeys.action = {sf::Keyboard::J};
luigiKeys.run = {sf::Keyboard::K};

InputHandler marioInput{marioKeys};
InputHandler luigiInput{luigiKeys};
```

## Core Loop

The Core owner should clamp unusually large delta times after pausing,
debugging, or dragging the window:

```cpp
dt = std::min(dt, 1.f / 15.f);
```

This reduces tunneling and large one-frame player movement. Collision handling
is still responsible for resolving actual terrain intersections.
