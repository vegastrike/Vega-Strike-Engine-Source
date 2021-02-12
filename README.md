<header>
<p style="font-weight:600; font-size:36px">Vega Strike Space Simulation Engine</p>
</header>

![CI - GitHub Actions - CodeQL](https://github.com/vegastrike/Vega-Strike-Engine-Source/workflows/CodeQL/badge.svg)
[![Gitter](https://badges.gitter.im/vegastrike/community.svg)](https://gitter.im/vegastrike/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Vega Strike](https://img.shields.io/badge/-Website-informational)](https://www.vega-strike.org)

```
#====================================
# @file   : README.md
# @brief  : quick repo guide
#====================================
```

Vega Strike Space Simulation Engine
===================================

Vega Strike is a Space Flight Simulator that allows a player to explore, trade, and fight in the vast open space. You start in an old beat up cargo ship, with endless possibilities in front of you and just enough cash to scrape together a life. Yet danger lurks in the space beyond.


Features
========

- All of the art assets are original.
- Never before seen ships modelled by the Vegastrike team!
- Astounding soundtrack!
- Multiple Systems
- Entire Sol sector and Vega sector included!
- Fly through huge multilayered asteroid fields
- Explore nebulae where your radar fritzes out
- Jump to other systems at blue jump points with the 'j' key
- Time compression
- Sound effects that make starships roar as they pass and zap as they die
- Bolt weapons with glowing lighting effects
- Cloaking that renders a ship partially/fully invisible
- Customizable fully working HUD with targetting displays, etc
- Customizable Alien Race logos that will change on all starships by editing TerranPriA.bmp and TechPriA.bmp
- Intelligent, Programmable AI
- Encounter epic battles happening in many systems
- The universe is fully dynamic and progresses on its own and independently of the player advancement
- Varied weapons on different starships
- Explosions that literally tear a starship into pieces
- Shields That Glow upon hit (in the location hit)
- Customizable missions
- Stars that fly past as you explore the system
- RAPID collision system that allows no problems with collisions
- BSP Trees that allow one to fly close to a starship, and in the runway of (well formed) models... unlike the victory from WC3.
- the Normal Force which bounces starships off of each other
- Respawn with the ':' key (shift and semicolon)
- Switch Ships with the '\[' key
- Joystick support for a full featured joystick


How to Run
==========

Either install Vega Strike from the binary installer for your platform, if available, or follow the instructions for compiling from source. (`Compiling Vegastrike`, below.)

- First change settings with `bin/vegasettings`
- Then run `bin/vegastrike` or double-click on the executable file in the bin directory.

Vegastrike command line parameters allow for different start settings and are as follows

```man
  vegastrike [-ddata_dir] [-px,y,z] [-jsector/starsystem] missionname

OPTIONS
  -d/my/data/dir
    specifies /my/data/dir as the path for finding the vega strike data.  De-
    fault is /usr/local/share/vegastrike/data

  -p1024,2405,1245090101
    Forces  the  player's  starting  location  to  be  at  x=  1024, y= 2405,
    z=1245090101

  -jgemini_sector/troy
    Forces the player to start in the troy system in gemini sector.

  missionname
    Specifies  a  mission  for  vegastrike  to  run.   Default  is   /usr/lo-
    cal/share/vegastrike/data/mission/exploration/explore_universe.mission.
```

Vegastrike takes a single command parameter indicating which mission it should load

```bash
bin/vegastrike mission/explore_universe.mission
```

is an example of a valid mission call

the -l flag (must be flushed with the system) will force a player to begin in a star system.

```bash
bin/vegastrike -lvega_sector/vega mission/bomber.mission
```

will force the bomber mission to run in the vega sector

If you encounter any issues while playing, please create an issue with the Vega Strike development team by [posting a new issue](https://github.com/vegastrike/Vega-Strike-Engine-Source/issues).

REQUIRED FILES
--------------

```bash
  /usr/local/bin/vegastrike
      The vegastrike engine.
  /usr/local/bin/vsinstall
      The Setup utility.
  /usr/local/bin/vslauncher
      The vegastrike save game and mission selection utility
  /usr/local/bin/vegasettings
      Internal installer program
  /usr/local/share/vegastrike
      The vegastrike data files
  /usr/local/lib/man/man1
      Directory containg the manual files
  ~/.vegastrike
      Directory containing user specific data managed by vegastrike.
  ~/.vegastrike/vegastrike.config
      User-specific configuration file
```

Compiling Vegastrike
====================

Compiling On Linux
------------------

1. Install the development dependencies:

   a. Run `sudo script/bootstrap`

   b. *OR* install the dependencies manually. Something like this:

   ```bash
   sudo apt-get -y install cmake g++ python-dev libboost-python-dev libboost-log-dev \
                   libboost-regex-dev libgl1-mesa-glx freeglut3-dev libopenal-dev \
                   libsdl-gfx1.2-dev libvorbis-dev libjpeg-dev libpng-dev libgtk-3-dev
   ```

   On Debian 10 "buster":

   ```bash
   sudo apt-get -y install git cmake python-dev build-essential automake autoconf libpng16-16 \
                   libpng-dev libpng-tools libjpeg62-turbo-dev libexpat1-dev libgtk-3-dev \
                   libopenal-dev libogg-dev libvorbis-dev libgl1-mesa-dev libsdl1.2-dev \
                   libpostproc-dev freeglut3-dev libboost-python1.67-dev libboost-log1.67-dev \
                   libboost-regex1.67-dev
   ```

   On Ubuntu 20.04 LTS "focal":

   ```bash
   sudo apt-get -y install git cmake python-dev build-essential automake autoconf libpng16-16 \
                   libpng-dev libpng-tools libjpeg62-dev libexpat1-dev libgtk-3-dev libopenal-dev \
                   libogg-dev libvorbis-dev libgl1-mesa-dev libsdl1.2-dev libopengl0 \
                   libpostproc-dev freeglut3-dev libboost-python1.67-dev libboost-log1.67-dev \
                   libboost-regex1.67-dev
   ```

   On openSUSE Leap 15.2:

   ```bash
   sudo zypper install libboost_log1_66_0-devel \
                       libboost_python-py2_7-1_66_0-devel \
                       libboost_python-py3-1_66_0-devel \
                       libboost_system1_66_0-devel \
                       libboost_filesystem1_66_0-devel \
                       libboost_thread1_66_0-devel \
                       libboost_regex1_66_0-devel \
                       libboost_chrono1_66_0-devel \
                       libboost_atomic1_66_0-devel \
                       cmake \
                       gcc-c++ \
                       freeglut-devel \
                       libopenal0 \
                       openal-soft-devel \
                       libSDL-1_2-0 \
                       libSDL-devel \
                       libvorbis-devel \
                       libjpeg-turbo \
                       libjpeg62-devel \
                       libpng16-devel \
                       expat \
                       libexpat-devel \
                       libgtk-2_0-0 \
                       gtk2-devel \
                       libgtk-3-0 \
                       gtk3-devel \
                       python-devel \
                       python3-devel \
                       git \
                       rpm-build
   ```

   On Fedora 30/31/32/33:

   ```bash
   sudo dnf install    git \
                       cmake \
                       boost-devel \
                       boost-python3-devel \
                       freeglut-devel \
                       gcc-c++ \
                       openal-soft-devel \
                       SDL-devel \
                       libvorbis-devel \
                       libjpeg-turbo-devel \
                       libpng-devel \
                       expat-devel \
                       gtk3-devel \
                       python2-devel \
                       python3-devel \
                       rpm-build \
                       make
   ```

   On Fedora 30 or 31, also install `boost-python2-devel`. (Apparently not available
   on Fedora 32 or 33.)

2. Build Vega Strike:

   a. Use the `build.sh` script in the `script` directory.

   b. *OR* configure and compile VS manually, using the ncurses ccmake frontend:

   ```bash
   mkdir build & cd build
   ccmake ../engine
   # (configure/edit options to taste in ccmake, press 'c' to save the selected options
   # and press 'g' to update the build configuration files used by the make build tool)
   make -j $(nproc) # (where $(nproc) returns the number of available CPU threads/cores on the system)
   mkdir ../bin && cp vegastrike ../bin/ && cp setup/vegasettings ../bin/ && cd ..
   ```

   c. *OR* configure and compile VS manually, using the command-line cmake frontend:

   ```bash
   mkdir build & cd build
   cmake ../engine
   make -j $(nproc) # (where $(nproc) returns the number of available CPU threads/cores on the system)
   mkdir ../bin && cp vegastrike ../bin/ && cp setup/vegasettings ../bin/ && cd ..
   ```

   __TIPS__:

   To enable verbose output for debugging purposes (will show compilation commands), pass the `VERBOSE=1` argument:

   ```bash
   make VERBOSE=1
   ```

   To enable/disable compile-time options with cmake, use `cmake -D<option>=<value>`. Example:

   ```bash
   cmake ../engine -DENABLE_PIE=ON -DUSE_PYTHON_3=ON -DCPU_SMP=2 -DCPUINTEL_native=ON -CMAKE_BUILD_TYPE=Debug
   ```

   __NOTE__:

   On some Ubuntu versions and derivatives, a bug exists whereby enabling
   PIE compilation (Position Independent Executables) results in the
   `file` utility incorrectly recognising the compiled vegastrike binary
   as a shared library instead of a position independent shared executable
   object.

   The effect of the bug is that vegastrike can still be started from the
   command line but that it will not be recognised as an executable by GUI
   file managers such as Nautilus and Dolphin.

   To avoid this scenario, turn off this flag by default and let packagers
   on other distributions turn this on if their OS is able to correctly deal
   with Position Independent Executables.

   For more info, see:

   - https://bugs.launchpad.net/ubuntu/+source/file/+bug/1747711
   - https://github.com/vegastrike/Vega-Strike-Engine-Source/issues/94

3. Download a copy of the assets/game data from [here](https://github.com/vegastrike/Assets-Production). You can either `git clone` this repository, or download it as a ZIP file and unzip it.

4. When you run vegasettings, specify the path to the assets/game data on the command line with `--target` followed by a space. E.g.:

   ```bash
   ./bin/vegasettings --target ../Assets-Production
   ```

   Do the same with vegastrike using `-d` and no space. E.g.:

   ```bash
   ./bin/vegastrike -d../Assets-Production
   ```

[Link to list of dependencies in wiki](http://vegastrike.sourceforge.net/wiki/HowTo:Compile_from_CVS)

If there are any problems with this installation method,
please create an issue with the Vega Strike development team
by [posting a new issue](https://github.com/vegastrike/Vega-Strike-Engine-Source/issues).

If you get compilation issues with the system `libboost`, download it manually from
[here](https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz) to `./ext/boost/`
and run `./script/build.sh -DUSE_SYSTEM_BOOST=NO`


Compiling On Windows
--------------------

Currently VegaStrike is not compiling on Windows. Any help will be appreciated to get it fix. For more information go [here](https://github.com/vegastrike/Vega-Strike-Engine-Source/issues/53)


Compiling On MacOS
------------------

Currently VegaStrike is not compiling on MacOS. Any help will be appreciated to get it fix. For more information go [here](https://github.com/vegastrike/Vega-Strike-Engine-Source/issues/78)

3. Packaging Vega Strike:

	After building Vega Strike, then packages can be built using:

```bash
make package
```

Gameplay
========

Interstellar Warp Transit (Jump Drive)
--------------------------------------

Most starships come equipped with a warp drive.  Unfortunately they can only be used at large singularities in the space-time continuum.  Your computer signals these points by placing glowing blue balls in those areas.  Their relative size indicates how small a starship must be to fit through the jump point.

To engage a jump drive, position your ship inside and press 'j'.

Regulations state that starships should be stopped before jumping-- disasters have resulted from starships travelling at any great speed into a jump point.

Intrastellar SPEC Drive
-----------------------

To travel inside star system, the ships are equiped with a SPEC drive that allow faster-than-light travel. This allows efficient travel between planets and stations inside the same star system. To toggle it press 'Shift-A'. To activate auto-pilot, that will handle this automatically, press 'A'.

Respawn
-------

If you sadly lose your life in combat you may respawn by pressing ':'

A new starship will be created for you by Bob.


Transfer Ship Command
---------------------

If you wish to transfer command to another starship, simply press '\[' to switch over.  This is useful if you have died and do not wish to call on Bob for help.


Controls
--------

- `Arrow keys` - Flight Sim style turning
- `\` - Full throttle
- `Backspace` - Zero throttle
- `=,+` - Accelerate
- `\,-` - Decelerate
- `Enter`- Fire missile
- `Space` - Fire Guns
- `Tab` - Afterburners
- `J` - Engage interstellar warp drive (use at clear blue balls)
- `*, Insert` - Spin Right
- `/, Delete` - Spin Left
- `~` (hold down) - Engage shelton slide
- `Y` - Match speed
- `T` - Target
- `P` - Target nearest to center of screen
- `N` - Target enemy targetting you
- `Shift-T` - Target enemies for your turret
- `M` - Change active missile
- `G` - Change active gun
- `V` - Switch targetting computer mode to navigation mode and then view mode
- `W` - Switch left targetting cpu
- `Shift-W` - Shift weapons computer mode to damage mode
- `:` - Respawn (Use after death to recreate your starship)
- `\[` - switch ships in battle (use to control allies)
- `F1` - toggle cockpit/background display
- `F2` - Left camera
- `F3` - Right camera
- `F4` - Back camera
- `F5` - Chase camera
- `F6` - Panning camera (s,a,w,z pan)
- `F7` - Target camera
- `F8` - Strange camera
- `a,d,w,z` - Pan ghost camera
- `F9` - Lower music volume
- `F10` - Raise music volume
- `F11` - Lower game volume
- `F12` - Raise game volume


Modding Vega Strike
===================

How to make Vegastrike Missions
-------------------------------

An example mission (this is stored in the test1.mission file)

A mission must begin with the headers:

```xml
<mission>
    <variables>
        <var name="defaultplayer" value="blue"/>
        <var name="mission_name" value="4 versus 4" />
        <var name="splashscreen" value="bad_guys_vs_good_guys.bmp" />
        <var name="system" value="sol" />
        <var name="description" value="4vs4.txt" />
    </variables>
```

Currently all of these options are ignored except for the "system".  It loads sol.system as the star system (which is in XML and stores all present planets)

The only other system included in this beta release is the blank.mission

which has 1 planet, 1 sun and 2 starbases.

After this, comes the actors in the mission, the flightgroups of fighters.

```xml
        <flightgroups>
```

You must begin the flight group tag as above, and terminate it after all of your flight groups

```xml
                <flightgroup name="blue" faction="confed" type="nova" ainame="default" waves="8" nr_ships="3">
```

The name will be used later for targetting and offset purposes.  The faction is a faction listed in factions.xml (should be self explanatory 0 is neutral 1 is happy -1 is mad) Currently confed and aera are the two active factions.  AI must be default in this version as no other AI scripts are yet written.  nr_ships indicates how many starships will be in this flight squadron.

```xml
            <pos x="10000.0" y="0.0" z="3000.0"/>
```

This indicates the flight group's position... be sure it is unique

```xml
            <rot x="180.0" y="180.0" z="180.0"/>
            <order order="tmptarget" target="omikron"/>
            <order priority="0" order="superiority" target="enemy"/>
            <order priority="1" order="bomber" target="omikron"/>
            <order priority="2" order="escort" target="blue"/>
```

the rest is in development

```xml
        </flightgroup>
```

you must end all flight group tags

continue with any other flightgroups... you can have as many as you want from as many named factions are you want...

```xml
      </flightgroups>
</mission>
```


Editing AI
==========

The AI is completely scriptable, and I have not spent all that long perfecting it.  There are included instructions about editing the AI scripts yourself.

Currently there is only 1 AI personality.  In the future there will be a method to assign different personalities to different starships.

it's the "default" personality.

2 files are responsible for the control of the "default" personality

default.agg.xml

and

default.int.xml

default.agg.xml means the "aggressive" AI.  it controls what it does to aggressively bring its vengeance upon its target.

When editing this file in notepad, you'll notice some tags and various numbers.

The first tag, AggressiveAI has a parameter time="4"

that's how often the AI checks if it should change its strategy.  This time can be any integer value... it can revise its plan more often or less often. 4 has been good because most maneuvers can mostly complete in 4 seconds and it's a good time to revise plans

underneath the beginning tag exist a list of tags that describes the logic the AI uses to figure out its next strategy.

there are a number of tags you can specify for the aggressive AI: distance, threat, hull, fshield, lshield, rshield, bshield, rand.

Each tag asserts if one of the tag-values above is between min and max.

The AI needs to make a CHOICE about what it does next, so it takes a look at the list of tags and determines if any of the assertions is true.

Nested statements mean that BOTH must be true. so you can say "if the distance is at most .5 and the hull is between .25 and .75 by writing:

```xml
<distance max=".5">
  <hull min=".25" max=".75" script="afterburnerslide.xml">
  </hull>
</distance>
```

this means "if the distance is at most half the range of my guns and the hull is between 1/4 and 3/4 of its capacity, then perform an afterburner slide.

It goes down the list of such assertions and chooses the appropriate AI script to run.

The facing and movement tags fill in the gaps where NONE of the list of the assertions are true and the ship is not either turning or moving anywhere:

```xml
<facing script="turntowards.xml">
</facing>
```

at the very bottom the `</AggressiveAI>` must show up to indicate that the AIscript has terminated.

The aggressive.int.xml is written out exactly the same as the aggressive.agg.xml

the difference is that scripts listed here INTERRUPT the current action!

An example from the game is:

```xml
<AggressiveAI close=".05">
  <hull max = ".5" script="evade.xml">
   <threat min=".4" script="afterburnerslide.xml">
   </threat>
  </hull>

  <distance max=".03" script="turnaway.xml">
  <!-- distance less than .1-->
  </distance>

</AggressiveAI>
```

this says:

If the hull is at most half and someone is threatening me with a 40% chance to hit... then evade and then afterburner slide

OR

if someone is at most .03 of my max range away from me TURN AWAY!

this will interrupt the current progress of any scripts

So that's how to use AI scripts.

If you want to know more about writing actual maneuvers (like turnaway.xml which as you can see is in the directory) please contact me at hellcatv@hotmail.com

you need to have a heavy background in vector math.


Hacking Vega Strike
-------------------

In this guide, any coding is located in square brackets ([]).
Number values may not be accurate.

### Guide 1: Hacking cash

Step 1
Locate you saved files. (Windows XP: Program files/VegaStrike/Vegastike-0.5.0/.vegastrike-0.5.0/save||||Mac: (disk)>Users>(user)>.vegastrike-0.5.0>saves>(savefile))
Step 2
Open the files using a word document editing program (preferably Notepad++)
Step 3
On the first line, you should see something roughly resembling this: [Crucible/Cephid_17^200000.000000^Llama.begin 119990000070.992740 -8999928.351833 -109989999927.749450]
On this line, find the numbers surrounded by carets “^”. This is your cash. Change it to what you want, preferably in the high millions. Or possibly even trillions. Go nuts.
Step 4
In order for the game not to go mad about this, you need to add this ending:[.000000]
Basically, if you have, say a quadrillion cash (1000000000000000) you still need to add .000000 on the end, making it stupidly long. Yes, we know, its annoying.
Step 5
Now you need to check if you are using commas or “‘” in your cash. Don’t.
Then save the file. .txt files work, however when you see them on the loading screen they end in .txt.
Step 6
You’re now richer than a very rich man on International very rich day! Yay!

### Guide 2: Getting the ship of your dreams

Step 1
Get your saved file from Guide 1.
Step 2
Now, find the cash (numbers in carets “^”). There is a name after this. At the start of the game, it is always “Llama.begin”
Step 3
Now you can change this name into any ship you like. However, it must be in the same format.
Step 4
Here are some good ships:
Goddard.milspec
Clydesdale.stock
Hyena.stock (Light, weak fighter. Good for getting used to combat with other fighters.)
Mule.stock (trader)
Step 5
There is a ship list either in the game files or on the internet, check that out for some good models. (remember: add .stock or .milspec on the end!!!! (milspec is only on some specialised ships)) Finally, save the file as before.

This guide was created by Munno 2010-10-08


Vega Strike Information
=======================

Submit comments or suggestions by opening an [issue](https://github.com/vegastrike/Vega-Strike-Engine-Source/issues)

And if you can design some missions it would rock!

Vega Strike Contacts
====================

Vega Strike is the product of many contributors from all around the world. If you need help, find a bug, want to request a feature, etc then please contact us all using one
of the following methods:

- [Gitter.im: Community](https://gitter.im/vegastrike/community)
- [Gitter.im: Packaging](https://gitter.im/vegastrike/vegastrike-packaging)
- [Gitter.im: Infrastructure](https://gitter.im/vegastrike/infrastructure-admins)
- [Mailing Lists](https://lists.vega-strike.org)
- [Vega Strike Forums](https://forums.vega-strike.org)

Bugs can be sent to one of the following:
  - Security related issues can be sent to [security@lists.vega-strike.org](mailto:security@lists.vega-strike.org).
  - General issues can be sent to [devel@lists.vega-strike.org](mailto:devel@lists.vega-strike.org) or filed as an [issue](https://github.com/vegastrike/Vega-Strike-Engine-Source/issues).

Vega Strike on Social Media
===========================
- [Facebook](https://www.facebook.com/VegaStrike-188522714499479/)
- [Twitter](https://twitter.com/vega_strike)
- [YouTube](https://www.youtube.com/channel/UC5p9ObADzS3sx9orZG7M91g/)
- [MeWe](http://www.mewe.com/join/vegastrike)
- [Diaspora: Pluspora](https://pluspora.com/tags/vegastrike)


Vega Strike Code Repository
===========================

[https://github.com/vegastrike/Vega-Strike-Engine-Source](https://github.com/vegastrike/Vega-Strike-Engine-Source)

Contributors
------------

This project exists thanks to all the people who contribute. [[Contribute](CONTRIBUTING.md)].
<a href="https://github.com/vegastrike/Vega-Strike-Engine-Source/graphs/contributors"><img src="https://opencollective.com/vega-strike/contributors.svg?width=890" /></a>

Backers
-------

Thank you to all our backers! 🙏 [[Become a backer](https://opencollective.com/vega-strike#backer)]

<a href="https://opencollective.com/vega-strike#backers" target="_blank"><img src="https://opencollective.com/vega-strike/backers.svg?width=890"></a>

Sponsors
--------

Support this project by becoming a sponsor. Your logo will show up here with a link to your website. [[Become a sponsor](https://opencollective.com/vega-strike#sponsor)]

<a href="https://opencollective.com/vega-strike/sponsor/0/website" target="_blank"><img src="https://opencollective.com/vega-strike/sponsor/0/avatar.svg"></a>

```EOF```
