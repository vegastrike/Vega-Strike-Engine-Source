Name: vegastrike-data
Summary: Vegastrike - a free 3D space fight simulator
Version: 0.2.1cvs
Release: 1
Copyright: GPL
Group: X11/GL/Games
Source: http://prdownloads.sourceforge.net/vegastrike/vegastrike-0.2.1cvs.tar.gz
Source1: http://prdownloads.sourceforge.net/vegastrike/vegastrike-0.2.1_unixdata.tar.gz
NoSource: 1
Packager: Alexander Rawass <alexannika@users.sourceforge.net>
BuildRoot: /vast2/alex/buildroot
Prefix: /usr/local

%description
Vegastrike is a free 3D space fight simulator under the GPL

Vega Strike is an Interactive Flight Simulator/Real Time Stratagy being
 developed for Linux and Windows in 3d OpenGL... With stunning Graphics,
 Vega Strike will be a hit for all gamers!!!


%prep
rm -rf $RPM_BUILD_ROOT/*

mkdir $RPM_BUILD_ROOT/vegastrike-data-0.2.1cvs

echo PREP END

%setup -n vegastrike-0.2.1cvs

echo SETUP END

%build
echo BUILD

%install
echo INSTALL
mkdir -p $RPM_BUILD_ROOT%{prefix}/games/vegastrike
cd $RPM_BUILD_ROOT%{prefix}/games/vegastrike
tar zxvf $RPM_SOURCE_DIR/vegastrike-0.2.1_unixdata.tar.gz
#chown -R root.root .
rm -Rf data/units/confed/truck_small
rm -Rf data/units/phantom
rm -Rf data/units/tesat

find . -type d|xargs --no-run-if-empty chmod go+rx
find . -type f|xargs --no-run-if-empty chmod go+r-w 

%clean
rm -rf $RPM_BUILD_ROOT/*

%files
%{prefix}/games/vegastrike
