# bxt Daemon

This folder contains backend related code.

# Permissions

Permission entries consist of multiple nodes. Each node represent a category the user can access. Wildcards are supported.

Following permission entries are supported:

### sections:

```yaml
sections.{branch}.{repository}.{architecture}
```

### packages:

_section-based permissions require corresponding section permissions as well_

```yaml
packages.get.{branch}.{repository}.{architecture}
packages.snap.{branch}.{repository}.{architecture}
packages.commit.{branch}.{repository}.{architecture}
packages.compare.{branch}.{repository}.{architecture}
packages.sync
```

### users:

```yaml
users.add
users.remove
users.update
users.get
```

### logs:

```yaml
logs
```
