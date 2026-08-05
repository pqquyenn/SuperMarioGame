# Player Animation Integration

This document describes the implemented player animation component and the
remaining texture-loading work owned by the gameplay/asset integration.

## Architecture

The implementation uses composition:

```text
Entity
    owns and renders sf::Sprite

Character
    owns SpriteAnimator and PlayerAnimationProfile

SpriteAnimator
    advances shared AnimationClip frame data

PlayerAnimationProfile
    maps form name + player motion to a clip
```

`SpriteAnimator` does not depend on Character, PlayerState, AssetManager,
collision, or input, so Enemy and Item code may reuse it later.

## Supported Player Motions

The initial enum contains only motions supported by current gameplay and the
current texture:

```text
Idle
Running
Jumping
Sliding
Dead
```

Rising and falling share the Jumping frame. Swimming, climbing, crouching, and
transformation animations should be added only when those mechanics exist.

## Current PlayerSpriteSheet Layout

Texture:

```text
assets/textures/characters/PlayerSpriteSheet.png
```

Size:

```text
403 x 266 pixels
```

Frame columns use a 16-pixel stride starting at `x = 1`:

| Motion | Frame index | X coordinate |
|---|---:|---:|
| Idle | 0 | 1 |
| Dead | 1 | 17 |
| Running | 2, 3, 4 | 33, 49, 65 |
| Sliding | 5 | 81 |
| Jumping | 6 | 97 |

Player/form rows currently used:

| Visual set | Y | Frame size |
|---|---:|---:|
| Mario Small | 9 | 16 x 16 |
| Mario Super | 25 | 16 x 32 |
| Luigi Small | 73 | 16 x 16 |
| Luigi Super | 89 | 16 x 32 |
| Fire powered form | 153 | 16 x 32 |

Small, Super, and Fire are still gameplay PlayerStates. They are not animation
enum values. The State form name selects a registered animation set.

## Texture Loading Contract

The AssetManager/PlayState owner should load the complete player texture once,
keep it alive, and assign the shared texture before rendering players:

```cpp
sf::Texture& playerTexture =
    AssetManager::getInstance().getTexture("PlayerSpriteSheet");

mario.setTexture(playerTexture);
luigi.setTexture(playerTexture);
```

The texture must be loaded successfully before `getTexture()` is called.
Character automatically selects the texture rectangle during `update(dt)`.

## Luigi Background Warning

The Luigi band (`y = 62..121`) uses an opaque blue background with RGB
`(27, 89, 153)`. Loading the PNG directly will render blue rectangles around
Luigi.

Do not apply that color as a global mask to the entire sheet. The same exact RGB
also occurs in valid Mario, Fire, and palette pixels. Use one of these safe
approaches:

1. Commit a cleaned atlas where only the Luigi-region background becomes
   transparent.
2. Extract and mask the Luigi region separately, producing a dedicated Luigi
   texture and corresponding local rectangles.
3. Replace the source sheet with a correctly transparent equivalent while
   preserving the documented frame coordinates.

The cleaned-atlas option is the simplest integration if the team approves an
asset change.

## Runtime Selection

Character selects motion in this order:

```text
Inactive -> Dead
Airborne -> Jumping
No horizontal velocity -> Idle
Facing opposite current velocity -> Sliding
Otherwise -> Running
```

The Dead frame is mapped and tested, but the current `Character::die()`
deactivates the Entity immediately, and inactive Entities are not rendered.
Displaying the death clip therefore requires the PlayState/session owner to add
a short death-animation phase before final deactivation or respawn.

Right-facing frames are reused for the left direction using negative X scale
and a frame-width origin. Collision dimensions and feet positioning remain
independent from sprite animation.

## Extending Forms

A future PlayerState such as Ice can register another `PlayerAnimationSet` in
the profile. SpriteAnimator itself does not need modification.
