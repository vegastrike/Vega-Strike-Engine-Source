# AGENTS.md

Working notes for AI coding agents (Claude Code, Cursor, Codex, Copilot, pi, Gemini, Junie, and friends) operating in
this repository: what it is, how it builds, how the code is laid out, and which conventions changes are reviewed
against.

**`CONTRIBUTING.rst` is authoritative.** It carries the licensing rules, the PR process, and the AI policy. Read it
before your first pull request. Where this file disagrees with it, `CONTRIBUTING.rst` wins and this file should be
corrected.

## What this repository is

The Vega Strike **core engine** — a C++14 space-simulation engine with embedded Python for game logic. Game data (ships,
systems, art, Python modules) lives in the separate [
`vegastrike/Assets-Production`](https://github.com/vegastrike/Assets-Production) repository. A change that spans both
needs two pull requests, one per repository.

Branches:

- `master` — development head, **SDL3**.
- `0.10.x` — long-term-supported release line, **SDL2**. Bug fixes are back-ported from `master` in batches; features
  land here only on request. Do not put SDL3 code on `0.10.x`, and do not include `<SDL2/SDL.h>` on `master`.
- `0.5.4` — an older LTS lineage that shares these repositories but diverged from `master` in 2013. It is not a branch
  of `master`; changes generally do not belong on both.
- Every other `0.[n].x` / `0.[n].[m]` branch is historical and unmaintained. Do not target one with new work, and do not
  open pull requests against them.

## Build and test

Use the scripts in `script/`. They exist so that bootstrapping, building, testing, installing and packaging each have
one canonical way of being done, and they are what CI calls. See the `Commands` section for the exact commands to run.

Preset names follow `[platform]-[generator]-[pie]-[gl]-[build-type]`; `CMakePresets.json` is the source of truth, and
both `script/build` and `script/test` pick their preset by that name. Build trees go under `build/<preset-name>/`. If no
script fits the task, the underlying commands are `cmake --preset <preset-name>` and
`cmake --build build/<preset-name>` — but prefer the script.

The engine binary is `vegastrike-engine` — older scripts and documentation may still call it `vegastrike`.

**Unit tests** use GoogleTest, enabled by default (`USE_GTEST=ON`), covering the engine and the `libraries/`
subprojects.

**A manual playtest is mandatory.** CI compiles, links and runs the GoogleTest suite on Linux, macOS and Windows; it
still cannot tell you whether the game plays. A change is judged against the manual test list
in [wiki: Pull Request Validation][prv]. State in the PR description which of those tests you ran. An untested PR is
closed, so if you cannot playtest the change, say so plainly instead of implying that you did.

## Commands

- install: `sudo script/bootstrap`                  # install build dependencies
- build: `script/build --preset-name=<preset-name>` # configure, build, copy the results into bin/
- test: `script/test --preset-name=<preset-name>`   # run the GoogleTest suite
- lint: `clang-tidy -p build/<preset-name> <file>`
- format: `clang-format -i <filename>`; prefer version 20 or 21

`script/install`, `script/package`, `script/clean` and `script/setup` cover the remaining tasks, and `script/cibuild` is
the CI entry point. PowerShell equivalents live alongside as `*.ps1`.

## Critical Rules

1. **C++14, and stay portable.** `CMAKE_CXX_STANDARD 14` is required; MSVC builds with `/std:c++14`, so C++17 constructs
   (init-statements in `if`, `std::optional`, structured bindings) break the Windows build.
2. **Keep functions small and legible.** Prefer several named steps over one dense expression.
3. **Name your values.** No unexplained literals; reuse an existing named default rather than restating its value.
4. **Braces on every `if`/`for`/`while`**, however short the body.
5. **`double` for positions and distances**, not `float`. Precision matters at the scale of a star system, and narrowing
   even temporarily has produced real bugs.
6. **Parse numbers locale-aware** — `locale_aware_stof` / `locale_aware_stod`, never `atof`.
7. **Fix problems at the call site.** Do not change library behaviour for one caller; add the capability behind an
   opt-in the caller asks for.
8. **Use a container that suits the lookup.** `std::map`/`std::set` when searching by key, not a linear scan of a
   vector.
9. **Do not `const_cast` away constness** to reach mutable state — make the path const-correct.
10. **Match the surrounding file.** Where a file deliberately keeps its dependencies light, keep them light.
11. **Do not reformat data files** when you are only changing a value; diff noise in JSON/XML costs review time.
12. **Comments carry intent.** Explain *why* a non-obvious thing is done. Do not narrate what the next line already
    says, and do not leave a comment recording what the code used to do — change history belongs in the pull request
    description.
13. **No references to private or local material** in code or comments: no local paths, no personal notes, nothing a
    reviewer on GitHub cannot open.
14. **Clean up only where you are working.** Dead configuration and stale comments in your area may go, in a commit
    separate from the functional change. Do not sweep the wider tree.
15. **Leave no debug output behind**: no stray `printf`/`fprintf`/`std::cout`, no commented-out experiments, no
    temporary files.
16. **One logical change per commit**, with a message that says what changed and why. Keep a feature and a cleanup in
    separate commits.
17. **Licensing is GPLv3-or-later.** Keep existing file headers intact. Under the AI policy in `CONTRIBUTING.rst`, an AI
    agent is never added to a file's copyright or licence statement — only the human submitting the work.

## Code layout

| Path                                                   | Contents                                                                                  |
|--------------------------------------------------------|-------------------------------------------------------------------------------------------|
| `engine/src/gfx/`                                      | rendering: meshes, textures, sprites, HUD, radar, starfield, quadtree, particles, cockpit |
| `engine/src/cmd/`                                      | engine-side game logic: bases, briefings, AI, factions, mission scripting                 |
| `engine/src/gui/`                                      | UI widgets, the legacy `TextPlane`/`PaintText` text path, the ImGui text path             |
| `engine/src/gldrv/`                                    | OpenGL driver layer: init, matrix, state, textures, window system                         |
| `engine/src/components/`                               | ship subsystems: reactor, shield, hull, drive, cloak, ECM, FTL, jump                      |
| `engine/src/audio/`                                    | OpenAL sound: codecs, renderers, listeners, sources                                       |
| `engine/src/python/`                                   | Python bindings and initialisation                                                        |
| `engine/src/resource/`                                 | asset loading: cargo, manifest, JSON, RNG                                                 |
| `engine/src/configuration/`                            | `config.json` parsing and the game-configuration model                                    |
| `engine/src/in_*.cpp`, `engine/src/ai/`                | input: mouse, keyboard, joystick to bindings and commands                                 |
| `engine/tools/`, `engine/objconv/`, `engine/launcher/` | asset converters, model tools, launcher                                                   |
| `libraries/`                                           | in-tree libraries; `libraries/cmd/` holds the portable object model (see below)           |

Top-level `engine/src/` files: `main.cpp`, `main_loop.cpp`, `physics.cpp`, `star_system.cpp`, `universe.cpp`,
`in_main.cpp`, `vs_logging.cpp`.

Two UI generations coexist. New interface work goes through ImGui and `libraries/vega_draw`; the legacy `TextPlane` and
base-room path is still live and shares call sites with the modern one, so a change in one is visible in the other. Know
which path you are in before you edit it.

## Facts worth knowing before you dig

- **The game-object model is not in `engine/src/`.** `Unit` — a ship, station, planet, missile, asteroid — is declared
  in `libraries/cmd/unit_generic.h`, together with the class hierarchy it builds on (`drawable` → `movable` →
  `damageable` / `armed` / `energetic` / `intelligent` / `jump_capable`). `engine/src/cmd/` holds the engine-side
  pieces: bases, briefings, AI, factories, mission scripting, rendering of those objects. Look in `libraries/cmd/` first
  for anything about how an object behaves; look in `engine/src/cmd/` for how it is driven and drawn.
- **`*_server.cpp` files are a relic of the old multiplayer mode** and are largely unmaintained. Do not extend them with
  new behaviour, and do not treat them as the reference for how a class works. `DISABLE_CLIENT` is legacy in the same
  way and is a candidate for replacement with CMake components or removal — ask before building anything on it.
- **Configuration reads go through the `GameConfig` accessors** in
  `engine/src/configuration/game_config.h` (`GetBool`, `GetFloat`, `GetDouble`, `GetString`, … by path into
  `config.json`). Add a value to the JSON and read it by path; do not hard-code a value that a player or asset package
  is meant to control.
- **Tests sit next to their subsystem**: `engine/src/{cmd,components,configuration,damage,resource}/tests/`, plus
  `libraries/*/`. A fix without a test in the neighbouring `tests/` directory needs a reason in the PR.
- **Running it** needs a data set (from `Assets-Production`, or an installed package). Then:
  `vegastrike-engine -D/path/to/data`. Useful switches: `-m <mod>`, `-j <system>`,
  `-l <location>`, `--debug 0-3`, `--test-audio`, `--benchmark`, `--help`.
- **Logging is `VS_LOG(level, message)`** with levels `trace`, `debug`, `info`, `important_info`, `warning`,
  `serious_warning`, `error`, `fatal`. Everything below `important_info` is filtered at the default verbosity, so a
  missing log line is usually filtered rather than absent. Raise the level with the `--debug` option (0–3) instead of
  changing the level in code. `--debug=1` allows `info` messages through; `--debug=2` allows `debug` messages through;
  and `--debug=3` allows `trace` messages (all messages, really) through the filter.
- **Logs** are written to `~/.vegastrike/logs/` alongside the per-user config and saves.
- **Asset tooling** lives in `engine/objconv/` (`vega-meshtool`, `vs-buildenv-trisort`, `vs-buildenv-asteroidgen`,
  `vs-buildenv-replace`) and `engine/tools/`; it converts the art pipeline's output into what the engine loads, so
  asset-format questions usually start there.
- **The CI matrix is the definition of "it builds" and "the tests pass"**: five Linux distributions, macOS and Windows,
  each building and running the suite (`.github/workflows/`). Building locally with the same preset name, including its
  exact `RelWithDebInfo` casing, reproduces what CI does.

## Style

Formatting is defined by the configuration files in the repository root. Do not invent rules beyond them:

- `.clang-format` — Google baseline, 4-space indent, no tabs, column limit 120, braces added around single-statement
  bodies.
- `.clang-tidy` — the warning set the project cares about. Do not silence a check to make a warning disappear.
- `.editorconfig` — LF line endings, UTF-8, final newline, no trailing whitespace.

Run `clang-format` on the files you touch, and only those files.

## Practical notes

- **Configuration is `config.json`.** The old `vegastrike.config` XML loading path is gone; only test fixtures still use
  the name. The game-data side of it lives in `Assets-Production`.
- **The logical window size and the render-output size are not always the same** (DPI scaling, windowed resize).
  Interface positioning belongs in logical display space; conflating the two has produced text-scaling bugs before.

## When in doubt

Ask on the pull request or in [Gitter](https://gitter.im/vegastrike/community) rather than guessing. A question costs
one message; a wrong assumption about a subsystem this size costs a reviewer an
afternoon.

[prv]: https://github.com/vegastrike/Vega-Strike-Engine-Source/wiki/Pull-Request-Validation
