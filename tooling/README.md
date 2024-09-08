# Developer Tooling

The Clang-Tidy helper script can be run on an already existing development build in a dev container with:

```
/bin/python3 /workspaces/source/tooling/clang-tidy.py -p /workspaces/source/build/debug -clang-version 18 -header-filter=/workspaces/source/daemon/.*
```
