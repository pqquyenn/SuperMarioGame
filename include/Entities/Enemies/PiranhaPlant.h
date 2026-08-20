#pragma once

#include "Entities/Enemies/Enemy.h"

// ============================================================
// PiranhaPlant – Cây ăn thịt mọc trong ống nước
// Hành vi: nhô lên từ ống → chờ ở đỉnh → thu xuống → chờ ở đáy → lặp lại
// Không thể bị dẫm (onStomped immune), chỉ chết bởi fireball hoặc star.
//
// Trạng thái (State Machine):
//   RISING       – Đang nhô lên khỏi ống
//   WAITING_TOP  – Chờ ở vị trí cao nhất
//   DESCENDING   – Đang thu xuống vào ống
//   WAITING_BOT  – Chờ ở vị trí thấp nhất (ẩn hoàn toàn trong ống)
// ============================================================
class PiranhaPlant : public Enemy {
public:
    // Enum trạng thái di chuyển
    enum class State { RISING, WAITING_TOP, DESCENDING, WAITING_BOT };

private:
    State currentState{State::WAITING_BOT}; // Bắt đầu ẩn trong ống

    float pipeTopY{0.f};       // Vị trí Y đỉnh ống (được set khi spawn)
    float riseHeight{24.f};    // Chiều cao tối đa nhô lên khỏi ống (24px)
    float riseSpeed{60.f};     // Tốc độ di chuyển lên/xuống (pixels/sec)
    float visibleDuration{2.0f};      // Thời gian chờ khi đã nhô lên
    float hiddenDuration{2.0f};       // Thời gian ẩn giữa các lần nhô lên
    float initialHiddenDelay{1.0f};   // Độ trễ ẩn trước lần nhô đầu tiên
    float waitTimer{0.f};      // Bộ đếm thời gian chờ hiện tại
    float currentRise{0.f};    // Khoảng cách đã nhô lên (0 = ẩn hoàn toàn)
    bool waitingForInitialRise{true};

    float animTimer{0.f};       // Timer luân phiên frame animation
    float animInterval{0.2f};   // Đổi frame mỗi 0.2s
    int currentFrame{0};        // Frame 0 hoặc 1

    sf::Vector2f plantSize{16.f, 24.f}; // Kích thước PiranhaPlant

public:
    PiranhaPlant(float x = 0.f, float y = 0.f);
    ~PiranhaPlant() override = default;

    // Đặt vị trí Y đỉnh ống (gọi sau khi tạo, trước game loop)
    void setPipeTopY(float y);

    // Điều khiển chu kỳ: thời gian hiện, thời gian ẩn lặp lại, và độ trễ
    // trước lần xuất hiện đầu tiên. Gọi hàm này sẽ khởi động lại chu kỳ ở
    // trạng thái ẩn để nhiều cây có thể được xếp lịch xen kẽ.
    void setCycleTiming(float visibleSeconds,
                        float hiddenSeconds,
                        float initialDelaySeconds);

    void update(float dt) override;
    void render(sf::RenderWindow& window) const override;
    void onStomped() override; // Immune – không làm gì
    bool canBeStomped() const override { return false; }

    sf::FloatRect getBounds() const override;

    // Getters cho debugging/testing
    State getCurrentState() const;
    float getCurrentRise() const;
    float getVisibleDuration() const { return visibleDuration; }
    float getHiddenDuration() const { return hiddenDuration; }
    float getInitialHiddenDelay() const { return initialHiddenDelay; }
};
