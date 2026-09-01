#pragma once

#include "Observer/Event.h"
#include <memory>

namespace observer_detail {
struct ObserverLifetimeToken {};
}

class Observer {
public:
    Observer()
        : lifetimeToken_(std::make_shared<observer_detail::ObserverLifetimeToken>()) {}

    virtual ~Observer() = default;

    Observer(const Observer&) = delete;
    Observer& operator=(const Observer&) = delete;
    Observer(Observer&&) = delete;
    Observer& operator=(Observer&&) = delete;

    virtual void onNotify(const GameEvent& event) = 0;

private:
    std::shared_ptr<observer_detail::ObserverLifetimeToken> lifetimeToken_;

    std::weak_ptr<observer_detail::ObserverLifetimeToken> getLifetimeToken() const noexcept {
        return lifetimeToken_;
    }

    friend class Subject;
};
