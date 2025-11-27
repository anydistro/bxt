# bxt Development Guide

## Project Overview

**bxt** is an ALPM repository management system for package distribution. The architecture consists of:

- **Backend (`vertical/src`)**: Modern C++23 application using Drogon HTTP framework with vertical slice architecture
- **Frontend (`web/`)**: React + TypeScript SPA built with Vite and Bun runtime
- **Build System**: Dual support for XMake (primary) and CMake with Conan dependency management
- **Database**: SQLite with sqlgen ORM for automatic schema generation from C++ structs
- **Serialization**: reflect-cpp for automatic JSON/binary serialization with zero boilerplate

### Core Architecture Principles

**Vertical Slice Architecture**: Features are organized as self-contained modules (e.g., `Users`, `Monitoring`) rather than technical layers. Each feature includes:
- Domain entities in feature-specific namespaces
- Controllers (Drogon HTTP handlers) 
- Module definition with DI configuration via Boost.DI

**Example module structure**:
```cpp
// features/Users/UsersModule.hpp
namespace bxt::Users {
class UsersModule {
    using controller_types = std::tuple<UserController>;
    
    auto configure(auto&& injector) {
        namespace di = boost::di;
        return di::make_injector(std::move(injector));
    }
    
    static std::string_view name() { return "Users"; }
};
}
```

## Development Environment

### Container-Based Development

**Primary workflow**: Dev container with VS Code (see `.devcontainer.json`)
- Ubuntu 24.04 with LLVM/Clang 20 toolchain (configurable via `LLVM_VERSION`)
- Uses `libc++` (NOT `libstdc++`) - all flags require `-stdlib=libc++`
- Pre-configured with all dependencies via Docker

**Quick start**:
```bash
# In VS Code: Reopen in Dev Container
# Copy box config: cp configs/box.yml build/bin/
# Run/debug via launch.json configurations
```

### Build Systems

**XMake (Recommended)**:
```bash
xmake config --mode=release --toolchain=clang
xmake build
xmake run bxtnext
```

**CMake (Alternative)**:
```bash
cmake -S . --preset debug
cmake --build build/debug
```

**Testing**:
```bash
# XMake: xmake config -o testing=true && xmake build bxtnext_tests
# CMake: Enable in conanfile.py: testing=True, then ctest in build directory
```

Test files use Catch2 and end with `_test.cpp` suffix.

## Key Technical Patterns

### Coroutine-Based Async I/O

**All HTTP handlers return `drogon::Task<drogon::HttpResponsePtr>`** and use C++20 coroutines:

```cpp
drogon::Task<drogon::HttpResponsePtr> UserController::create_user(drogon::HttpRequestPtr req) {
    auto [uow, err, repository] = co_await m_uow_factory.with_rw_repositories<domain::User>();
    if (err) { co_return make_error_response(...); }
    
    auto result = repository.create(user);
    co_await uow->commit();
    co_return make_json_response(result);
}
```

**Never use blocking I/O** in handlers - the event loop processes async tasks via `coro::io_scheduler`.

### Unit of Work Pattern

**Database transactions** follow the UoW pattern with factory-based creation:

```cpp
// SqlgenUnitOfWorkFactory creates transactional scopes
auto [uow, err, repo1, repo2] = co_await factory.with_rw_repositories<Entity1, Entity2>();
// Explicit commit/rollback required for ManualCommit strategy
co_await uow->commit();
```

- `SqlgenRepository<T>` provides CRUD operations backed by SQLite via sqlgen library
- Uses `rfl::Reflector<T>` for automatic table schema generation
- All repository methods return `cpperr::result<T, CrudError>` (expected-like error handling)
- Controllers use structured bindings to unpack UoW, error, and repositories in one line

### Serialization with reflect-cpp

**Plain structs auto-reflect** - no manual code needed:

```cpp
// This is ALL you need - reflect-cpp handles JSON serialization automatically
struct PackageMetadata {
    std::string name;
    std::string version;
    std::string arch;
};

// Serialize/deserialize
auto json = rfl::json::write(metadata);
auto result = rfl::json::read<PackageMetadata>(json);
```

**Custom Reflectors only for domain entities with validation**:

```cpp
// Only use custom Reflector when you need:
// - Field validation
// - Business logic in constructors
// - Invariant enforcement
namespace rfl {
template <>
struct Reflector<bxt::Users::domain::User> {
    struct ReflType {
        std::string name;
        rfl::Validator<std::string, rfl::Email> email;
    };
    
    static User to_class(ReflType const& v) noexcept(false) {
        return User(v.name, v.email);  // Throws if validation fails
    }
    
    static ReflType from_class(User const& u) noexcept {
        return {u.name(), u.email()};
    }
};
}
```

**JSON columns in database** with `sqlgen::JSON<T>`:

```cpp
struct PackageBlob {
    std::optional<int> id;
    std::string name;
    sqlgen::JSON<PackageMetadata> metadata;  // Auto-serialized to JSONB column
};

// Usage - no manual serialization needed
blob.metadata = pkg_meta;           // Direct assignment
auto const& meta = blob.metadata.value();  // Access deserialized value
```

**When to use what**:
- ✅ Plain structs for data transfer objects (DTOs) - zero boilerplate
- ✅ `sqlgen::JSON<T>` for nested/complex fields in database
- ✅ Custom `Reflector<T>` ONLY for domain entities with validation/business logic
- ❌ NEVER write manual `to_json()`/`from_json()` methods
- ❌ NEVER use `rfl::Rename<>` or type aliases unless hiding implementation details

### Error Handling

**Uses [cpperr](cpperr/)** - A standalone Rust-inspired error handling library:

```cpp
// Define error kinds as enums
enum class MyError { NotFound, InvalidInput };

// cpperr::result<T, E> wraps std::expected with error<E>
cpperr::result<User, MyError> find_user(int id) {
    if (!exists(id)) {
        return cpperr::make_error(MyError::NotFound, "user not in database");
    }
    return User{id};
}

// TRY macro propagates errors (like Rust's ? operator)
cpperr::result<User, MyError> get_user(int id) {
    auto user = TRY(find_user(id));  // Auto-propagates error or extracts value
    return user;
}
```

**Key features**:
- Type-safe error kinds (enums) with automatic source location tracking
- `TRY` macro for error propagation (Rust-style `?` operator)
- Flattened error cause chains (no heap allocations in hot path)
- Error conversion via `convert_error<TargetError>(source)` template
- Full trace with `.trace()` method showing cause chain

### Dependency Injection

**Boost.DI** is used for compile-time DI. Module `configure()` methods chain injectors:

```cpp
auto injector = di::make_injector(
    di::bind<IService>().to<ServiceImpl>().in(di::singleton),
    di::bind<coro::io_scheduler>().to(m_scheduler)
);
```

Controllers are auto-registered in `Application` class via module's `controller_types` tuple.

## Configuration & Deployment

### Runtime Configuration

**Required files in working directory** (`build/bin/` for dev):
- `box.yml`: Repository structure (branches, architectures, sync settings)
- `settings.toml`: Persistence paths (`box-path`, `lmdb-path`)
- `web/`: Frontend static files served at `/index.html`

**Example box.yml**:
```yaml
branches: [unstable, testing, stable]
repositories:
  [core, extra]:
    architecture: x86_64
    (alpm.sync):
      repo-url: "repo.manjaro.org"
```

### Docker Deployment

**Production**: Multi-stage Dockerfile with XMake build
```bash
docker compose up production  # Runs on port 80
```

**Development**: Container with mounted workspace
```bash
docker compose -f docker-compose.dev.yml up  # Ports 8080, 3000
```

## Frontend Integration

**Tech stack**: React 18 + TanStack Query + Drogon REST API
- Build: `bun run vite-build` (uses Bun runtime, not npm)
- Dev server: `bun run vite-dev` (port 3000)
- File manager UI uses Chonky for repository browsing

**API integration**: Axios with custom hooks in `web/src/hooks/BxtHooks.ts`, WebSocket support via `BxtWebSocketHooks.ts`.

## Code Quality

### Commit Convention

**Conventional Commits** enforced via `commitlint.config.ts`:
- Types: `feat|fix|refactor|test|docs|build|ci|chore|perf|style`
- Scopes: `daemon|db-cli|web`
- Example: `feat(daemon): add user authentication`
- Max line length: 90 chars

### Static Analysis

**Clang-Tidy** support via CMake preset:
```bash
cmake --preset clang-tidy
```

Custom configuration in `tooling/clang-tidy.py` with patches in `cmake/patches/`.

## Common Gotchas

1. **Always use `libc++`**: `-stdlib=libc++` required in all compiler/linker flags
2. **File naming conventions**: 
   - PascalCase for classes/structs: `RepositoryBase.hpp`, `UnitOfWork.hpp`
   - snake_case for type utilities and free functions: `result_types.hpp`, `to_string.hpp`
   - Tests end with `_test.cpp`, disabled code goes in `_disabled/` dirs
3. **Namespace structure**: Features use `bxt::FeatureName::`, shared code uses `bxt::core::*`, `bxt::infra::*`, or `bxt::utils::*`
4. **Shared code organization**: 
   - `core/` - Pure business logic (NO framework dependencies)
   - `infra/` - Framework adapters (Drogon, logging, database)
   - `utils/` - Pure utility functions
5. **No barrel files**: Include specific headers directly (`core/types/Path.hpp`, not `shared/common.hpp`)
6. **Conan dependencies**: Modify `conanfile.py` and `cmake/deps.cmake` together when adding packages
7. **LMDB format**: Controlled by `BXT_LMDB_DATA_FORMAT` CMake var (currently msgpack, supports any reflect-cpp format)
8. **Signal-safe crash tracing**: Separate `signal_tracer` binary handles stack traces (see `signal/`)
9. **reflect-cpp usage**: Plain structs auto-reflect - only use custom `Reflector<T>` for domain entities with validation
10. **JSON in database**: Use `sqlgen::JSON<T>` wrapper, never write manual serialization code

## References

- Main docs: `CONTRIBUTING.md`
- Architecture diagram: `web/public/architecture.svg`
- API specs: `vertical/api/*.yaml`
