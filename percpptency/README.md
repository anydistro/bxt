# Percpptency

**Persistence for C++ Entities** - A KISS & YAGNI library for repository pattern, domain events, and entity persistence.

Built following strict KISS (Keep It Simple, Stupid) and YAGNI (You Aren't Gonna Need It) principles - extracted from [bxt](https://github.com/anydistro/bxt) project's sqlgen infrastructure.

## Philosophy

- **KISS (Keep It Simple, Stupid)**: Minimal abstractions, leverage reflect-cpp for zero-boilerplate serialization
- **YAGNI (You Aren't Gonna Need It)**: Only what's needed right now - repositories, units of work, domain events
- **Zero-ceremony**: Plain structs with automatic reflection, no manual serialization code

## Features

- **Repository Pattern**: Type-safe CRUD operations with cpperr-based error handling
- **Unit of Work**: Transaction management with auto/manual commit strategies
- **Domain Events**: Aggregate roots with event collection and publishing
- **Event Bus**: Use [eventpp](https://github.com/wqking/eventpp) - header-only, type-safe event dispatcher
- **Adapter-based**: Currently supports SQLite via sqlgen, extensible to other backends

## Core Concepts

### Entity
Any struct that can be reflected via `rfl::Reflector<T>`. No inheritance required.

```cpp
struct User {
    std::optional<int> id;
    std::string name;
    rfl::Validator<std::string, rfl::Email> email;
};
```

### Aggregate Root
Entity that collects domain events during its lifecycle.

```cpp
class Order : public percpptency::AggregateRoot<Order> {
    // Business logic methods call record_event()
    void place_order() {
        record_event(OrderPlacedEvent{...});
    }
};
```

### Repository
CRUD interface automatically implemented for any reflectable type.

```cpp
auto [uow, err, repo] = co_await factory.with_rw_repositories<User>();
auto result = TRY(repo.create(user));
co_await uow->commit();
```

## Architecture

```
percpptency/
├── core/           # Pure domain concepts (no framework deps)
│   ├── AggregateRoot.hpp
│   ├── DomainEvent.hpp
│   └── Repository.hpp
└── adapters/       # Storage implementations
    └── sqlgen/     # SQLite adapter via sqlgen + reflect-cpp
        ├── SqlgenRepository.hpp
        ├── SqlgenUnitOfWork.hpp
        └── SqlgenUnitOfWorkFactory.hpp
```

## Dependencies

Dependencies are automatically fetched via CMake FetchContent:
- **reflect-cpp**: Automatic serialization/deserialization (auto-fetched)
- **sqlgen**: Type-safe SQL query builder for SQLite (auto-fetched)
- **eventpp**: Event dispatcher for pub-sub (auto-fetched)
- **frozen**: Compile-time containers (auto-fetched)

Parent project must provide:
- **cpperr**: Result-based error handling (from bxt workspace)
- **coro**: C++20 coroutine support (from drogon)

## Usage

See examples in `vertical/src/features/` for integration with Drogon HTTP framework.
