# Trạng thái SOLID của Đặng Minh Nhật

Tài liệu này chỉ theo dõi các phần thuộc trách nhiệm của **Đặng Minh Nhật**
trong `docs/solid-violations-and-refactoring-plan.md`.

## Tổng quan

| SOLID | Trạng thái | Kết quả |
|---|---|---|
| SOLID-03 | Hoàn thành | Level runtime dùng manifest `.level`; content, portal, camera và stage transition được điều khiển bằng dữ liệu. |
| SOLID-04 | Hoàn thành phần của Nhật | Generic collision chỉ thực hiện detection/separation và chuyển gameplay reaction qua polymorphic hooks. |
| SOLID-07 | Hoàn thành | Factory là registry thuần; asset provider và default registration được inject tại composition root. |
| SOLID-11 | Hoàn thành phần của Nhật | Có automated tests cho AABB, tile separation, level parser, portal và factory lookup. |

## SOLID-03 — Data-driven level

- `.level` là entry point của gameplay; `.txt` chỉ còn là terrain được manifest
  tham chiếu.
- Enemy, item, platform, block content, portal, anchor, camera zone và stage
  rules được đọc từ data.
- `Level.cpp` không còn level ID, map-specific warp helpers hoặc exact
  hard-coded block coordinates.
- Validator kiểm tra alias, record, file, ID, portal target, camera coverage và
  `next_stage`.
- Test: `solid03_level_definition_tests` và
  `solid03_portal_runtime_tests`.

## SOLID-04 — Generic collision boundary

- `CollisionManager` không đọc `sf::Keyboard`, không branch theo level ID hoặc
  tọa độ map và không biết concrete enemy/item.
- AABB contact, tile separation, grounded, headroom và moving-platform
  correction vẫn nằm trong physics.
- Gameplay reactions đi qua `Entity` hooks và `TileCollisionHandler`.
- Test: `solid04_collision_tests` và `solid04_tile_collision_tests`.

## SOLID-07 — Testable entity factory

- `EntityFactory` chỉ giữ `registerType()` và `create()`; unknown type trả
  `nullptr`.
- Runtime tiếp tục dùng `EntityFactory::getInstance()`, nhưng test có thể tạo
  factory cục bộ với registry độc lập.
- `EntityAssetProvider` tách texture lookup khỏi factory.
- `AssetManagerEntityAssetProvider` là runtime adapter.
- `registerDefaultEntityTypes()` sở hữu concrete creators và được gọi tại
  `Game` composition root.
- Đã xóa `registerDefaultEntities()` và `static bool initialized`.
- `Level` và `LevelWorldBuilder` không còn tự đăng ký defaults.
- Test: `solid07_factory_tests`, gồm 16 checks với fake/no-op provider.

## SOLID-11 — Automated tests thuộc Nhật

Các subsystem được giao cho Nhật đều có normal và edge/failure cases:

- AABB: overlap, contact side và separated rectangles.
- Tile resolution: landing, ceiling, wall, non-solid overlap, skipped collision,
  crawl headroom và moving platform.
- Level parser: valid manifests, malformed records, alias/file/ID/navigation
  failures.
- Portal: wrong direction/area, anchor destination, camera behavior, full 1-2
  portal chain và next-stage chain.
- Factory: custom/default lookup, unknown type, fake provider, registry isolation
  và repeatable registration.

## Cách xác minh

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure

cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Kết quả gần nhất:

```text
Debug:   6/6 tests passed
Release: 6/6 tests passed
```

## Kết luận

SOLID-03, SOLID-04, SOLID-07 và phần test SOLID-11 thuộc trách nhiệm của Nhật
đã hoàn thành theo acceptance criteria hiện tại. Những dependency asset nằm
bên trong animation của concrete enemy không thuộc phạm vi SOLID-07 này.
