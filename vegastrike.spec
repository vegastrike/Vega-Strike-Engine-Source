Name: vegastrike
Summary: Vegastrike - a free 3D space fight simulator
Version: 0.0.9cvs
Release: 1
Copyright: GPL
Group: X11/GL/Games
Source: http://vegastrike.sourceforge.net/download/vegastrike-0.0.9cvs.tar.gz
Packager: Alexander Rawass <alexannika@users.sourceforge.net>
BuildRoot: /tmp/vegastrike
Prefix: /usr/local

%description
Vegastrike is a free 3D space fight simulator under the GPL

Vega Strike is an Interactive Flight Simulator/Real Time Stratagy being
 developed for Linux and Windows in 3d OpenGL... With stunning Graphics
 reminiscent of Wing Commander, Vega Strike will be a hit for all gamers!!!


%prep
rm -rf $RPM_BUILD_ROOT
%setup -n vegastrike-0.0.9cvs

%build
./configure --prefix=/usr/local
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%{prefix}/games/vegastrike/bin/vegastrike
%{prefix}/games/vegastrike/data/9x12.alp
%{prefix}/games/vegastrike/data/9x12.bmp
%{prefix}/games/vegastrike/data/9x12.font
%{prefix}/games/vegastrike/data/afterburnerslide.xml
%{prefix}/games/vegastrike/data/afterburn-turntowards-itts.xml
%{prefix}/games/vegastrike/data/afterburn-turntowards.xml
%{prefix}/games/vegastrike/data/aitest.xml
%{prefix}/games/vegastrike/data/armorf.bmp
%{prefix}/games/vegastrike/data/armorf.spr
%{prefix}/games/vegastrike/data/armorl.alp
%{prefix}/games/vegastrike/data/armorl.bmp
%{prefix}/games/vegastrike/data/armorl.spr
%{prefix}/games/vegastrike/data/auto.alp
%{prefix}/games/vegastrike/data/auto.bmp
%{prefix}/games/vegastrike/data/auto.spr
%{prefix}/games/vegastrike/data/beamtexture.bmp
%{prefix}/games/vegastrike/data/blasterpower.alp
%{prefix}/games/vegastrike/data/blasterpower.bmp
%{prefix}/games/vegastrike/data/blasterpower.spr
%{prefix}/games/vegastrike/data/bolt.bmp
%{prefix}/games/vegastrike/data/Capskn3.bmp
%{prefix}/games/vegastrike/data/Capwin~2.bmp
%{prefix}/games/vegastrike/data/crosshairs.alp
%{prefix}/games/vegastrike/data/crosshairs.bmp
%{prefix}/games/vegastrike/data/crosshairs.spr
%{prefix}/games/vegastrike/data/cube_light.bmp
%{prefix}/games/vegastrike/data/cube_sphere.bmp
%{prefix}/games/vegastrike/data/default.agg.xml
%{prefix}/games/vegastrike/data/default.final.xml
%{prefix}/games/vegastrike/data/default.int.xml
%{prefix}/games/vegastrike/data/defaultscared1.xml
%{prefix}/games/vegastrike/data/default.scared.xml
%{prefix}/games/vegastrike/data/default.test.xml
%{prefix}/games/vegastrike/data/disabled-cockpit.cpt
%{prefix}/games/vegastrike/data/Dralthi.bmp
%{prefix}/games/vegastrike/data/dralthi.xmesh
%{prefix}/games/vegastrike/data/dralthi.xunit
%{prefix}/games/vegastrike/data/dralthi.xunit.bsp
%{prefix}/games/vegastrike/data/drydock-Cube_27-Material1.xmesh
%{prefix}/games/vegastrike/data/drydock-Cube_27-Material.xmesh
%{prefix}/games/vegastrike/data/drydock-Cube_27.xunit
%{prefix}/games/vegastrike/data/drydock-Cube_27.xunit.bsp
%{prefix}/games/vegastrike/data/dumbfire.xmesh
%{prefix}/games/vegastrike/data/Dumbfire.xunit
%{prefix}/games/vegastrike/data/Dumbfire.xunit.xai
%{prefix}/games/vegastrike/data/earth.bmp
%{prefix}/games/vegastrike/data/eject.alp
%{prefix}/games/vegastrike/data/eject.bmp
%{prefix}/games/vegastrike/data/eject.spr
%{prefix}/games/vegastrike/data/evade.xml
%{prefix}/games/vegastrike/data/explode_orange0.alp
%{prefix}/games/vegastrike/data/explode_orange0.bmp
%{prefix}/games/vegastrike/data/explode_orange10.alp
%{prefix}/games/vegastrike/data/explode_orange10.bmp
%{prefix}/games/vegastrike/data/explode_orange11.alp
%{prefix}/games/vegastrike/data/explode_orange11.bmp
%{prefix}/games/vegastrike/data/explode_orange12.alp
%{prefix}/games/vegastrike/data/explode_orange12.bmp
%{prefix}/games/vegastrike/data/explode_orange13.alp
%{prefix}/games/vegastrike/data/explode_orange13.bmp
%{prefix}/games/vegastrike/data/explode_orange14.alp
%{prefix}/games/vegastrike/data/explode_orange14.bmp
%{prefix}/games/vegastrike/data/explode_orange15.alp
%{prefix}/games/vegastrike/data/explode_orange15.bmp
%{prefix}/games/vegastrike/data/explode_orange16.alp
%{prefix}/games/vegastrike/data/explode_orange16.bmp
%{prefix}/games/vegastrike/data/explode_orange17.alp
%{prefix}/games/vegastrike/data/explode_orange17.bmp
%{prefix}/games/vegastrike/data/explode_orange18.alp
%{prefix}/games/vegastrike/data/explode_orange18.bmp
%{prefix}/games/vegastrike/data/explode_orange1.alp
%{prefix}/games/vegastrike/data/explode_orange1.bmp
%{prefix}/games/vegastrike/data/explode_orange2.alp
%{prefix}/games/vegastrike/data/explode_orange2.bmp
%{prefix}/games/vegastrike/data/explode_orange3.alp
%{prefix}/games/vegastrike/data/explode_orange3.bmp
%{prefix}/games/vegastrike/data/explode_orange4.alp
%{prefix}/games/vegastrike/data/explode_orange4.bmp
%{prefix}/games/vegastrike/data/explode_orange5.alp
%{prefix}/games/vegastrike/data/explode_orange5.bmp
%{prefix}/games/vegastrike/data/explode_orange6.alp
%{prefix}/games/vegastrike/data/explode_orange6.bmp
%{prefix}/games/vegastrike/data/explode_orange7.alp
%{prefix}/games/vegastrike/data/explode_orange7.bmp
%{prefix}/games/vegastrike/data/explode_orange8.alp
%{prefix}/games/vegastrike/data/explode_orange8.bmp
%{prefix}/games/vegastrike/data/explode_orange9.alp
%{prefix}/games/vegastrike/data/explode_orange9.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange0.alp
%{prefix}/games/vegastrike/data/explode_sml_orange0.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange10.alp
%{prefix}/games/vegastrike/data/explode_sml_orange10.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange11.alp
%{prefix}/games/vegastrike/data/explode_sml_orange11.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange12.alp
%{prefix}/games/vegastrike/data/explode_sml_orange12.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange13.alp
%{prefix}/games/vegastrike/data/explode_sml_orange13.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange14.alp
%{prefix}/games/vegastrike/data/explode_sml_orange14.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange15.alp
%{prefix}/games/vegastrike/data/explode_sml_orange15.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange16.alp
%{prefix}/games/vegastrike/data/explode_sml_orange16.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange17.alp
%{prefix}/games/vegastrike/data/explode_sml_orange17.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange18.alp
%{prefix}/games/vegastrike/data/explode_sml_orange18.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange19.alp
%{prefix}/games/vegastrike/data/explode_sml_orange19.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange1.alp
%{prefix}/games/vegastrike/data/explode_sml_orange1.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange2.alp
%{prefix}/games/vegastrike/data/explode_sml_orange2.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange3.alp
%{prefix}/games/vegastrike/data/explode_sml_orange3.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange4.alp
%{prefix}/games/vegastrike/data/explode_sml_orange4.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange5.alp
%{prefix}/games/vegastrike/data/explode_sml_orange5.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange6.alp
%{prefix}/games/vegastrike/data/explode_sml_orange6.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange7.alp
%{prefix}/games/vegastrike/data/explode_sml_orange7.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange8.alp
%{prefix}/games/vegastrike/data/explode_sml_orange8.bmp
%{prefix}/games/vegastrike/data/explode_sml_orange9.alp
%{prefix}/games/vegastrike/data/explode_sml_orange9.bmp
%{prefix}/games/vegastrike/data/explosion_orange.ani
%{prefix}/games/vegastrike/data/explosion_sml_orange.ani
%{prefix}/games/vegastrike/data/factions.xml
%{prefix}/games/vegastrike/data/fuelstat.alp
%{prefix}/games/vegastrike/data/fuelstat.bmp
%{prefix}/games/vegastrike/data/fuelstat.spr
%{prefix}/games/vegastrike/data/glsetup.txt
%{prefix}/games/vegastrike/data/hellcat.bmp
%{prefix}/games/vegastrike/data/hellcat.xmesh
%{prefix}/games/vegastrike/data/hellcat.xunit
%{prefix}/games/vegastrike/data/hellcat.xunit.bak
%{prefix}/games/vegastrike/data/hellcat.xunit.bsp
%{prefix}/games/vegastrike/data/hornet.bmp
%{prefix}/games/vegastrike/data/hornet-cockpitalp.bmp
%{prefix}/games/vegastrike/data/hornet-cockpit.bmp
%{prefix}/games/vegastrike/data/hornet-cockpit.cpt
%{prefix}/games/vegastrike/data/hornet-cockpit.jpg
%{prefix}/games/vegastrike/data/hornet-cockpit.spr
%{prefix}/games/vegastrike/data/hornet-mesh.xml
%{prefix}/games/vegastrike/data/hornet-top.alp
%{prefix}/games/vegastrike/data/Makefile
%{prefix}/games/vegastrike/data/hornet-top.bmp
%{prefix}/games/vegastrike/data/hornet-top.spr
%{prefix}/games/vegastrike/data/hornet.xunit
%{prefix}/games/vegastrike/data/jalthi.bmp
%{prefix}/games/vegastrike/data/jalthi.xmesh
%{prefix}/games/vegastrike/data/jalthi.xunit
%{prefix}/games/vegastrike/data/jalthi.xunit.bsp
%{prefix}/games/vegastrike/data/jupiter.bmp
%{prefix}/games/vegastrike/data/kickstop.xml
%{prefix}/games/vegastrike/data/locationselect.alp
%{prefix}/games/vegastrike/data/locationselect.ani
%{prefix}/games/vegastrike/data/locationselect.bmp
%{prefix}/games/vegastrike/data/locationselect_up.alp
%{prefix}/games/vegastrike/data/locationselect_up.ani
%{prefix}/games/vegastrike/data/locationselect_up.bmp
%{prefix}/games/vegastrike/data/Makefile.am
%{prefix}/games/vegastrike/data/Makefile.am~
%{prefix}/games/vegastrike/data/Makefile.in
%{prefix}/games/vegastrike/data/mars.bmp
%{prefix}/games/vegastrike/data/massdrivertexture.bmp
%{prefix}/games/vegastrike/data/mercury.bmp
%{prefix}/games/vegastrike/data/moon.bmp
%{prefix}/games/vegastrike/data/mouse.alp
%{prefix}/games/vegastrike/data/mouse.bmp
%{prefix}/games/vegastrike/data/mouse.spr
%{prefix}/games/vegastrike/data/neptune.bmp
%{prefix}/games/vegastrike/data/neutrontexture.bmp
%{prefix}/games/vegastrike/data/particletexture.bmp
%{prefix}/games/vegastrike/data/Planetary-Ring.xmesh
%{prefix}/games/vegastrike/data/Planetary-Ring.xunit
%{prefix}/games/vegastrike/data/Planetary-Ring.xunit.bsp
%{prefix}/games/vegastrike/data/Planetary-Ring.xunit_shield.bsp
%{prefix}/games/vegastrike/data/pluto.bmp
%{prefix}/games/vegastrike/data/radar.alp
%{prefix}/games/vegastrike/data/radar.bmp
%{prefix}/games/vegastrike/data/radar.spr
%{prefix}/games/vegastrike/data/Rapier.bmp
%{prefix}/games/vegastrike/data/rapier.xmesh
%{prefix}/games/vegastrike/data/rapier.xunit
%{prefix}/games/vegastrike/data/rapier.xunit.bsp
%{prefix}/games/vegastrike/data/saturn.bmp
%{prefix}/games/vegastrike/data/saturn-ring.alp
%{prefix}/games/vegastrike/data/saturn-ring.bmp
%{prefix}/games/vegastrike/data/selectbox.bmp
%{prefix}/games/vegastrike/data/selectbox.spr
%{prefix}/games/vegastrike/data/shelton-slide.xml
%{prefix}/games/vegastrike/data/shield.bmp
%{prefix}/games/vegastrike/data/shieldf.alp
%{prefix}/games/vegastrike/data/shieldf.bmp
%{prefix}/games/vegastrike/data/shieldf.spr
%{prefix}/games/vegastrike/data/skilledabslide.xml
%{prefix}/games/vegastrike/data/sun.bmp
%{prefix}/games/vegastrike/data/supernova.bmp
%{prefix}/games/vegastrike/data/TerranPriA128.bmp
%{prefix}/games/vegastrike/data/TerranPriA.bmp
%{prefix}/games/vegastrike/data/TerranPriRGB.bmp
%{prefix}/games/vegastrike/data/TerranSecA.bmp
%{prefix}/games/vegastrike/data/TerranSecRGB.bmp
%{prefix}/games/vegastrike/data/testmission.txt
%{prefix}/games/vegastrike/data/test.xml
%{prefix}/games/vegastrike/data/turnaway.xml
%{prefix}/games/vegastrike/data/turntowardsitts.xml
%{prefix}/games/vegastrike/data/turntowards.xml
%{prefix}/games/vegastrike/data/uranus.bmp
%{prefix}/games/vegastrike/data/venus.bmp
%{prefix}/games/vegastrike/data/weapon_list.xml
%{prefix}/games/vegastrike/objconv/3ds2xml
%{prefix}/games/vegastrike/objconv/obj2xml
%{prefix}/games/vegastrike/objconv/wcp2xml
