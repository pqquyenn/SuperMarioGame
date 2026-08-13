#include "Observer/Subject.h"
#include <algorithm>
#include <utility>

ObserverConnection::ObserverConnection(
    std::weak_ptr<observer_detail::SubjectState> subjectState,
    std::shared_ptr<Registration> registration
)
    : subjectState_(std::move(subjectState)),
      registration_(std::move(registration)) {}

ObserverConnection::~ObserverConnection() {
    disconnect();
}

ObserverConnection::ObserverConnection(ObserverConnection&& other) noexcept
    : subjectState_(std::move(other.subjectState_)),
      registration_(std::move(other.registration_)) {}

ObserverConnection& ObserverConnection::operator=(ObserverConnection&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    disconnect();
    subjectState_ = std::move(other.subjectState_);
    registration_ = std::move(other.registration_);
    return *this;
}

void ObserverConnection::disconnect() noexcept {
    if (!registration_) {
        subjectState_.reset();
        return;
    }

    registration_->connected = false;
    if (auto subjectState = subjectState_.lock(); subjectState && subjectState->owner) {
        subjectState->owner->removeRegistration(registration_);
    }

    registration_.reset();
    subjectState_.reset();
}

bool ObserverConnection::isConnected() const noexcept {
    return registration_ && registration_->connected &&
           !registration_->lifetime.expired() && !subjectState_.expired();
}

Subject::Subject()
    : state_(std::make_shared<observer_detail::SubjectState>()) {
    state_->owner = this;
}

Subject::~Subject() {
    if (state_) {
        state_->owner = nullptr;
    }

    for (const auto& registration : registrations_) {
        if (registration) {
            registration->connected = false;
        }
    }
    registrations_.clear();
}

ObserverConnection Subject::addObserver(Observer* observer) {
    pruneRegistrations();

    if (!observer) {
        return {};
    }

    const auto duplicate = std::find_if(
        registrations_.begin(),
        registrations_.end(),
        [observer](const RegistrationPtr& registration) {
            return registration && registration->connected &&
                   registration->observer == observer;
        }
    );

    if (duplicate != registrations_.end()) {
        // The first connection owns the registration. Returning an empty
        // connection prevents a duplicate caller from disconnecting it.
        return {};
    }

    auto registration = std::make_shared<Registration>();
    registration->observer = observer;
    registration->lifetime = observer->getLifetimeToken();
    registrations_.push_back(registration);

    return ObserverConnection{state_, std::move(registration)};
}

void Subject::removeObserver(Observer* observer) {
    if (!observer) {
        return;
    }

    for (const auto& registration : registrations_) {
        if (registration && registration->observer == observer) {
            registration->connected = false;
        }
    }

    pruneRegistrations();
}

void Subject::notify(const GameEvent& event) {
    pruneRegistrations();

    // A snapshot keeps dispatch stable when an observer unregisters itself or
    // registers another observer from inside onNotify().
    const auto snapshot = registrations_;
    for (const auto& registration : snapshot) {
        if (!registration || !registration->connected || !registration->observer) {
            continue;
        }

        // Keep the token alive for the duration of the callback. If the
        // observer was destroyed, never dereference its raw pointer.
        const auto lifetime = registration->lifetime.lock();
        if (!lifetime) {
            registration->connected = false;
            continue;
        }

        registration->observer->onNotify(event);
    }

    pruneRegistrations();
}

void Subject::removeRegistration(const RegistrationPtr& registration) {
    if (!registration) {
        return;
    }

    registration->connected = false;
    registrations_.erase(
        std::remove(
            registrations_.begin(),
            registrations_.end(),
            registration
        ),
        registrations_.end()
    );
}

void Subject::pruneRegistrations() {
    registrations_.erase(
        std::remove_if(
            registrations_.begin(),
            registrations_.end(),
            [](const RegistrationPtr& registration) {
                return !registration || !registration->connected ||
                       !registration->observer || registration->lifetime.expired();
            }
        ),
        registrations_.end()
    );
}
