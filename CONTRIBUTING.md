<!--
SPDX-FileCopyrightText: 2024 Roman Gilg <romangg@manjaro.org>
SPDX-License-Identifier: MIT
-->
# Contributing to bxt

- [Contributing to bxt](#contributing-to-bxt)
  - [Dev Setup](#dev-setup)
    - [Prerequisits](#prerequisits)
    - [VS Code](#vs-code)
  - [Submission Guideline](#submission-guideline)
    - [Preparing Changes](#preparing-changes)
    - [Commit Message Guideline](#commit-message-guideline)

## Dev Setup
### Prerequisits
- Docker or Podman (not tested)
- For VS Code the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

### VS Code
- Reopen in Dev container (slow on first run)
- Install recommended extensions into Dev container
- In VS Code terminal: `cp /workspaces/bxt/configs/box.yml /workspaces/build/bin/`
- Run and debug Daemon (select from bottom controls or left debug panel)
- Open site in your browser at `localhost:8080`

## Submission Guideline
### Preparing Changes
We are always happy about fixes for bugs or small design issues. Thanks in advance!
If you plan to do larger changes, please create first an issue ticket to let us discuss your plans,
so in the end your changes will align with our overall plans for the website and can be merged.

### Commit Message Guideline
The [Conventional Commits 1.0.0][conventional-commits] specification is applied with the following
amendments:

* Allowed commit types [are these](commitlint.config.ts#L34-L44). The "chore" type is allowed, but try to not overuse it.
* Allowed scopes [are these](commitlint.config.ts#L16-L18)
* Any line of the message must be 90 characters or shorter.
* Angular's [Revert][angular-revert] and [Subject][angular-subject] policies are applied.

[angular-revert]: https://github.com/angular/angular/blob/3cf2005a936bec2058610b0786dd0671dae3d358/CONTRIBUTING.md#revert
[angular-subject]: https://github.com/angular/angular/blob/3cf2005a936bec2058610b0786dd0671dae3d358/CONTRIBUTING.md#subject
[conventional-commits]: https://www.conventionalcommits.org/en/v1.0.0/#specification
