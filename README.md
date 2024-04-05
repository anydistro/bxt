<div align="center">
    <img src="frontend/public/logo-full.png" alt="drawing" width="200"/>
</div>

#

> This software is in pre-alpha state. While the core functionality is implemented, things are expected to be broken.

`bxt` is a repository management system that allows to create ALPM repositories (called "Boxes" here) in a declarative way and operate with them using command line tools or a web-API.

This repository contains following projects:

- **daemon**: a server side backend application that handles all the repository work aspects.
- **frontend**: a client web application allowing users to interact with the server in a convenient manner.

### Features:

- **Clear Structure**: Box consists of sections, a branch/repository/architecture triplet (e.g., stable/core/x86_64) allowing straightforward navigation in a file manager-like UI;
- **Simple Package Management**: packages can be added, removed and updated using a plain drag-and-drop;
- **Synchronization**: Box's state can synchronized with a remote repository of arbitrary structure;
- **Automatic Deployment Support**: packages can be pushed via a basic HTTP API designed to be used in scripts;
- **User Management**: bxt is made to have a hierarchical multi-user system with per-user granular permission control;
- **Candidates System**: the package's source (either sync or manual/automatic push) preference is fully configurable;

### Setup:

To build and run this application you can use Docker

```bash
docker compose up production
```
