#pragma once

#include "Entities/Entity.h"

// ============================================================
// Item – Lớp cơ sở cho tất cả vật phẩm trong game
// Kế thừa từ Entity. Cung cấp thuộc tính chung:
//   size, collected, getBounds(), isCollected()
// Subclass: Mushroom, FireFlower, Coin
// ============================================================
class Item : public Entity {
protected:
    sf::Vector2f size{32.f, 32.f}; // Kích thước hiển thị / va chạm
    bool collected{false};         // Đã được thu thập chưa

public:
    Item(float x = 0.f, float y = 0.f);
    virtual ~Item() = default;

    // Mỗi Item phải định nghĩa hành vi khi Mario thu thập
    virtual void onCollect() = 0;
    virtual bool tryCollect(class Character& character);

    // Xử lý va chạm và hành vi đa hình
    bool shouldSkipTileCollision() const override { return false; }
    virtual void reverseDirection() {}

    // Collision box dựa trên size
    sf::FloatRect getBounds() const override;

    void onCollision(Entity& other, const sf::FloatRect& overlap) override;
    void onWallCollision() override;

    bool isCollected() const;
};
