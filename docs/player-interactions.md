# Player Interaction Rules

This document defines the intended interactions between playable characters,
player forms, temporary effects, items, damage, and external game systems.

## Implementation Status

The player-mechanics code currently implements:

- Profile-driven Mario, Luigi, and future characters
- One active State and multiple active Effects
- Classic tier-aware power-up transitions
- NES-style `Fire -> Small` damage
- Timed hit invincibility, Star, and durable shield Effects
- Forced death, Effect cleanup, and Small-form respawning
- Form-dependent collision-body height with feet-position preservation
- Generic special-action and projectile-request APIs
- Configurable input bindings
- Observer events for hits, deaths, and collected power-ups

The following integration remains for the responsible teammates:

- Assign final textures and animation frames to each form
- Create actual projectile entities from projectile requests
- Connect Items, CollisionManager, PlayState, HUD, lives, and Game Over

## Player Composition

A playable character consists of:

- One permanent `CharacterProfile` containing base statistics and innate
  capabilities.
- Exactly one active `PlayerState` representing the current form.
- Zero or more active `PlayerEffect` objects representing independent,
  usually temporary modifiers.

Example:

```text
Profile: Luigi
State: FireState
Effects: StarEffect + SpeedBoostEffect
```

## Character Profiles

Profiles contain permanent character differences.

| Character | Main advantage |
|---|---|
| Mario | Faster acceleration and horizontal movement |
| Luigi | Stronger jump, more held-jump lift, and longer jump hold |

Adding a new character should normally require a new profile and concrete
character class, without copying shared movement, State, Effect, damage, or
power-up methods.

## Player States

`PlayerState` represents one mutually exclusive form:

```text
SmallState OR SuperState OR FireState OR IceState OR another form
```

Changing State replaces the previous State. A State controls form-level
properties such as body size, animation set, brick-breaking capability,
special action, and damage downgrade.

## Player Effects

`PlayerEffect` represents an additional modifier that can coexist with the
current State and with other Effects.

Examples:

- Damage invincibility
- Star invincibility
- Shield
- Speed boost
- Jump boost

Effects may use a timer, durability, or another expiration rule. State changes
do not remove active Effects unless a specific game rule requires it.

## Classic Tiered Power-Up Rules

This project uses a generalized version of the original NES tier progression.
Advanced power-ups are always consumed, but Small characters first become
Super instead of immediately receiving the advanced form.

| Current State | Collected Item | Result |
|---|---|---|
| Small | Mushroom | Super |
| Small | Fire Flower | Super |
| Small | Ice Flower | Super |
| Small | Other advanced form item | Super |
| Super | Fire Flower | Fire |
| Super | Ice Flower | Ice |
| Super | Other advanced form item | Corresponding form |
| Fire | Ice Flower | Ice |
| Ice | Fire Flower | Fire |
| Same advanced form | Matching item | Remain in that form; optionally award points |

The item is removed only when the player's power-up receiving method returns
success.

## Power-Up Contact Sequence

```text
CollisionManager detects player-item contact
    -> Item requests that the player receive a power-up
        -> Character checks its current State at that moment
            -> Character performs the appropriate transition
                -> Item is removed after successful application
```

Eligibility must be checked at collection time. For example, a player may
release a Fire Flower as Super, take damage before touching it, and reach it as
Small. Under the classic tiered rule, collecting it then produces SuperState.

## Normal Damage

The selected classic NES damage progression is:

```text
Fire -> Small
Ice -> Small
Other advanced form -> Small
Super -> Small
Small -> death
```

If the team deliberately chooses a more forgiving modern rule, advanced forms
may downgrade to Super first. That alternative must be applied consistently to
every concrete State.

After a nonlethal hit, Character automatically adds a brief
`DamageInvincibilityEffect` so one continuous collision cannot cause repeated
damage across several frames.

## Effect Interaction with Damage

Effects inspect damage before the current State processes it.

```text
Damage event
    -> Active Effects may absorb or block it
        -> If not blocked, current PlayerState downgrades or causes death
```

Examples:

- `DamageInvincibilityEffect` blocks damage until its timer expires.
- `StarEffect` blocks damage for its duration and is not consumed by defeating
  one enemy.
- `ShieldEffect` owns its own durability and loses one charge when it absorbs
  damage.

Shield durability is not Character health. The classic player does not need a
numeric HP field because the current form acts as the damage layer.

## Void and Forced Death

Falling into a void, being crushed, or running out of time calls
`Character::die(DeathCause)` and bypasses normal damage protection.

```text
Forced death
    -> Clear active Effects
    -> Clear velocity
    -> Deactivate the player
    -> Gameplay system decrements lives and handles respawn or Game Over
```

Star, shield, and hit-invincibility Effects do not protect against a void.

## Special Actions

Input and Commands request one generic special action:

```text
InputHandler
    -> Command
        -> Character::useSpecialAbility()
            -> current PlayerState decides the behavior
```

Examples:

- Small and Super: no special action
- Fire: fireball
- Ice: iceball
- Cape: cape action or glide activation
- Raccoon: tail action or flight activation

Adding a new State should not require changing `InputHandler` or the Command
interface.

## System Ownership

| Responsibility | Owner |
|---|---|
| Detect player-item or player-enemy contact | Collision system |
| Create, move, and remove items | Item system |
| Decide player transition and capability result | Character / PlayerState |
| Maintain temporary modifiers | Character / PlayerEffect |
| Apply character movement and jump physics | Character |
| Create/update/render players during gameplay | PlayState |
| Decrement lives, respawn, and enter Game Over | Gameplay/session system |
| Update score, lives, and HUD | Observer/UI system |

Collision and Item code should use public Character methods and should not
inspect concrete State classes with `dynamic_cast` or character-type
`if-else` chains.

## Deterministic Collision Order

If damage and item collection occur during the same frame, the gameplay team
must use a documented order. Recommended order:

1. Resolve lethal environment conditions.
2. Resolve harmful collisions.
3. Resolve collectible collisions.
4. Resolve movement and grounded corrections as required by the physics loop.

This ensures that power-up collection checks the player's current State after
damage has been resolved.
