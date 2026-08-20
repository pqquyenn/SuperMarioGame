# Nhật-Owned SOLID Plan After Dev Merge / Kế hoạch SOLID của Nhật sau merge `dev`

Audit date / Ngày rà soát: **2026-08-21**

Branch and commit / Nhánh và commit:
`feature/level-collision` at `df7389e` (`merge code from dev`)

Owner / Người phụ trách: **Đặng Minh Nhật — Level, Tilemap & Collision**

Source / Tài liệu nguồn:
[`docs/solid-violations-and-refactoring-plan.md`](docs/solid-violations-and-refactoring-plan.md)

## Scope / Phạm vi

This plan checks only the SOLIDs assigned to Nhật:

- **SOLID-03:** data-driven level content.
- **SOLID-04:** separation of generic physics from gameplay and level scripts.
- **SOLID-07:** factory dependency inversion.
- **SOLID-11:** Nhật's tests for parser, AABB, tile collision, portal, and
  factory lookup.

Kế hoạch này chỉ kiểm tra SOLID do Nhật phụ trách. SOLID-01, 02, 05, 06, 08,
09, 10 và 12 không nằm trong kế hoạch thực hiện này. SOLID-09 chỉ xuất hiện như
một dependency bên ngoài vì SOLID-07 cần asset-provider interface của Quyền.

## Audit baseline / Baseline kiểm tra

The current working tree builds in both configurations and all current tests
pass:

```text
Debug build: mario.exe and all test targets passed
Debug CTest: 4/4 passed
Release build: mario.exe and all test targets passed
Release CTest: 4/4 passed
```

However, this is not yet a reproducible clean-checkout baseline:

- `tests/collision_tests.cpp` is untracked.
- `tests/fixtures/invalid.level` is untracked.
- `tests/fixtures/malformed.level` is untracked.
- `tests/fixtures/missing-files.level` is untracked.
- `tests/tile_collision_tests.cpp` is untracked.
- `tests/fixtures/collision-map.txt` is untracked.
- `CMakeLists.txt` already references `tests/collision_tests.cpp`.
- `CMakeLists.txt` already references `tests/tile_collision_tests.cpp`.
- `tests/level_definition_tests.cpp` already references all three fixtures.

Vì vậy build hiện tại pass nhờ các file untracked trong working tree. Nếu clone
hoặc checkout sạch từ commit `df7389e`, CMake sẽ thiếu source của
`solid04_tests`, còn test SOLID-03 sẽ thiếu fixture.

## Current status / Trạng thái hiện tại

| SOLID | Status / Trạng thái | Evidence / Bằng chứng | Decision / Kết luận |
|---|---|---|---|
| SOLID-03 | Core implemented, not fully closed / Đã xong lõi, chưa đóng hoàn toàn | `.level` model, loader, builder, catalog and three manifests exist; normal menu/default paths use `.level`. `Level.cpp` still contains legacy `.txt`, `levelId`, coordinate warp, and debug hidden-map paths. | Finish legacy isolation and portal tests; do not rewrite the loader. |
| SOLID-04 | Implemented after merge / Đã khôi phục sau merge | `CollisionManager` now depends only on generic entity, tile, geometry, and tile-handler abstractions. Gameplay reactions use entity hooks and `TileCollisionHandler`; portal input remains in `PlayState`/`Level`; crawl/stand-up is preserved. Debug and Release SOLID-04 tests pass. | Commit the implementation and retain the architecture regression checks. |
| SOLID-07 | Not implemented / Chưa thực hiện | `EntityFactory` is a singleton; default creators call `AssetManager::getInstance()` and use `static bool initialized`; `LevelWorldBuilder` fetches the global factory itself. | Inject factory/assets and move default registration out of the factory core. |
| SOLID-11 (Nhật) | Partially implemented and repository-incomplete / Hoàn thành một phần, repository chưa đầy đủ | Parser/catalog, geometry, and tile/platform/headroom tests exist. New tests and fixtures are still untracked; portal and factory suites do not exist. | Commit the current safety net, then add portal and factory coverage. |

## Priority and execution order / Ưu tiên và thứ tự thực hiện

1. **P0 — SOLID-04 regression (implemented; commit pending):** generic collision
   separation is restored without losing crawl/stand-up changes from `dev`.
2. **P0 — SOLID-11 baseline:** add the currently untracked tests and fixtures
   to the repository and prove a clean checkout configures, builds, and tests.
3. **P1 — SOLID-03 closure:** isolate/remove legacy stage entry and hard-coded
   portal/debug paths; add portal/camera tests.
4. **P1 — SOLID-07:** inject factory construction dependencies and add factory
   tests.
5. **P2 — SOLID-11 completion:** finish tile/platform integration and static
   architecture checks.

## P0 — Repair the reproducible test baseline

### Goal / Mục tiêu

Make the repository build from committed files rather than relying on files
that happen to exist in the current working tree.

### Files / Các file

| File | English responsibility | Trách nhiệm tiếng Việt |
|---|---|---|
| `tests/collision_tests.cpp` | AABB/contact geometry regression tests | Test hồi quy AABB và hướng va chạm |
| `tests/fixtures/invalid.level` | Unknown alias validation fixture | Fixture alias không hợp lệ |
| `tests/fixtures/malformed.level` | Multi-error manifest fixture | Fixture manifest có nhiều lỗi |
| `tests/fixtures/missing-files.level` | Missing terrain/background fixture | Fixture thiếu terrain/background |
| `CMakeLists.txt` | Register reproducible CTest targets | Đăng ký test target có thể chạy lại |

### Tasks / Công việc

1. Add all four untracked test files to the repository.
2. Configure a fresh build directory to verify no generated/stale file masks a
   missing source.
3. Run Debug and Release builds and CTest.
4. Keep these files separate from generated `build/` output.

### Acceptance / Tiêu chí hoàn thành

- A clean clone/checkout can configure without “Cannot find source file”.
- `solid03_level_definition_tests` and `solid04_collision_tests` run without
  local-only fixtures.
- Debug and Release CTest both pass.

## SOLID-04 — Restore generic collision after merge

### Problem found / Vấn đề tìm thấy

The merge kept the new crawl feature but restored the old collision core.
Before this fix, `CollisionManager.cpp` contained:

- `sf::Keyboard::isKeyPressed()`;
- `Level*`, `getLevelId()`, map-specific coordinates and warp methods;
- scoring and block/item spawning;
- `dynamic_cast` branches for `Character`, `Enemy`, `Item`, and `Fireball`.

Merge đã giữ chức năng crawl nhưng đưa implementation collision cũ trở lại.
The implementation now satisfies the SOLID-04 boundary and preserves the crawl
feature. / Implementation hiện tại đã đáp ứng ranh giới SOLID-04 và giữ nguyên
chức năng crawl.

### Existing abstractions to reuse / Abstraction đã có cần tái sử dụng

- `CollisionGeometry` and `CollisionContact` for pure AABB/contact math.
- `Entity::beginTileCollision()`, `onLanded()`, `onWallCollision()`, and
  `shouldSkipTileCollision()`.
- `Character::onCollision()` for player/enemy interaction.
- `Item::onCollision()` and entity wall/landing hooks.
- `TileCollisionHandler`, implemented by `Level`, for block and coin reactions.
- `Level::tryActivatePortalForInput()` for portal input outside generic physics.
- `CollisionManager::tryStandUp()` for merged crawl headroom behavior.

### Target API / API mục tiêu

```cpp
static void resolveTileCollisions(
    Entity& entity,
    TileMap& map,
    TileCollisionHandler* tileHandler = nullptr
);

static void resolveEntityCollisions(Entity& a, Entity& b);

static bool tryStandUp(Character& character, const TileMap& map);
```

`tryStandUp()` remains because it is required by the merged crawl feature.
`tryEnterDownWarp()` must leave `CollisionManager`; portal activation belongs to
`Level`/input integration.

### Files / Các file

| File | English responsibility | Trách nhiệm tiếng Việt |
|---|---|---|
| `include/Physics/CollisionManager.h` | Generic physics API plus crawl headroom query | API physics tổng quát và kiểm tra khoảng đứng lên |
| `src/Physics/CollisionManager.cpp` | Separation, contacts, moving platform, stand-up query | Tách va chạm, contact, platform và kiểm tra đứng lên |
| `include/Physics/CollisionGeometry.h` | Pure collision math | Phép tính va chạm thuần |
| `include/Physics/TileCollisionHandler.h` | Boundary for tile gameplay reactions | Ranh giới phản ứng gameplay của tile |
| `include/Entities/Entity.h` | Polymorphic collision hooks | Hook va chạm đa hình |
| `src/Level/Level.cpp` | Block, coin, and portal behavior | Hành vi block, coin và portal |
| `src/States/PlayState.cpp` | Input-triggered portals and stand-up finalization | Portal theo input và hoàn tất trạng thái đứng |
| `src/States/WinState.cpp` | Preserve stand-up behavior during win sequence | Giữ hành vi đứng lên trong win sequence |

### Tasks / Công việc

1. Change tile collision to depend on `TileCollisionHandler*`, not `Level*`.
2. Delegate entity interaction through `onCollision()` in both directions.
3. Delegate landing, wall reversal/explosion, and collision-skip behavior to
   entity hooks.
4. Delegate question block, brick, coin, and similar map reactions to the
   `Level` tile handler.
5. Remove all keyboard reads and portal branches from physics.
6. Remove all level IDs, warp coordinates, score rules, and concrete
   enemy/item selection from physics.
7. Preserve `tryStandUp()` and all calls added by the crawl merge.
8. Ensure `PlayState`, `WinState`, enemy, item, and fireball callers compile
   against the restored API.

### Tests / Kiểm thử

Add `tests/tile_collision_tests.cpp` with lightweight fake entities and a fake
tile handler. Cover:

- landing and grounded callback;
- ceiling separation and tile-handler callback;
- left/right wall separation;
- enemy/item wall hook;
- fireball landing bounce and wall explosion;
- `shouldSkipTileCollision()`;
- crouched character blocked/free headroom;
- moving-platform top carry, side contact, and underside contact.

### Architecture checks / Kiểm tra kiến trúc

The following search must return no result in `src/Physics` except an allowed
`Character` reference for moving-platform/stand-up operations:

```text
sf::Keyboard
getLevelId
levelId
warpTo
warpPipe
spawnItemFromBlock
AssetManager
```

### Acceptance / Tiêu chí hoàn thành

- Generic collision does not read input or know a stage/map.
- Generic collision does not decide score, block content, portal destination,
  or concrete enemy/item behavior.
- Crawl and standing under a low ceiling still work after the refactor.
- AABB and tile resolution run in headless tests.
- Adding a basic enemy/item does not require editing `CollisionManager`.

## SOLID-03 — Close remaining legacy paths

### What is already complete / Phần đã hoàn thành

- `LevelDefinition`, loader, symbol catalog, and world builder exist.
- `1-1.level`, `1-2.level`, and `1-3.level` are tracked.
- Menu and `PlayState` default entry paths use `.level`.
- Enemy, item, platform, block, anchor, portal, camera, and stage rules are
  represented in manifests.
- Loader validation covers aliases, duplicate IDs, bounds, files, portal
  targets, and camera records.

Do not rewrite these components. Không viết lại các phần này.

### Remaining problem / Phần còn lại

`Level.cpp` and debug state flow still contain compatibility behavior:

- `loadHiddenMap("underground.txt")` from `PlayState`;
- fallback branches using `levelId` and exact warp coordinates;
- public map-specific methods such as `warpPipeA_Entry()` and
  `warpPipeC1_Exit()`.

Terrain remaining in `.txt` is intentional. The problem is using `.txt` as a
stage entry or using C++ map scripts, not using `.txt` as terrain referenced by
a `.level` manifest.

Terrain nằm trong `.txt` là đúng thiết kế. Phần cần loại bỏ/cô lập là `.txt`
được dùng làm entry point của stage và các script warp hard-code trong C++.

### Files / Các file

| File | English responsibility | Trách nhiệm tiếng Việt |
|---|---|---|
| `include/Level/Level.h` | Runtime level façade | API runtime của level |
| `src/Level/Level.cpp` | Manifest runtime, portal and compatibility flow | Runtime manifest, portal và luồng tương thích |
| `src/States/PlayState.cpp` | Debug level/area commands | Lệnh debug level/area |
| `assets/maps/1.1/1-1.level` | 1-1 areas, portals and camera zones | Area, portal và camera 1-1 |
| `assets/maps/1.2/1-2.level` | 1-2 areas, portals and camera zones | Area, portal và camera 1-2 |
| `assets/maps/1.3/1-3.level` | 1-3 stage rules | Rule màn 1-3 |
| `tests/level_definition_tests.cpp` | Manifest parser and stage regression | Test parser và hồi quy stage |
| `tests/level_portal_tests.cpp` *(new)* | Portal/area/camera behavior | Test hành vi portal/area/camera |

### Tasks / Công việc

1. Verify every production stage transition passes a `.level` path.
2. Replace the debug hidden-map load with a manifest anchor/area transition.
3. Remove map-specific warp helpers after their manifest portals are proven.
4. If legacy `.txt` stage entry must remain, isolate it in a clearly named
   compatibility adapter; do not mix it into the data-driven path.
5. Keep `next_stage`, kill plane, boundaries, camera zones, block content and
   portal destinations authoritative in each manifest.
6. Add runtime tests for portal activation, area changes, anchor velocity,
   camera selection, wrong activation direction, and missing targets.

### Acceptance / Tiêu chí hoàn thành

- Normal gameplay starts and transitions through `.level` files only.
- No production portal behavior branches on level ID or exact map coordinates.
- Adding/changing an enemy, item, platform, block, portal, anchor, camera zone,
  or rule requires data changes only.
- Existing 1-1, 1-2, and 1-3 content remains unchanged.
- Legacy compatibility, if retained, is isolated and tested separately.

## SOLID-07 — Remove global dependencies from entity construction

### Problem found / Vấn đề tìm thấy

- `EntityFactory` is only accessible as a singleton.
- `registerDefaultEntities()` owns all concrete registration and texture
  selection.
- Every creator reaches `AssetManager::getInstance()`.
- `static bool initialized` leaks initialization state across tests.
- `LevelWorldBuilder` fetches and initializes the global factory itself.
- Unknown types return a silent `nullptr`.

### Dependency / Dependency bên ngoài

Consume the narrow asset-provider interface supplied by SOLID-09's owner. Nhật
only integrates that interface into entity construction; redesigning the full
asset catalog is outside this plan.

### Proposed API direction / Hướng API đề xuất

```cpp
struct EntityCreationContext {
    IAssetProvider& assets;
    TileMap* tileMap;
};

using CreatorFunc = std::function<std::unique_ptr<Entity>(
    const sf::Vector2f&,
    const EntityCreationContext&
)>;
```

The concrete shape may change during implementation, but dependencies must be
explicit and testable.

### Files / Các file

| File | English responsibility | Trách nhiệm tiếng Việt |
|---|---|---|
| `include/Factories/EntityFactory.h` | Isolated registry API | API registry độc lập |
| `src/Factories/EntityFactory.cpp` | Lookup and creation only | Chỉ lookup và tạo entity |
| `include/Factories/EntityCreationContext.h` *(new)* | Explicit creation dependencies | Dependency tạo entity tường minh |
| `src/Factories/DefaultEntityRegistration.cpp` *(new)* | Built-in entity registration | Đăng ký entity mặc định |
| `include/Level/LevelWorldBuilder.h` | Injected factory consumer | Consumer nhận factory qua injection |
| `src/Level/LevelWorldBuilder.cpp` | Build typed runtime collections | Tạo collection runtime đúng kiểu |
| `tests/entity_factory_tests.cpp` *(new)* | Registry/fake-provider tests | Test registry/fake provider |

### Tasks / Công việc

1. Make an isolated `EntityFactory` instance constructible for tests and
   application composition.
2. Inject the factory into `LevelWorldBuilder`; remove `getInstance()` and
   `registerDefaultEntities()` calls from `build()`.
3. Move built-in registration to a composition-root registration function.
4. Pass assets and tile-map services through `EntityCreationContext`.
5. Remove `static bool initialized`; define and test duplicate-registration
   behavior explicitly.
6. Return a structured error containing the unknown type name.
7. Where practical, move RedKoopa's tile-map setup into its creator context so
   the builder does not branch on that concrete type.

### Tests / Kiểm thử

Cover:

- register and create a known type;
- unknown type and readable error;
- duplicate registration policy;
- two isolated factories do not share state;
- fake/no-op asset provider;
- enemy/item type mismatch reported by the world builder;
- context-dependent entity creation without a global singleton.

### Acceptance / Tiêu chí hoàn thành

- `EntityFactory::create()` performs registry lookup and invokes a creator; it
  does not select textures or know all concrete entities.
- Factory creators do not call `AssetManager::getInstance()`.
- `LevelWorldBuilder` receives a factory dependency explicitly.
- Factory tests run without loading production assets.
- Unknown entity names fail with a clear structured message.

## SOLID-11 — Nhật's final test matrix

| Test target | Current state / Hiện tại | Required completion / Phần cần hoàn thành |
|---|---|---|
| `solid03_level_definition_tests` | Test source tracked; required fixtures untracked | Track fixtures; retain valid/invalid alias, path, bounds and three-stage cases. |
| `solid03_portal_tests` | Missing | Add activation, wrong area/direction, anchor, velocity, camera and final-stage cases. |
| `solid04_collision_tests` | CMake target tracked; test source untracked | Track geometry test and retain top/bottom/left/right/edge cases. |
| `solid04_tile_collision_tests` | Implemented; source and fixture untracked | Track the files; retain tile separation, hooks, crawl headroom and moving-platform cases. |
| `solid07_factory_tests` | Missing | Add isolated registry, fake provider, duplicate and unknown-type cases. |

All suites must be headless and must contain at least one normal case and one
failure/edge case. Tất cả test phải chạy không cần mở SFML window.

## Pull request plan / Kế hoạch PR

### PR 1 — Test baseline repair

- Track the four current untracked test files.
- Prove clean configure/build/CTest in Debug and Release.
- No gameplay changes.

### PR 2 — SOLID-04 merge regression fix (implemented; commit pending)

- Restored `TileCollisionHandler` and entity-hook delegation.
- Preserved crawl/stand-up behavior from `dev`.
- Added tile, headroom, and moving-platform regression tests.

### PR 3 — SOLID-03 legacy closure

- Move remaining debug/portal behavior to manifest areas and anchors.
- Isolate or remove legacy stage-entry code.
- Add portal/camera runtime tests.

### PR 4 — SOLID-07 factory injection

- Consume the agreed asset-provider interface.
- Inject factory/context and move default registration.
- Add factory tests.

Do not combine architecture refactoring with map layout changes, new enemies,
or movement balancing. Không gộp refactor kiến trúc với chỉnh map, thêm enemy
hoặc cân bằng movement trong cùng PR.

## Verification / Kiểm tra

Run from a clean repository root:

```powershell
cmake -S . -B build-clean
cmake --build build-clean --config Debug
ctest --test-dir build-clean -C Debug --output-on-failure
cmake --build build-clean --config Release
ctest --test-dir build-clean -C Release --output-on-failure
```

Manual regression for Nhật's scope:

1. Start 1-1, 1-2 and 1-3 from `.level` paths.
2. Verify every Down/Right portal and camera area.
3. Verify hidden areas do not incorrectly follow Mario when their camera is
   fixed.
4. Verify block contents, including the 1-2 StarItem brick at tile `(40, 19)`.
5. Verify landing, ceilings, walls, moving platforms, fireball bounce/explosion,
   and crouch-to-stand under clear/blocked headroom.
6. Verify unknown manifest aliases and unknown factory types report readable
   errors.

## Definition of done for Nhật / Định nghĩa hoàn thành của Nhật

- SOLID-03 normal gameplay is fully manifest-driven; legacy behavior is
  isolated or removed.
- SOLID-04 generic physics contains no keyboard, level script, map coordinate,
  scoring, block-content, or concrete enemy/item rules.
- SOLID-04 preserves the crawl/stand-up feature merged from `dev`.
- SOLID-07 entity construction has explicit factory/asset dependencies and can
  be tested with isolated fakes.
- All parser, AABB, tile, portal, platform, headroom, and factory tests are
  committed and pass from a clean checkout.
- Debug and Release builds pass without relying on untracked files.
