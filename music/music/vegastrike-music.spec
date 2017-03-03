Name: vegastrike-music
Summary: Vegastrike - a free 3D space fight simulator (music files)
Version: 0.4.1
Release: 1
Copyright: GPL
Group: Amusements/Games
Source: vegastrike-music.tar.gz
URL: http://vegastrike.sourceforge.net
Packager: Krister Kjellström aka Starchild <k00_kjr@k.kth.se>
BuildRoot: %{_tmppath}/music
Prefix: /usr/local
Provides: vegastrike-music
Requires: vegastrike-data


%description
Vega Strike Celeste - Trade, Fight and Explore the Universe

Vega Strike is a 3d OpenGL GPL Action RPG space sim for Windows/Linux/MacOSX that allows a player to trade and bounty hunt in the spirit of Elite. You start in a llama cargo ship, with endless possibility before you and just enough cash to scrape together a life. Yet danger lurks in the space beyond.

This archive contains the music files necessary to hear music in VegaStrike.  These files are *not* essential to play the game.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -n music

%build
echo "nothing to build"

%install
echo "Installing"
mkdir -p $RPM_BUILD_ROOT/usr/local/games/vegastrike/data/music
cp -R . $RPM_BUILD_ROOT/usr/local/games/vegastrike/data/music

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/games/vegastrike/data/music


%changelog

* Sat Jan 03 2004 Daniel Aleksandrow <dandandaman@users.sourceforge.net>
- changed data dir to /usr/local/games/vegastrike/data

* Tue Sep 30 2003 Krister Kjellström <k00_kjr@k.kth.se>
- Updated description and paths, etc for 0.4.1
- replaced /tmp with {_tmppath}
- Added comments below

########################################################
#
#         Note:
#
# Should be made with -bb and --target noarch,
# ie: rpmbuild -bb vagastrike-music.spec --target noarch
#
######################################################## 
