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

Vega Strike Engine 0.10.x

#### Changes
- TODO

### Vega Strike Engine API Version 2

Vega Strike Engine 0.9.x

#### Changes
- TO DO

### Vega Strike Engine API Version 1

Vega Strike Engine 0.8.x

#### Changes
- Introduces the Vega Strike Engine API Detection Tooling. The Vega Strike Engine API is otherwise identital to Vega Strike Engine API Version 1.
- Game Assets can now utilize the `EngineVersionData` object to retrieve the game version (e.g 0.8.0, 0.8.1, 0.9.0) and the Vega Strike Engine API Version, thus allowing Game Assets to determine compatibility with the game engine.

### Vega Strike Engine API Version 0

Vega Strike Engine 0.7.x and earlier.

This is the default value and matchees any engine prior to the introduction of the Vega Strike Engine API.

#### Changes
- There were no specific changes in 0.7.x or 0.6.x.
- Change history before 0.6.x is unknown.
