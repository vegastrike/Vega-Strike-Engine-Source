# Vega Strike Engine API Revisions

Online: https://github.com/vegastrike/Vega-Strike-Engine-Source/wiki/Vega-Strike-Engine-API

## Overview

The Engine API is utilized by the game assets in order to produce a working game.
Historically this was not well documented and the game assets were unable to determine
if they were compatible with a given version of the engine. Starting in Vega Strike Engine 0.8.0
the Engine API Version tooling was introduced.

If the Engine API is not available, then the game assets can assume that the running Vega Strike Engine
is version 0.7.x or earlier. No guarantees are then made to the stability of the API or anything being
available.

The below documents the changes to the Engine API starting with Vega Strike Engine 0.8.0, Engine API Version 1.

## Detecting Engine Versions

Game Assets can use the following block to check for the Vega Strike Engine API:

```python
        
        # Check if the `EngineVersion` object is available
        hasVersion = hasattr(VS, 'EngineVersion')

        # get the engine version tuple in a displayable format
        # if it needs to be compared, then use the original tuple version
        ev = (
            VS.EngineVersion().GetVersion()
            if hasVersion
            else (0, 7, 0, 'unknown') # 0.7.x was the last version without this API
        )
        engineVersion = '.'.join(
            [
                str(i)
                for i in ev
            ]
        )

        # get the Engine API Version, default to zero if the EngineVersion is not available
        apiVersion = (
            VS.EngineVersion().GetAssetAPIVersion()
            if hasVersion
            else 0
        )

        # log out what was found
        trace(TRACE_WARNING, "::: What's in VS object %s :::" %(dir(VS)))
        trace(TRACE_WARNING, "::: Engine Version {0} :::".format(engineVersion))
        trace(TRACE_WARNING, "::: Asset API Version {0} :::".format(apiVersion))
```

## Change Log

### Vega Strike Engine API Version 3

Vega Strike Engine 0.10.x and 0.11.x (master)

#### Changes
- **Asset API Version**: Reports `3` via `VS.EngineVersion().GetAssetAPIVersion()`.
- **Base Computer Python Module (`base_computer`)**:
  - Exposes the `base_computer` built-in module and the `base_computer.TransactionType` enumeration (`BUY_CARGO`, `SELL_CARGO`, `BUY_UPGRADE`, `SELL_UPGRADE`, `BUY_SHIP`, `ACCEPT_MISSION`, `NULL_TRANSACTION`).
  - Integrates `GetString` infrastructure (`python/infra/get_string.cpp`) to dynamically invoke Python methods and generate player information and upgrade displays.
- **Cargo Python Class Enhancements**:
  - Adds dedicated Python constructors for `VS.Cargo`.
  - Exposes comprehensive accessor and mutator methods:
    - Pricing, Mass & Volume: `SetPrice()`, `GetPrice()`, `SetMass()`, `GetMass()`, `SetVolume()`, `GetVolume()`
    - Inventory: `SetQuantity()`, `GetQuantity()`
    - Identity & Categories: `SetContent()`, `GetContent()`, `SetCategory()`, `GetCategory()`, `GetDescription()`
    - Flags & Function: `SetMissionFlag()`, `GetMissionFlag()`, `SetFunctionality()`
- **Unit Enhancements & Updates**:
  - Adds `Unit.Derelict()` method to configure derelict hulls directly from scripts.
  - Restores `Unit.PercentOperational(content, category, countHullAndArmorAsFull)` in 0.11.x.
  - Removes deprecated `Unit.GetMoment()` in 0.11.x in favor of dynamic ship mass inertia calculations.
- **Briefing Module Registration**:
  - Registers the built-in `Briefing` module in Python initialization table, providing cinematic script controls:
    - Ship management: `addShip()`, `removeShip()`, `getShipPosition()`, `setShipPosition()`, `setCloak()`
    - Order queues: `enqueueOrder()`, `replaceOrder()`
    - Camera control: `setCamPosition()`, `setCamOrientation()`
    - Execution: `terminate()`

### Vega Strike Engine API Version 2

Vega Strike Engine 0.9.x

#### Changes
- **Asset API Version**: Reports `2` via `VS.EngineVersion().GetAssetAPIVersion()`.
- **Base UI Overload Support**:
  - Overloads `Base.TextBox()` to accept 3D `Vector` tuples for foreground and background colors in addition to `GFXColor`, preventing script errors when alpha is omitted.
- **Unit & Flightgroup Controls**:
  - Restores and standardizes unit thrust, weapon management, and flightgroup methods:
    - Thrust vectors: `Unit.LateralThrust()`, `Unit.LongitudinalThrust()`, `Unit.VerticalThrust()`
    - Weapons & Turrets: `Unit.Fire()`, `Unit.ToggleWeapon()`, `Unit.SelectAllWeapon()`, `Unit.SetTurretAI()`, `Unit.DisableTurretAI()`
    - Systems: `Unit.ActivateCloak()`, `Unit.ActivateJumpDrive()`, `Unit.EjectCargo()`, `Unit.SetHull()`, `Unit.Split()`, `Unit.addCredits()`, `Unit.leach()`, `Unit.setECM()`
    - Flightgroups: `Unit.ReTargetFg()`
  - Removes outdated `PositionITTS` method.
- **Modern Python Compatibility**:
  - Python runtime integration updated for Python 3.11+ compatibility (removing obsolete `<eval.h>` includes and frame handling).

### Vega Strike Engine API Version 1

Vega Strike Engine 0.8.x

#### Changes
- **Engine Version & API Detection Tooling**:
  - Introduces the `VS.EngineVersion` class:
    - `GetVersion()`: Returns a 4-tuple `(major, minor, patch, tweak_hash)` (e.g. `(0, 8, 0, "hash")`).
    - `GetAssetAPIVersion()`: Returns the integer Asset API version (`1`).
  - Added startup logging reporting engine version and asset API version.
- **Director Unicode Support**:
  - Adds `Director.isUtf8SaveGame()` allowing scripts to determine if savegame files are UTF-8 encoded.
- **Unit Method Identifier Normalization**:
  - Normalizes unit method names to camelCase:
    - `Unit.energyData()` (replaces `Unit.EnergyData`)
    - `Unit.fuelData()` (replaces `Unit.FuelData`)
    - `Unit.getNumMounts()` (replaces `Unit.GetNumMounts`)

### Vega Strike Engine API Version 0

Vega Strike Engine 0.7.x and earlier.

This is the default value and matches any engine prior to the introduction of the `VS.EngineVersion` detection API.

#### Changes
- **Vega Strike 0.7.0**:
  - **Python 3 Port**: The engine was ported from Python 2 to Python 3. Embedded modules migrated to `PyModule_Create` / `PyImport_AppendInittab`. Game asset Python scripts require Python 3 syntax.
  - Transitioned from internal vendored Boost to system Boost.Python linked against Python 3.
- **Vega Strike 0.6.0**:
  - Build system modernized to CMake. Python API identical to 0.5.3 running on Python 2.7.
- **Vega Strike 0.5.3**:
  - Added `Unit.unsetMissionRelevant()`.
- **Vega Strike 0.5.1**:
  - Added Base video playback: `Base.HasObject()`, `Base.PlayVideo()`, `Base.SetDJEnabled()`, `Base.SetVideoCallback()`, `Base.StopVideo()`.
  - Added Director string lists: `Director.loadStringList()`, `Director.saveStringList()`.
  - Added Universe/Faction helpers: `VS.GetJumpPath()`, `VS.adjustRelationModifier()`, `VS.getRelationModifier()`, `VS.getStarDate()`, `VS.getStarTime()`, `VS.playSoundCockpit()`.
  - Added Unit relation queries: `Unit.getFactionRelation()`, `Unit.getRelationFromFaction()`, `Unit.getRelationToFaction()`, `Unit.rayCollide()`.
- **Legacy API History (0.1.x through 0.5.0)**:
  - For the complete historical evolution of the Python API from its introduction in 2001 (0.1.x) through 0.5.0, see [doc/legacy-history-migration.md](legacy-history-migration.md#7-python-engine-api-evolution-01x--050).
