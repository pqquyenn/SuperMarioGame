# SOLID-03 Walkthrough / Hướng dẫn SOLID-03

## English

### What changed

`.level` is now the normal entry point for a playable stage. The terrain is
still kept in the related ASCII `.txt` file, but gameplay metadata is owned by
the manifest:

```text
assets/maps/1.1/1-1.level  ->  terrain=1-1.txt
assets/maps/1.2/1-2.level  ->  terrain=1-2.txt
assets/maps/1.3/1-3.level  ->  terrain=1-3.txt
```

The menu and the default `PlayState` paths use `.level`. `next_stage` in a
manifest controls level completion; an empty value means the final stage.

### Runtime loading flow

1. `PlayState` calls `Level::loadLevel("1.1/1-1.level")`.
2. `LevelDefinitionLoader` locates the manifest and loads
   `assets/config/entities.catalog`.
3. Relative `terrain` and `background` paths are resolved from the manifest’s
   directory.
4. The loader parses records, resolves entity aliases, and validates files,
   coordinates, IDs, portals, platforms, blocks, and camera zones.
5. `LevelWorldBuilder` uses `EntityFactory` to create enemies, items, and
   moving platforms from the validated definition.
6. `Level` keeps the definition and exposes portal, camera, boundary, kill
   plane, area, and next-stage data to gameplay code.

### Files added

- `include/Level/LevelDefinition.h` — manifest data structures.
- `include/Level/LevelDefinitionLoader.h` and
  `src/Level/LevelDefinitionLoader.cpp` — parser, path resolution, and
  validation.
- `include/Level/EntitySymbolCatalog.h` and
  `src/Level/EntitySymbolCatalog.cpp` — runtime alias catalog loader.
- `include/Level/LevelWorldBuilder.h` and
  `src/Level/LevelWorldBuilder.cpp` — runtime enemy/item/platform creation.
- `assets/config/entities.catalog` — aliases used by `.level` files.
- `assets/maps/1.1/1-1.level`, `assets/maps/1.2/1-2.level`, and
  `assets/maps/1.3/1-3.level` — migrated stage manifests.
- `tests/level_definition_tests.cpp` and `tests/fixtures/` — parser and
  validation tests.

### Files changed

- `src/Level/Level.cpp` and `include/Level/Level.h` now load and retain a
  `LevelDefinition`, build runtime objects from it, and consume data-driven
  portals, camera zones, block contents, boundaries, and rules.
- `src/Physics/CollisionManager.cpp` asks `Level` to activate manifest portals
  for data-driven stages. The old coordinate-based warp path remains only for
  legacy maps.
- `MenuState.cpp`, `PlayState.h/.cpp`, `WinState.h/.cpp`, and
  `LevelCompleteState.h/.cpp` now pass `.level` paths and use `next_stage`.
- `docs/symbol.json` documents the same `entity_symbols` mappings as the
  runtime catalog.

### Entity aliases

The catalog format is one record per line:

```text
<enemy|item> <symbol> <EntityFactory type>
```

Current aliases:

```text
E1 Goomba              E2 UndergroundGoomba
E3 Koopa               E4 RedKoopa
E5 GreenParatroopa     E6 RedParatroopa
E7 PiranhaPlant
I1 Coin                I2 Mushroom
I3 1UpMushroom         I4 FireFlower
I5 StarItem
```

Example manifest records:

```text
[entities]
entity id=g01 symbol=E1 area=overworld tile=24,12 direction=-1
entity id=plant01 symbol=E7 area=underground tile=116.5,25 direction=0

[items]
item id=coin01 symbol=I1 area=overworld tile=30,8
```

The alias is resolved by the catalog, not by a C++ `switch`. An unknown alias
reports the manifest path and source line, for example:

```text
assets/maps/example/example.level:18: unknown entity symbol 'E99'
```

### Coordinates and record rules

- `tile=x,y` and `bounds=x,y,w,h` use tile coordinates.
- Fractional tile coordinates are supported and are not rounded during load.
- `speed` and anchor `velocity` use pixels per second.
- `direction` is `-1`, `0`, or `1`.
- IDs are unique within their record category.
- All positions, portal triggers, and camera rectangles must fit inside the
  terrain dimensions.
- `platform bounds=min,max` follows the platform’s movement axis.
- `PowerupByForm` chooses Mushroom or FireFlower from the player form.
- Block contents can use `I1`–`I5`; the loader converts the alias to the
  registered factory type.

### Adding content to a stage

To add an enemy, edit only the stage manifest:

```text
[entities]
entity id=g18 symbol=E1 area=overworld tile=190.25,12 direction=1
```

To add a new item or moving platform, add a record to `[items]` or
`[platforms]`. To add a portal, define an anchor and then point a portal at the
anchor:

```text
[anchors]
anchor id=secret_entry area=secret tile=2,30 velocity=0,0

[portals]
portal id=secret_pipe area=overworld activation=down \
       trigger=50,10,2,2 target=secret_entry
```

When the new content requires a new runtime entity type, register that type in
`EntityFactory` and add its alias to `entities.catalog`; ordinary placement
changes do not require C++ edits.

### `.txt` compatibility

`.level` is the primary path. A request such as `loadLevel("1.1/1-1.txt")`
first derives and loads `1.1/1-1.level`, so older callers can transition
without immediately changing every call site. If no matching manifest exists,
the loader falls back to the legacy terrain-only reader. Hidden/debug map
loading remains a separate legacy path and is not used by normal menu gameplay.

### Verification

From the repository root:

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The focused test target is `solid03_tests`. If the local CMake/MSBuild toolchain
cannot start because of an environment-specific Windows `PATH` issue, the
same focused test can be run directly:

```powershell
g++ -std=c++17 -Iinclude -Isrc -Ibuild/_deps/sfml-src/include `
  tests/level_definition_tests.cpp `
  src/Level/EntitySymbolCatalog.cpp `
  src/Level/LevelDefinitionLoader.cpp `
  -o build/solid03_tests.exe
build/solid03_tests.exe
```

The expected result is:

```text
SOLID-03 level definition tests passed
```

## Tiếng Việt

### Thay đổi chính

`.level` là entry point chính để load một màn chơi. Terrain vẫn giữ ở file
ASCII `.txt` tương ứng, còn manifest quản lý toàn bộ metadata gameplay:

```text
assets/maps/1.1/1-1.level  ->  terrain=1-1.txt
assets/maps/1.2/1-2.level  ->  terrain=1-2.txt
assets/maps/1.3/1-3.level  ->  terrain=1-3.txt
```

Menu và `PlayState` mặc định đã dùng đường dẫn `.level`. `next_stage` quyết
định màn kế tiếp; để trống nghĩa là màn cuối.

### Luồng load runtime

1. `PlayState` gọi `Level::loadLevel("1.1/1-1.level")`.
2. `LevelDefinitionLoader` tìm manifest và đọc
   `assets/config/entities.catalog`.
3. Đường dẫn `terrain` và `background` tương đối được resolve từ thư mục
   chứa manifest.
4. Loader parse record, resolve alias enemy/item, rồi validate file, tọa độ,
   ID, portal, platform, block và camera zone.
5. `LevelWorldBuilder` dùng `EntityFactory` để tạo enemy, item và platform.
6. `Level` giữ definition và cung cấp API cho portal, camera, boundary, kill
   plane, area và next-stage.

### Alias enemy/item

File `assets/config/entities.catalog` có format:

```text
<enemy|item> <symbol> <EntityFactory type>
```

Ví dụ:

```text
enemy E1 Goomba
enemy E7 PiranhaPlant
item I1 Coin
item I5 StarItem
```

Trong manifest:

```text
[entities]
entity id=g01 symbol=E1 area=overworld tile=24,12 direction=-1

[items]
item id=i01 symbol=I1 area=overworld tile=30,8
```

Alias được resolve từ catalog, không dùng C++ `switch`. Alias sai sẽ báo file
và dòng của manifest.

### Quy tắc tọa độ

- `tile=x,y` và `bounds=x,y,w,h` dùng đơn vị tile.
- Tọa độ thập phân được giữ nguyên, không tự round.
- `speed` và `velocity` dùng pixel/giây.
- `direction` chỉ nhận `-1`, `0`, `1`.
- ID không được trùng trong cùng nhóm record.
- Vị trí entity, block, platform, portal trigger và camera zone phải nằm
  trong terrain.
- `PowerupByForm` tự chọn Mushroom hoặc FireFlower theo form của player.

### Thêm nội dung màn chơi

Thêm enemy chỉ cần sửa manifest:

```text
[entities]
entity id=g18 symbol=E1 area=overworld tile=190.25,12 direction=1
```

Thêm item/platform vào `[items]` hoặc `[platforms]`. Portal gồm một anchor và
một portal trỏ tới anchor:

```text
[anchors]
anchor id=secret_entry area=secret tile=2,30 velocity=0,0

[portals]
portal id=secret_pipe area=overworld activation=down trigger=50,10,2,2 target=secret_entry
```

Nếu tạo loại entity runtime mới thì đăng ký factory và alias một lần; việc
đặt thêm entity vào màn không cần sửa C++.

### Tương thích `.txt`

`.level` là đường dẫn chính. Nếu caller cũ gọi
`loadLevel("1.1/1-1.txt")`, loader sẽ đổi sang `1.1/1-1.level` trước, nên có
thể migrate caller từng bước. Chỉ khi không có manifest tương ứng thì loader
mới dùng legacy terrain-only reader. Hidden/debug map vẫn là đường dẫn legacy
riêng, không phải flow gameplay bình thường.

### Kiểm thử

Chạy từ thư mục gốc repository:

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Test tập trung là `solid03_tests`, kiểm tra parse manifest hợp lệ, alias `E1`
và `E7`, đường dẫn terrain, `.txt` compatibility, duplicate ID, alias sai,
file thiếu, record malformed và giới hạn platform/portal/camera.
