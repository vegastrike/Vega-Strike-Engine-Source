Name: vegastrike
Summary: Vegastrike - a free 3D space fight simulator
Version: 0.2.0
Release: 1
Copyright: GPL
Group: X11/GL/Games
Source: http://prdownloads.sourceforge.net/vegastrike/vegastrike-0.1.0.tar.gz
Packager: Alexander Rawass <alexannika@users.sourceforge.net>
BuildRoot: /tmp/vegastrike
Prefix: /usr/local

%description
Vegastrike is a free 3D space fight simulator under the GPL

Vega Strike is an Interactive Flight Simulator/Real Time Stratagy being
 developed for Linux and Windows in 3d OpenGL... With stunning Graphics,
 Vega Strike will be a hit for all gamers!!!


%prep
rm -rf $RPM_BUILD_ROOT
%setup -n vegastrike-0.1.0

%build
./configure --prefix=/usr/local
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%{prefix}/vegastrike
%{prefix}/select
%{prefix}/objconv/3ds2xml
%{prefix}/objconv/obj2xml
%{prefix}/objconv/wcp2xml
