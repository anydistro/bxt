<div align="center">
    <img src="../../frontend/public/logo-full.png" alt="drawing" width="200"/>
</div>

#

<div align="center">
Architecture outline
</div>

## Introduction

This is an overview of the b[x] architecture for daemon components. This should make it easier for contributors to understand the specifics of this project's layout and design.

## Core Components

C++ daemon is developed with Domain-Driven Design principles in mind and split into 3 layers:

- **Domain layer**:
  - `core/domain` - business logic (entities, value objects, repositories)
- **Application layer**:
  - `core/application` - application layer that is used to orchestrate the business logic (DTOs, Services)
- **Infrastructure layer**:
  - `persistence` - persistence logic implementation (databases, configuration)
  - `infrastructure` - infrastructure logic implementation (application layer implementations, networking, events, notifications)
  - `presentation` - presentation logic (http-controllers, filters, messages)

The code also contains `utilities` directory. It's for various cross-cutting concerns or third-party libraries bindings.
