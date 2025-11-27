# Percpptency Tests

Comprehensive test suite for the percpptency library using Catch2.

## Test Coverage

### Core Components

- **DomainEvent_test.cpp**: Tests for event value semantics
  - Timestamp generation
  - Unique ID generation  
  - Copy/move semantics

- **AggregateRoot_test.cpp**: Tests for CRTP aggregate base class
  - Event recording via `record_event()`
  - Event collection and clearing
  - Copy/move semantics with events
  - `take_uncommitted_events()` and `mark_events_as_committed()`

- **AggregateReflector_test.cpp**: Tests for generic reflector pattern
  - Simple aggregates with constructor-based creation
  - Validated aggregates with `create_from_data()` factory
  - JSON serialization round-trips
  - Concept validation (`AggregateWithData`, `ConstructibleFromData`, `HasCreateFromData`)
  - `to_class()` and `from_class()` methods

- **Repository_test.cpp**: Tests for repository interface
  - CRUD operations (`create`, `update`, `delete_by_id`, `get_by_id`)
  - Bulk operations (`all`, `count`)
  - Error handling (NotFound, etc.)
  - CrudError enum to_string

- **UnitOfWork_test.cpp**: Tests for transaction coordination
  - Event collection from multiple aggregates
  - `add_events()` and `take_uncommitted_events()`
  - TransactionError enum to_string

## Building and Running

### CMake Build

```bash
# Configure with tests enabled
cmake -B build -DPERCPPTENCY_BUILD_TESTS=ON

# Build tests
cmake --build build --target percpptency_tests

# Run all tests
cd build && ctest --output-on-failure

# Or run directly
./build/tests/percpptency_tests
```

### Run Specific Tests

```bash
# Run tests by tag
./build/tests/percpptency_tests "[percpptency][event]"
./build/tests/percpptency_tests "[percpptency][aggregate]"
./build/tests/percpptency_tests "[percpptency][reflector]"

# List all tests
./build/tests/percpptency_tests --list-tests

# Verbose output
./build/tests/percpptency_tests -v high
```

## Test Patterns

### Testing Aggregates

```cpp
class TestAggregate : public AggregateRoot<TestAggregate> {
public:
    struct Data {
        std::string id;
        std::string name;
    };

    TestAggregate() = default;
    
    void do_something() {
        record_event(SomethingHappened {m_data.id});
    }

    Data m_data;
};
```

### Testing Repositories

```cpp
class MockRepository : public Repository<TestEntity> {
public:
    std::vector<TestEntity> storage;

    ResultVoid create(TestEntity& entity) override {
        storage.push_back(entity);
        return {};
    }
    // ... implement other methods
};
```

## Dependencies

- Catch2 3.x (header-only testing framework)
- reflect-cpp (automatic serialization)
- libc++ (C++23 standard library)

## Notes

- Tests use value semantics for events (no pointers)
- All aggregates are copyable and movable
- Generic reflector supports both simple and validated construction patterns
- Mock implementations avoid external dependencies (no database needed)
