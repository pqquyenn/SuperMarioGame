# SOLID-07 Walkthrough / Hướng dẫn SOLID-07

## English

### What changed

`EntityFactory` is now a small registry instead of also being responsible for
loading assets and knowing every concrete entity type.

Before the refactor, `EntityFactory.cpp`:

- included every enemy and item header;
- called the global `AssetManager` from creator lambdas;
- owned `registerDefaultEntities()`;
- used a function-local `static bool initialized`, so one test could affect
  another factory instance.

After the refactor, the responsibilities are separated:

```text
Game (composition root)
    |
    +-- AssetManagerEntityAssetProvider
    |
    +-- registerDefaultEntityTypes(...)
            |
            +-- EntityFactory registry
                    |
                    +-- LevelWorldBuilder / gameplay create()
```

The runtime still uses `EntityFactory::getInstance()` for compatibility, while
tests can construct independent `EntityFactory` objects.

### Public interfaces

The provider abstraction exposes only the dependency needed while a creator
selects its initial texture:

```cpp
class EntityAssetProvider {
public:
    virtual ~EntityAssetProvider() = default;
    virtual const sf::Texture& getTexture(
        const std::string& name) const = 0;
};
```

`AssetManagerEntityAssetProvider` is the runtime adapter. Tests provide a fake
or no-op implementation without loading image files.

Default entity types are installed through the composition function:

```cpp
void registerDefaultEntityTypes(
    EntityFactory& factory,
    EntityAssetProvider& assets);
```

The registrar owns concrete entity includes, creator lambdas, and texture-key
selection. `EntityFactory::create()` remains a registry lookup and returns
`nullptr` for an unknown name.

### Runtime registration flow

1. `Game` creates an `AssetManagerEntityAssetProvider` backed by the existing
   `AssetManager` singleton.
2. `Game` calls `registerDefaultEntityTypes()` before creating game states.
3. The registration lambdas capture the provider by reference. The provider is
   declared before `GameStateManager`, so it outlives all states and gameplay
   entity creation.
4. Assets can still be loaded later by the menu. A creator asks the provider
   for a texture only when `create()` is called.
5. `LevelWorldBuilder` receives an `EntityFactory&` and only requests the types
   resolved by the `.level` manifest.

`Level` and `LevelWorldBuilder` no longer perform hidden default registration.
Tests that load a `Level` without constructing `Game` install the default types
explicitly during test setup.

### Adding a new entity type

For a new built-in type:

1. Add its runtime alias to `assets/config/entities.catalog` when it must be
   available from a `.level` manifest.
2. Add one creator to `registerDefaultEntityTypes()`.
3. Request initial textures through `EntityAssetProvider`, not through
   `AssetManager::getInstance()` inside the registration lambda.
4. Add a factory or level-definition test for the new name.

For an isolated tool or test, no default registrar is required:

```cpp
EntityFactory factory;
factory.registerType("TestEntity", [](const sf::Vector2f& position) {
    return std::make_unique<TestEntity>(position);
});

auto entity = factory.create("TestEntity", {12.f, 34.f});
```

Each local factory starts with an empty registry, so test state cannot leak
through the runtime singleton or a static initialization flag.

### Files added

- `include/Factories/EntityAssetProvider.h` and
  `src/Factories/EntityAssetProvider.cpp` — provider interface and runtime
  `AssetManager` adapter.
- `include/Factories/DefaultEntityRegistration.h` and
  `src/Factories/DefaultEntityRegistration.cpp` — built-in entity composition.
- `tests/entity_factory_tests.cpp` — headless registry/provider tests.
- `docs/solid-07-walkthrough.md` — this document.

### Main files changed

- `EntityFactory` now contains only local/runtime registry operations.
- `Game` owns the runtime provider and performs composition.
- `LevelWorldBuilder` receives its factory explicitly.
- `Level` no longer triggers default registration while loading a manifest.
- `CMakeLists.txt` exposes `solid07_factory_tests` through CTest.
- `tests/level_portal_tests.cpp` installs defaults with a no-op provider because
  that executable does not construct `Game`.

### Verification

The focused factory executable covers 16 checks:

- unknown type failure;
- custom creator lookup and position forwarding;
- default `Goomba` and `Coin` creation with fake assets;
- texture requests routed through the provider;
- independent local registries;
- repeatable default registration without static state;
- the `Mushroom`, `FireFlower`, and `StarItem` debug-spawn names.

Run the complete suite with:

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure

cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Verified result for both Debug and Release: `6/6 tests passed`.

### Scope boundary

This change removes global asset access from `EntityFactory` and its default
registration path. Some concrete enemies still query `AssetManager` while
changing animation frames or gameplay states. Those entity-internal animation
dependencies are outside SOLID-07 and should be addressed with the owning
entity/asset work rather than expanding this factory refactor.

---

## Tiếng Việt

### Thay đổi chính

`EntityFactory` giờ là một registry nhỏ, không còn đồng thời chịu trách nhiệm
chọn asset và biết toàn bộ concrete enemy/item.

Trước refactor, `EntityFactory.cpp`:

- include tất cả enemy và item;
- gọi trực tiếp `AssetManager` singleton trong creator lambda;
- sở hữu `registerDefaultEntities()`;
- dùng `static bool initialized`, khiến trạng thái đăng ký có thể rò rỉ giữa
  các factory trong test.

Sau refactor, trách nhiệm được tách như sau:

```text
Game (composition root)
    |
    +-- AssetManagerEntityAssetProvider
    |
    +-- registerDefaultEntityTypes(...)
            |
            +-- EntityFactory registry
                    |
                    +-- LevelWorldBuilder / gameplay create()
```

Runtime vẫn dùng `EntityFactory::getInstance()` để tránh thay đổi lớn ở gameplay,
nhưng test có thể tạo `EntityFactory` cục bộ và độc lập.

### Interface mới

`EntityAssetProvider` chỉ cung cấp dependency cần thiết để creator chọn texture
ban đầu:

```cpp
class EntityAssetProvider {
public:
    virtual ~EntityAssetProvider() = default;
    virtual const sf::Texture& getTexture(
        const std::string& name) const = 0;
};
```

`AssetManagerEntityAssetProvider` là adapter dùng ở runtime. Test có thể truyền
fake/no-op provider mà không cần load file ảnh thật.

Các entity mặc định được đăng ký qua composition function:

```cpp
void registerDefaultEntityTypes(
    EntityFactory& factory,
    EntityAssetProvider& assets);
```

Registrar sở hữu concrete includes, creator lambda và texture key.
`EntityFactory::create()` chỉ lookup registry; tên không tồn tại trả `nullptr`.

### Luồng đăng ký runtime

1. `Game` tạo `AssetManagerEntityAssetProvider` dựa trên `AssetManager` hiện có.
2. `Game` gọi `registerDefaultEntityTypes()` trước khi tạo game state.
3. Creator lambda giữ reference tới provider. Provider được khai báo trước
   `GameStateManager`, nên sống lâu hơn toàn bộ state và entity creation.
4. Menu vẫn có thể load asset sau bước đăng ký; texture chỉ được lấy khi
   `create()` thực sự chạy.
5. `LevelWorldBuilder` nhận `EntityFactory&` và tạo type đã được resolve từ
   manifest `.level`.

`Level` và `LevelWorldBuilder` không còn tự đăng ký entity ngầm. Portal tests
không khởi tạo `Game`, vì vậy test setup đăng ký defaults bằng no-op provider.

### Cách thêm entity mới

Đối với built-in entity dùng trong level:

1. Thêm alias vào `assets/config/entities.catalog`.
2. Thêm creator vào `registerDefaultEntityTypes()`.
3. Lấy texture ban đầu qua `EntityAssetProvider`, không gọi
   `AssetManager::getInstance()` trong registration lambda.
4. Thêm factory test hoặc level-definition test cho tên mới.

Test hoặc tool độc lập có thể đăng ký type riêng mà không cần default registrar:

```cpp
EntityFactory factory;
factory.registerType("TestEntity", [](const sf::Vector2f& position) {
    return std::make_unique<TestEntity>(position);
});

auto entity = factory.create("TestEntity", {12.f, 34.f});
```

Mỗi local factory bắt đầu với registry rỗng, nên không còn static state rò rỉ
giữa các test.

### File được thêm

- `include/Factories/EntityAssetProvider.h` và
  `src/Factories/EntityAssetProvider.cpp` — provider interface và adapter.
- `include/Factories/DefaultEntityRegistration.h` và
  `src/Factories/DefaultEntityRegistration.cpp` — đăng ký built-in entities.
- `tests/entity_factory_tests.cpp` — test registry/provider không mở window.
- `docs/solid-07-walkthrough.md` — tài liệu này.

### File chính được sửa

- `EntityFactory` chỉ còn local/runtime registry operations.
- `Game` sở hữu provider và thực hiện composition.
- `LevelWorldBuilder` nhận factory tường minh.
- `Level` không đăng ký defaults khi load manifest.
- `CMakeLists.txt` thêm CTest target `solid07_factory_tests`.
- Portal runtime tests tự cài defaults bằng no-op provider.

### Kiểm thử

Factory test có 16 checks:

- unknown type trả `nullptr`;
- custom creator nhận đúng type và vị trí;
- tạo `Goomba` và `Coin` bằng fake provider;
- texture key đi qua provider;
- local registries độc lập;
- default registration gọi lại được mà không phụ thuộc static state;
- ba tên debug spawn `Mushroom`, `FireFlower` và `StarItem`.

Chạy toàn bộ test:

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure

cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Kết quả đã xác minh cho cả Debug và Release: `6/6 tests passed`.

### Giới hạn phạm vi

Thay đổi này loại bỏ global asset access khỏi `EntityFactory` và default
registration. Một số concrete enemy vẫn truy cập `AssetManager` khi đổi frame
animation hoặc gameplay state. Phần dependency nội bộ của entity nằm ngoài
SOLID-07 và nên được xử lý cùng owner của entity/asset thay vì mở rộng refactor
factory này.
