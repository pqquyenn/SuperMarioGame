#pragma once

#include "Observer/Observer.h"
#include <vector>

class Subject {
private:
    std::vector<Observer*> observers;

public:
    void addObserver(Observer* observer);
    void removeObserver(Observer* observer);
    void notify(const GameEvent& event);
};
