Name: vegastrike
Summary: Vegastrike - a free 3D space fight simulator
Version: 0.2.9CVS
Release: 1
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
Vegastrike is a free 3D space fight simulator under the GPL

Vega Strike is an Interactive Flight Simulator/Real Time Stratagy being
 developed for Linux and Windows in 3d OpenGL... With stunning Graphics,
 Vega Strike will be a hit for all gamers!!!


%prep
rm -rf $RPM_BUILD_ROOT

%setup

%build
./configure --prefix=/usr/local 
make

%install
make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/local/bin/vegastrike
/usr/local/bin/vslauncher
