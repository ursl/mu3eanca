# prod/common — shared Perl for prod workflows

Used by `relval`, `startGT`, and `prompt0`. Workflow-specific modules live next to
their drivers (`Relval.pm`, `StartGT.pm`, `Prompt.pm`), not here.

| Module | Role |
|--------|------|
| `ProdConfig.pm` | Config files + host overlay (`host-*.cfg` or `config-*.cfg`) + `[repo]` / `[task]` |
| `Setup.pm` | Clone / checkout / merge / submodules / cmake / make install / relink |

Drivers add this directory with `use lib …/../common`.
