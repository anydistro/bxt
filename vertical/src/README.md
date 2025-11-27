# BXT Source Directory Organization

This document describes the organization of the `vertical/src/` directory following the refactoring from the nested `shared/` structure to a flat, clean architecture.

## Directory Structure

```
vertical/src/
├── core/                      ← Pure business logic (framework-agnostic)
│   ├── types/                 ← Common type utilities
│   │   ├── Path.hpp           (filesystem path utilities)
│   │   ├── TimePoint.hpp      (time handling)
│   │   └── to_string.hpp      (string conversion utilities)
│   ├── result_types.hpp       ← Pure Result/Task type aliases (NO Drogon)
│   ├── error/                 ← Error handling
│   │   ├── Error.hpp          (error class)
│   │   ├── Macro.hpp          (error macros)
│   │   ├── Utils.hpp          (error utilities)
│   │   ├── kinds.hpp          (error type aliases)
│   │   └── tests/             (error tests)
│   ├── domain/                ← Domain-driven design building blocks
│   │   ├── RepositoryBase.hpp           (base repository interface)
│   │   ├── ReadOnlyRepositoryBase.hpp   (read-only repository)
│   │   ├── repository_errors.hpp        (repository error types)
│   │   └── value_objects/
│   │       └── Name.hpp       (Name value object)
│   └── di/                    ← Dependency injection
│       └── Concepts.hpp       (DI concepts)
│
├── infra/                     ← Infrastructure adapters (framework-specific)
│   ├── drogon/                ← Drogon HTTP framework
│   │   ├── types.hpp          (ResponseTask, HttpTask - Drogon-specific types)
│   │   ├── Helpers.hpp        (HTTP helper functions)
│   │   ├── Macro.hpp          (HTTP macros)
│   │   ├── Request.hpp        (request parsing)
│   │   ├── ToJsonResponse.hpp (JSON response helpers)
│   │   ├── UoWController.hpp  (Unit of Work controller base)
│   │   └── middleware/
│   │       └── LogMiddleware.hpp (logging middleware)
│   ├── logging/               ← Logging implementation
│   │   ├── Logging.hpp        (logging macros: logi, logw, loge, etc.)
│   │   ├── StructuredLogging.hpp (structured logging)
│   │   ├── PrettyFormatter.hpp   (console formatter)
│   │   ├── FileFormatter.hpp     (file formatter)
│   │   ├── FormattingUtils.hpp   (formatting utilities)
│   │   └── tests/             (logging tests)
│   ├── sqlgen/                ← Database adapter
│   │   ├── SqlgenError.hpp
│   │   ├── SqlgenHeaders.hpp
│   │   ├── SqlgenRepository.hpp         (repository implementation)
│   │   ├── SqlgenSettings.hpp
│   │   ├── SqlgenUnitOfWork.hpp         (transaction management)
│   │   ├── SqlgenUnitOfWorkFactory.hpp  (UoW factory)
│   │   └── tests/             (sqlgen tests)
│   ├── reflect/               ← Reflection adapters
│   │   ├── SqlgenJsonParser.hpp
│   │   ├── SqlgenReflectorParser.hpp
│   │   ├── StringReflector.hpp
│   │   └── tests/             (reflection tests)
│   ├── fmt/                   ← fmt library adapters
│   │   └── BxtToStringFormatter.hpp
│   ├── network/               ← HTTP client
│   │   ├── NetworkClient.hpp
│   │   ├── NetworkClient.cpp
│   │   ├── NetworkError.hpp
│   │   └── PaginationMessages.hpp
│   └── lmdb/                  ← LMDB adapter (currently disabled)
│       ├── LmdbCursorIterator.hpp
│       ├── LmdbDatabase.hpp
│       ├── LmdbEnvironment.hpp
│       ├── LmdbError.hpp
│       ├── LmdbOptions.hpp
│       ├── LmdbRepository.hpp
│       ├── LmdbUnitOfWork.hpp
│       ├── internal/
│       └── tests/
│
├── utils/                     ← Pure utility functions
│   ├── Base64.hpp
│   ├── ConvertTo.hpp
│   ├── Hash.hpp
│   ├── Locked.hpp
│   ├── MemoryLiterals.hpp
│   └── ranges/
│       ├── ConvertView.hpp
│       └── TryTo.hpp
│
├── features/                  ← Vertical slice features
│   ├── Users/
│   │   ├── UserController.hpp
│   │   ├── UserController.cpp
│   │   ├── UserDTO.hpp
│   │   ├── UserConvert.hpp
│   │   ├── UserMessages.hpp
│   │   ├── UsersModule.hpp
│   │   ├── domain/
│   │   │   ├── User.hpp
│   │   │   └── Permission.hpp
│   │   └── tests/
│   └── Monitoring/
│       ├── MetricsController.hpp
│       ├── MetricsController.cpp
│       ├── MetricsMessages.hpp
│       └── MonitoringModule.hpp
│
├── signal/                    ← Signal handling (crash reporting)
│   ├── SignalHandler.hpp
│   ├── SignalHandler.cpp
│   └── signal_tracer.cpp      (separate executable)
│
└── application.cpp            ← Main application entry point
```

## Architecture Principles

### Separation of Concerns

- **`core/`** = Pure business logic
  - **NO framework dependencies** (no Drogon, fmt, spdlog)
  - Only STL, coro, and business logic
  - Portable and testable
  - Framework-agnostic

- **`infra/`** = Infrastructure code
  - Framework-specific implementations
  - All HTTP, database, and logging adapters
  - Depends on external frameworks (Drogon, spdlog, etc.)

- **`utils/`** = Pure helper functions
  - No business logic
  - General-purpose utilities
  - Reusable across projects

- **`features/`** = Vertical slices
  - Self-contained feature modules
  - Each feature has domain, controllers, DTOs, module definition
  - Follows vertical slice architecture

### File Naming Conventions

- **PascalCase** for class/struct files
  - Example: `RepositoryBase.hpp`, `UnitOfWork.hpp`, `Name.hpp`
  
- **snake_case** for:
  - Free function utilities: `to_string.hpp`
  - Type alias files: `result_types.hpp`, `kinds.hpp`, `repository_errors.hpp`
  - Matches C++ ecosystem conventions (STL, Boost)

### Include Patterns

**Before refactoring:**
```cpp
#include "shared/common.hpp"           // Barrel file
#include "shared/errors.hpp"           // Barrel file
#include "shared/log.hpp"              // Barrel file
#include "shared/adapters/drogon/Helpers.hpp"
```

**After refactoring:**
```cpp
#include "core/result_types.hpp"      // Specific includes
#include "core/types/Path.hpp"
#include "core/error/kinds.hpp"
#include "infra/drogon/Helpers.hpp"
#include "infra/logging/Logging.hpp"
```

### Key Changes from Old Structure

1. **No barrel files** - All includes are explicit and direct
2. **Flat src/ structure** - Everything moved from `shared/` to `src/`
3. **Clear layering** - `core/` is pure, `infra/` has dependencies
4. **Drogon types separated** - `infra/drogon/types.hpp` contains framework-specific types
5. **Proper test organization** - Tests live next to the code they test

## Build System

### CMake
The `CMakeLists.txt` now collects sources from:
- `core/*.cpp`
- `infra/*.cpp`
- `utils/*.cpp`
- `features/*.cpp`
- `signal/*.cpp`

Include directory is just `vertical/src/` (no subdirectory-specific includes needed).

### XMake
Similar organization in `xmake.lua`:
- Source files from all subdirectories
- Single include directory: `vertical/src/`
- Separate target for `signal_tracer`

## Migration Notes

### For New Code

When adding new code, choose the appropriate directory:

- **Pure business logic?** → `core/`
  - No Drogon, fmt, spdlog dependencies
  - Domain entities, value objects, repository interfaces
  
- **Framework adapter?** → `infra/`
  - HTTP controllers, database implementations
  - Logging, networking, serialization
  
- **General utility?** → `utils/`
  - Hash functions, converters, range utilities
  
- **Feature module?** → `features/`
  - Self-contained vertical slice
  - Has domain, controller, module definition

### For Existing Code

All imports have been updated from:
- `shared/...` → `core/...`, `infra/...`, `utils/...`
- Barrel files removed (common.hpp, errors.hpp, log.hpp)
- More explicit, IWYU-friendly includes

## Benefits

✅ **No barrel files** - Faster compilation, clearer dependencies  
✅ **Framework-agnostic core** - Portable, testable business logic  
✅ **Clear separation** - core/infra/utils boundaries well-defined  
✅ **Maintainable file sizes** - Most files under 200 lines  
✅ **IWYU-friendly** - Explicit includes, no transitive dependencies  
✅ **Modern C++ conventions** - Follows community best practices  
✅ **Single responsibility** - Each file has one clear purpose  
✅ **Easier navigation** - Flat structure, logical grouping

## Related Documentation

- Main project docs: `CONTRIBUTING.md`
- Architecture diagram: `web/public/architecture.svg`
- API specifications: `vertical/api/*.yaml`
- Original refactoring plan: `REFACTOR_PLAN.md`
