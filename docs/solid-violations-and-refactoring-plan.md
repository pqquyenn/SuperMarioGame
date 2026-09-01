# Đánh Giá SOLID và Kế Hoạch Refactor Theo Phân Công Nhóm

Cập nhật lần cuối: 2026-08-13  
Phạm vi: kiến trúc hiện tại trên nhánh `feature/player-mechanics`

## Mục đích

Tài liệu này chỉ ra những phần đang tuân thủ hoặc chưa tuân thủ tốt các nguyên
tắc SOLID, phân tích ảnh hưởng thực tế đến khả năng mở rộng và kiểm thử, đồng
thời phân công việc refactor dựa trên trách nhiệm trong
[`plan/plan.md`](../plan/plan.md).

SOLID là nhóm nguyên tắc định hướng thiết kế, không phải luật tuyệt đối. Một
giải pháp viết trực tiếp hoặc hard-code có thể phù hợp với đồ án nhỏ gồm ba màn.
Nó trở thành vấn đề khi buộc phải sửa nhiều file không liên quan, làm trùng lặp
luật gameplay, hoặc khiến một phần không thể được kiểm thử độc lập.

## Những điểm tốt cần giữ lại

Quá trình refactor nên bảo toàn các thiết kế tốt đang có:

- `GameState` và `GameStateManager` sử dụng đa hình và trì hoãn chuyển state.
- `Entity`, `Character`, `Enemy`, và `Item` tạo ra hệ phân cấp dùng chung hợp lý.
- `PlayerState` tách hành vi và chuyển đổi của Small, Super, và Fire.
- Các lớp `Command` tách hành động người chơi khỏi phương thức của `Character`.
- `EntityFactory` dùng registry thay vì một khối `switch` để tạo entity.
- `Subject` và `Observer` giúp `Character` không phụ thuộc trực tiếp vào HUD.
- `std::unique_ptr` biểu diễn quyền sở hữu state, entity, effect, và projectile.
- `CharacterProfile` biểu diễn khác biệt di chuyển giữa Mario và Luigi bằng dữ
  liệu thay vì sao chép toàn bộ logic.

Đây là những phần đã thể hiện tốt đóng gói, kế thừa, composition, và đa hình lúc
chạy. Nên cải tiến từng bước, không thay toàn bộ bằng một lần viết lại lớn.

## Tổng hợp SOLID

| Khu vực | SRP | OCP | LSP | ISP | DIP | Vấn đề chính |
|---|---|---|---|---|---|---|
| Game loop | Khá tốt | Tốt | N/A | Tốt | Yếu | Có fixed update nhưng chưa thực sự dùng |
| Game states | Tốt | Tốt | Tốt | Tốt | Một phần | State biết manager cụ thể và SFML |
| `PlayState` | Kém | Yếu | N/A | N/A | Kém | Quá nhiều hệ thống gameplay nằm trong một lớp |
| Player states | Tốt | Tốt | Tốt | Tốt | Tốt | Liên kết trong FSM nhỏ là chấp nhận được |
| Command/input | Một phần | Một phần | Tốt | Tốt | Kém | Đọc bàn phím toàn cục khiến khó kiểm thử |
| Entity hierarchy | Tốt | Tốt | Khá tốt | Một phần | Một phần | `Character` đang đảm nhận quá nhiều việc |
| Entity factory | Một phần | Một phần | Tốt | Tốt | Kém | Registry mặc định phụ thuộc singleton asset |
| Level system | Kém | Kém | N/A | N/A | Kém | Load, spawn, portal, camera, world ownership bị trộn |
| Collision system | Kém | Kém | N/A | N/A | Kém | Physics bị trộn với interaction, input, điểm, và luật map |
| Observer/HUD | Một phần | Một phần | Tốt | Tốt | Một phần | Lifetime của observer và luật tính điểm chưa rõ ràng |
| Asset/audio | Một phần | Yếu | N/A | Một phần | Kém | Singleton toàn cục và catalog viết trong C++ |
| Save system | Tốt | Một phần | N/A | Tốt | Yếu | I/O trực tiếp, chưa validate và version hóa |

## Phân tích chi tiết

### SOLID-01: Kiến trúc fixed timestep chưa thực sự hoạt động

**Chủ trì:** Phan Quỳnh Quyền (Engine & Core Architecture)  
**Phối hợp:** Lê Phan Đức Mân về player physics; Đặng Minh Nhật về collision

[`Game::fixedUpdate()`](../src/Core/Game.cpp) đang rỗng, trong khi di chuyển,
trọng lực, và collision chạy từ `PlayState::update(dt)` bằng delta time thay đổi.
Comment nói physics chạy theo bước cố định, nhưng code hiện tại chưa đảm bảo điều
đó.

- **Nguyên tắc bị ảnh hưởng:** SRP và tính nhất quán của abstraction.
- **Ảnh hưởng mở rộng:** Khi physics phức tạp hơn, hành vi phụ thuộc FPS dễ xuất
  hiện.
- **Ảnh hưởng kiểm thử:** Không thể phát lại một số bước physics cố định để tái
  hiện kết quả ổn định.
- **Đánh giá:** Ý tưởng thiết kế tốt, nhưng implementation chưa hoàn tất nên dễ
  gây hiểu nhầm.

**Nhiệm vụ:** Thêm `fixedUpdate(float)` vào `GameState` và `GameStateManager`;
đưa mô phỏng di chuyển/collision vào `PlayState::fixedUpdate`; giữ UI, animation,
và presentation trong `update`. Nếu không đủ thời gian hoàn thiện an toàn, bỏ
accumulator chưa dùng và ghi rõ game chủ động dùng variable delta time có giới
hạn.

**Tiêu chí hoàn thành:**

- Physics chỉ có đúng một đường update.
- Pause và chuyển state không tích lũy một bước physics quá lớn.
- Quãng đường sau cùng một thời lượng mô phỏng gần như không phụ thuộc render FPS.

### SOLID-02: `PlayState` là một gameplay god object

**Chủ trì:** Phan Quỳnh Quyền  
**Phối hợp:** tất cả thành viên cung cấp interface của subsystem mình sở hữu

[`PlayState`](../src/States/PlayState.cpp) tạo player, gán asset, xử lý debug,
cập nhật physics, resolve interaction, quản lý fireball, death/respawn, camera,
map, HUD, và render world.

- **Nguyên tắc bị ảnh hưởng:** SRP, đồng thời ảnh hưởng OCP và DIP.
- **Ảnh hưởng mở rộng:** Thêm projectile, camera zone, death rule, hoặc nhân vật
  mới thường phải thêm nhánh vào `PlayState`.
- **Ảnh hưởng kiểm thử:** Test cần SFML, asset, map, và nhiều object cụ thể cùng lúc.
- **Đánh giá:** Phù hợp như lớp tích hợp ban đầu, nhưng không nên là nơi chứa mọi
  luật gameplay lâu dài.

**Nhiệm vụ:** Giữ `PlayState` làm coordinator cấp cao và tách dần:

1. `PlayerLifecycleController` cho spawn, death, và respawn.
2. `ProjectileSystem` cho tạo, update, collision, và xóa projectile.
3. `CameraController` cho follow và hành vi theo room/region.
4. `DebugController` cho map viewer và phím debug.

**Tiêu chí hoàn thành:**

- `PlayState::update` chủ yếu điều phối thay vì tự cài đặt luật.
- Component được tách có thể chạy test mà không render frame.
- Không thay đổi hành vi gameplay trong PR chỉ làm extraction.

### SOLID-03: Nội dung level bị hard-code trong `Level.cpp`

**Chủ trì:** Đặng Minh Nhật (Level, Tilemap & Collision)  
**Phối hợp:** Lương Nhật Minh cung cấp tên/default của enemy và item; Phan Quỳnh
Quyền review load và error handling

[`Level.cpp`](../src/Level/Level.cpp) chứa trực tiếp tọa độ enemy, đường đi của
platform, nội dung block, đích pipe, vị trí camera, và nhánh theo level ID. Level
còn được suy ra từ chuỗi tên file.

- **Nguyên tắc bị ảnh hưởng:** SRP và OCP.
- **Ảnh hưởng mở rộng:** Thêm hoặc chỉnh level cần sửa và compile lại C++.
- **Ảnh hưởng kiểm thử:** Không thể kiểm thử luật level dưới dạng dữ liệu độc lập.
- **Đánh giá:** Nhanh cho prototype nhưng là trở ngại lớn nhất khi thêm level.

**Nhiệm vụ:** Định nghĩa `LevelDefinition` đọc từ sidecar như `1-1.level`, tối
thiểu mô tả:

- vị trí bắt đầu của player;
- loại entity và vị trí spawn;
- path, speed, và mode của moving platform;
- nội dung block;
- trigger và destination của portal;
- camera region hoặc room bounds.

`Level` chỉ nên sở hữu runtime world. Một loader/builder riêng parse, validate,
và tạo object runtime.

**Tiêu chí hoàn thành:**

- Thêm enemy, item block, platform, hoặc portal không cần sửa C++.
- Type name sai hoặc definition lỗi trả thông báo rõ ràng.
- Level 1-1, 1-2, và 1-3 giữ nguyên nội dung hiện tại.
- Xóa kiểm tra block dựa trên tọa độ X chính xác khỏi gameplay code.

### SOLID-04: `CollisionManager` trộn physics với gameplay và script level

**Chủ trì:** Đặng Minh Nhật  
**Phối hợp phần player:** Lê Phan Đức Mân  
**Phối hợp phần enemy/item:** Lương Nhật Minh

[`CollisionManager.cpp`](../src/Physics/CollisionManager.cpp) vừa kiểm tra AABB
và đẩy entity ra khỏi tile, vừa đọc bàn phím, cộng điểm, xử lý item đặc biệt,
stomp/damage enemy, kiểm tra level ID/tọa độ, và kích hoạt pipe cụ thể.

Nhiều `dynamic_cast` cũng cho thấy thiết kế `onCollision` hiện chỉ đa hình một
phần; manager vẫn biết nhiều concrete type.

- **Nguyên tắc bị ảnh hưởng:** SRP, OCP, và DIP.
- **Ảnh hưởng mở rộng:** Thêm entity/interaction thường phải sửa file collision
  trung tâm.
- **Ảnh hưởng kiểm thử:** Test collision cơ bản lại cần `Level`, input, item cụ
  thể, và luật gameplay.
- **Đánh giá:** Tập trung AABB resolution là tốt; tập trung kiến thức về mọi
  interaction vào cùng lớp là không tốt.

**Phân công:**

- **Nhật:** Giữ detection, overlap, tile separation, grounded, và moving platform
  correction trong physics. Chuyển portal sang dữ liệu level.
- **Mân:** Định nghĩa contract phía player cho damage, stomp, bounce, nhận
  power-up, và ability mà không làm physics biết riêng Mario/Luigi.
- **Minh:** Đưa collection/reaction của item và enemy vào phương thức đa hình
  của `Item`/`Enemy` hoặc một interaction system riêng.

**Tiêu chí hoàn thành:**

- Generic collision không đọc `sf::Keyboard`.
- Generic collision không branch theo `levelId` hoặc tọa độ map.
- Thêm một item đơn giản không cần sửa `CollisionManager`.
- AABB và tile resolution có thể test bằng fixture nhỏ.

### SOLID-05: Input phụ thuộc trực tiếp vào bàn phím toàn cục của SFML

**Chủ trì:** Lê Phan Đức Mân (Player Mechanics & Control)

[`InputHandler.cpp`](../src/Input/InputHandler.cpp) gọi trực tiếp
`sf::Keyboard::isKeyPressed`. Một số hành động đi qua Command, trong khi
`setRunning` và `setJumpHeld` được gọi trực tiếp.

- **Nguyên tắc bị ảnh hưởng:** DIP và một phần OCP.
- **Ảnh hưởng mở rộng:** Gamepad, replay, AI, hoặc network input đều cần sửa
  `InputHandler`.
- **Ảnh hưởng kiểm thử:** Test không thể cung cấp input frame giả mà không dùng
  state bàn phím thật.
- **Đánh giá:** Default key binding là hard-code hợp lý; polling trực tiếp trong
  command translator thì không.

**Nhiệm vụ:** Thêm abstraction `InputSource` hoặc `PlayerInputFrame` bất biến.
SFML adapter tạo action state left/right/jump/action/run; `InputHandler` dịch
state đó thành hành động player.

**Tiêu chí hoàn thành:**

- Test cung cấp input frame mà không cần SFML keyboard.
- Binding hiện tại hoạt động không đổi.
- Có test cho hai hướng đối nghịch, nhấn jump mới, giữ jump, run, và nhấn action mới.

### SOLID-06: `Character` đảm nhận quá nhiều trách nhiệm

**Chủ trì:** Lê Phan Đức Mân

[`Character`](../src/Entities/Character.cpp) quản lý movement physics, form state,
effect tạm thời, animation, death, respawn, collision body, observer event, và
projectile request.

- **Nguyên tắc bị ảnh hưởng:** SRP.
- **Ảnh hưởng mở rộng:** Swimming, climbing, damage model mới, hoặc animation mới
  dễ tác động phần không liên quan.
- **Ảnh hưởng kiểm thử:** Test một concern phải khởi tạo gần như toàn bộ player.
- **Đánh giá:** Vẫn dùng được cho game hiện tại; không cần viết lại toàn bộ theo
  ECS. Nên tách các policy nhỏ dựa trên nhu cầu test.

**Nhiệm vụ:** Ưu tiên tách object tính toán/policy thuần:

- movement integration/policy;
- player form transition policy;
- effect collection/update;
- animation selection.

**Tiêu chí hoàn thành:**

- Mario và Luigi tiếp tục dùng chung logic qua `CharacterProfile`.
- Player state và movement test được mà không cần render window.
- Không tạo implementation Mario/Luigi trùng lặp.

### SOLID-07: Factory registry tốt nhưng quá trình tạo object phụ thuộc toàn cục

**Chủ trì:** Đặng Minh Nhật  
**Phối hợp:** Phan Quỳnh Quyền về asset-provider interface; Lương Nhật Minh về
đăng ký entity

[`EntityFactory`](../src/Factories/EntityFactory.cpp) dùng registry hợp lý, nhưng
`registerDefaultEntities()` include mọi concrete type, chọn texture, gọi
`AssetManager` singleton, và dùng static flag tồn tại xuyên qua test.

- **Nguyên tắc bị ảnh hưởng:** SRP, OCP, và DIP.
- **Ảnh hưởng mở rộng:** Entity built-in mới vẫn cần sửa hàm đăng ký trung tâm.
- **Ảnh hưởng kiểm thử:** Factory test cần asset thật và khó reset registry.
- **Đánh giá:** Registry là lựa chọn tốt; dependency khi construct cần rõ ràng.

**Nhiệm vụ:** Cho registration nhận asset provider hoặc đặt registration trong
composition root. Có cơ chế reset có kiểm soát chỉ cho test nếu cần; tránh static
self-registration dễ lỗi thứ tự khởi tạo.

**Tiêu chí hoàn thành:**

- `create()` vẫn chỉ lookup registry đơn giản.
- Factory test được với fake/no-op asset provider.
- Entity name không tồn tại trả lỗi hoặc structured failure rõ ràng.

### SOLID-08: Lifetime của Observer và nguồn tính điểm chưa rõ ràng

**Chủ trì:** Lương Nhật Minh (Enemies AI, Items & UI)  
**Phối hợp:** Lê Phan Đức Mân về notification boundary của `Character`

[`Subject`](../src/Observer/Subject.cpp) giữ raw observer pointer, cho phép đăng
ký trùng, và dựa vào caller để unsubscribe trước khi object bị hủy.
[`HUD::onNotify`](../src/UI/HUD.cpp) còn hard-code `+200` cho coin và `+100` cho
enemy dù `GameEvent` đã có `value`, tạo hai nguồn có thể quyết định điểm.

- **Nguyên tắc bị ảnh hưởng:** SRP, OCP, và an toàn lifetime.
- **Ảnh hưởng mở rộng:** Enemy có score khác cần sửa HUD hoặc nhận sai điểm.
- **Ảnh hưởng kiểm thử:** Producer và consumer có thể hiểu event khác nhau.
- **Đánh giá:** Observer giúp giảm coupling là tốt; score trùng lặp và lifetime
  không quản lý là không tốt.

**Nhiệm vụ:** Chọn một nguồn duy nhất quyết định điểm. Khuyến nghị producer hoặc
`ScoringRules` đặt `event.value`, HUD chỉ cộng và hiển thị state. Chặn observer
trùng và thêm cleanup rõ ràng hoặc RAII connection.

**Tiêu chí hoàn thành:**

- Coin/enemy có score riêng cập nhật HUD qua `event.value`.
- Gọi `addObserver` trùng không làm phần thưởng tăng hai lần.
- Hủy/unregister observer không để dangling pointer có thể được gọi.
- Observer dispatch có unit test.

### SOLID-09: Asset loader là global service và catalog bị hard-code

**Chủ trì:** Phan Quỳnh Quyền

[`AssetManager.cpp`](../src/Core/AssetManager.cpp) chứa danh sách lớn tên/path
trong C++, trong khi nhiều nơi gọi singleton trực tiếp. Một số UI class còn tự
thử nhiều đường dẫn font tương đối.

- **Nguyên tắc bị ảnh hưởng:** OCP và DIP.
- **Ảnh hưởng mở rộng:** Thêm hoặc chuyển asset cần compile lại và có thể sửa
  nhiều lớp.
- **Ảnh hưởng kiểm thử:** Consumer không nhận được fake asset provider.
- **Đánh giá:** Singleton tiện lợi và phù hợp để minh họa pattern, nhưng không
  nên đồng thời định nghĩa toàn bộ asset lẫn fallback working directory.

**Nhiệm vụ:** Load asset catalog hiện có qua một asset root đã resolve. Cung cấp
interface nhỏ kiểu `IAssetProvider` cho nơi chỉ cần lookup. Có thể giữ singleton
ở composition root nếu đề bài yêu cầu.

**Tiêu chí hoàn thành:**

- HUD và gameplay không tự liệt kê `../`, `../../`, `../../../` cho font.
- Mapping asset name/path chỉ có một nguồn chính thức.
- Asset thiếu báo cả key lẫn resolved path.

### SOLID-10: Save file chưa được validate và version hóa

**Chủ trì:** Phan Quỳnh Quyền

[`SaveSystem.cpp`](../src/Core/SaveSystem.cpp) ghi các giá trị theo thứ tự trực
tiếp và không kiểm tra input thiếu hoặc sai định dạng.

- **Nguyên tắc bị ảnh hưởng:** Chủ yếu là robustness và DIP, không phải vi phạm
  SOLID nghiêm trọng.
- **Ảnh hưởng mở rộng:** Thêm field có thể làm hỏng save cũ.
- **Ảnh hưởng kiểm thử:** Logic gắn với filesystem, dù vẫn test được bằng file tạm.
- **Đánh giá:** Đủ cho prototype, rủi ro nếu compatibility save được chấm.

**Nhiệm vụ:** Thêm version, parse vào `SaveData` tạm, validate toàn bộ, và chỉ
ghi đè dữ liệu caller sau khi parse thành công hoàn toàn.

**Tiêu chí hoàn thành:**

- Save thiếu, bị cắt, sai dữ liệu, hoặc version không hỗ trợ fail an toàn.
- Load thất bại không thay đổi dữ liệu cũ của caller.
- Có test save/load round-trip.

### SOLID-11: Chưa có automated test target

**Chủ trì hạ tầng:** Phan Quỳnh Quyền  
**Sở hữu test:** mỗi thành viên test subsystem của mình

[`CMakeLists.txt`](../CMakeLists.txt) chỉ tạo executable game, chưa có
`enable_testing()`, test executable, hoặc thư mục test.

- **Nguyên tắc bị ảnh hưởng:** Không trực tiếp vi phạm SOLID, nhưng testability
  thấp khiến khó phát hiện regression kiến trúc.
- **Đánh giá:** Manual gameplay test là cần thiết nhưng không đủ cho state
  transition và luật deterministic.

**Phân công:**

- **Quyền:** Thêm test target/framework/CI instruction; test state manager và save.
- **Mân:** Test player-state transition, input edge, movement policy, effect lifetime.
- **Nhật:** Test AABB, tile separation, level parser, portal, và factory lookup.
- **Minh:** Test enemy/item transition, observer dispatch, và HUD score.

**Tiêu chí hoàn thành:**

- Test chạy qua CMake/CTest bằng một command được ghi trong tài liệu.
- Test không cần mở cửa sổ đồ họa.
- Mỗi subsystem có ít nhất một normal case và một edge/failure case.

### SOLID-12: Xử lý sprite và tọa độ atlas nằm trong gameplay class

**Chủ trì:** Lê Phan Đức Mân  
**Phối hợp:** Phan Quỳnh Quyền về asset preprocessing/loading

[`Luigi.cpp`](../src/Entities/Luigi.cpp) xóa hai màu nền chính xác lúc runtime và
chứa giới hạn dòng atlas. Animation profile cũng dùng tọa độ atlas cố định.

- **Nguyên tắc bị ảnh hưởng:** SRP và OCP ở asset boundary.
- **Ảnh hưởng mở rộng:** Export lại sheet với nền/layout khác cần sửa code.
- **Ảnh hưởng kiểm thử:** Image cleanup xảy ra trong lúc setup nhân vật thay vì
  bước preprocess/validate asset.
- **Đánh giá:** Frame coordinate hợp lý cho atlas cố định. Runtime color key là
  bản vá tốt nhưng không nên là asset pipeline cuối cùng.

**Nhiệm vụ:** Export sprite sheet có transparency hoặc thêm atlas metadata rõ
ràng vào asset config. `Luigi` chỉ nên chịu trách nhiệm khác biệt gameplay, không
phải sửa ảnh.

**Tiêu chí hoàn thành:**

- Luigi không có nền mà không cần so sánh RGB runtime.
- Animation frame được mô tả ở một profile/metadata thống nhất.

## Tổng hợp phân công

### Phan Quỳnh Quyền — Engine & Core Architecture

Thứ tự ưu tiên:

1. Thêm hạ tầng automated test (`SOLID-11`).
2. Chọn và hoàn thiện một timing model (`SOLID-01`).
3. Tách `PlayState` mà không đổi hành vi (`SOLID-02`).
4. Tập trung asset path/catalog (`SOLID-09`).
5. Làm save parsing/version an toàn (`SOLID-10`).
6. Cung cấp interface asset/service nhỏ cho Nhật và Minh.

File chính: `Core/*`, `States/*`, `CMakeLists.txt`.

### Lê Phan Đức Mân — Player Mechanics & Control

Thứ tự ưu tiên:

1. Làm input có thể inject và test (`SOLID-05`).
2. Thêm test player state, input edge, và movement (`SOLID-11`).
3. Định nghĩa interaction API phía player cho việc tách collision (`SOLID-04`).
4. Chỉ tách policy khỏi `Character` ở nơi test cho thấy có lợi (`SOLID-06`).
5. Thay runtime cleanup của Luigi bằng asset/metadata sạch (`SOLID-12`).

File chính: `Entities/Character.*`, `Entities/Mario.*`, `Entities/Luigi.*`,
`PlayerStates/*`, `PlayerEffects/*`, `Commands/*`, `Input/*`.

### Đặng Minh Nhật — Level, Tilemap & Collision

Thứ tự ưu tiên:

1. Định nghĩa và validate data-driven level (`SOLID-03`).
2. Tách generic physics khỏi level/interaction rule (`SOLID-04`).
3. Chuyển portal, camera region, block content, entity spawn, và platform khỏi
   C++ sang level data.
4. Tách factory registration khỏi global asset singleton (`SOLID-07`).
5. Thêm test parser, AABB, tile, portal, và factory (`SOLID-11`).

File chính: `Level/*`, `Physics/*`, `Factories/*`.

### Lương Nhật Minh — Enemies AI, Items & UI

Thứ tự ưu tiên:

1. Dùng một nguồn duy nhất cho score event (`SOLID-08`).
2. Làm observer registration an toàn lifetime và không trùng (`SOLID-08`).
3. Chuyển reaction riêng của enemy/item khỏi collision physics (`SOLID-04`).
4. Cung cấp entity name/default cho level và factory data-driven
   (`SOLID-03`, `SOLID-07`).
5. Thêm test enemy, item, observer, và HUD (`SOLID-11`).

File chính: `Entities/Enemies/*`, `Entities/Items/*`, `Observer/*`, `UI/*`.

## Thứ tự triển khai đề xuất

Không nên làm toàn bộ refactor trong một branch. Mỗi PR nên nhỏ và giữ nguyên
hành vi nếu mục tiêu chỉ là thay đổi cấu trúc.

### Giai đoạn 1: Tạo safety net và thống nhất nơi sở hữu hành vi

1. Quyền thêm test target.
2. Mỗi thành viên thêm characterization test cho hành vi hiện tại.
3. Minh thống nhất nguồn score trước khi di chuyển collision event.
4. Mân thêm injectable input mà không đổi control.

### Giai đoạn 2: Xóa hard-code theo map

1. Nhật định nghĩa `LevelDefinition` và migrate một level làm proof of concept.
2. Minh validate entity/item identifier dùng trong definition.
3. Quyền cung cấp asset root và error handling tập trung.
4. Chỉ migrate các level còn lại sau khi level đầu khớp hành vi cũ.

### Giai đoạn 3: Tách điều phối khỏi luật gameplay

1. Nhật tách generic collision resolution.
2. Mân và Minh đưa reaction player/enemy/item vào interface mình sở hữu.
3. Quyền tách controller khỏi `PlayState` và kết nối các system.

### Giai đoạn 4: Timing và cleanup

1. Quyền đưa physics sang fixed-step thật nếu nhóm chọn mô hình đó.
2. Mân tách player policy ở nơi có test chứng minh lợi ích.
3. Thay runtime sprite cleanup và các asset path trùng lặp còn lại.
4. Chạy manual gameplay test đầy đủ sau khi automated test qua hết.

## Quy tắc làm việc khi refactor

- Không gộp architectural extraction với thay đổi cân bằng gameplay trong cùng
  một commit.
- Giữ nguyên hành vi trước; cải thiện hành vi trong PR tiếp theo.
- Owner của interface review thay đổi ảnh hưởng subsystem của họ.
- Dùng tên config rõ nghĩa thay vì tọa độ/con số không giải thích.
- Không thêm `levelId` hoặc branch tọa độ mới vào generic physics.
- Không thêm polling `sf::Keyboard` bên ngoài SFML input adapter.
- Không thêm danh sách fallback asset path bên ngoài asset loader.
- Bug đã sửa nên có regression test khi subsystem cho phép.

## Định nghĩa hoàn thành

Kế hoạch được xem là hoàn tất khi:

- Gameplay hiện tại của 1-1, 1-2, và 1-3 vẫn hoạt động.
- Nội dung level mới được thêm mà không compile lại gameplay code.
- Generic collision không chứa level ID, tọa độ map, hoặc keyboard read.
- Player control/state transition test được mà không cần keyboard/window.
- Score chỉ có một nguồn quyết định.
- CMake cung cấp một command chạy automated test lặp lại được.
- `PlayState` điều phối system nhỏ thay vì tự cài đặt luật của chúng.

---

# English Version — SOLID Review and Team Refactoring Plan

Last reviewed: 2026-08-13  
Scope: current `feature/player-mechanics` project architecture

## Purpose

This document identifies places where the current design follows or strains
SOLID principles, explains the practical effect on extension and testing, and
assigns refactoring work according to the ownership defined in
[`plan/plan.md`](../plan/plan.md).

SOLID principles are design guidelines, not absolute rules. A direct or
hard-coded implementation can be a reasonable choice for a small three-level
course project. It becomes a problem when it forces unrelated files to change,
duplicates gameplay rules, or prevents isolated testing.

## Current strengths to preserve

The refactoring should preserve these existing design choices:

- `GameState` and `GameStateManager` use polymorphism and deferred transitions.
- `Entity`, `Character`, `Enemy`, and `Item` provide useful shared abstractions.
- `PlayerState` isolates Small, Super, and Fire behavior and transitions.
- `Command` objects separate named player actions from `Character` methods.
- `EntityFactory` uses a registry instead of a creation switch statement.
- `Subject` and `Observer` keep `Character` independent from the HUD.
- `std::unique_ptr` expresses ownership for states, entities, effects, and
  projectiles.
- `CharacterProfile` represents Mario/Luigi movement differences as data.

These areas already demonstrate encapsulation, inheritance, composition, and
runtime polymorphism. They should be improved incrementally rather than
replaced in a large rewrite.

## SOLID summary

| Area | SRP | OCP | LSP | ISP | DIP | Main concern |
|---|---|---|---|---|---|---|
| Core game loop | Mostly good | Good | N/A | Good | Weak | Fixed-update path is present but unused |
| Game states | Good | Good | Good | Good | Partial | States know concrete manager and SFML |
| `PlayState` | Poor | Weak | N/A | N/A | Poor | Too many gameplay systems are coordinated in one class |
| Player states | Good | Good | Good | Good | Good | Small finite-state coupling is acceptable |
| Commands/input | Partial | Partial | Good | Good | Poor | Direct static keyboard polling prevents isolated tests |
| Entity hierarchy | Good | Good | Mostly good | Partial | Partial | `Character` is becoming too broad |
| Entity factory | Partial | Partial | Good | Good | Poor | Default registry creates assets through a singleton |
| Level system | Poor | Poor | N/A | N/A | Poor | Loading, spawning, portals, camera, and world ownership are mixed |
| Collision system | Poor | Poor | N/A | N/A | Poor | Physics, interaction rules, input, scoring, and map rules are mixed |
| Observer/HUD | Partial | Partial | Good | Good | Partial | Raw subscriptions and duplicated scoring rules |
| Asset/audio services | Partial | Weak | N/A | Partial | Poor | Global singleton access and C++ asset catalog |
| Save system | Good | Partial | N/A | Good | Weak | Direct file I/O and no validation/versioning |

## Detailed findings

### SOLID-01: The fixed-timestep architecture is not actually used

**Owner:** Phan Quỳnh Quyền (Engine & Core Architecture)  
**Support:** Lê Phan Đức Mân for player physics; Đặng Minh Nhật for collision

[`Game::fixedUpdate()`](../src/Core/Game.cpp) is empty, while movement, gravity,
and collisions execute from `PlayState::update(dt)` with variable frame time.
The comments promise deterministic fixed-step physics, but the implementation
does not provide it.

- **Principles affected:** SRP and consistency of abstraction.
- **Extension impact:** Frame-rate-dependent behavior becomes more likely as
  physics grows.
- **Testability impact:** Replaying a known number of physics steps cannot
  reliably reproduce a result.
- **Assessment:** The intended style is good; the incomplete implementation is
  bad because it presents a guarantee that the game does not meet.

**Task:** Add `fixedUpdate(float)` to `GameState`/`GameStateManager`, put player
movement and collision simulation in `PlayState::fixedUpdate`, and leave UI,
animation, and presentation in variable `update`. If the team cannot complete
that safely before submission, remove the unused accumulator and document that
the game intentionally uses bounded variable delta time.

**Acceptance criteria:**

- Physics has exactly one update path.
- Pausing and state changes do not accumulate a large physics step.
- Movement over a fixed simulated duration is approximately independent of
  render frame rate.

### SOLID-02: `PlayState` is a gameplay god object

**Owner:** Phan Quỳnh Quyền  
**Support:** all subsystem owners for the interfaces they expose

[`PlayState`](../src/States/PlayState.cpp) creates the player, loads assets,
handles debug controls, updates physics, resolves interactions, manages
fireballs, controls death/respawn, updates the camera, changes maps, updates the
HUD, and renders the world.

- **Principle affected:** SRP; also OCP and DIP.
- **Extension impact:** A new projectile, camera zone, death rule, or character
  type requires another branch in `PlayState`.
- **Testability impact:** Tests require live SFML state, assets, maps, and many
  concrete objects at once.
- **Assessment:** Acceptable as an early integration class, but poor as a
  permanent home for every gameplay rule.

**Task:** Keep `PlayState` as the high-level coordinator, but extract focused
components in small steps:

1. `PlayerLifecycleController` for spawn, death, and respawn.
2. `ProjectileSystem` for creation, update, collision, and removal.
3. `CameraController` for normal follow and region/room behavior.
4. `DebugController` for map-viewer and debug-only shortcuts.

**Acceptance criteria:**

- `PlayState::update` reads as orchestration rather than rule implementation.
- Extracted components can be exercised without rendering a frame.
- No gameplay behavior changes during extraction.

### SOLID-03: Level content is embedded in `Level.cpp`

**Owner:** Đặng Minh Nhật (Level, Tilemap & Collision)  
**Support:** Lương Nhật Minh supplies enemy/item type names and defaults;
Phan Quỳnh Quyền reviews loading/error behavior

[`Level.cpp`](../src/Level/Level.cpp) contains C++ tables and branches for enemy
coordinates, moving-platform paths, block contents, pipe destinations, camera
positions, and level IDs. It also infers the level from filename text.

- **Principles affected:** SRP and OCP.
- **Extension impact:** Adding or adjusting a level requires changing and
  recompiling C++.
- **Testability impact:** Level rules cannot be tested as data independently
  from the runtime world.
- **Assessment:** Hard-coded coordinates are expedient for a prototype but are
  the largest obstacle to adding levels.

**Task:** Define a `LevelDefinition` loaded from a sidecar file such as
`1-1.level`. It should describe at least:

- player start;
- entity type and spawn position;
- moving-platform path, speed, and mode;
- block content;
- portal trigger and destination;
- camera region or room bounds.

`Level` should own the runtime world. A separate loader/builder should parse and
validate the definition and create the runtime objects.

**Acceptance criteria:**

- Adding an enemy, item block, platform, or portal to a level does not require a
  C++ change.
- Invalid type names and malformed definitions produce useful errors.
- Existing levels 1-1, 1-2, and 1-3 retain their current content.
- Exact block X-coordinate checks are removed from gameplay code.

### SOLID-04: `CollisionManager` mixes physics with gameplay and level scripts

**Primary owner:** Đặng Minh Nhật  
**Player interaction support:** Lê Phan Đức Mân  
**Enemy/item interaction support:** Lương Nhật Minh

[`CollisionManager.cpp`](../src/Physics/CollisionManager.cpp) performs AABB
checks and physical separation, but it also reads keyboard input, applies score
events, selects special item behavior, handles enemy damage/stomping, checks
level IDs and coordinates, and activates named pipe transitions.

Its repeated `dynamic_cast` checks mean the current `onCollision` design is only
partly polymorphic.

- **Principles affected:** SRP, OCP, and DIP.
- **Extension impact:** A new entity or interaction often requires modifying a
  central collision file.
- **Testability impact:** Basic collision tests unexpectedly need `Level`, input
  state, concrete items, and gameplay rules.
- **Assessment:** Centralized AABB resolution is good. Centralized knowledge of
  every gameplay interaction is bad.

**Task split:**

- **Nhật:** Keep detection, overlap calculation, tile separation, grounded
  correction, and platform correction in the physics layer. Move portal
  definitions to level data.
- **Mân:** Define the player-side interaction contract for damage, stomp,
  bounce, power-up receipt, and abilities without making physics know Mario or
  Luigi specifically.
- **Minh:** Put item collection and enemy reaction behavior in `Item`/`Enemy`
  polymorphic methods or a focused interaction visitor/system.

**Acceptance criteria:**

- The generic collision layer does not read `sf::Keyboard`.
- The generic collision layer does not branch on `levelId` or exact map
  coordinates.
- Adding a simple item does not require editing `CollisionManager`.
- AABB and tile-resolution functions can be tested with small fixtures.

### SOLID-05: Input is coupled to the global SFML keyboard

**Owner:** Lê Phan Đức Mân (Player Mechanics & Control)

[`InputHandler.cpp`](../src/Input/InputHandler.cpp) calls
`sf::Keyboard::isKeyPressed` directly. It also performs some actions through
commands while directly invoking `setRunning` and `setJumpHeld` for others.

- **Principles affected:** DIP and partial OCP.
- **Extension impact:** Gamepad, replay, AI, and network input require changes
  to `InputHandler`.
- **Testability impact:** A test cannot supply an input frame without a real
  keyboard/global SFML state.
- **Assessment:** Default key bindings are reasonable hard-coding. Direct
  polling inside the command translator is not.

**Task:** Introduce an `InputSource` or immutable `PlayerInputFrame` abstraction.
The SFML adapter should produce action states such as left, right, jump, action,
and run. `InputHandler` should translate that state into player actions.

**Acceptance criteria:**

- Tests can provide an input frame without SFML keyboard state.
- Existing keyboard bindings behave unchanged.
- Opposing directions, fresh jump press, held jump, run, and fresh action press
  have automated tests.

### SOLID-06: `Character` has too many responsibilities

**Owner:** Lê Phan Đức Mân

[`Character`](../src/Entities/Character.cpp) manages movement physics, form
state, temporary effects, animation selection, death, respawn, collision-body
size, observer events, and projectile requests.

- **Principle affected:** SRP.
- **Extension impact:** Swimming, climbing, new damage models, or additional
  animations increase the chance of modifying unrelated behavior.
- **Testability impact:** Testing one concern requires constructing most player
  state.
- **Assessment:** Still workable for the present game; a full entity-component
  rewrite is not justified. Focused extraction is preferable.

**Task:** First extract pure calculation/policy objects rather than replacing
the hierarchy:

- movement integration/policy;
- player form transition policy;
- effect collection/update;
- animation selection.

**Acceptance criteria:**

- Mario and Luigi still share `Character` behavior through profiles.
- Player state and movement logic can be tested without a render window.
- No duplicated Mario/Luigi implementations are introduced.

### SOLID-07: Factory registration is extensible, but construction is globally coupled

**Owner:** Đặng Minh Nhật  
**Support:** Phan Quỳnh Quyền for asset-provider interface; Lương Nhật Minh for
entity registrations

[`EntityFactory`](../src/Factories/EntityFactory.cpp) uses a good registry, but
`registerDefaultEntities()` includes every concrete type, selects textures, and
calls the global `AssetManager`. Its function-local static initialization flag
also leaks state between tests.

- **Principles affected:** SRP, OCP, and DIP.
- **Extension impact:** Every built-in entity still requires editing the
  central default-registration function.
- **Testability impact:** Factory tests depend on the real asset singleton and
  cannot reset registration cleanly.
- **Assessment:** The registry pattern is good; construction dependencies need
  to be explicit.

**Task:** Make registration receive an asset provider or register creators from
a composition-root function. Add a controlled reset only for tests if needed.
Avoid fragile static self-registration across translation units.

**Acceptance criteria:**

- `create()` remains a simple registry lookup.
- Factory behavior can be tested with a fake/no-op asset provider.
- Unknown entity names return a clear error or structured failure.

### SOLID-08: Observer lifetime and scoring authority are unclear

**Owner:** Lương Nhật Minh (Enemies AI, Items & UI)  
**Support:** Lê Phan Đức Mân for the `Character` notification boundary

[`Subject`](../src/Observer/Subject.cpp) stores raw observer pointers, accepts
duplicates, and depends on callers to unsubscribe before destruction.
[`HUD::onNotify`](../src/UI/HUD.cpp) also hard-codes `+200` for coins and `+100`
for enemies even though `GameEvent` has a `value`. This creates two possible
authorities for scoring.

- **Principles affected:** SRP and OCP; lifetime safety is also affected.
- **Extension impact:** Enemies with different scores require HUD changes or
  silently receive the wrong score.
- **Testability impact:** Event producers and consumers can disagree about
  expected values.
- **Assessment:** Observer decoupling is good. Score duplication and unmanaged
  subscription lifetime are bad.

**Task:** Decide on one scoring authority. Recommended: event producers or a
`ScoringRules` service set `event.value`, and HUD only presents accumulated
state. Prevent duplicate observers and add explicit subscription cleanup or an
RAII connection object.

**Acceptance criteria:**

- A coin/enemy with a custom score updates the HUD using its event value.
- Duplicate `addObserver` calls do not duplicate rewards.
- Destroying/unregistering an observer cannot leave a callable dangling pointer.
- Observer dispatch has unit tests.

### SOLID-09: Asset loading is a hard-coded global service

**Owner:** Phan Quỳnh Quyền

[`AssetManager.cpp`](../src/Core/AssetManager.cpp) contains a large C++ list of
logical names and paths, while many consumers call the singleton directly.
Several UI classes separately search multiple relative font paths.

- **Principles affected:** OCP and DIP.
- **Extension impact:** Adding or relocating an asset can require recompilation
  and edits in multiple classes.
- **Testability impact:** Consumers cannot receive a fake asset provider.
- **Assessment:** A singleton is convenient for the current project and valid
  as a pattern demonstration, but it should not also define every asset and
  working-directory fallback.

**Task:** Load the existing asset catalog through one resolved asset root.
Expose a small `IAssetProvider`-style interface to systems that only need
lookups. Keep the singleton at the composition root if required by the course.

**Acceptance criteria:**

- HUD and gameplay code do not enumerate `../`, `../../`, and `../../../` font
  paths.
- Asset name/path mapping has one authoritative source.
- Missing assets produce a diagnostic containing both key and resolved path.

### SOLID-10: Save files are not validated or versioned

**Owner:** Phan Quỳnh Quyền

[`SaveSystem.cpp`](../src/Core/SaveSystem.cpp) serializes positional values
directly to a file and does not validate failed or partial input.

- **Principles affected:** Mostly robustness and DIP rather than a severe SOLID
  violation.
- **Extension impact:** Adding a save field can break older files.
- **Testability impact:** Logic is coupled to filesystem I/O, although temporary
  file tests remain possible.
- **Assessment:** Fine for a prototype; risky if save compatibility is graded.

**Task:** Add a format version, parse into a temporary `SaveData`, validate all
fields, and only replace the caller's data after complete success.

**Acceptance criteria:**

- Missing, truncated, malformed, and unsupported-version saves fail cleanly.
- A failed load leaves the caller's previous data unchanged.
- Save/load round-trip has automated tests.

### SOLID-11: There is no automated test target

**Infrastructure owner:** Phan Quỳnh Quyền  
**Test ownership:** each teammate tests their own subsystem

[`CMakeLists.txt`](../CMakeLists.txt) only creates the game executable. There is
no `enable_testing()`, test executable, or test directory.

- **Principles affected:** Not a direct SOLID violation, but low testability
  makes SOLID regressions difficult to detect.
- **Assessment:** Manual gameplay testing is necessary, but it is not sufficient
  for state transitions and deterministic rules.

**Task split:**

- **Quyền:** Add the test target, framework, and CI/build instructions; test
  state-manager transitions and save parsing.
- **Mân:** Test player-state transitions, input edges, movement policies, and
  effect lifetime.
- **Nhật:** Test AABB, tile separation, level-definition parsing, portals, and
  factory lookup.
- **Minh:** Test enemy/item state transitions, observer dispatch, and HUD score
  updates.

**Acceptance criteria:**

- Tests run through CMake/CTest with one documented command.
- Tests do not require opening a graphical window.
- Each owned subsystem has at least one normal case and one edge/failure case.

### SOLID-12: Sprite cleanup and atlas coordinates live in gameplay classes

**Owner:** Lê Phan Đức Mân  
**Support:** Phan Quỳnh Quyền for asset preprocessing/loading

[`Luigi.cpp`](../src/Entities/Luigi.cpp) removes two exact background colors at
runtime and contains exact vertical atlas bounds. Animation profiles also use
fixed atlas coordinates.

- **Principles affected:** SRP and OCP at the asset boundary.
- **Extension impact:** Re-exporting the sheet with a new background or layout
  requires a code change.
- **Testability impact:** Pixel cleanup occurs during character setup instead of
  as an asset validation/preprocessing step.
- **Assessment:** Frame coordinates are acceptable for a fixed atlas. Runtime
  color-key cleanup is a useful patch but should not be the final asset pipeline.

**Task:** Export a transparent player sprite sheet or add explicit atlas metadata
to asset configuration. Keep `Luigi` responsible for gameplay differences, not
image repair.

**Acceptance criteria:**

- Luigi has no visible background without runtime RGB comparisons.
- Animation frame definitions are documented in one profile/metadata location.

## Ownership summary

### Phan Quỳnh Quyền — Engine & Core Architecture

Priority order:

1. Add automated test infrastructure (`SOLID-11`).
2. Decide and implement one timing model (`SOLID-01`).
3. Decompose `PlayState` without changing behavior (`SOLID-02`).
4. Centralize asset-path/catalog loading (`SOLID-09`).
5. Harden save parsing and versioning (`SOLID-10`).
6. Provide narrow asset/service interfaces needed by Nhật and Minh.

Primary files: `Core/*`, `States/*`, `CMakeLists.txt`.

### Lê Phan Đức Mân — Player Mechanics & Control

Priority order:

1. Make player input injectable and testable (`SOLID-05`).
2. Add player-state, input-edge, and movement tests (`SOLID-11`).
3. Define player interaction methods required by collision extraction
   (`SOLID-04`).
4. Extract focused policies from `Character` only where tests justify it
   (`SOLID-06`).
5. Replace runtime Luigi image cleanup with a clean asset/metadata solution
   (`SOLID-12`).

Primary files: `Entities/Character.*`, `Entities/Mario.*`, `Entities/Luigi.*`,
`PlayerStates/*`, `PlayerEffects/*`, `Commands/*`, `Input/*`.

### Đặng Minh Nhật — Level, Tilemap & Collision

Priority order:

1. Define and validate data-driven level definitions (`SOLID-03`).
2. Separate generic physics from level and interaction rules (`SOLID-04`).
3. Move portals, camera regions, block contents, entity spawns, and platforms
   out of C++ and into level data.
4. Decouple factory registration from the global asset singleton (`SOLID-07`).
5. Add parser, AABB, tile-resolution, portal, and factory tests (`SOLID-11`).

Primary files: `Level/*`, `Physics/*`, `Factories/*`.

### Lương Nhật Minh — Enemies AI, Items & UI

Priority order:

1. Make score handling use one authoritative event value (`SOLID-08`).
2. Make observer registration lifetime-safe and duplicate-safe (`SOLID-08`).
3. Move enemy/item-specific reactions out of collision physics (`SOLID-04`).
4. Supply data-driven entity names and content defaults to the level/factory
   work (`SOLID-03`, `SOLID-07`).
5. Add enemy, item, observer, and HUD tests (`SOLID-11`).

Primary files: `Entities/Enemies/*`, `Entities/Items/*`, `Observer/*`, `UI/*`.

## Recommended delivery sequence

Avoid attempting all refactors in one branch. Use small behavior-preserving PRs.

### Phase 1: Safety net and behavior ownership

1. Quyền adds the test target.
2. Each member adds characterization tests for current behavior.
3. Minh resolves scoring authority before collision events are moved.
4. Mân introduces injectable input without changing controls.

### Phase 2: Remove map-specific hard-coding

1. Nhật defines `LevelDefinition` and migrates one level as a proof of concept.
2. Minh validates entity/item identifiers used by definitions.
3. Quyền supplies centralized asset-root/error handling.
4. Migrate the other levels only after the first level matches current behavior.

### Phase 3: Separate orchestration and rules

1. Nhật extracts generic collision resolution.
2. Mân and Minh move player/enemy/item reactions behind their owned interfaces.
3. Quyền extracts controllers from `PlayState` and wires the systems together.

### Phase 4: Timing and cleanup

1. Quyền moves physics to a real fixed-step path if the team selects that model.
2. Mân extracts player policies where tests show value.
3. Replace runtime sprite cleanup and remaining duplicated asset paths.
4. Run complete manual gameplay tests after all behavior-preserving tests pass.

## Team rules for the refactor

- Do not combine architectural extraction with gameplay balance changes in the
  same commit.
- Preserve public behavior first; improve behavior in a following PR.
- The owner of an interface approves changes that affect their subsystem.
- Prefer named configuration fields over unexplained coordinates or numbers.
- Do not add new `levelId` or exact-coordinate branches to generic physics.
- Do not add new direct `sf::Keyboard` polling outside the SFML input adapter.
- Do not add new direct asset-path fallback lists outside the asset loader.
- Every fixed bug should receive a regression test when the subsystem permits it.

## Definition of completion

This plan is complete when:

- Existing gameplay for levels 1-1, 1-2, and 1-3 remains functional.
- New level content can be added without recompiling gameplay code.
- Generic collision code has no level IDs, map coordinates, or keyboard reads.
- Player controls and state transitions can be tested without a keyboard/window.
- Scoring has one source of truth.
- CMake exposes a repeatable automated test command.
- `PlayState` coordinates focused systems instead of implementing their rules.
