========================
Contributor's Guidelines
========================

Welcome to Vega Strike!

There are many ways to contribute - coding, art work, documentation, testing, and much more.
This repository covers the core Vega Strike Engine that sits at the heart of it all, and
the guidlines here are specific to working with this repository. Please see the
`Vega Strike Community Contribution`_ guidelines for other areas of contribution.

Overview
========

A Vega Strike game consists of two parts:

- The Vega Strike Core Engine
- Game Assets

The Vega Strike Core Engine provides a game engine for space-based games though a C++
game engine that integrates with Python. The Game Assets extend the game engine through
the Python interface and related art work, models, etc.

This repository contain the Vega Strike Core Engine. The `Vega Strike: Under the Coldest Sea`_
game assets are separately available - `Vega Strike: UtCS Production`_.

Communications
==============

There are a variety of places that the Vega Strike developers have been located throughout the project history.
Below are the official places to find the current developers:

* Gitter.im: https://gitter.im/vegastrike/community
* FreeNode IRC: #vegastrike
* Vega Strike Forums: https://forums.vega-strike.org/

Licensing
=========

Please bear in mind that we are bound to accept contributions under the following licenses. This means that if you submit your work for use in Vega Strike, it will be automatically licensed under GPL unless you state one of the other licenses:
* (GPL) GNU General Public License
* (LGPL) GNU Lesser General Public License
* (GPDL) GNU General Public Documentation License
* (PD) Public Domain
* (CC-BY) Creative Commons By Attribution license
* (CC-SA) Creative Commons Share Alike license
* (CC-BY-SA) Creative Commons By Attribution Share Alike license

Please note that we do not allow licenses not mentioned above, in particular:
* (CC-NC) Creative Commons Non-Commercial license or any combination with CC-BY and CC-SA

For a more updated list, please see `Vega Srike: Licensing`_.

Building
========

Information on building Vega Strike is available in the README at https://github.com/vegastrike/Vega-Strike-Engine-Source#compiling-vegastrike.

Installing
==========

TODO - The Vega Strike installer stills needs attention. Information to come.

Testing
=======

Currently the best way to test the game is to play it. We would like to build out an automated test suite for the
Vega Strike Core Engine. The minimum tests required for changes are documented at `Vega Strike: Pull Request Validation`_
and are a series of manual game play tests that we have determined provide sufficient coverage for current development.

Bugs, Features, etc.
====================

Bug, Feature Requests, etc for the Vega Strike Core Engine can be filed at `Vega Strike: Issues`_.

Pull Requests
=============

We use a fork model of development for changes with all changes required to be submitted via a pull request in the GitHub
repository:

1. Fork the GitHub repository into your own GitHub user
2. Make your desired changes.
3. Ensure you've documented your code.
4. Test using the procedures outlined at `Vega Strike: Pull Request Validation`_
5. Submit a pull request at `Vega Strike:: Pull Requests`_.

.. note:: Documentation only changes (RST, MD, etc) do not require step 4.

When submitting a Pull Request please document:
* Whether you've run the pull request validation tests and any issues you ran into doing so
* What you're changing and why
* Note any related `Vega Strike: Issues`_ or `Vega Strike: Milestones`_.

.. _`Vega Strike Community Contribution` :: https://wiki.vega-strike.org/HowTo:Contribute
.. _`Vega Strike: Under the Coldest Sea` :: https://github.com/vegastrike/Assets-Masters
.. _`Vega Strike: UtCS Production` :: https://github.com/vegastrike/Assets-Production
.. _`Vega Srike: Licensing` :: https://wiki.vega-strike.org/Development:Graphics_Requirements#Licenses
.. _`Vega Strike: Issues` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/issues
.. _`Vega Strike: Milestones` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/milestones
.. _`Vega Strike: Pull Request Validation` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/wiki/Pull-Request-Validation
.. _`Vega Strike:: Pull Requests` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/pulls
