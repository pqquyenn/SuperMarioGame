---
name: tilemap-builder-2d
description: Quy tắc xây dựng Tilemap 2D, Layering, Camera Warp Logic (Overworld/Underworld) cho Super Mario Bros.
---

# Tilemap Builder 2D - Super Mario Bros

## 1. QUY TẮC BẢN ĐỒ DẠNG TEXT (.TXT) CHO HANG NGẦM
Bản đồ level được định nghĩa bằng file ASCII text. 
- **Vùng trên mặt đất (Overworld)**: Nằm ở khu vực các dòng phía trên (`Row 0` đến `Row 14`).
- **Vùng Hang ngầm (Underworld)**: Nằm ở các dòng phía dưới (`Row 15` trở đi), được bao quanh bởi tường đá (`V`/`U`), chứa gạch, đồng xu ngầm (`o`) và các ống cống.

### Bảng Ký Tự (Tile Mapping)
- `G`: Đất (Ground)
- `B`: Gạch (Brick)
- `?`: Hộp dấu hỏi (Mystery Block)
- `H`: Khối đá cứng / Bậc thang (Hard Block / Stair)
- `V`, `U`: Khung viền Hang Ngầm (Underground Brick / Floor)
- `o`: Đồng xu ngầm (Coin)
- `.`: Ô trống Air (Không vẽ)

### Ống cống đặc biệt (Warp Pipes)
Để giữ nguyên tỉ lệ lưới 1:1, các ống cống chuyển cảnh sử dụng ký tự thay thế:
- **Warp Down (Chui xuống Hang ngầm)**:
  - `1`: Nửa trái miệng ống (PipeTopLeft IN)
  - `2`: Nửa phải miệng ống (PipeTopRight IN - chỉ dùng để va chạm)
- **Warp Up (Chui lên Mặt đất)**:
  - `3`: Nửa trái miệng ống (PipeTopLeft OUT)
  - `4`: Nửa phải miệng ống (PipeTopRight OUT - chỉ dùng để va chạm)

## 2. LOGIC CHUYỂN CẢNH VÀ ĐỌC TỌA ĐỘ (WARP LOGIC)

1. **Chui xuống Hang ngầm (Warp Down)**:
   - Khi đứng trên ống cống xuống (`1` & `2`) và kích hoạt sự kiện (nhấn phím XUỐNG).
   - Dịch chuyển Camera và vị trí Render từ vùng Overworld xuống vùng Underworld:
     - `Camera_Y = 15 * TILE_SIZE` (Khung nhìn nhảy xuống Row 15).
   - Thiết lập giới hạn Camera (Camera Bounds) ở vùng Underworld để không bị cuộn ngược lên trên mặt đất.

2. **Chui lên Mặt đất (Warp Up / Exit Pipe)**:
   - Khi đi vào miệng ống cống chui lên (`3` & `4`) dưới Hang ngầm (kích hoạt bằng phím PHẢI nếu đi ngang, hoặc tự động).
   - Dịch chuyển Camera: `Camera_Y = 0` để đưa khung nhìn trở lại mặt đất.
   - Thiết lập lại giới hạn Camera (Camera Bounds) ở vùng Overworld.

## 3. THAY ĐỔI BACKGROUND & MÀU SẮC
Màu nền sẽ được render động tùy thuộc vào vị trí hiện tại của Camera để tạo cảm giác chuyển cảnh:
- Khi `Camera_Y < 15 * TILE_SIZE` (Đang ở Overworld): Xóa nền (clear) bằng màu XANH BẦU TRỜI (`0x5C94FC`).
- Khi `Camera_Y >= 15 * TILE_SIZE` (Đang ở Underworld): Xóa nền (clear) bằng màu ĐEN TUYỀN (`0x000000`).

## 4. QUY TẮC PHÂN LỚP RENDER (LAYER ORDER)
Mọi đối tượng khi render PHẢI tuân theo đúng thứ tự Z-Index từ sau ra trước:
- **LỚP 1: BACKGROUND (Nền phía sau - Không có va chạm)**: Chỉ render Bầu trời, Đám mây, Ngọn núi, Bụi cây (chân chạm hàng đất Row 13, tính bù trừ padding). Các đối tượng này nằm ở tọa độ cố định, KHÔNG nằm trong mảng Tilemap va chạm.
- **LỚP 2: TILEMAP LAYER (Khối vật lý - Có va chạm AABB)**: Mảng Tilemap 2D chứa Ground, Brick, Khối đá, Warp Pipes, Hang ngầm. Render bằng Grid Snap `Screen_Pos = cell * TILE_SIZE`.
