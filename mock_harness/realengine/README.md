# Real-engine remote control

Plays real missions in the real engine, the way a player would (clicking
base links, pressing bound keys), to check what the mock harness cannot:
the C++ side of the Python bindings, the real GUI rooms, docking and
jumping.

```
# build the engine (Release; RelWithDebInfo also works now)
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j vegastrike
python3 mock_harness/realengine/run_real_missions.py --binary build/vegastrike --mission cargo
```

The run uses Xvfb (`--display :0` to watch instead) and its own home
directory (`VS_HOMEDIR`), so the user's `~/.privgold100` saves are never
touched.  Exit status 0 means the mission reached an outcome (completed and
paid, or lost because the player was shot down) and the engine printed no
Python exception; getting stuck, an engine crash or a Python exception is
a failure.  Mission offers are random (`--seed` seeds the scripts' random
numbers but the offers still vary), so a run may use another base of the
system when the first has no matching offer.

## How it works

* `VS_REMOTE_CONTROL=<file.py>` makes the engine import that file and call
  its `tick(docked)` at the start of every frame, in space and in bases
  (`src/remote_control.cpp`).  Without the variable nothing changes.
* The `VSRemote` module gives scripts what a human has but `VS`/`Base` do
  not: `GetLinks()`/`GetTexts()` of the current base room,
  `ClickLink(index)`/`ClickLinkNum(num)` (a real left click routed through
  `Room::Click` at a point of the link, `BaseInterface::ScriptedClick`),
  `KeyCommand(name, 'tap'|'press'|'release')` for any command bound in
  `vegastrike.config` (`DockKey`, `JumpKey`, ...), `GetObjectives()`,
  `GetMessages(n)`, `GetJumpPoints()`, `GetDockingPorts(unit)` and
  `GetSaveStringBytes()`.
* `vs_remote.py` (inside the engine) serves a line-based JSON protocol on
  127.0.0.1: `{"eval": expr}` / `{"exec": code}`.  `remote.py` starts the
  engine and talks to it; `session.py` has the player actions (walk to a
  room, click, buy an upgrade, route, jump, dock).
* Flight is not simulated by a pilot AI yet: the player uses the game's
  own autopilot (`AutoPilotTo`) and is then moved the rest of the way
  (`SetCurPosition`) before pressing `JumpKey`/`DockKey`.  Everything the
  mission scripts see (jumps, arrival, docking) is the real engine.
  Capital ships are docked at their docking ports (not yet seen working:
  no run has drawn a capital-ship delivery since this was added).

## Missions

* `cargo`: New Game, buy a jump drive in the upgrade room (the starting
  Tarsus has none; the run adds the missing credits), accept the first
  cargo offer in the mission computer, fly to the destination system
  through the jump points, dock at the base named in the objective, check
  the cargo was taken and the pay arrived.
* `patrol`: accept a patrol (or nav-point attack) mission and scan every
  point in its objectives.  Scanning stays out of planets' automatic
  landing zones (landing stops the simulation, and with it the mission).
  The hull is topped up when below half (a test cheat); if the player is
  shot down anyway the run counts as a legitimate loss.
