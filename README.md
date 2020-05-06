<header>
<p style="font-weight:600; font-size:36px">Vega Strike Space Simulation Engine</p>
</header>

[![Build Status](https://travis-ci.org/vegastrike/Vega-Strike-Engine-Source.svg?branch=master)](https://travis-ci.org/vegastrike/Vega-Strike-Engine-Source)
[![Gitter](https://badges.gitter.im/vegastrike/community.svg)](https://gitter.im/vegastrike/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

```
#====================================
# @file   : README.md
# @brief  : quick repo guide
#====================================
```

**Vega Strike Space Simulation Engine**
=====================================


**How to Run**
=====================================

Either install Vega Strike from the binary installer for your platform, if available, or follow the instructions for compiling from source. (`Compiling Vegastrike`, below.)

- Change settings with command `bin/vssetup`
- Run command `bin/vegastrike` or double-click the executable file in the bin directory.


**Features**
=====================================

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
- BSP Trees that allow one to fly close to a starship, and in the runway of (well formed) models...unlike the victory from WC3.
- the Normal Force which bounces starships off of each other
- Respawn with the ':' key (shift and semicolon)
- Switch Ships with the '\[' key
- Joystick support for a full featured joystick


**Running Vegastrike**
=====================================

Run setup and then vegastrike

```bash
bin/vssetup
bin/vegastrike
```

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

**REQUIRED FILES**

```bash
  /usr/local/bin/vegastrike
      The vegastrike engine.
  /usr/local/bin/vsinstall
      The Setup utility.
  /usr/local/bin/vslauncher
      The vegastrike save game and mission selection utility
  /usr/local/bin/vssetup
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


**Interstellar Warp Transit (Jump Drive)**
=====================================

Most starships come equipped with a warp drive.  Unfortunately they can only be used at large singularities in the space-time continuum.  Your computer signals these points by placing glowing blue balls in those areas.  Their relative size indicates how small a starship must be to fit through the jump point.

To engage a jump drive, position your ship inside and press 'j'.

Regulations state that starships should be stopped before jumping-- disasters have resulted from starships travelling at any great speed into a jump point.


**Time Compression**
=====================================

Often interstellar travel requires going great distances. To facilitate this, we have provided time compression.  Press F9 to increase time compression to get where you're going.  Press F10 to drop out of time compression.  F10 will happen automatically if enemies are nearby.


**Respawn**
=====================================

If you sadly lose your life in combat you may respawn by pressing ':'

A new starship will be created for you by Bob.


**Transfer Ship Command**
=====================================

If you wish to transfer command to another starship, simply press '\[' to switch over.  This is useful if you have died and do not wish to call on Bob for help.


**Controls**
=====================================

- Arrow keys: Flight Sim style turning
- \ full throttle
- backspace zero throttle
- =,+ accel
- \- decel
- ENTER fire missile
- space Fire Guns
- backspace  stop
- TAB Afterburners
- j engage interstellar warp drive (use at clear blue balls)
- *,ins Spin Right
- /,del Spin Left
- ~ (hold down, engage shelton slide)
- y match speed
- t target
- p target nearest to center of screen
- n target enemy targetting you
- T target enemies for your turret
- m change active missile
- g change active gun
- v switch targetting computer mode to navigation mode and then view mode
- w switch left targetting cpu
- Shift-W shift weapons computer mode to damage mode

- F1 toggle cockpit/background display
- F2 left
- F3 right
- F4 back
- F5 Chasecam
- F6 Panning Cam (s,a,w,z pan)
- F7 Target Cam
- F8 Strange Cam
- F9 Increase Time Compression
- F10 Reset Time Compression
- F11 Zoom in
- F12 Zoom out

- a,d,w,z pan ghost cam

- : respawn -- use after death to recreate your starship
- \[ switch ships in battle (use to control allies)


**Compiling Vegastrike**
=====================================

**Compiling On Linux**
-------------------------------------

1. Install the dependencies. Something like this:

```bash
sudo apt-get -y install cmake g++ python-dev libboost-python-dev libgl1-mesa-glx freeglut3-dev \
                libopenal-dev libsdl-gfx1.2-dev libvorbis-dev libjpeg-dev libpng-dev libgtk2.0-dev
```

Or on Debian 10:

```bash
sudo apt-get -y install git cmake python-dev build-essential automake autoconf libpng16-16 \
                libpng-dev libpng-tools libjpeg62-turbo-dev libexpat1-dev libgtk2.0-dev \
                libopenal-dev libogg-dev libvorbis-dev libgl1-mesa-dev libsdl1.2-dev \
                libavcodec-dev libavcodec-extra libavformat-dev libavresample-dev libavutil-dev \
                libavdevice-dev libpostproc-dev freeglut3-dev libxmu-dev libxi-dev \
                libboost-python1.67-dev
```

Or on Ubuntu 20.04:

```bash
sudo apt-get -y install git cmake python-dev build-essential automake autoconf libpng16-16 \
                libpng-dev libpng-tools libjpeg62-dev libexpat1-dev libgtk2.0-dev libopenal-dev \
                libogg-dev libvorbis-dev libgl1-mesa-dev libsdl1.2-dev libavcodec-dev \
                libavcodec-extra libavformat-dev libavresample-dev libavutil-dev libavdevice-dev \
                libpostproc-dev freeglut3-dev libxmu-dev libxi-dev libboost1.67-all-dev
```

2. use the `vsbuild.sh` script in the `sh` directory.

OR

2a. Configure and compile VS manually using the ncurses ccmake frontend:

```bash
mkdir build & cd build
ccmake ../engine
# (configure/edit options to taste in ccmake, press 'c' to save the selected options
# and press 'g' to update the build configuration files used by the make build tool)
make -jN (where N is the number of available CPU threads/cores on your system)
mkdir ../bin && cp vegastrike ../bin/ && cp setup/vssetup ../bin/ && cd ..
```

OR

2b. Configure and compile VS manually, using the command-line cmake frontend:

```bash
mkdir build & cd build
cmake ../engine 
make -jN # (where N is the number of available CPU threads/cores on your system)
mkdir ../bin && cp vegastrike ../bin/ && cp setup/vssetup ../bin/ && cd ..
```
To enable/disable compile-time options with cmake, use `cmake -D<option>`. Example:

```bash
cmake ../engine -DSYSTEM_BOOST=ON -DUSE_PYTHON_3=ON -DCPU_SMP=2 -DCPUINTEL_native=ON -CMAKE_BUILD_TYPE=Debug
```

[Link to list of dependencies in wiki](http://vegastrike.sourceforge.net/wiki/HowTo:Compile_from_CVS)

If there are any problems with this installation method, please create an issue with the Vega Strike development team by [posting a new issue](https://github.com/vegastrike/Vega-Strike-Engine-Source/issues).


**Compiling On Windows**
-------------------------------------

[HowTo:VCPP Compiling](http://vegastrike.sourceforge.net/wiki/HowTo:VCPP_Compiling)


**Compiling On OSX**
-------------------------------------

[HowTo:Compile on OSX](http://vegastrike.sourceforge.net/wiki/HowTo:Compile_on_OSX)



**Modding Vega Strike**
=====================================

**How to make Vegastrike Missions**
-------------------------------------

An example mission(this is stored in the test1.mission file)

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

Currently all of these options are ignored except for the "system".  it loads sol.system  as the star system (which is in XML and stores all present planets)

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

The name will be used later for targetting and offset purposes.  The faction is a faction listed in factions.xml (should be self explanatory 0 is neutral 1 is happy -1 is mad) Currently confed and aera are the two active factions.  AI must be default in this version as no ohter AI scripts are yet written.  nr_ships indicates how many starships will be in this flight squadron.

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
      </flightgroup>
</mission>
```


**Editing AI**
=====================================

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

there are a number of tags you can specify for the aggressive AI: distance, threat, hull, fshield lshield rshield bshield rand

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

If you want to know more about writing actual maneuvers (like turnaway.xml  which as you can see is in the directory) please contact me at hellcatv@hotmail.com

you need to have a heavy background in vector math.


**Hacking Vega Strike**
-------------------------------------

In this guide, any coding is located in square brackets ([]).
Number values may not be accurate.

**Guide 1: Hacking cash**
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

**Guide 2: Getting the ship of your dreams**
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


**Vegastrike Information**
=====================================

Mail comments or suggestions to vegastrike-users@lists.sourceforge.net

And if you can design some missions it would rock!



**Vega Strike Code Repository**
=====================================

[https://github.com/vegastrike/Vega-Strike-Engine-Source](https://github.com/vegastrike/Vega-Strike-Engine-Source)


```EOF```
