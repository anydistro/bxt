# OpenAPI/AsyncAPI Implementation Plan
## Modern Vertical Slice Architecture for bxt

> **Goal**: Implement OpenAPI 3.0 and AsyncAPI 3.0 specifications using modern vertical slice architecture, migrating from the legacy daemon codebase to the new vertical structure with significant refactoring and improvements.

---

## Architecture Overview

### Current State (Legacy Daemon)
- **Layered architecture**: core → infrastructure → persistence → presentation
- **Horizontal separation**: Domain logic scattered across layers
- **Tight coupling**: DI using kangaru, complex dependency chains
- **Old error handling**: Custom Error.h with make_error_with_source
- **Database**: LMDB with cereal serialization
- **Framework**: Drogon HTTP + Kangaru DI

### Target State (Modern Vertical)
- **Vertical slices**: Self-contained feature modules
- **Feature autonomy**: Each feature owns domain + infra + presentation
- **Loose coupling**: Boost.DI with minimal cross-module dependencies  
- **Modern error handling**: cpperr with ERR_TRY macros
- **Database**: PostgreSQL with sqlgen ORM (automatic schema from C++ structs)
- **Framework**: Drogon HTTP + Boost.DI + cpperr
- **Auth**: Drogon built-in sessions (optional Keycloak integration)
- **Events**: Domain events pattern (replacing EventBus)

---

## Vertical Slice Structure

Each feature follows this pattern (examples from existing Users and Monitoring modules):

```
features/
└── FeatureName/
    ├── FeatureNameModule.hpp         # DI configuration, controller registration
    ├── FeatureNameController.cpp/hpp # HTTP endpoints (Drogon handlers)
    ├── FeatureNameMessages.hpp       # Request/Response DTOs
    ├── FeatureNameConvert.hpp        # DTO ↔ Domain conversions
    ├── domain/
    │   ├── FeatureEntity.hpp         # Core domain entities
    │   ├── FeatureService.hpp        # Business logic (optional)
    │   └── FeatureRepository.hpp     # Repository interface (optional)
    └── tests/
        └── FeatureName_test.cpp      # Domain + integration tests
```

### Module Template

```cpp
namespace bxt::FeatureName {

class FeatureNameModule {
public:
    using controller_types = std::tuple<
        FeatureNameController,
        AnotherController  // Multiple controllers per feature OK
    >;
    
    auto configure(auto&& injector) {
        namespace di = boost::di;
        return di::make_injector(
            std::move(injector),
            // Feature-specific bindings
            di::bind<IRepository>().to<RepositoryImpl>().in(di::singleton)
        );
    }
    
    static std::string_view name() { return "FeatureName"; }
};

} // namespace bxt::FeatureName
```

---

## Feature Breakdown from OpenAPI/AsyncAPI

### Phase 1: Core Features (Essential for MVP)

#### 1.1 **Authentication** (`features/Auth/`)
**Endpoints**: 
- `POST /api/auth/session` - Browser-based cookie auth
- `POST /api/auth/token` - CLI/API bearer token auth  
- `GET /api/auth/logout` - Session logout
- `GET /api/auth/whoami` - Current user info

**Architecture Decision**: Use **Drogon built-in session management** (drops JWT)

**Domain Model**:
```cpp
namespace bxt::Auth::domain {

enum class AuthError {
    InvalidCredentials,
    SessionExpired,
    InsufficientPermissions,
    UserDisabled
};

struct UserCredentials {
    value_objects::Name username;
    std::string password_hash;  // bcrypt
    std::set<Permission> permissions;
    bool is_active;
};

class AuthService {
    // Session-based auth (uses Drogon Session)
    Result<UserCredentials, AuthError> authenticate(
        value_objects::Name const& username,
        std::string const& password
    );
    
    // For API clients (simple token, not JWT)
    Result<std::string, AuthError> create_api_token(
        value_objects::Name const& username,
        std::chrono::seconds ttl
    );
    
    Result<UserCredentials, AuthError> validate_api_token(
        std::string const& token
    );
};

}
```

**Drogon Session Integration**:
```cpp
// Controller example
drogon::Task<drogon::HttpResponsePtr> 
AuthController::login(drogon::HttpRequestPtr req) {
    auto credentials = ERR_CO_TRY_AUTO(
        parse_login_request(req),
        HttpError
    );
    
    auto user = ERR_CO_TRY_AUTO(
        m_auth_service.authenticate(credentials.username, credentials.password),
        HttpError
    );
    
    // Use Drogon's built-in session
    auto session = req->session();
    session->insert("username", user.username);
    session->insert("permissions", user.permissions);
    
    co_return to_json_response({{"status", "logged_in"}});
}
```

**Migration from Daemon**:
- Source: `presentation/web-controllers/AuthController.{cpp,h}`
- **Remove**: `presentation/Token.{cpp,h}` + `JwtOptions.h` (no more JWT)
- Modernize: Replace kangaru DI → Boost.DI
- Modernize: Error handling → cpperr
- **Change**: JWT → Drogon Session (simpler, built-in)
- **New**: API token system for CLI clients (simple bearer tokens, not JWT)
- **New**: Session timeout configuration
- **Future**: Keycloak OAuth2 integration (infrastructure only, no implementation yet)

**Keycloak Integration (Infrastructure Prep)**:
```cpp
// infra/auth/KeycloakAdapter.hpp (stub for future)
namespace bxt::infra::auth {

class KeycloakAdapter {
    // OAuth2 endpoints
    std::string authorization_endpoint;
    std::string token_endpoint;
    std::string userinfo_endpoint;
    
    // For future implementation
    Result<OAuthToken, AuthError> exchange_code_for_token(std::string code);
    Result<UserInfo, AuthError> get_user_info(std::string access_token);
};

} // namespace bxt::infra::auth
```

---

#### 1.2 **Packages** (`features/Packages/`)
**Endpoints**:
- `GET /api/packages/{branch}/{repository}/{architecture}` - List packages
- `POST /api/packages/commit` - Commit package transaction
- `POST /api/packages/snap/branch` - Snap between branches
- `POST /api/advanced/packages/snap` - Advanced snap operations

**Note**: Sync moved to `features/Packages/sync/` submodule (see section 1.2.1)

**Domain Model with Events**:
```cpp
namespace bxt::Packages::domain {

// Domain Events (replacing EventBus pattern)
namespace events {
    struct PackageAdded {
        Package package;
        value_objects::Name added_by;
        TimePoint when;
    };
    
    struct PackageRemoved {
        Package::Id package_id;
        value_objects::Name removed_by;
        TimePoint when;
    };
    
    struct PackageUpdated {
        Package old_package;
        Package new_package;
        value_objects::Name updated_by;
        TimePoint when;
    };
    
    struct PackageMoved {
        Package package;
        Section from_section;
        Section to_section;
        value_objects::Name moved_by;
        TimePoint when;
    };
}

struct PackageVersion {
    std::string version;
    std::filesystem::path filepath;
    std::optional<std::filesystem::path> signature_path;
    TimePoint added_at;
};

enum class PoolCandidate {
    Sync,      // From upstream mirror sync
    Overlay,   // User overlay packages
    Pool       // Local pool packages
};

class Package {
    value_objects::Name name;
    Section section;
    bool is_any_architecture;
    std::map<PoolCandidate, PackageVersion> candidates;
    PoolCandidate selected_candidate;
    
    // Domain event collection
    std::vector<std::unique_ptr<DomainEvent>> pending_events;
    
    void add(PoolCandidate candidate, PackageVersion version) {
        candidates[candidate] = version;
        pending_events.push_back(
            std::make_unique<events::PackageAdded>(/* ... */)
        );
    }
};

enum class CommitAction { Add, Delete, Move, Copy };

struct PackageCommit {
    std::vector<PackageAction> actions;
    value_objects::Name committer;
    TimePoint timestamp;
};

// New: Repository using domain events
class PackageRepository {
    ResultTask<void, PackageError> save(Package& package) {
        // 1. Persist package to PostgreSQL
        co_await m_db.save(package);
        
        // 2. Publish domain events
        for (auto& event : package.pending_events) {
            co_await m_event_publisher.publish(std::move(event));
        }
        package.pending_events.clear();
        
        co_return {};
    }
};

class PackageService {
    ResultTask<std::vector<Package>, PackageError> 
        list_packages(Section const& section, Pagination pagination);
    
    ResultTask<void, PackageError>
        commit_transaction(PackageCommit const& commit);
};

}
```

**Migration from Daemon**:
- Source: `presentation/web-controllers/PackageController.{cpp,h}`
- Source: `infrastructure/PackageService.{cpp,h}`
- Source: `core/domain/entities/Package.h`
- Source: `persistence/box/BoxRepository.{cpp,h}` - **MAJOR REFACTOR**
- Modernize: LMDB + cereal → PostgreSQL + sqlgen
- Modernize: Error propagation → ERR_CO_TRY_AUTO macros
- Modernize: EventBus → Domain Events pattern
- Refactor: Separate package queries from mutations
- Refactor: Remove complex writeback scheduler logic
- **New**: Package versioning history (audit log via events)
- **New**: Transaction rollback support (PostgreSQL ACID)
- **New**: Package signature verification (GPG)

**BoxRepository Refactoring Analysis**:

**Current Issues** (from daemon code):
1. **Complex writeback mechanism**: Hooks + scheduler + exporter
2. **Tightly coupled**: Store + Exporter + Scheduler all in constructor
3. **Mixed concerns**: Persistence + file system export + scheduling
4. **Error handling**: Old bxt::Error system
5. **LMDB traversal**: Manual accept() pattern with NavigationAction

**Proposed Solution - Domain Events**:
```cpp
// OLD (Daemon - BoxRepository.cpp lines 47-59):
void BoxRepository::make_writeback_hook(Section section, UoW uow) {
    m_exporter.add_dirty_sections({section});
    uow->hook([this, section] {
        coro::sync_wait(m_scheduler.schedule([](auto self, auto section) {
            co_await self->m_exporter.export_to_disk();
        }(this, section)));
    }, "Box::Exporter::WriteBack");
}

// NEW (Vertical - Clean separation):
class PackageRepository {
    ResultTask<void, PackageError> save(Package& package) {
        // 1. Save to PostgreSQL
        auto entity = to_entity(package);
        co_await m_sqlgen_repo.insert(entity);
        
        // 2. Publish events (async, non-blocking)
        for (auto& event : package.pending_events) {
            // Event handlers will export to filesystem
            co_await m_event_bus.publish(std::move(event));
        }
        
        package.pending_events.clear();
        co_return {};
    }
};

// Event handler (separate concern)
class PackageExportHandler {
    void handle(events::PackageAdded const& event) {
        // Export ALPM database to filesystem
        m_exporter.export_package(event.package);
    }
};
```

**Benefits**:
- ✅ Single Responsibility: Repository only persists
- ✅ Loose coupling: Export/sync via event handlers
- ✅ Testability: Can test persistence without filesystem
- ✅ Extensibility: Add new event handlers without changing repository
- ✅ Clearer flow: Events make side effects explicit

---

#### 1.2.1 **Sync** (`features/Packages/sync/`) - Submodule
**Endpoints**:
- `POST /api/packages/sync` - Sync packages from upstream

**Why Separate Submodule**:
- Complex logic: HTTP client, file downloads, ALPM parsing
- Different dependencies: httplib, libarchive
- Async heavy: Thread pool for parallel downloads
- WebSocket integration: Progress broadcasting

**Domain Model**:
```cpp
namespace bxt::Packages::sync {

// Sync-specific domain events
namespace events {
    struct SyncStarted {
        std::string architecture;
        value_objects::Name triggered_by;
        TimePoint when;
    };
    
    struct SyncProgress {
        std::string architecture;
        value_objects::Name package_name;
        std::string version;
        int current;
        int total;
        TimePoint when;
    };
    
    struct SyncCompleted {
        std::string architecture;
        int packages_added;
        int packages_removed;
        std::chrono::seconds duration;
        TimePoint when;
    };
    
    struct SyncFailed {
        std::string architecture;
        std::string error_message;
        TimePoint when;
    };
}

struct RemotePackageInfo {
    std::string name;
    std::string filename;
    std::string version;
    std::string sha256_hash;
    std::optional<std::string> signature;
};

enum class SyncError {
    NetworkError,
    InvalidPackage,
    ChecksumMismatch,
    SignatureInvalid,
    DiskFull,
    ConcurrentSyncInProgress
};

class SyncService {
    // Sync packages from upstream ALPM repository
    ResultTask<SyncResult, SyncError> 
        sync_architecture(std::string architecture);
    
    // Get list of available packages from remote
    ResultTask<std::vector<RemotePackageInfo>, SyncError>
        fetch_remote_package_list(std::string architecture);
    
    // Download single package
    ResultTask<Package, SyncError>
        download_package(RemotePackageInfo const& info, Section target);
    
    // Verify package integrity
    Result<void, SyncError>
        verify_package_checksum(std::filesystem::path package_file, 
                               std::string expected_sha256);
};

// Sync coordinator (orchestrates the process)
class SyncCoordinator {
    ResultTask<void, SyncError> execute_sync(std::string architecture) {
        // Emit SyncStarted event
        co_await m_events.publish(events::SyncStarted{...});
        
        // Fetch remote package list
        auto remote_packages = ERR_CO_TRY_AUTO(
            m_sync_service.fetch_remote_package_list(architecture),
            SyncError
        );
        
        // Download in parallel with progress tracking
        int current = 0;
        for (auto const& pkg_info : remote_packages) {
            auto package = ERR_CO_TRY_AUTO(
                m_sync_service.download_package(pkg_info, section),
                SyncError
            );
            
            // Emit progress event (triggers WebSocket broadcast)
            co_await m_events.publish(events::SyncProgress{
                architecture, pkg_info.name, pkg_info.version,
                ++current, remote_packages.size()
            });
            
            // Save package (triggers PackageAdded event)
            co_await m_package_repo.save(package);
        }
        
        // Emit completion event
        co_await m_events.publish(events::SyncCompleted{...});
        
        co_return {};
    }
};

}
```

**Migration from Daemon**:
- Source: `infrastructure/alpm/ArchRepoSyncService.{cpp,h}`
- Source: `infrastructure/alpm/ArchRepoOptions.h`
- Modernize: EventBus dispatch → Domain Events
- Modernize: Error handling → cpperr
- Modernize: Manual HTTP → httplib with connection pooling
- Refactor: Extract download logic from sync logic
- **New**: Concurrent sync prevention (one per architecture)
- **New**: Incremental sync (only changed packages)
- **New**: Retry mechanism for failed downloads
- **New**: Progress estimation based on file sizes

---

#### 1.3 **Logs** (`features/Logs/`)
**Endpoints**:
- `GET /api/log` - Global activity log
- `GET /api/log/{branch}/{repository}/{architecture}` - Section-specific log
- `GET /api/log/{branch}/{repository}/{architecture}/{package_name}` - Package history

**Domain Model**:
```cpp
namespace bxt::Logs::domain {

enum class LogEntryType { Add, Remove, Update, Move, Copy, Sync, Deploy };

struct LogEntry {
    std::string id;  // UUID
    LogEntryType type;
    std::optional<Section> section;
    std::optional<value_objects::Name> package_name;
    std::optional<std::string> package_version;
    std::optional<value_objects::Name> user_name;
    TimePoint timestamp;
    Json::Value details;  // Flexible metadata
};

class LogService {
    ResultTask<PaginatedResult<LogEntry>, LogError>
        query_logs(LogFilter filter, Pagination pagination);
    
    ResultTask<std::vector<LogEntry>, LogError>
        get_package_history(Section const& section, value_objects::Name package);
};

}
```

**Migration from Daemon**:
- Source: `presentation/web-controllers/LogController.{cpp,h}`
- Source: `event_log/` (entire directory)
- Source: `persistence/lmdb/LogEntryRepositories.h`
- Modernize: Event-based logging → Direct repository writes
- Modernize: LMDB → SQLite with full-text search
- Refactor: Split by log type (commit, sync, deploy)
- **New**: Log retention policies (time-based cleanup)
- **New**: Log export (CSV, JSON)
- **New**: Advanced filtering (date ranges, text search, user)

---

#### 1.4 **Sections** (`features/Sections/`)
**Endpoints**:
- `GET /api/sections` - List all sections

**Domain Model**:
```cpp
namespace bxt::Sections::domain {

struct Section {
    std::string branch;      // e.g., "stable", "testing", "unstable"
    std::string repository;  // e.g., "core", "extra", "community"
    std::string architecture; // e.g., "x86_64", "aarch64"
};

class SectionService {
    ResultTask<std::vector<Section>, SectionError> get_all_sections();
    
    ResultTask<bool, SectionError> section_exists(Section const& section);
};

}
```

**Migration from Daemon**:
- Source: `presentation/web-controllers/SectionController.{cpp,h}`
- Source: `persistence/config/SectionRepository.{cpp,h}`
- Source: `utilities/configuration/`
- Modernize: YAML config + in-memory → SQLite
- Refactor: Make sections dynamic (not config-file driven)
- **New**: Section metadata (description, maintainer)
- **New**: Section creation/deletion APIs (for admins)

---

#### 1.5 **Users** (Already exists in `vertical/src/features/Users/`)
**Endpoints**:
- `GET /api/users` - List users
- `POST /api/users` - Create user
- `PUT /api/users/{user_name}` - Update user
- `DELETE /api/users/{user_name}` - Delete user

**Status**: ✅ **Already implemented** with modern vertical architecture
**Improvements needed**:
- Migrate from old bxt error handling to cpperr
- Add ERR_CO_TRY macros throughout controller
- Add permission inheritance/roles (currently flat permission set)

---

### Phase 2: Advanced Features

#### 2.1 **Pivot** (`features/Pivot/`)
**Endpoints**:
- `POST /api/pivot` - Compare packages across sections

**Domain Model**:
```cpp
namespace bxt::Pivot::domain {

struct PivotRequest {
    std::vector<Section> sections;
};

struct PivotEntry {
    value_objects::Name package_name;
    std::map<Section, std::optional<std::string>> versions;
};

class PivotService {
    ResultTask<std::vector<PivotEntry>, PivotError>
        compare_sections(std::vector<Section> const& sections);
};

}
```

**Migration from Daemon**:
- Source: `presentation/web-controllers/CompareController.{cpp,h}`
- Modernize: Inefficient multi-query → Single JOIN query
- **New**: Highlight version mismatches
- **New**: Export comparison results

---

#### 2.2 **Search** (`features/Search/`)
**Endpoints**:
- `GET /api/search?query={q}` - Global package search
- `GET /api/search/{branch}/{repository}/{architecture}?query={q}` - Section search

**Domain Model**:
```cpp
namespace bxt::Search::domain {

struct SearchResult {
    value_objects::Name package_name;
    Section section;
    std::string version;
    std::string description;  // From ALPM metadata
    float relevance_score;
};

class SearchService {
    ResultTask<PaginatedResult<SearchResult>, SearchError>
        search_packages(std::string query, std::optional<Section> section);
};

}
```

**Migration from Daemon**: ⚠️ **Not in daemon - NEW feature**
**Implementation**:
- SQLite FTS5 (full-text search)
- Index package names + descriptions
- Fuzzy matching support

---

#### 2.3 **Workflows** (`features/Workflows/`)
**Endpoints**:
- `GET /api/workflows` - List workflows
- `POST /api/workflows` - Create workflow  
- `GET /api/workflows/{workflow_id}` - Get workflow details
- `PUT /api/workflows/{workflow_id}` - Update workflow
- `DELETE /api/workflows/{workflow_id}` - Delete workflow
- `POST /api/workflows/{workflow_id}/regenerate-key` - Regenerate API key

**Domain Model**:
```cpp
namespace bxt::Workflows::domain {

struct Workflow {
    std::string id;  // UUID
    value_objects::Name name;
    std::vector<Section> allowed_sections;
    std::string api_key_hash;  // bcrypt hash
    bool is_active;
    TimePoint created_at;
    TimePoint last_used_at;
};

class WorkflowService {
    Result<Workflow, WorkflowError> create_workflow(
        value_objects::Name name,
        std::vector<Section> sections
    );
    
    Result<std::string, WorkflowError> regenerate_api_key(std::string workflow_id);
    
    Result<Workflow, WorkflowError> authenticate_api_key(std::string api_key);
};

}
```

**Migration from Daemon**: ⚠️ **Not in daemon - NEW feature**
**Purpose**: CI/CD integration for automated deployments
**Security**: API key rotation, scope-limited permissions

---

#### 2.4 **Metrics** (Exists in `vertical/src/features/Monitoring/`)
**Endpoints**:
- `GET /api/metrics` - Service metrics
- `GET /health` - Health check

**Status**: ✅ **Partially implemented** in Monitoring module
**Improvements**:
- Add package count metrics per section
- Add database size metrics
- Add sync duration tracking
- Export Prometheus-compatible metrics

---

### Phase 3: WebSocket Features

#### 3.1 **Sync WebSocket** (`features/SyncWebSocket/`)
**Channel**: `/api/ws`

**Messages**:
1. `SyncStarted` - When sync begins for architecture
2. `SyncProgress` - Per-package progress updates  
3. `SyncCompleted` - Sync finished or cancelled

**Domain Model**:
```cpp
namespace bxt::SyncWebSocket::domain {

enum class SyncMessageType { Started, Progress, Completed };

struct SyncMessage {
    SyncMessageType type;
    std::string architecture;
    TimePoint when;
    
    // For Progress type:
    std::optional<value_objects::Name> package_name;
    std::optional<std::string> package_version;
    std::optional<int> current;
    std::optional<int> total;
};

class SyncBroadcaster {
    void broadcast_sync_started(std::string architecture);
    void broadcast_sync_progress(std::string architecture, 
                                 value_objects::Name package,
                                 std::string version,
                                 int current, int total);
    void broadcast_sync_completed(std::string architecture);
};

}
```

**Migration from Daemon**:
- Source: `infrastructure/ws/WSController.{cpp,h}`
- Source: `infrastructure/ws/SyncProtocolHandler.h`
- Modernize: Custom WebSocket protocol → Drogon WebSocket
- **New**: Add reconnection support (client-side)
- **New**: Add message sequence numbers (prevent duplicates)

---

## Implementation Priorities

### Sprint 1: Foundation (2 weeks)
1. ✅ Complete cpperr migration (Phases 2-6 from migration plan)
2. Create Auth feature module (cookie + bearer token)
3. Create Sections feature module (list sections)
4. Set up SQLite database schema via sqlgen

### Sprint 2: Core Package Management (3 weeks)
5. Create Packages feature module (list, commit, sync)
6. Migrate BoxRepository → SqlgenRepository
7. Implement package transaction support
8. Create Logs feature module (all log endpoints)

### Sprint 3: Advanced Features (2 weeks)
9. Create Pivot feature module (comparison)
10. Create Search feature module (FTS5)
11. Create Workflows feature module (CI/CD API keys)
12. Enhance Metrics module

### Sprint 4: Real-time Updates (1 week)
13. Create SyncWebSocket feature module
14. Implement WebSocket broadcaster
15. Connect sync service to WebSocket events

---

## Database Schema Migration (LMDB → SQLite)

### Current State (Daemon - LMDB)
- **Packages**: Cereal-serialized PackageRecord structs
- **Users**: Custom LMDB repository
- **Logs**: EventBus → LMDB persistence
- **Sections**: YAML config file

### Target State (Vertical - SQLite)
- **Automatic schema generation** from C++ structs via sqlgen
- **Type-safe queries** with compile-time validation
- **Migrations** via schema versioning

### Example sqlgen Entity

```cpp
namespace bxt::Packages::domain {

struct PackageEntity {
    std::string id;  // Primary key
    std::string name;
    std::string section_branch;
    std::string section_repository;
    std::string section_architecture;
    std::string version;
    std::string filepath;
    std::optional<std::string> signature_filepath;
    bool is_any_architecture;
    std::string pool_candidate;  // "sync", "overlay", "pool"
    TimePoint added_at;
    TimePoint updated_at;
};

}

// sqlgen auto-generates:
// - CREATE TABLE packages (...)
// - INSERT, UPDATE, DELETE, SELECT methods
// - SqlgenRepository<PackageEntity> for CRUD
```

**Migration Strategy**:
1. Run both databases in parallel (LMDB read-only, SQLite read-write)
2. Background migration job copies data
3. Validate data consistency
4. Switch over, deprecate LMDB

---

## Error Handling Strategy

### Vertical Slice Error Hierarchy

Each feature defines its own error enums:

```cpp
// features/Packages/domain/PackageErrors.hpp
namespace bxt::Packages::domain {

enum class PackageError {
    NotFound,
    AlreadyExists,
    InvalidVersion,
    SignatureInvalid,
    DiskFull,
    SyncFailed
};

inline std::string_view to_string(PackageError e) { /* ... */ }

}
```

### Error Conversions

Define conversions between feature errors and generic errors:

```cpp
// features/Packages/PackageErrorConversions.hpp
namespace cpperr {

template<>
inline bxt::err::CrudError convert_error<bxt::err::CrudError>(
    bxt::Packages::domain::PackageError source
) {
    switch (source) {
        case PackageError::NotFound: return CrudError::NotFound;
        case PackageError::AlreadyExists: return CrudError::AlreadyExists;
        case PackageError::InvalidVersion: return CrudError::InvalidData;
        case PackageError::SignatureInvalid: return CrudError::InvalidData;
        case PackageError::DiskFull: return CrudError::StorageError;
        case PackageError::SyncFailed: return CrudError::StorageError;
    }
}

}
```

### Controller Usage

```cpp
// features/Packages/PackageController.cpp
drogon::Task<drogon::HttpResponsePtr> 
PackageController::list_packages(drogon::HttpRequestPtr req, /* params */) {
    // Auto-convert PackageError → HttpError via ERR_CO_TRY_AUTO
    auto [uow, pkg_repo] = ERR_CO_TRY_AUTO(
        m_uow_factory.with_ro_repositories<domain::PackageEntity>(),
        HttpError
    );
    
    auto packages = ERR_CO_TRY_AUTO(
        pkg_repo.find_by_section(section),
        HttpError
    );
    
    co_return to_json_response(packages);
}
```

---

## Testing Strategy

### Unit Tests (Domain Layer)
- Test business logic without I/O
- Use Catch2 test framework
- Example: `features/Packages/tests/Package_test.cpp`

```cpp
TEST_CASE("Package version comparison", "[packages]") {
    auto v1 = PackageVersion::from_string("6.11.5-1");
    auto v2 = PackageVersion::from_string("6.11.4-1");
    
    REQUIRE(v1 > v2);
}
```

### Integration Tests (Full Feature)
- Test HTTP endpoints with in-memory database
- Use Drogon test client
- Example: `features/Packages/tests/PackageController_test.cpp`

```cpp
TEST_CASE("List packages endpoint", "[integration]") {
    auto client = drogon::HttpClient::newHttpClient("http://localhost:8080");
    
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/api/packages/stable/core/x86_64");
    
    auto resp = client->sendRequest(req);
    REQUIRE(resp->statusCode() == drogon::k200OK);
}
```

---

## API Versioning Strategy

### URL-based Versioning (Recommended)
```
/api/v1/packages/...
/api/v2/packages/...
```

### Benefits:
- Clear version separation
- Easy to deprecate old versions
- No header parsing needed

### Migration Path:
1. Start with `/api/...` (implicit v1)
2. Add `/api/v1/...` aliases
3. Introduce breaking changes in `/api/v2/...`
4. Deprecate v1 after grace period

---

## Authentication & Authorization

### JWT Token Structure

```json
{
  "sub": "username",
  "exp": 1700000000,
  "iat": 1699990000,
  "type": "access",
  "permissions": [
    "packages:read",
    "packages:write",
    "users:read"
  ]
}
```

### Permission Model

```cpp
namespace bxt::Auth::domain {

enum class Permission {
    // Package permissions
    PackagesRead,
    PackagesWrite,
    PackagesSync,
    
    // User permissions
    UsersRead,
    UsersWrite,
    UsersDelete,
    
    // Admin permissions
    AdminAll
};

class PermissionChecker {
    bool has_permission(AuthSession const& session, Permission perm);
    bool has_any_permission(AuthSession const& session, std::vector<Permission> perms);
};

}
```

### Drogon Middleware

```cpp
// infra/drogon/middleware/AuthMiddleware.hpp
class AuthMiddleware : public drogon::MiddlewareBase {
public:
    void invoke(drogon::HttpRequestPtr const& req,
                drogon::MiddlewareNextCallback&& next,
                drogon::MiddlewareCallback&& callback) override {
        // Extract JWT from cookie or Authorization header
        // Validate token
        // Inject user into request attributes
        next(std::move(callback));
    }
};
```

---

## Performance Optimizations

### 1. Database Query Optimization
- Use sqlgen's batch operations for bulk inserts
- Add indexes on frequently queried columns (name, section, version)
- Use prepared statements (sqlgen does this automatically)

### 2. Caching Strategy
- In-memory cache for sections (rarely change)
- Redis-like cache for package lists (TTL: 5 minutes)
- Invalidate cache on package commits

### 3. Pagination Best Practices
```cpp
struct Pagination {
    int page = 1;
    int limit = 50;  // Default
    int max_limit = 1000;  // Prevent abuse
};

template<typename T>
struct PaginatedResult {
    std::vector<T> items;
    int total_count;
    int current_page;
    int total_pages;
    bool has_next;
    bool has_previous;
};
```

### 4. Async I/O Everywhere
- All HTTP handlers return `coro::task<drogon::HttpResponsePtr>`
- Use `co_await` for database queries
- Use `ERR_CO_TRY` macros for error propagation

---

## Deployment Architecture

### Container Structure

```dockerfile
# Multi-stage build
FROM ubuntu:24.04 AS builder
RUN apt-get update && apt-get install -y \
    llvm-20 clang-20 libc++-20-dev \
    cmake xmake
WORKDIR /build
COPY . .
RUN xmake config --mode=release --toolchain=clang && \
    xmake build bxtnext

FROM ubuntu:24.04
COPY --from=builder /build/bin/bxtnext /usr/local/bin/
COPY configs/box.yml /etc/bxt/
COPY web/dist/ /var/www/bxt/
CMD ["bxtnext"]
```

### Docker Compose (Development)

```yaml
version: '3.8'
services:
  bxt:
    build: .
    ports:
      - "8080:8080"
    volumes:
      - ./configs:/etc/bxt
      - ./data:/var/lib/bxt
    environment:
      - DATABASE_PATH=/var/lib/bxt/bxt.db
      - LOG_LEVEL=debug
```

---

## Monitoring & Observability

### Metrics (Prometheus-compatible)

```cpp
// features/Monitoring/MetricsService.hpp
namespace bxt::Monitoring::domain {

struct ServiceMetrics {
    int total_packages;
    int total_users;
    int total_workflows;
    size_t database_size_bytes;
    std::map<std::string, int> packages_per_architecture;
    std::map<std::string, int> packages_per_section;
    TimePoint last_sync_time;
    std::chrono::seconds uptime;
};

}
```

### Health Check Levels

1. **Shallow** (`/health`): Process alive?
2. **Deep** (`/health?check=deep`): Database reachable? Disk space OK?

---

## Migration Timeline

| Week | Task |
|------|------|
| 1-2  | Complete cpperr migration, set up Auth module |
| 3-4  | Packages module (list, commit), database migration script |
| 5-6  | Packages module (sync, snap), Logs module |
| 7-8  | Sections module, Pivot module, Search module |
| 9-10 | Workflows module, WebSocket integration |
| 11   | Performance testing, optimization |
| 12   | Documentation, deployment guides |

---

## Success Criteria

✅ **Functional**:
- All OpenAPI endpoints implemented
- WebSocket sync notifications working
- 100% test coverage on domain layer
- < 100ms p99 latency on read endpoints

✅ **Technical**:
- Zero compilation warnings
- All code passes clang-tidy
- Vertical slices are independent (no cross-feature dependencies)
- cpperr error handling throughout

✅ **Operational**:
- Docker deployment works
- Health checks integrated with load balancer
- Metrics exportable to Prometheus
- Database migrations automated

---

## Next Steps

1. ✅ Complete cpperr migration (current task)
2. Create Auth feature module scaffold
3. Write comprehensive tests for Auth
4. Implement Packages feature module
5. Set up CI/CD pipeline for automated testing
6. Create API documentation (Swagger UI)

---

## References

- **Vertical Slice Architecture**: https://jimmybogard.com/vertical-slice-architecture/
- **Drogon HTTP Framework**: https://github.com/drogonframework/drogon
- **Boost.DI**: https://github.com/boost-ext/di
- **sqlgen ORM**: Internal library (automatic schema from structs)
- **cpperr**: `/cpperr/` directory (Rust-inspired error handling)
