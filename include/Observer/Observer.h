#pragma once

#include "Observer/Event.h"

class Observer {
public:
    virtual ~Observer() = default;
    virtual void onNotify(const GameEvent& event) = 0;
};
