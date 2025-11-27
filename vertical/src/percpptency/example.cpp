/* === Example usage of percpptency ===
 * 
 * This example shows how to use percpptency with SQLite backend
 */

#include <iostream>
#include <percpptency/adapters/sqlgen/SqlgenUnitOfWorkFactory.hpp>
#include <percpptency/core/AggregateRoot.hpp>
#include <rfl/Reflector.hpp>

using namespace percpptency;
using namespace percpptency::adapters::sqlgen;

// 1. Define your entity - just a plain struct
struct User {
    std::optional<int> id;
    std::string name;
    std::string email;
};

// 2. (Optional) Define domain events
struct UserCreatedEvent : public DomainEvent {
    std::string user_name;
    std::string user_email;
};

// Example domain aggregate with events using generic reflector pattern
class UserAggregate : public percpptency::AggregateRoot<UserAggregate> {
public:
    // Persistence data - can use sqlgen types pragmatically
    struct Data {
        std::optional<int> id;
        std::string name;
        std::string email;
    };
    
    struct UserCreatedEvent : public percpptency::DomainEvent {
        std::string name;
        std::string email;
    };

    // Factory method
    static UserAggregate create(std::string name, std::string email) {
        UserAggregate user(Data{std::nullopt, name, email});
        user.record_event(UserCreatedEvent{name, email});
        return user;
    }
    
    // Business logic
    void change_email(std::string new_email) {
        m_data.email = std::move(new_email);
        // Could record UserEmailChangedEvent here
    }
    
    // Accessors
    std::string const& name() const { return m_data.name; }
    std::string const& email() const { return m_data.email; }

private:
    friend struct rfl::Reflector<UserAggregate>;  // For generic reflector
    explicit UserAggregate(Data data) { m_data = std::move(data); }
};

// No manual Reflector needed - generic AggregateReflector kicks in!

// 4. Use it with repositories
Task<void> example_usage() {
    // Create factory pointing to your database
    SqlgenUnitOfWorkFactory factory{"./users.db"};

    // Get unit of work + repository in one line
    auto [uow, err, user_repo] = co_await factory.with_rw_repositories<User>();
    
    if (err) {
        std::cerr << "Failed to create transaction\n";
        co_return;
    }

    // Create a user
    User new_user{std::nullopt, "Alice", "alice@example.com"};
    auto create_result = user_repo.create(new_user);
    
    if (!create_result) {
        std::cerr << "Failed to create user\n";
        co_return;
    }

    // Commit the transaction
    co_await uow->commit();

    std::cout << "User created successfully!\n";
}

// 5. With aggregate roots and events using eventpp
#include <eventpp/eventdispatcher.h>

Task<void> aggregate_example() {
    SqlgenUnitOfWorkFactory factory{"./users.db"};
    
    // eventpp event dispatcher
    eventpp::EventDispatcher<std::type_index, void(DomainEvent const&)> event_bus;

    // Subscribe to events
    event_bus.appendListener(
        std::type_index(typeid(UserCreatedEvent)),
        [](DomainEvent const& base_evt) {
            auto const& evt = static_cast<UserCreatedEvent const&>(base_evt);
            std::cout << "User created: " << evt.user_name << "\n";
        });

    // Create user aggregate (events are recorded internally)
    auto user = UserAggregate::create("Bob", "bob@example.com");

    // Get UoW + repository  
    auto [uow, err, user_repo] = co_await factory.with_rw_repositories<UserAggregate>();
    
    // Repository automatically collects events from aggregate during create
    // Entity is modified in-place (events are extracted)
    auto result = user_repo.create(user);
    
    if (!result) {
        co_return;
    }
    
    // Commit transaction
    co_await uow->commit();
    
    // After successful commit, publish all collected events
    auto events = uow->take_uncommitted_events();
    for (auto const& event : events) {
        event_bus.dispatch(std::type_index(typeid(*event)), *event);
    }
}
