##
# ucsv.py
#
# Copyright (c) 2001-2002 Daniel Horn
# Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#

from __future__ import print_function
import sys
import xml
import xml.dom
import xml.dom.minidom
import math

internalkeys=["Key","Directory","Name","FoFID","Object_Type","Combat_Role","Textual_Description","Hud_image","Unit_Scale","Cockpit","CockpitX","CockpitY","CockpitZ","Mesh","Shield_Mesh","Rapid_Mesh","BSP_Mesh","Use_BSP","Use_Rapid","Mass","Moment_Of_Inertia","Convex_Hull_Volume","Fuel_Capacity","Hull","Armor_Front_Top_Right","Armor_Front_Top_Left","Armor_Front_Bottom_Right","Armor_Front_Bottom_Left","Armor_Back_Top_Right","Armor_Back_Top_Left","Armor_Back_Bottom_Right","Armor_Back_Bottom_Left","Shield_Front_Top_Right","Shield_Back_Top_Left","Shield_Front_Bottom_Right","Shield_Front_Bottom_Left","Shield_Back_Top_Right","Shield_Front_Top_Left","Shield_Back_Bottom_Right","Shield_Back_Bottom_Left","Shield_Recharge","Shield_Leak","Warp_Capacitor","Primary_Capacitor","Reactor_Recharge","Jump_Drive_Present","Jump_Drive_Delay","Wormhole","Outsystem_Jump_Cost","Warp_Usage_Cost","Afterburner_Type","Afterburner_Usage_Cost","Maneuver_Yaw","Maneuver_Pitch","Maneuver_Roll","Yaw_Governor","Pitch_Governor","Roll_Governor","Afterburner_Accel","Forward_Accel","Retro_Accel","Left_Accel","Right_Accel","Top_Accel","Bottom_Accel","Afterburner_Speed_Governor","Default_Speed_Governor","ITTS","Radar_Color","Radar_Range","Tracking_Cone","Max_Cone","Lock_Cone","Hold_Volume","Can_Cloak","Cloak_Min","Cloak_Rate","Cloak_Energy","Cloak_Glass","Repair_Droid","ECM_Rating","ECM_Resist","Ecm_Drain","Hud_Functionality","Max_Hud_Functionality","Lifesupport_Functionality","Max_Lifesupport_Functionality","Comm_Functionality","Max_Comm_Functionality","FireControl_Functionality","Max_FireControl_Functionality","SPECDrive_Functionality","Max_SPECDrive_Functionality","Slide_Start","Slide_End","Activation_Accel","Activation_Speed","Upgrades","Sub_Units","Sound","Light","Mounts","Net_Comm","Dock","Cargo_Import","Cargo","Explosion","Equipment_Space","Num_Animation_Stages"]
coordkeys={"Unit_Scale":1,"CockpitX":1,"CockpitY":1,"CockpitZ":1,"Mesh":1,"Light":1,"Mounts":1,"Dock":1,"Sub_Units":1}

def splitMounts(mnt):
    whereo=mnt.find("{")
    wherec=mnt.find("}")
    ret=[]
    while (whereo!=-1 and wherec!=-1):
        ret.append(mnt[whereo+1:wherec])
        mnt=mnt[wherec+1:]
        whereo=mnt.find("{")
        wherec=mnt.find("}")
    return ret

def restringizeMount(o):
    if (len(o)==0):
        return ""
    ret=o[0]
    for i in o[1:]:
        ret+=";"+i
    return ret;

def combineMount(orig,coord):
    orig=orig.split(";")
    coord=coord.split(";")
    return restringizeMount([orig[0],orig[1],orig[2],orig[3],coord[4],coord[5],coord[6],coord[7],coord[8],coord[9],coord[10],coord[11],coord[12],coord[13],coord[14],orig[15],orig[16]])


def combineMounts(orig,coords):
    orig=splitMounts(orig)
    coords=splitMounts(coords)
    N=len(orig)
    ret=""
    if (len(coords)<N):
        N=len(coords)
    for i in range(N):
        ret+="{"+combineMount(orig[i],coords[i])+"}";
    for i in orig[N:]:
        ret+="{"+i+"}"
    for i in coords[N:]:
        ret+="{"+i+"}"    
    return ret
def fixupKeys(xkeys,xlist,ikeys,ilist,usedkeys={}):
    imap={}
    ret=[]
    for i in range (len(ikeys)):
        imap[ikeys[i]]=ilist[i]
    for x in range (len(xkeys)):
        xkey=xkeys[x]
        if (xkey in imap):
            if (len(xlist)==0 or xkey in usedkeys or len(usedkeys)==0):
                if (len(usedkeys) and xkey=="Mounts"):
                    ret.append(combineMounts(xlist[x],imap[xkey]))
                else:
                    ret.append(imap[xkey])
            else:
                ret.append(xlist[x])
        else:
            if (len(xlist)>x):
                ret.append(xlist[x])
            else:
                ret.append("")
    return ret
def prefixCmp(pre,whole):
    if (len(whole)<len(pre)):
        return 0
    return pre==whole[0:len(pre)]

def printncommas(n):
    retstring=""
    for k in range(n):
        retstring+=','
    return retstring
def printn1(n,delim=","):
    retstring=""
    for k in range(n):
        retstring+='1'
        retstring+=delim
    return retstring
def printn0(n,delim=','):
    retstring=""
    for k in range(n):
        retstring+='0'
        retstring+=delim
    return retstring

def getParentVal(node,val):
    i = node.parentNode
    while(i):
        j = i.firstChild
        while (j):
	    try:
	        if (j.tagName!='var'):
		    return None
		if (j.getAttribute('name')==val):
		    return j.getAttribute('value')
	    except:
	        pass
	    j = j.nextSibling		
	    i = node.parentNode
    return getParentVal(node,val)

def toPair (s):
    if (not s):
        return None
    k=s.split(' ')
    return (float(k[0]),float(k[1]),float(k[2]))

def getVal(node,val):
    for i in node.getElementsByTagName('var'):
        if (i.getAttribute('name')==val):
	    return i.getAttribute('value')
    return getParentVal(node,val)

def getValND(node,val):
    for i in node.getElementsByTagName('var'):
        if (i.getAttribute('name')==val):
	    return i.getAttribute('value')
    return None

def removeVal(node,val):
    for i in node.getElementsByTagName('var'):
        if (i.getAttribute('name')==val):
	    node.removeChild(i)
	    
def usage():
    print("Usage:\n\tCall with name of Xunit file to append to Master Unit file")
    return

def xmltocaps(dom):
    if(dom.nodeType==1):
        dom.tagName=dom.tagName.capitalize()
    curchild=dom.firstChild
    while (curchild):
        if(curchild.nodeType==1):
            xmltocaps(curchild)
        curchild=curchild.nextSibling
    return

def getAttributeValueCIS(node,attributename):
    for key in node.attributes.keys():
        if node.getAttributeNode(key).name.lower()==attributename.lower():
            return node.getAttribute(key);
    return ""
def defaultFloat (s,f=0):
    if (len(s)):
        return float(s)
    return f
def getFaction(filename):
    unitname=filename[max(filename.rfind('/')+1,0):]
    findex=filename.find("factions")+9
    faction=""
    if findex!=8:
        faction=filename[findex:]
	faction=faction[0:faction.find('/')]
    else:
        return (unitname,"");
    return (unitname+"__"+faction,faction)

def printdirectory(filename):
    index=filename.rfind('/')
    return filename[0:index]

def printname(filename):
    filename=filename[max(filename.rfind('/')+1,0):]
    index=filename.find('.')
    if index==-1:
        index=len(filename)
    return filename[0:index]

def printFoFID():
    return "FIXME"

def printobjecttype():
    return "FIXME"

def printcombatrole(desc):
    return getAttributeValueCIS(desc,"combatrole")

def printtextdesc(desc):
    return "WRITEME"

def printhudimage(defense):
    return getAttributeValueCIS(defense,"hudimage")

def printunitscale(unit):
    return getAttributeValueCIS(unit,"scale")

def printcockpit(cockpit):
    return getAttributeValueCIS(cockpit,"file")+','+getAttributeValueCIS(cockpit,"x")+','+getAttributeValueCIS(cockpit,"y")+','+getAttributeValueCIS(cockpit,"z")

def printmesh(mesh):
    return '{'+getAttributeValueCIS(mesh,'file')+';;}'

def printsmesh(smesh):
    return getAttributeValueCIS(smesh,'file')

def printrmesh(rmesh):
    return getAttributeValueCIS(rmesh,'file')

def printbmesh(bmesh):
    return getAttributeValueCIS(bmesh,'file')

def printusebsp(bmesh):
    return getAttributeValueCIS(bmesh,'usebsp')

def printuserapid(rmesh):
    return getAttributeValueCIS(rmesh,'rapid')

def printmass(stat):
    return getAttributeValueCIS(stat,'mass')

def printmoment(stat):
    return getAttributeValueCIS(stat,'momentofinertia')

def printfuelcap(stat):
    mass=float(getAttributeValueCIS(stat,'mass'))
    return str(mass/12)

def printhull(hull):
    return getAttributeValueCIS(hull,'strength')

def printarmor(armor):
    front=float(getAttributeValueCIS(armor,'Front'))
    back=float(getAttributeValueCIS(armor,'Back'))
    left=float(getAttributeValueCIS(armor,'left'))
    right=float(getAttributeValueCIS(armor,'right'))
    FTR=str((front+right)/4.0)
    FTL=str((front+left)/4.0)
    FBR=str((front+right)/4.0)
    FBL=str((front+left)/4.0)
    BTR=str((back+right)/4.0)
    BTL=str((back+left)/4.0)
    BBR=str((back+right)/4.0)
    BBL=str((back+left)/4.0)
    retstring=""
#armorFTR
    retstring+=FTR
    retstring+=','
#armorFTL
    retstring+=FTL
    retstring+=','
#armorFBR
    retstring+=FBR
    retstring+=','
#armorFBL
    retstring+=FBL
    retstring+=','
#armorBTR
    retstring+=BTR
    retstring+=','
#armorBTL
    retstring+=BTL
    retstring+=','
#armorBBR
    retstring+=BBR
    retstring+=','
#armorBBL
    retstring+=BBL
    retstring+=','
    return retstring

def printshield(shield):
    front=getAttributeValueCIS(shield,'Front')
    back=getAttributeValueCIS(shield,'Back')
    left=getAttributeValueCIS(shield,'left')
    right=getAttributeValueCIS(shield,'right')
    tmp=getAttributeValueCIS(shield,'leak')
    if (len(tmp)):
        leak=str(float(tmp)/100.0)
    else:
        leak="0.0"
    recharge=getAttributeValueCIS(shield,'recharge')
    retstring=""
#shieldFTR, front
    retstring+=front
    retstring+=','
#shieldFTL, back
    retstring+=back
    retstring+=','
#shieldFBR, right
    retstring+=right
    retstring+=','
#shieldFBL, left
    retstring+=left
    retstring+=','
#shieldBTR
    retstring+=','
#shieldBTL
    retstring+=','
#shieldBBR
    retstring+=','
#shieldBBL
    retstring+=','
#shieldRecharge
    retstring+=recharge
    retstring+=','
#shieldLeak
    retstring+=leak
    retstring+=','
    return retstring

def printwarpcap(reactor):
    return getAttributeValueCIS(reactor,"warpenergy")

def printprimarycap(reactor):
    return getAttributeValueCIS(reactor,"limit")

def printreactorrecharge(reactor):
    return getAttributeValueCIS(reactor,"recharge")

def printjumpdrivepresent(jump):
    invpres=getAttributeValueCIS(jump,"missing")
    if (invpres!=""):
        invpres=1-int(invpres)
    else:
        invpres=1
    if (invpres):
        return "TRUE"
    else:
        return "FALSE"

def printjumpdrivedelay(jump):
    return getAttributeValueCIS(jump,"delay")

def printwormhole(jump):
    return getAttributeValueCIS(jump,"wormhole")

def printoutsyscost(jump):
    return getAttributeValueCIS(jump,"jumpenergy")

def printinsyscost(jump):
    return getAttributeValueCIS(jump,"insysenergy")

def printafterburnenergy(energy):
    return getAttributeValueCIS(energy,"afterburnenergy")

def printmyaw(maneuver):
    return getAttributeValueCIS(maneuver,"yaw")

def printmpitch(maneuver):
    return getAttributeValueCIS(maneuver,"pitch")

def printmroll(maneuver):
    return getAttributeValueCIS(maneuver,"roll")

def printcyaw(computer):
    return getAttributeValueCIS(computer,"yaw")

def printcpitch(computer):
    return getAttributeValueCIS(computer,"pitch")

def printcroll(computer):
    return getAttributeValueCIS(computer,"roll")

def printengineaccels(engine):
    retstring=""
#afterburner accel
    retstring+=getAttributeValueCIS(engine,"Afterburner")
    retstring+=','
#forward accel
    retstring+=getAttributeValueCIS(engine,"forward")
    retstring+=','
#retro accel
    retstring+=getAttributeValueCIS(engine,"retro")
    retstring+=','
#left accel
    retstring+=getAttributeValueCIS(engine,"left")
    retstring+=','
#right accel
    retstring+=getAttributeValueCIS(engine,"right")
    retstring+=','
#top accel
    retstring+=getAttributeValueCIS(engine,"top")
    retstring+=','
#bottom accel
    retstring+=getAttributeValueCIS(engine,"bottom")
    retstring+=','
    return retstring

def printaftspeedgov(computer):
    return getAttributeValueCIS(computer,"afterburner")

def printspeedgov(computer):
    return getAttributeValueCIS(computer,"maxspeed")

def printradar(radar,isblank,istemplate,faction):
    retstring=""
#itts
    retstring+=getAttributeValueCIS(radar,"itts")
    retstring+=','
#radar color
    retstring+=getAttributeValueCIS(radar,"color")
    retstring+=','
#radar range
    rang=getAttributeValueCIS(radar,"range")
    if (rang==""):
        rang="300000000";
        if isblank or faction=="upgrades":
            rang="0"
        elif istemplate:
            range="3e38"
    retstring+=rang
    retstring+=','
#tracking cone
    if(getAttributeValueCIS(radar,"trackingcone")!=""):
        retstring+=str(math.acos(float(getAttributeValueCIS(radar,"trackingcone")))/math.pi*180)
    elif (isblank or faction=="upgrades"):
        retstring+="0"
    elif (istemplate):
        retstring+="180"
    else:
        retstring+="5"
    retstring+=','
#max_cone
    if(getAttributeValueCIS(radar,"maxcone")!=""):
        retstring+=str(math.acos(float(getAttributeValueCIS(radar,"maxcone")))/math.pi*180)
    elif(isblank or faction=="upgrades"):
        retstring+="0"
    else:
        retstring+="180"
    retstring+=','
#lock cone
    if(getAttributeValueCIS(radar,"lockcone")!=""):
        retstring+=str(math.acos(float(getAttributeValueCIS(radar,"lockcone")))/math.pi*180)
    elif (isblank or faction=="upgrades"):
        retstring+="0"
    elif (istemplate):
        retstring+="180"
    else:
        retstring+="10"
    retstring+=','
    return retstring

def printholdvolume(hold):
    return getAttributeValueCIS(hold,"volume")

def printcloak(cloak):
    retstring=""
#can cloak
    missing=getAttributeValueCIS(cloak,"missing")
    if(missing!=""):
        missing=str(1-float(missing))
    else:
        missing="1"
    retstring+=missing
    retstring+=','
#cloak min
    retstring+=getAttributeValueCIS(cloak,"cloakmin")
    retstring+=','
#cloak rate
    retstring+=getAttributeValueCIS(cloak,"cloakrate")
    retstring+=','
#cloak energy
    retstring+=getAttributeValueCIS(cloak,"cloakenergy")
    retstring+=','
#cloak glass
    retstring+=getAttributeValueCIS(cloak,"cloakglass")
    retstring+=','
    return retstring

def printrepdroid(defense):
    return getAttributeValueCIS(defense,"repairdroid")

def printrepdroid(defense):
    return getAttributeValueCIS(defense,"repairdroid")

def printupgrade(upgrade):
    name=getAttributeValueCIS(upgrade,"file")
    return "{"+name+";;}"

def printsubunit(subunit):
    return "{"+getAttributeValueCIS(subunit,"file")+';'+getAttributeValueCIS(subunit,"x")+';'+getAttributeValueCIS(subunit,"y")+';'+getAttributeValueCIS(subunit,"z")+';'+getAttributeValueCIS(subunit,"ri")+';'+getAttributeValueCIS(subunit,"rj")+';'+getAttributeValueCIS(subunit,"rk")+';'+getAttributeValueCIS(subunit,"qi")+';'+getAttributeValueCIS(subunit,"qj")+';'+getAttributeValueCIS(subunit,"qk")+';'+str(math.acos(defaultFloat(getAttributeValueCIS(subunit,"restricted"),-1))*180./math.pi)+'}'

def printsound(sound):
    return getAttributeValueCIS(sound,"shieldhit")+';'+getAttributeValueCIS(sound,"armorhit")+';'+getAttributeValueCIS(sound,"hullhit")+';'+getAttributeValueCIS(sound,"jump")+';'+getAttributeValueCIS(sound,"explode")+';'+getAttributeValueCIS(sound,"cloak")+';'+getAttributeValueCIS(sound,"engine")

def printlight(light):
    return '{'+getAttributeValueCIS(light,"file")+';'+getAttributeValueCIS(light,"x")+';'+getAttributeValueCIS(light,"y")+';'+getAttributeValueCIS(light,"z")+';'+getAttributeValueCIS(light,"size")+';'+getAttributeValueCIS(light,"red")+';'+getAttributeValueCIS(light,"green")+';'+getAttributeValueCIS(light,"blue")+';'+getAttributeValueCIS(light,"alpha")+';'+getAttributeValueCIS(light,"activationspeed")+'}'

def modifysize(size,name):
    ret=size
    if (name.find(".template")!=-1):
        if (size.lower().find("autotracking")==-1):
            ret+=" AUTOTRACKING"
        if (size.lower().find("special")==-1):
            ret+=" SPECIAL"
    return ret
def printmount(mount,name):
    return "{"+getAttributeValueCIS(mount,"weapon")+';'+getAttributeValueCIS(mount,"ammo")+';'+getAttributeValueCIS(mount,"volume")+';'+modifysize(getAttributeValueCIS(mount,"size"),name)+';'+getAttributeValueCIS(mount,"x")+';'+getAttributeValueCIS(mount,"y")+';'+getAttributeValueCIS(mount,"z")+';'+getAttributeValueCIS(mount,"xyscale")+';'+getAttributeValueCIS(mount,"zscale")+';'+getAttributeValueCIS(mount,"ri")+';'+getAttributeValueCIS(mount,"rj")+';'+getAttributeValueCIS(mount,"rk")+';'+getAttributeValueCIS(mount,"qi")+';'+getAttributeValueCIS(mount,"qj")+';'+getAttributeValueCIS(mount,"qk")+";1;1}"

def printdock(dock):
    return '{'+getAttributeValueCIS(dock,"dockinternal")+';'+getAttributeValueCIS(dock,"x")+';'+getAttributeValueCIS(dock,"y")+';'+getAttributeValueCIS(dock,"z")+';'+getAttributeValueCIS(dock,"size")+';0}'

def printimport(category):
    retstring=""
    categoryname=getAttributeValueCIS(category,"file")
    imports=category.getElementsByTagName("Import")
    for animport in imports:
        retstring+="{"+categoryname+';'+getAttributeValueCIS(animport,"price")+';'+getAttributeValueCIS(animport,"pricestddev")+';'+getAttributeValueCIS(animport,"quantity")+';'+getAttributeValueCIS(animport,"quantitystddev")+'}'
    return retstring

def printcargo(category):
    retstring=""
    categoryname=getAttributeValueCIS(category,"file")
    cargo=category.getElementsByTagName("Cargo")
    for acargo in cargo:
        retstring+="{"+getAttributeValueCIS(acargo,"file")+';'+categoryname+';'+getAttributeValueCIS(acargo,"price")+';'+getAttributeValueCIS(acargo,"quantity")+';'+getAttributeValueCIS(acargo,"mass")+';'+getAttributeValueCIS(acargo,"volume")+';1;1;'+getAttributeValueCIS(acargo,"description")+';0}'
    return retstring

def printexplosion(explosion):
    return getAttributeValueCIS(explosion,"explosionani")

##
##  START HERE
##
usecoords=0
for inputunit in sys.argv[1:]:
    if inputunit=="-xyz":
        usecoords=1
        continue
    isblank = (inputunit.find(".blank")!=-1)
    istemplate = (inputunit.find(".template")!=-1)
    ismilspec = isblank==0 and istemplate==0
    print("Adding "+inputunit)
    inpxml = open(inputunit,"r")
    try:
      intermed = xml.dom.minidom.parseString(inpxml.read())
    except:
      print(inputunit+" failed")
      continue
    inpxml.close()
    xmltocaps(intermed)
    retstring=""
#key
    (unitname,faction)=getFaction(inputunit)
    retstring+=unitname+","
#directory
    retstring+=printdirectory(inputunit)+","
#name
    retstring+=printname(inputunit)+","
#FoFID
    retstring+=printFoFID()+","
#Object Type
    retstring+=printobjecttype()+","
#combat role    
    descs=intermed.getElementsByTagName('Description')
    for desc in descs:
        retstring+=printcombatrole(desc)
    retstring+=','
#text description
    descs=intermed.getElementsByTagName('Description')
    for desc in descs:
        retstring+=printtextdesc(desc)
    retstring+=','
#hud image
    defenses=intermed.getElementsByTagName('Defense')
    for defense in defenses:
        retstring+=printhudimage(defense)
    retstring+=','
#unit scale
    units=intermed.getElementsByTagName('Unit')
    for unit in units:
        retstring+=printunitscale(unit)
    retstring+=','
#cockpit
    cockpits=intermed.getElementsByTagName('Cockpit')
    for cockpit in cockpits:
        retstring+=printcockpit(cockpit)
    retstring+=','
    if(len(cockpits)==0):
        retstring+=printncommas(3)
#mesh
    meshes=intermed.getElementsByTagName('Meshfile')
    for mesh in meshes:
        retstring+=printmesh(mesh)
    retstring+=','
#shieldmesh
    smeshes=intermed.getElementsByTagName('Shieldmesh')
    for smesh in smeshes:
        retstring+=printsmesh(smesh)
    retstring+=','
#rapidmesh
    rmeshes=intermed.getElementsByTagName('Rapidmesh')
    for rmesh in rmeshes:
        retstring+=printrmesh(rmesh)
    retstring+=','
#bspmesh
    bmeshes=intermed.getElementsByTagName('Bspmesh')
    for bmesh in bmeshes:
        retstring+=printbmesh(bmesh)
    retstring+=','
#use bsp
    bmeshes=intermed.getElementsByTagName('Bspmesh')
    for bmesh in bmeshes:
        retstring+=printusebsp(bmesh)
    retstring+=','
#use rapid
    rmeshes=intermed.getElementsByTagName('Rapidmesh')
    for rmesh in rmeshes:
        retstring+=printuserapid(rmesh)
    retstring+=','
#mass
    stats=intermed.getElementsByTagName('Stats')
    for stat in stats:
        retstring+=printmass(stat)
    retstring+=','
#moment of inertia
    stats=intermed.getElementsByTagName('Stats')
    for stat in stats:
        retstring+=printmoment(stat)
    retstring+=','
#Volume (convex hull approximation)
    retstring+=','
#Fuel capacity
    stats=intermed.getElementsByTagName('Stats')
    for stat in stats:
        retstring+=printfuelcap(stat)
    retstring+=','
#hull
    hulls=intermed.getElementsByTagName('Hull')
    for hull in hulls:
        retstring+=printhull(hull)
    retstring+=','
#armor, all 8 + ,
    armors=intermed.getElementsByTagName('Armor')
    for armor in armors:
        retstring+=printarmor(armor)
    if (len(armors)==0):
        retstring+=printncommas(8)
#shields, all 8 + ,recharge,leak,
    shields=intermed.getElementsByTagName('Shields')
    for shield in shields:
        retstring+=printshield(shield)
    if (len(shields)==0):
        retstring+=printncommas(10)
#warp capacity
    reactors=intermed.getElementsByTagName('Reactor')
    for reactor in reactors:
        retstring+=printwarpcap(reactor)
    retstring+=','
#primary capacity
    reactors=intermed.getElementsByTagName('Reactor')
    for reactor in reactors:
        retstring+=printprimarycap(reactor)
    retstring+=','
#reactor recharge
    reactors=intermed.getElementsByTagName('Reactor')
    for reactor in reactors:
        retstring+=printreactorrecharge(reactor)
    retstring+=','
#jump drive present
    jumps=intermed.getElementsByTagName('Jump')
    jumpstr="FALSE"
    for jump in jumps:
        tmp=printjumpdrivepresent(jump)
        if (len(tmp)):
            jumpstr=tmp
    retstring+=jumpstr
    retstring+=','
#jump drive delay
    jumps=intermed.getElementsByTagName('Jump')
    for jump in jumps:
        retstring+=printjumpdrivedelay(jump)
    retstring+=','
#wormhole
    jumps=intermed.getElementsByTagName('Jump')
    for jump in jumps:
        retstring+=printwormhole(jump)
    retstring+=','
#outsystem jump cost
    jumps=intermed.getElementsByTagName('Jump')
    for jump in jumps:
        retstring+=printoutsyscost(jump)
    retstring+=','
#warp usage cost
    jumps=intermed.getElementsByTagName('Jump')
    for jump in jumps:
        retstring+=printinsyscost(jump)
    retstring+=','
#afterburner type
    retstring+="0"
    retstring+=','
#afterburner usage cost
    energies=intermed.getElementsByTagName('Energy')
    for energy in energies:
        retstring+=printafterburnenergy(energy)
    retstring+=','
#yaw
    maneuvers=intermed.getElementsByTagName('Maneuver')
    for maneuver in maneuvers:
        retstring+=printmyaw(maneuver)
    retstring+=','
#pitch
    maneuvers=intermed.getElementsByTagName('Maneuver')
    for maneuver in maneuvers:
        retstring+=printmpitch(maneuver)
    retstring+=','
#roll
    maneuvers=intermed.getElementsByTagName('Maneuver')
    for maneuver in maneuvers:
        retstring+=printmroll(maneuver)
    retstring+=','
#yaw governor
    computers=intermed.getElementsByTagName('Computer')
    for computer in computers:
        retstring+=printcyaw(computer)
    retstring+=','
#pitch governor
    computers=intermed.getElementsByTagName('Computer')
    for computer in computers:
        retstring+=printcpitch(computer)
    retstring+=','
#roll governor
    computers=intermed.getElementsByTagName('Computer')
    for computer in computers:
        retstring+=printcroll(computer)
    retstring+=','
#all accels and commas
    engines=intermed.getElementsByTagName('Engine')
    for engine in engines:
        retstring+=printengineaccels(engine)
    if (len(engines)==0):
        retstring+=printncommas(7)
#afterburner governor
    computers=intermed.getElementsByTagName('Computer')
    for computer in computers:
        retstring+=printaftspeedgov(computer)
    retstring+=','
#default speed governor
    computers=intermed.getElementsByTagName('Computer')
    for computer in computers:
        retstring+=printspeedgov(computer)
    retstring+=','
#radar stuff
    radars=intermed.getElementsByTagName("Radar")
    for radar in radars:
        retstring+=printradar(radar,isblank,istemplate,faction)
    if (len(radars)==0):
        if isblank or faction=="upgrades":
            retstring+="0,0,0,0,0,0,"
        elif istemplate:
            retstring+="true,true,3e38,180,180,180,"
	else:
	    retstring+="false,true,300000000,5,180,10,"
#hold volume
    holds=intermed.getElementsByTagName("Hold")
    for hold in holds:
        retstring+=printholdvolume(hold)
    retstring+=','
#cloak stuff
    cloaks=intermed.getElementsByTagName("Cloak")
    for cloak in cloaks:
        retstring+=printcloak(cloak)
    if (len(cloaks)==0):
        retstring+=printncommas(5)
#repair droid
    defenses=intermed.getElementsByTagName("Defense")
    for defense in defenses:
        retstring+=printrepdroid(defense)
    retstring+=','
#ecm rating currently b0rken
    retstring+="0"
    retstring+=','
#ecm resist currently b0rken
    retstring+="0"
    retstring+=','
#ecm power drain currently b0rken
    retstring+="0"
    retstring+=','
#HUD func
#max HUD func
    if (faction=="upgrades" and unitname.find("hud")==-1):
        retstring+=printn0(32,';')
        retstring+="0,"
        retstring+=printn0(32,';')
        retstring+="0,"
    else:
        retstring+=printn1(32,';')
        retstring+="1,"
        retstring+=printn1(32,';')
        retstring+="1,"
#lifesupport func
    if (faction!="upgrades"):
        funcstr="1"
    else:
        funcstr="0"
    retstring+=funcstr
    retstring+=','
#max lifesupport func
    retstring+=funcstr
    retstring+=','
#comm func
    retstring+=funcstr
    retstring+=','
#max comm func
    retstring+=funcstr
    retstring+=','
#firecontrol func
    retstring+=funcstr
    retstring+=','
#max firecontrol func
    retstring+=funcstr
    retstring+=','
#SPECdrive func
    retstring+=funcstr
    retstring+=','
#max SPECdrive func
    retstring+=funcstr
    retstring+=','
#slide start
    retstring+="0"
    retstring+=','
#slide end
    retstring+="0"
    retstring+=','
#activation accel
    retstring+=','
#activation speed
    retstring+=','
#upgrade
    upgrades=intermed.getElementsByTagName('Upgrade')
    capac={}
    for upgrade in upgrades:
        name=getAttributeValueCIS(upgrade,"file")
        if (name.find("capacitance")!=-1):
            if name in capac:
                capac[name]+=1
            else:
                capac[name]=1;
    #print capac
    for upgrade in upgrades:
        name=getAttributeValueCIS(upgrade,"file")
        if (name in capac):
            num=capac[name]
            if (num<=0):
                continue
            #print "found: "+name+" "+str(num)
            if (num>=256):
                capac[name]-=256;
                nam=name+".256";
            elif (num>=64):
                capac[name]-=64;
                name=name+".64";
            elif (num>=16):
                capac[name]-=16
                name=name+".16";
            elif (num>=4):
                capac[name]-=4
                name=name+".4";
            else:
                capac[name]-=1
        retstring+= "{"+name+";;}"
    retstring+=','
#sub units
    subunits=intermed.getElementsByTagName('Subunit')
    for subunit in subunits:
        retstring+=printsubunit(subunit)
    retstring+=','
#sound
    sounds=intermed.getElementsByTagName('Sound')
    for sound in sounds:
        retstring+=printsound(sound)
    retstring+=','
#light
    lights=intermed.getElementsByTagName('Light')
    for light in lights:
        retstring+=printlight(light)
    retstring+=','
#mounts    
    mounts= intermed.getElementsByTagName('Mount')
    for mount in mounts:
        retstring+=printmount(mount,unitname)
    retstring+=','
#net_comm ??? ... damned if i know
    retstring+=','
#dock
    docks= intermed.getElementsByTagName('Dock')
    for dock in docks:
        retstring+=printdock(dock)
    retstring+=','
#cargo import
    categories=intermed.getElementsByTagName('Category')
    for category in categories:
        retstring+=printimport(category)
    retstring+=','
#cargo
    retstring+='"'
    categories=intermed.getElementsByTagName('Category')
    for category in categories:
        retstring+=printcargo(category)
    retstring+='",'
#explosion
    explosions=intermed.getElementsByTagName('Defense')
    for explosion in explosions:
        retstring+=printexplosion(explosion)
    retstring+=','
#equipment space
    retstring+="0" #Dummy value
    retstring+=','
#num animation points
    retstring+="0"
    #write to file
    retstring+="\n"
    o=file("units.bak.csv","w");
    where=retstring.find(",")
    unitname=retstring[0:where+1]
    masterunitfile=file("units.csv","r")
    line=masterunitfile.readline()
    import csv
    externalkeys=csv.semiColonSeparatedList(line,",");
    foundme=0
    usedlist={}
    if usecoords!=0:
        usedlist=coordkeys
    while (line!=""):
        if (prefixCmp(unitname,line)):
            o.write(csv.writeList(fixupKeys(externalkeys,
                                            csv.semiColonSeparatedList(line,","),
                                            internalkeys,
                                            csv.semiColonSeparatedList(retstring,","),
                                            usedlist)));
            foundme=1
        else:
            o.write(line);
        line=masterunitfile.readline();
    if (foundme==0):
        o.write(csv.writeList(fixupKeys(externalkeys,
                                        [],
                                        internalkeys,
                                        csv.semiColonSeparatedList(retstring,","))));
    masterunitfile.close()
    o.close()
    import os
    os.rename("units.bak.csv","units.csv");
