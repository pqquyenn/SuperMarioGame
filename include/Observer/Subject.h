#pragma once

#include "Observer/Observer.h"
#include <memory>
#include <vector>

class Subject;

namespace observer_detail {
struct SubjectState {
    Subject* owner{nullptr};
};

struct ObserverRegistration {
    Observer* observer{nullptr};
    std::weak_ptr<ObserverLifetimeToken> lifetime;
    bool connected{true};
};
}

class ObserverConnection {
public:
    ObserverConnection() = default;
    ~ObserverConnection();

    ObserverConnection(const ObserverConnection&) = delete;
    ObserverConnection& operator=(const ObserverConnection&) = delete;

    ObserverConnection(ObserverConnection&& other) noexcept;
    ObserverConnection& operator=(ObserverConnection&& other) noexcept;

    void disconnect() noexcept;
    bool isConnected() const noexcept;

private:
    using Registration = observer_detail::ObserverRegistration;

    ObserverConnection(
        std::weak_ptr<observer_detail::SubjectState> subjectState,
        std::shared_ptr<Registration> registration
    );

    std::weak_ptr<observer_detail::SubjectState> subjectState_;
    std::shared_ptr<Registration> registration_;

    friend class Subject;
};

class Subject {
private:
    using Registration = observer_detail::ObserverRegistration;
    using RegistrationPtr = std::shared_ptr<Registration>;

    std::shared_ptr<observer_detail::SubjectState> state_;
    std::vector<RegistrationPtr> registrations_;

    void removeRegistration(const RegistrationPtr& registration);
    void pruneRegistrations();

public:
    Subject();
    ~Subject();

    Subject(const Subject&) = delete;
    Subject& operator=(const Subject&) = delete;
    Subject(Subject&&) = delete;
    Subject& operator=(Subject&&) = delete;

    ObserverConnection addObserver(Observer* observer);
    void removeObserver(Observer* observer);
    void notify(const GameEvent& event);

    friend class ObserverConnection;
};
