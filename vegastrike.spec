Name: vegastrike
Summary: Vegastrike - a free 3D space fight simulator (program files)
Version: 0.3
Release: gcc3.2
Copyright: GPL
Group: Amusements/Games
Source: vegastrike.tar.gz
URL: http://vegastrike.sourceforge.net
Packager: Daniel Aleksandrow <dandandaman@users.sourceforge.net>
BuildRoot: /tmp/vsbuild
Prefix: /usr/local
Provides: vegastrike

%description
Vega Strike is a GPL 3d OpenGL Action RPG space sim for Windows/Linux that allows a player to trade and bounty hunt in the spirit of Elite. You start in an old beat up Wayfarer cargo ship, with endless possibility before you and just enough cash to scrape together a life. Yet danger lurks in the space beyond.

this archive contains the program files necessary to play Vegastrike.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -n vegastrike

%build
aclocal
autoconf
automake -a --add-missing
./configure
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
cp vssetup $RPM_BUILD_ROOT/usr/local/bin/
cp vslauncher $RPM_BUILD_ROOT/usr/local/bin/
cp src/networking/soundserver $RPM_BUILD_ROOT/usr/local/bin/
cp src/vegastrike $RPM_BUILD_ROOT/usr/local/bin/
mkdir -p $RPM_BUILD_ROOT/usr/local/doc/vegastrike/
cp README $RPM_BUILD_ROOT/usr/local/doc/vegastrike/

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/vegastrike
/usr/local/bin/vslauncher
/usr/local/bin/vssetup
/usr/local/bin/soundserver
/usr/local/games/vegastrike/data/soundserver
/usr/local/doc/vegastrike/README
