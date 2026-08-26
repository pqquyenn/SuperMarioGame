# EntitySpawner experiment

This folder is an isolated prototype. It is intentionally outside `src/`,
`include/`, and the targets listed in `CMakeLists.txt`, so it does not affect
the Mario executable or the normal test suite.

## Model

An `EntitySpawnerConfig` contains:

- a stable spawner ID;
- the entity identity/factory name;
- the spawn position;
- the spawn condition;
- initial and respawn delays;
- optional random interval variation;
- maximum simultaneously alive entities;
- an optional total spawn limit;
- initial enabled/spawn behavior and a deterministic random seed.

Supported conditions are:

- `AfterTime`: periodic spawning while capacity is available;
- `AfterEntityDisappeared`: useful for coins and temporary items;
- `AfterEnemyDefeated`: respawns only after a confirmed defeat;
- `External`: waits for game logic such as score, player proximity, a switch,
  or a match-time threshold to call `requestExternalSpawn()`.

`update()` returns a `SpawnRequest` instead of constructing a game entity.
This keeps the experiment independent from SFML and `EntityFactory`. A future
Level adapter can translate the request through `EntityFactory` and place the
result into the appropriate enemy or item collection.

The returned instance ID must later be supplied to `notifyEntityEnded()`.
If the caller cannot create the requested entity, it should call
`cancelSpawn()` so limits and counters are rolled back.

## Standalone test

The test is not part of CMake. It can be compiled manually without SFML:

```powershell
cl /std:c++17 /EHsc EntitySpawner.cpp entity_spawner_tests.cpp
./entity_spawner_tests.exe
```

The tests cover timers, disappearance and defeat conditions, external
triggers, enable/disable behavior, capacity and total limits, randomized
interval bounds, and failed-spawn rollback.
