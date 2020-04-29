Name: vegastrike-data
Summary: Vegastrike - a free 3D space fight simulator (data files)
Version: 0.5.0
Release: 1
Copyright: GPL
Group: Amusements/Games
Source: vegastrike-data.tar.gz
URL: http://vegastrike.sourceforge.net
Packager: Krister Kjelltröm aka Starchild <k00_kjr@k.kth.se>
BuildRoot: %{_tmppath}/data
Prefix: /usr/local
Provides: vegastrike-data
Requires: vegastrike >= 0.5.0




%description
Vega Strike - Upon the Coldest Sea

Vega Strike is a 3d OpenGL GPL Action RPG space sim for Windows/Linux/MacOSX that allows a player to trade and bounty hunt in the spirit of Elite. You start in a llama cargo ship, with endless possibility before you and just enough cash to scrape together a life. Yet danger lurks in the space beyond.

This archive contains the data files essential to play the game.
 
%prep
rm -rf $RPM_BUILD_ROOT

%setup -n data

%build
echo "nothing to build"

%install
echo "Installing"
mkdir -p $RPM_BUILD_ROOT/usr/local/games/vegastrike/data
mkdir -p $RPM_BUILD_ROOT/usr/local/bin/
mkdir -p $RPM_BUILD_ROOT/usr/local/man/man1/
cp vslauncher $RPM_BUILD_ROOT/usr/local/bin/
cp vsinstall $RPM_BUILD_ROOT/usr/local/bin/
cp documentation/vsinstall.1 $RPM_BUILD_ROOT/usr/local/man/man1/
cp documentation/vslauncher.1 $RPM_BUILD_ROOT/usr/local/man/man1/
cp -R . $RPM_BUILD_ROOT/usr/local/games/vegastrike/data

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc /usr/local/man/man1/vslauncher.1
%doc /usr/local/man/man1/vsinstall.1
# Normal files
/usr/local/games/vegastrike/data
%attr(755, root, root) /usr/local/bin/vslauncher
%attr(755, root, root) /usr/local/bin/vsinstall

%changelog

* Sat Jan 03 2004 Daniel Aleksandrow <dandandaman@users.sourceforge.net>
- changed data dir to /usr/local/games/vegastrike/data

* Tue Sep 30 2003 Krister Kjellström <k00_kjr@k.kth.se>
- Updated the description and paths, etc for 0.4.1
- Replaced /tmp with {_tmppath}
- Added attr() in front of the binaries in files section,
- don't know if they do any good:)
- Added comments below
- Added echo message after install phase: 'This pakage...
 
################################################################
#
#            Note:
#
# Before building, make sure vsinstall and vslauncher
# is in the appropriet place.
# Also make sure there is no music subdirectory present, unless,
# of course, you intend to include it:)
#
# Should be made with -bb and --target noarch, ie:
# rpmbuild -bb vegastrike-data.spec --target noarch
#
################################################################
