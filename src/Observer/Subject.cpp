#include "Observer/Subject.h"
#include <algorithm>

void Subject::addObserver(Observer* observer) {
    observers.push_back(observer);
}

void Subject::removeObserver(Observer* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Subject::notify(const GameEvent& event) {
    for (auto* obs : observers) {
        if (obs) {
            obs->onNotify(event);
        }
    }
}
