Name: vegastrike
Summary: Vegastrike - a free 3D space fight simulator (program files)
Version: 0.4.1
Release: gcc3.2_1
Copyright: GPL
Group: Amusements/Games
Source: vegastrike.tar.gz
URL: http://vegastrike.sourceforge.net
Packager: Krister Kjelltröm aka Starchild <k00_kjr@k.kth.se>
BuildRoot: %{_tmppath}/vsbuild
Prefix: /usr/local
Provides: vegastrike

%description
Vega Strike is a GPL 3d OpenGL Action RPG space sim for Windows/Linux/MacOSX that allows a player to trade and bounty hunt in the spirit of Elite. You start in a llama cargo ship, with endless possibility before you and just enough cash to scrape together a life. Yet danger lurks in the space beyond.

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
mkdir -p $RPM_BUILD_ROOT/usr/local/doc/vegastrike/
mkdir -p $RPM_BUILD_ROOT/usr/local/man/man1/
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
mkdir -p $RPM_BUILD_ROOT/usr/local/share/vegastrike
cp vssetup $RPM_BUILD_ROOT/usr/local/bin/
#cp src/networking/soundserver $RPM_BUILD_ROOT/usr/local/bin/
cp src/vegastrike $RPM_BUILD_ROOT/usr/local/bin/
cp README $RPM_BUILD_ROOT/usr/local/doc/vegastrike/
cp doc/vegastrike.1 $RPM_BUILD_ROOT/usr/local/man/man1/
cp doc/vssetup.1 $RPM_BUILD_ROOT/usr/local/man/man1/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%docdir /usr/local/doc/vegastrike
/usr/local/doc/vegastrike/README
%doc /usr/local/man/man1/vegastrike.1
%doc /usr/local/man/man1/vssetup.1
%attr(755, root, root) /usr/local/bin/vegastrike
%attr(755, root, root) /usr/local/bin/vssetup
#%attr(755, root, root) /usr/local/bin/soundserver
%dir /usr/local/share/vegastrike/

%changelog

* Sat Jan 03 2004 Daniel Aleksandrow <dandandaman@users.sourceforge.net>
- removed specified openal dependency in favour of automatic detection

* Mon Sep 29 2003 Krister Kjellström  <K00_kjr@k.kth.se>
- Updated description, etc for 0.4.1
- added attr tags, don't know if there is any point to them:p
- they where fun adding:)
- Also put {_tmppath} in the buildroot.
- Added creation of /usr/local/share/vegastrike/

################################################################
#
#            Note:
#
# Before building, make sure vssetup is in the appropriet place.
#
################################################################
