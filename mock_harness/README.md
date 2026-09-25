# Mock-engine campaign harness

A thin, pure-Python stand-in for the Vega Strike engine that runs the real
Privateer Gemini Gold data pack (bases, campaigns, missions, dynamic
universe) under Python 3 and plays the campaigns end to end with an
automated player.  It exists to find Python 2 -> 3 port bugs and campaign
logic bugs without having to play the game by hand.

```
python3 mock_harness/run_campaign.py                     # full playthrough
python3 mock_harness/run_campaign.py --scenario all      # every scenario
python3 mock_harness/run_campaign.py --scenario main --report r.txt --events e.txt
python3 -m unittest discover -s mock_harness/tests
```

Exit status is 0 only if every phase of the scenario completed, no
unexpected mission failure happened and the data pack raised no Python
exceptions.  A full Privateer + Righteous Fire + bonus (ISO) playthrough
takes about 25 seconds.

## Layout

* `mockvs/` - the mock engine.
  * `modules.py` builds the `VS`, `Base`, `Director` and `Briefing`
    modules and installs them into `sys.modules` exactly like the engine
    (plus the engine's `sys.path` entries and `import sys` in `__main__`).
  * `boostargs.py` checks arguments the way Boost.Python does: an `int`
    parameter rejects floats (`3/2`), `unsigned int` rejects negatives,
    vectors must be tuples, arity is exact.  This catches the most common
    class of Python 3 port bug.
  * `engine.py` - star systems (from `sectors/*.system`), the galaxy
    (`universe/wcuniverse.xml`), factions and relation modifiers, save
    data (`Director.*SaveData`, float32 like the engine), missions
    (`LoadMission*`, `terminateMission`, per-mission game time, the
    "last constructed Director.Mission wins" rule, `active_scripts`
    bookkeeping), docking and base selection (`bases/<type>_<faction>_<tod>.py`),
    jumping (with `visited_<system>` marking), cargo (including the
    engine's `addCargo` quirks), upgrades/mounts, the new-game flow from
    `main_menu.mission` and `New_Game`.
  * `units.py` - `VS.Unit` (a weak handle like `UnitContainer`: killed
    units become null), `VS.Cargo` (a value type), `VS.un_iter`.
  * `base.py` - base rooms/links/objects and click semantics (event
    masks, `down`/`up`/`click` event data for GUI.py, goto/launch/comp).
  * `ai.py` - deliberately simple flight and combat: ships fly straight
    lines and, once per game second ("turn"), every armed ship in weapon
    range of its target deals `--damage` points.  Everyone starts with
    `--npc-hp` (10000); a ship becomes a "bad guy" with `--enemy-hp` (20)
    as soon as it shoots the player or a mission-relevant ship, or the
    player shoots it.  Stations are scenery.
  * `divcheck.py` - optional import hook rewriting every `/` in the data
    pack to record where two ints were divided with a non-integral
    result (Python 2 would have floored).  On by default; the report
    lists suspicious sites.
* `driver/` - the automated player.
  * `game.py` / `pilot.py` - the "hands": click links, launch, route
    across jump points, fly, fight, dock, tractor.
  * `campaign_model.py` - reads `campaign_lib` trees and simulates
    (without side effects) what `Campaign.getCurrentNode` would do if
    the player docked at each candidate base, to decide where to go next.
    No storyline is hard coded.
  * `strategies.py` - per mission type flight strategies (directions,
    ambush, patrol, cleansweep, escort, defend, bounty, rescue, drone...).
  * `campaign_driver.py` - the main loop, fixer conversations, choices,
    progress/stall detection.
  * `scenario.py` - which branches to take and how to tune the universe.

## Scenarios

| scenario | what it covers |
| --- | --- |
| `full` | Privateer, Righteous Fire, bonus campaign via the ISO (Demetria) route: dump Syrai's contraband for Destinee, divert Kaydence's escort to Newcastle |
| `full-refuse-first` | same but refuses every offer once (reject/reconsider dialogs); bonus via the pirate/Soren route |
| `main` | main campaign only |
| `main-escort-dies` | Hunter Toth is fragile: Masterson's first escort fails and the campaign must end cleanly on its failure branch |
| `bonus-syrai-soren` | bonus campaign: undercover but deliver the contraband, Syrai then Soren |
| `bonus-senator-dies` | Kaydence's senator is destroyed: the no-failure mission must branch to the ISO arc |

`--fragile NAME` gives ships of that type 1 hp for ad-hoc failure tests.

## Things the harness does that a plain script run would not

* The bonus (freetrader) campaign is switched off in the shipped data:
  its first node requires docking at a base named `DoNotEnter`.  The
  driver re-points those conditions at Munchen (Tingerhoff) so the bonus
  missions can be exercised; the data pack itself is unchanged.
* The Steltek gun needed to start Righteous Fire is picked up from the
  derelict in Delta Prime when the driver passes by (a human has to know
  to do this too).

## Known differences from the real engine

* No physics, sensors, energy, shields or real weapons; damage is the
  turn-based model above.  Speeds come from `units.csv`.
* The player is repaired when docking (`Options.repair_on_dock`).
* `bool(VS.Unit)` works in the mock.  The engine only exports
  `__nonzero__`, which Python 3 ignores (every unit is truthy there,
  making loops like `while (un): ... un = VS.getUnit(i)` spin forever);
  that is an engine fix, see the report.
* Time does not pass while docked (matching the engine's default
  `simulate_while_docked=false`), except for the 16 director frames the
  engine runs when a base is created.
