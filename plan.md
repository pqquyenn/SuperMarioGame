# Kế Hoạch Phát Triển Game 2D Super Mario Bros (C++ / SFML)

Dự án phát triển game 2D Super Mario theo định hướng Lập trình hướng đối tượng (OOP) nâng cao, đáp ứng các tiêu chí chấm điểm và áp dụng **5 Design Patterns** chuẩn mực kiến trúc phần mềm. Kế hoạch này được thiết kế chi tiết dành cho nhóm 4 thành viên: **Phan Quỳnh Quyền**, **Lê Phan Đức Mân**, **Đặng Minh Nhật**, và **Lương Nhật Minh**.

---

## 1. Cấu Trúc Thư Mục Dự Án (Project Folder Structure)

Dưới đây là cấu trúc thư mục chuẩn modular cho dự án C++ (sử dụng SFML hoặc SDL2). Mọi file `.h` và `.cpp` được tổ chức phân lớp rõ ràng để tránh xung đột mã nguồn khi 4 người cùng làm việc trên Git.

```text
SuperMarioGame/
├── assets/                          # Quản lý tài nguyên game
│   ├── textures/                    # Hình ảnh, Spritesheets (mario.png, tileset.png, enemies.png)
│   ├── audio/                       # Âm thanh (jump.wav, coin.wav, BGM.ogg)
│   ├── fonts/                       # Phông chữ (PixelFont.ttf)
│   └── maps/                        # Dữ liệu bản đồ (level1.json, level2.json, level3.json)
├── include/                         # Header files (.h / .hpp)
│   ├── Core/                        # Quản lý vòng lặp game, Asset, Sound & Save
│   │   ├── Game.h
│   │   ├── AssetManager.h
│   │   ├── SoundManager.h
│   │   └── SaveSystem.h
│   ├── States/                      # Quản lý trạng thái màn hình (State Pattern)
│   │   ├── GameState.h
│   │   ├── GameStateManager.h
│   │   ├── MenuState.h
│   │   ├── PlayState.h
│   │   ├── PauseState.h
│   │   └── GameOverState.h
│   ├── Entities/                    # Đối tượng trong game (Inheritance & Polymorphism)
│   │   ├── Entity.h
│   │   ├── Character.h
│   │   ├── Mario.h
│   │   └── Luigi.h
│   │   ├── Enemies/
│   │   │   ├── Enemy.h
│   │   │   ├── Goomba.h
│   │   │   ├── Koopa.h
│   │   │   └── PiranhaPlant.h
│   │   └── Items/
│   │       ├── Item.h
│   │       ├── Mushroom.h
│   │       ├── FireFlower.h
│   │       └── Coin.h
│   ├── PlayerStates/                # Trạng thái nâng cấp của Mario (State Pattern)
│   │   ├── PlayerState.h
│   │   ├── SmallState.h
│   │   ├── SuperState.h
│   │   └── FireState.h
│   ├── Commands/                    # Điều khiển nhân vật (Command Pattern)
│   │   ├── Command.h
│   │   ├── MoveCommand.h
│   │   ├── JumpCommand.h
│   │   └── FireCommand.h
│   ├── Input/
│   │   └── InputHandler.h
│   ├── Level/                       # Quản lý bản đồ & Camera
│   │   ├── Tile.h
│   │   ├── TileMap.h
│   │   ├── Level.h
│   │   └── Camera.h
│   ├── Physics/                     # Xử lý va chạm
│   │   └── CollisionManager.h
│   ├── Factories/                   # Khởi tạo đối tượng tự động (Factory Pattern)
│   │   └── EntityFactory.h
│   ├── Observer/                    # Hệ thống sự kiện (Observer Pattern)
│   │   ├── Observer.h
│   │   ├── Subject.h
│   │   └── Event.h
│   └── UI/                          # Giao diện hiển thị
│       └── HUD.h
├── src/                             # Source files (.cpp) tương ứng với các header
│   ├── main.cpp
│   ├── Core/
│   ├── States/
│   ├── Entities/
│   │   ├── Enemies/
│   │   └── Items/
│   ├── PlayerStates/
│   ├── Commands/
│   ├── Input/
│   ├── Level/
│   ├── Physics/
│   ├── Factories/
│   ├── Observer/
│   └── UI/
├── CMakeLists.txt                   # Cấu hình biên dịch dự án bằng CMake
├── .gitignore                       # Loại bỏ các file tạm (build/, .vs/, *.exe)
└── README.md                        # Hướng dẫn cài đặt và chạy game
```

---

## 2. 5 Design Patterns Áp Dụng Trong Game

Để đạt **trọn vẹn 25/25 điểm Design Patterns** theo bảng tiêu chí, dự án sẽ áp dụng 5 mẫu thiết kế sau:

### 1. Singleton Pattern
* **Vị trí áp dụng:** `AssetManager`, `SoundManager`.
* **Mục đích:** Đảm bảo chỉ có duy nhất 1 instance quản lý việc load và nạp dữ liệu ảnh/âm thanh vào bộ nhớ RAM. Giúp toàn bộ ứng dụng gọi đến tài nguyên mà không cần truyền tham chiếu khắp nơi hay load lặp đi lặp lại file tài nguyên.

### 2. Factory Method Pattern
* **Vị trí áp dụng:** `EntityFactory`.
* **Mục đích:** Khi đọc dữ liệu bản đồ (`level1.json` hoặc `.txt`), thay vì sử dụng các câu lệnh `if-else` chằng chịt để khởi tạo `Goomba`, `Koopa`, `Mushroom`, `Coin`, `EntityFactory` sẽ đứng ra tạo đối tượng động dựa vào mã Tile/Tag.

### 3. State Pattern
* **Vị trí áp dụng:** 
  1. `GameStateManager` (`MenuState`, `PlayState`, `PauseState`, `GameOverState`).
  2. `PlayerState` cho Mario (`SmallState`, `SuperState`, `FireState`).
* **Mục đích:** Thay đổi hành vi của đối tượng khi trạng thái của nó thay đổi. Ví dụ: Mario ở dạng `SmallState` khi ăn nấm sẽ biến đổi sang `SuperState`.

### 4. Command Pattern
* **Vị trí áp dụng:** `InputHandler` và các lớp `Command` (`JumpCommand`, `MoveCommand`, `FireCommand`).
* **Mục đích:** Tách biệt việc bắt phím bấm (Keyboard Input) với hành động thực tế của nhân vật.

### 5. Observer Pattern
* **Vị trí áp dụng:** `Subject`, `Observer`, `HUD` (ScoreManager).
* **Mục đích:** Khi Mario ăn đồng xu, tiêu diệt quái vật hoặc mất mạng, nhân vật sẽ phát ra sự kiện (`notify`). Lớp `HUD` đăng ký nhận tin nhắn sẽ tự động cập nhật số điểm, số coin, và mạng sống trên màn hình.

---

## 3. Phân Chia Công Việc Chi Tiết Cho 4 Thành Viên

| Thành viên | Vai trò chính | Trách nhiệm chi tiết & Công việc cụ thể | File đảm nhận (`include/` & `src/`) |
| :--- | :--- | :--- | :--- |
| **Phan Quỳnh Quyền** | **Engine & Core Architecture** | 1. Xây dựng Game Loop chuẩn (FPS capping ~60 FPS, DeltaTime).<br>2. Cài đặt **Singleton Pattern** cho `AssetManager` & `SoundManager`.<br>3. Cài đặt **State Pattern** cho quản lý màn hình (`GameStateManager`, `MenuState`, `PlayState`, `PauseState`, `GameOverState`).<br>4. Lập trình tính năng Save/Load game (`SaveSystem`) lưu điểm số, level qua file text/binary. | `Core/Game.*`<br>`Core/AssetManager.*`<br>`Core/SoundManager.*`<br>`Core/SaveSystem.*`<br>`States/*` |
| **Lê Phan Đức Mân** | **Player Mechanics & Control** | 1. Xây dựng lớp cơ sở `Character` và 2 lớp derivied `Mario`, `Luigi` (Inheritance/Polymorphism).<br>2. Cài đặt **Command Pattern** (`InputHandler`, `JumpCommand`, `MoveCommand`, `FireCommand`).<br>3. Cài đặt **State Pattern** cho dạng biến hình của Mario (`SmallState`, `SuperState`, `FireState`).<br>4. Vật lý di chuyển nhân vật: gia tốc, trọng lực, nhảy, trượt đất, chạy nhanh. | `Entities/Entity.*`<br>`Entities/Character.*`<br>`Entities/Mario.*`<br>`Entities/Luigi.*`<br>`PlayerStates/*`<br>`Commands/*`<br>`Input/*` |
| **Đặng Minh Nhật** | **Level, Tilemap & Collision** | 1. Cài đặt bộ nạp bản đồ (`TileMap`, `Level`) đọc từ file cấu hình map (`level1.json` / `.txt`). Tối thiểu **3 levels**.<br>2. Xây dựng Camera cuộn (Smooth Scrolling Camera) đi theo nhân vật.<br>3. Xây dựng bộ xử lý va chạm `CollisionManager` (AABB physics): Nhân vật vs Địa hình, Gạch chấm hỏi (`?`), Gạch vỡ, Ống chui.<br>4. Cài đặt **Factory Pattern** (`EntityFactory`) để đọc map và tự động sinh quái/vật phẩm. | `Level/Tile.*`<br>`Level/TileMap.*`<br>`Level/Level.*`<br>`Level/Camera.*`<br>`Physics/CollisionManager.*`<br>`Factories/EntityFactory.*` |
| **Lương Nhật Minh** | **Enemies AI, Items & UI** | 1. Xây dựng cây phân cấp Quái vật: `Enemy` -> `Goomba`, `Koopa`, `PiranhaPlant`. Lập trình AI di chuyển qua lại, phát hiện vực thẫm, xoay mai rùa Koopa.<br>2. Xây dựng cây phân cấp Vật phẩm: `Item` -> `Mushroom`, `FireFlower`, `Coin`. Quản lý logic xuất hiện từ gạch.<br>3. Cài đặt **Observer Pattern** kết nối sự kiện Game với màn hình hiển thị.<br>4. Xây dựng giao diện `HUD` (điểm số, số coin, thời gian còn lại, số mạng sống). | `Entities/Enemies/*`<br>`Entities/Items/*`<br>`Observer/*`<br>`UI/HUD.*` |

---

## 4. Quy Trình Làm Việc Phân Nhánh Trên GitHub (Git Workflow)

Quy trình Feature Branch Workflow với các branch:
- `main`: Nhánh production.
- `dev`: Nhánh tích hợp code.
- Các branch feature: `feature/core-engine`, `feature/player-mechanics`, `feature/level-collision`, `feature/enemy-items-hud`.

---

## 5. Kế Hoạch Kiểm Thử & Kiểm Tra (Verification Plan)

### Automated Build & Compilation Check
* CMakeLists.txt chuẩn hóa biên dịch trên SFML 2.5/2.6.

### Manual Verification Matrix
* Kiểm tra điều khiển, nhảy, biến hình Mario.
* Kiểm tra nạp 3 level, scrolling camera, va chạm AABB.
* Kiểm tra AI kẻ thù, nẩy quái, rùa trượt, Observer & HUD update.
