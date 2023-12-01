# norootforbuild

Name:           vegastrike
Version:        0.8.2
Release:        1
Summary:        3D OpenGL spaceflight simulator
Group:          Amusements/Games
License:        GPLv2+
URL:            http://vegastrike.sourceforge.net/
Source0:        http://master.dl.sourceforge.net/project/%{name}/%{name}/0.5.13/%{name}-src-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build
BuildRequires:  freeglut-devel gtk2-devel python-devel
BuildRequires:  libjpeg-devel libpng-devel boost-devel 
BuildRequires:  SDL_mixer-devel openal-devel libvorbis-devel
BuildRequires:  gcc-c++
BuildRequires:  freealut-devel
%if 0%{?suse_version}
BuildRequires:  xorg-x11-libXmu-devel
BuildRequires:  libexpat-devel
%endif
%if 0%{?fedora} > 0
BuildRequires:  expat-devel
%endif
%if 0%{?mdkversion} >= 2006
%ifarch x86_64
BuildRequires:	lib64mesagl1-devel lib64x11_6-devel lib64expat1-devel
%else
BuildRequires:	libmesaglu1-devel libmesagl1-devel libx11_6-devel libexpat1-devel
%endif
%else
BuildRequires:	xorg-x11-devel Mesa-devel
%endif
Requires:       xdg-utils opengl-games-utils
%if 0%{?suse_version}
Recommends:     %{name}-data >= %{version}
%endif
Autoreqprov:	on

%description
Vega Strike is a GPL 3D OpenGL Action RPG space sim that allows a player to
trade and bounty hunt. You start in an old beat up Wayfarer cargo ship, with
endless possibility before you and just enough cash to scrape together a life.
Yet danger lurks in the space beyond.


%prep
%setup -q -a 1 -n %{name}-src-%{version}
iconv -f ISO-8859-1 -t UTF-8 README > README.tmp
touch -r README README.tmp
mv README.tmp README
sed -i 's/-lboost_python-st/-lboost_python/g' Makefile.in
# we want to use the system version of expat.h
rm objconv/mesher/expat.h


%build
%configure --with-data-dir=%{_datadir}/%{name} --with-boost=system \
  --enable-release --enable-flags="$RPM_OPT_FLAGS -DBOOST_PYTHON_NO_PY_SIGNATURES" \
  --disable-ffmpeg --disable-ogre --enable-stencil-buffer
make %{?_smp_mflags}


%install
#make install PREFIX=$RPM_BUILD_ROOT doesn't work
%makeinstall

mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/%{name}
chmod +x $RPM_BUILD_ROOT%{_prefix}/objconv/*
mv $RPM_BUILD_ROOT%{_prefix}/objconv/* \
  $RPM_BUILD_ROOT%{_libexecdir}/%{name}
for i in asteroidgen base_maker mesh_xml mesher replace tempgen trisort \
         vsrextract vsrmake; do
  mv $RPM_BUILD_ROOT%{_bindir}/$i $RPM_BUILD_ROOT%{_libexecdir}/%{name}
done


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING DOCUMENTATION README.md
%{_bindir}/vega*
%{_bindir}/vs*
%{_libexecdir}/%{name}


%changelog

-------------------------------------------------------------------
2020-11 27 15:13:00 UTC - benjamenmeyer

- Updated version to 0.8.0 for master development branch

-------------------------------------------------------------------
2020-04-19 19:14:00 UTC - benjamenmeyer

- Updated version to 0.7.0 for master development branch

-------------------------------------------------------------------
2020-02-11 14:23:00 +0000 - pyramid

- Upgraded to upstream 0.5.3 source release

-------------------------------------------------------------------
Mon Jan 30 05:06:13 UTC 2012 - klaussfreire@gmail.com

- Upgraded to upstream 0.5.1.beta2 source release
  * Intel MESA DRI fixes
  * Fixed beam weapons
  * Fixed repair bot levels
  * Cargo mules - allow cargo inside fleet vessels
  * Offline Vertex de-duplication by mesh_tool
  * Fixed "-d" option, supporting alternate data dirs
  * Set correct unit for cargo on vdu manifest. 
    Should be m^3 for cubic meters, not m^2.
  * Some smaller fixes and enhancements and code cleanup
  * Patches #3211661, #3250877, #3353864, #3353868, #3376191, 
    #3376207, #3373653, #3373650, #3372844, #3373595, #3373583, 
    #3438260, #3372847, #3372846, #3372837, #3372839, #3372494,
    #3372591, #3372832, #2121764 applied
  * Issues #3211697, #3315526, #3413675, #3413685, #3434184,
    #3440480, #3440493, #3441556, #3311033, #3462191, #3325181,
    #3191110, #3473367, #3325177 fixed

-------------------------------------------------------------------
Tue Mar  8 17:27:55 UTC 2011 - klaussfreire@users.sourceforge.net

- Upgraded to upstream 0.5.1.beta1-2 source release (by patch)

-------------------------------------------------------------------
Fri Mar  4 01:17:52 UTC 2011 - klaussfreire@users.sourceforge.net

- Fidex module search path ordering so that mod modules take precedence over system modules

-------------------------------------------------------------------
Sat Feb 26 18:37:48 UTC 2011 - klaussfreire@users.sourceforge.net

- Merged upstream changes from SVN rev 13079-13087
- Reduced message clutter in the console for release builds.
- feature fix for tracker id #2143066 (upgarde item named "basic repair & refuel).
- font fix for stroke font size on base computer screen.

-------------------------------------------------------------------
Fri Feb 25 04:40:33 UTC 2011 - klaussfreire@users.sourceforge.net

- Upgraded to upstream 0.5.1.beta1 source release

-------------------------------------------------------------------
Sun Nov 23 15:40:40 CET 2008 - claes.backstrom@fsfe.org

- New version of vegastrike-0.4.2-vssetup-fix.patch

-------------------------------------------------------------------
Wed May 21 21:49:18 CEST 2008 - claes.backstrom@fsfe.org

- Initial package 

