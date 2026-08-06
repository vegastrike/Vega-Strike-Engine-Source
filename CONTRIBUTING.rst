========================
Contributor's Guidelines
========================

Welcome to Vega Strike!

There are many ways to contribute - coding, art work, documentation, testing, and much more.
This repository covers the core Vega Strike Engine that sits at the heart of it all, and
the guidelines here are specific to working with this repository. Please see the
`Vega Strike: Community Contribution`_ guidelines for other areas of contribution.

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

* Gitter.im: https://gitter.im/vegastrike/community, https://gitter.im/vegastrike/vegastrike-packaging
* FreeNode IRC: #vegastrike
* Vega Strike Forums: https://forums.vega-strike.org/

AI :robot:
==========

The adoption of artificial intelligence presents both significant opportunities and legitimate ethical, security, and operational
concerns. Recognizing the magnitude of these challenges, we take the risks associated with AI seriously. Rather than prohibiting
its use—which risks driving adoption unmonitored into the shadows—this policy establishes clear guidelines for its safe,
transparent, and responsible integration. By defining explicit boundaries, we ensure that our teams can leverage AI effectively
while maintaining strict oversight, quality control, and risk mitigation across all projects.

As a general rule, we follow the same policies on AI contributions as the `Linux Kernel: AI Policy`_ with the following changes:
- Use only our accepted licenses, see `Vega Strike: Licensing`_ for details.
- Follow our coding style
- AI agents may not be added to the licensing statements in the files, only the human author making the contribution.
- Like with human contributed changes all changes must be play tested by the author as appropriate. Do not simply use the AI to make a contribution that has never been actually appropriately tested. Lack of testing will have your PR closed.
- Code requires comments; do not outright remove comments. Comments carry the intent; while code tells what actually happens.

Licensing
=========

Vega Strike and our various Game Assets are licensed under a variety of licenses as documented by `Vega Strike: Licensing`_.

This repository specifically covers the Vega Strike Engine which is simply licensed under the GNU General Public License v3 or later.

Engine Code
-----------

All specific code that is fully under the control of this project and not imported from another project
needs to be licensed under the GNU General Public License v3 or later.

Libraries
---------

There are times that third party libraries have to get integrated directly to the Vega Strike Engine code because
of how they are distributed. Such libraries need to follow these rules:

1. They must be linkable against code licensed with the GNU General Public License v3 or later.
2. They must be distributable with a valid Open Source License, as defined by the Open Source Definition and maintained by the Open Source Initiative.
3. Wrappers to them should be either in the project's original license or under the Vega Strike Engine's license.

When integrating a new third party library please be prepared to discuss its licensing.

History
-------

The Vega Strike Engine code was originally licensed under the GNU General Public License v2 or later.
It was migrated to GNU General Public v3 or later after moving to GitHub and in full compliance with the existing license.

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
5. Submit a pull request at `Vega Strike: Pull Requests`_.

.. note:: Documentation only changes (RST, MD, etc) do not require step 4.

When submitting a Pull Request please document:
* Whether you've run the pull request validation tests and any issues you ran into doing so
* What you're changing and why
* Note any related `Vega Strike: Issues`_ or `Vega Strike: Milestones`_.

Packaging
=========
|gitter-packagers| `Vega Strike: Gitter - Packagers`_

We directly support packaging on a few platforms. Packaging for addition platforms is more than welcome.
We greatly appreciate the work you do as we would not be able to reach our users without you.

Originally Vega Strike was packaged as a single package for the Vega Strike: Under the Coldest Sea game; however,
we would like to migrate to using two packages:

* Vega Strike Game Engine
* Vega Strike: Under the Coldest Sea

The Vega Strike Game Engine package would provide the core functionality provided by this repository, and should be of minimal size.
The Vega Strike: Under the Coldest Sea package would provide the game assets, icons, desktop integrations, etc for the original game
and depend on the Vega Strike Game Engine package to operate. This change is a work in progress; a release has not yet been set for
when it will take effect.

This model would allow additional games to be developed using the Vega Strike Game Engine with just swapping out the game asset
package and setting a dependency on the Vega Strike Game Engine package.

.. note:: Source Packages would will be very large especially for the data packages. This cannot be avoided. Both source and binary
    packages should be provided.

Please join us at `Vega Strike: Gitter - Packagers`_ to if you have any questions.

.. |gitter-packagers| image:: https://badges.gitter.im/vegastrike/vegastrike-packaging.svg

.. _`Vega Strike: Community Contribution` :: https://wiki.vega-strike.org/HowTo:Contribute
.. _`Vega Strike: Under the Coldest Sea` :: https://github.com/vegastrike/Assets-Masters
.. _`Vega Strike: UtCS Production` :: https://github.com/vegastrike/Assets-Production
.. _`Vega Strike: Licensing` :: https://wiki.vega-strike.org/Development:Graphics_Requirements#Licenses
.. _`Vega Strike: Issues` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/issues
.. _`Vega Strike: Milestones` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/milestones
.. _`Vega Strike: Pull Request Validation` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/wiki/Pull-Request-Validation
.. _`Vega Strike: Pull Requests` :: https://github.com/vegastrike/Vega-Strike-Engine-Source/pulls
.. _`Vega Strike: Gitter - Packagers` :: https://gitter.im/vegastrike/vegastrike-packaging?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge
.. _`Linux Kernel: AI Policy` :: https://docs.kernel.org/process/coding-assistants.html
