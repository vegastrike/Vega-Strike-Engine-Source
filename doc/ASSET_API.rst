============================
Vega Strike Engine Asset API
============================

This document provides a changelog of the Vega Strike Engine Asset API.

What is the Vega Strike Engine Asset API?
-----------------------------------------

The Vega Strike Engine Asset API is the Python-based bindings used by the Game Assets.

The Vega Strike Engine itself provides the ability for Game Assets, such as
Vega Strike: Upon the Coldest Sea, to create a 3D Space-based game. The Game Assets
and the Vega Strike Engine interface using the Vega Strike Engine Asset API.

Vega Strike Engine Versioning
-----------------------------

As of version 0.8.0 the Vega Strike Engine Asset API, aka Asset API, provides an interface
for determining the versions information related to the Vega Strike Engine including:

- the official version fo the Vega Strike Engine (f.e `0.8.0`)
- the version of the Asset API implemented by the Vega Strike Engine using a simple incrementing
  integer value.

For simplicity, the Asset API Version is a simple incrementing interger value with a behavior
like that of the Android SDK version. The Asset API and the Vega Strike Engine versions have a
loose relationship. Game Assets should use the Asset API Version as the basis for the
determining available functionality.

Discovering the Vega Strike Engine Versions
+++++++++++++++++++++++++++++++++++++++++++

The following code block can be used to determine the version of the Asset API
and provides protection against versions from before the version interface was
provided.

.. code-block::python

        # Check if the `VS` object has the version interface
        hasVersion = hasattr(VS, 'EngineVersion')
        # get the engine version tuple in a displayable format
        # if it needs to be compared, then use the original tuple version
        ev = (
            VS.EngineVersion().GetVersion()
            if hasVersion
            else (0, 7, 0, 'unknown') # 0.7.x was the last version without this API
        )

        # Convert the Vega Strike Engine Version Information
        # to a string so it can be printed
        engineVersion = '.'.join(
            [
                str(i)
                for i in ev
            ]
        )

        # Get the Vega Strike Engine API Version
        apiVersion = (
            VS.EngineVersion().GetAssetAPIVersion()
            if hasVersion
            else 0
        )

        # Log the information
        trace(TRACE_WARNING, "::: What's in VS object %s :::" %(dir(VS)))
        trace(TRACE_WARNING, "::: Engine Version {0} :::".format(engineVersion))
        trace(TRACE_WARNING, "::: Asset API Version {0} :::".format(apiVersion))

        # Check against `apiVersion` here to ensure the required version

When does the Asset API Version Change?
---------------------------------------

There is a loose relationship between the Vega Strike Engine Version (e.g `0.8.0`) and the
version of the Asset API (e.g `1`).  The Vega Strike Engine Version will change with each
release of the Vega Strike Engine; however, the Asset API version will only change when
there are changes to the Asset API such as:

- New functionality
- Renaming existing functionality (e.g someFunction to SomeFunction)
- Removing functionality
- Changing behavior of an existing function

What kind of changes avoid changing the Engine API version?

- Refactoring the Asset API in a manner that does not change its behavior.
- Changes to code not impacting the behavior of the Asset API.

Essentially, any time a change happens to the Asset API that may effect the Game Assets.

Asset API Version History
-------------------------

Version 1
+++++++++

    Initial Release of the Asset API Version Interface which introduces:
    - `VS.EngineVersion()` to retrieve the Engine Version Object
    - `VS.EngineVersion().GetVersion()` to retrieve the version of Vega Strike (e.g `0.8.0`)
    - `VS.EngineVersion().GetAssetAPIVersion()` to retrieve the version of the Assets API.

Version 0
+++++++++

    Any version of the Vega Strike Engine prior to version 0.8.0 when the Asset API Version Interface
    was introduced.

    The Asset API is stable from version 0.6.0 through 0.7.x.

    The stability of the Asset API prior to version 0.6.0 is unknown. Game Assets should use caution
    when using these versions.
