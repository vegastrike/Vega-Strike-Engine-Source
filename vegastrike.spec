Name: vegastrike
Summary: Vegastrike - a free 3D space fight simulator
Version: 0.2.9.4
Release: 2
Copyright: GPL
Group: X11/GL/Games
Source: vegastrike.tar.gz
Packager: Jonathan Hunt <jhuntnz@users.sourceforge.net>
BuildRoot: /tmp/vsbuild
Prefix: /usr/local
Icon: vegastrike.xpm
Provides: vegastrike
Requires: vegastrike-data 

%description
Vega Strike is a GPL 3d OpenGL Action RPG space sim for Windows/Linux that allows a player to trade and bounty hunt in the spirit of Elite. You start in an old beat up Wayfarer cargo ship, with endless possibility before you and just enough cash to scrape together a life. Yet danger lurks in the space beyond.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -n vegastrike

%build
./configure --prefix=$RPM_BUILD_ROOT/usr/local 
make

%install
make install
cp vssetup $RPM_BUILD_ROOT/usr/local/bin/
mkdir $RPM_BUILD_ROOT/usr/local/share
mkdir $RPM_BUILD_ROOT/usr/local/share/vegastrike
mkdir $RPM_BUILD_ROOT/usr/local/share/vegastrike/data
mv $RPM_BUILD_ROOT/usr/local/bin/soundserver $RPM_BUILD_ROOT/usr/local/share/vegastrike/data/
%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/vegastrike
/usr/local/bin/vslauncher
/usr/local/bin/vssetup
/usr/local/bin/soundserver
/usr/local/share/vegastrike/data/soundserver
