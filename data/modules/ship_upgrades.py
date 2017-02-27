import vsrandom
import VS
import debug

#
# IMPORTANT NOTE: Right now, there are no engines in the upgrades list.
#    But, anyway, I left the Engine/Reactor dual upgrade system.
#    Engines will always fail, without more consequences than an error
#    messages, but if after balancing they get reintroduced (which makes
#    sense, somewhat), or if other mods require them (also possible),
#    then it makes sense to leave it.
#

###########################################################
#                                                         #
#                      CONFIGURATION                      #
#                                                         #
#   IMPORTANT: dataset changes are LIKELY to require      #
#      configuration changes. In the following section    #
#      is a list of required upgrade names and bits,      #
#      and also functions building the required names.    #
#      They're used throughout the code to compose        #
#      the upgrade names and categories.                  #
#                                                         #
###########################################################


# Weapons category construction:
#    The code will select a weapon within the category
#    constructed by concatenating the base category,
#    one prefix, and one postfix. All combinations should
#    be possible
upgrades_weapons_category             = "upgrades/Weapons"
upgrades_weapons_prefixes             = ["Beam_Arrays_","Mounted_Guns_"]
upgrades_weapons_postfixes            = ["Light","Medium","Heavy"]
upgrades_weapons_default_weapon       = "laser" #the default weapon, used by basic ships
upgrades_weapons_default_weapon_count = 2 #number of deafault weapons to (try to) install


# Shield names - what's the shield with 'faces' faces and of level 'level'?
shieldMaxLevel = { 2:14 , 4:14 } # key is number of faces

def shieldBuildName(faces,level):
    facestring = { 2:"dual",4:"quad" }
    maxlevel   = shieldMaxLevel.get(faces,14)
    if (level > maxlevel):
        level = maxlevel
    return "%sshield%02d" % (facestring.get(faces,""),level+1)

def shieldBuildCategory(faces,level):
    facestring = { 
        2:"upgrades/Shield_Systems/Standard_Dual_Shields",
        4:"upgrades/Shield_Systems/Standard_Quad_Shields" 
        }
    return facestring.get(faces, "upgrades/Shield_Systems")

# Engine enhancements (afterburners):
afterburnerMaxLevel = 5

def afterburnerBuildName(level):
    if (level > afterburnerMaxLevel):
        level = afterburnerMaxLevel
    return "mult_overdrive%02d" % level

def afterburnerBuildCategory(level):
    return "upgrades/Overdrive"    

# Sensors category construction:
#    Similar to weapons category construction
upgrades_sensors_category             = "upgrades/Sensors"
upgrades_sensors_prefixes             = [""]
upgrades_sensors_postfixes            = ["Common","Confed","Highborn","Rlaan"]

# Reactors
reactorMaxLevel = 14

def reactorBuildName(level):
    if (level > reactorMaxLevel):
        level = reactorMaxLevel
    return "reactor%02d" % level

def reactorBuildCategory(level):
    return "upgrades/Reactors/Standard"    

# Engines
engineMaxLevel = 0

def engineBuildName(level):
    if (level > reactorMaxLevel):
        level = reactorMaxLevel
    return "engine%02d" % level

def engineBuildCategory(level):
    return "upgrades/Engines/Standard"    

# Hull upgrades
upgrades_hull_category = None

# Turret upgrades
upgrades_turrets_category = "upgrades/Weapons/Turrets"


# Armnor enhancements
armorMaxLevel = 5

def armorBuildName(level):
    if (level > armorMaxLevel):
        level = armorMaxLevel
    return "armor%02d" % level

def armorBuildCategory(level):
    return "upgrades/Armor"    


# Ammo - try primary, then secondary
upgrades_ammo_category_pri = "upgrades/Ammunition/Common"
upgrades_ammo_category_sec = "upgrades/Ammunition"

# Repair systems - try primary, then secondary
upgrades_repair_category_pri="upgrades/Repair_Systems/Research"
upgrades_repair_category_sec="upgrades/Repair_Systems"



###########################################################
#                                                         #
#                      IMPLEMENTATION                     #
#                                                         #
#   Dataset changes are UNLIKELY to require               #
#      implementation changes. In the following section   #
#                                                         #
###########################################################

def GetDiffInt (diff):
    ch=0
    if (diff<=0.1):
        ch=0
    elif (diff<=0.3):
        ch=1-vsrandom.randrange(0,2)
    elif (diff<=0.5):
        ch=2-vsrandom.randrange(0,3)
    elif (diff<=0.7):
        ch=3-vsrandom.randrange(0,4)
    elif (diff<=0.9):
        ch=4-vsrandom.randrange(0,5)
    else:
        ch=5-vsrandom.randrange(0,6)
    return ch

# This function makes a string based on the difficulty. In this way it can be restricted to light or medium mounts when the difficulty is low, avoiding unaffordable weapons
def GetDiffCargo (diff, base_category, all_category, use_all, postfixes, dont_use_all=0):
    cat=all_category
    ch=dont_use_all
    #this makes ch only 1
    if (diff<=0.2):
        ch=1
    elif (diff<=0.4):
        ch=2-vsrandom.randrange(dont_use_all,3)
    elif ((diff<=0.7) or use_all):
        ch=3-vsrandom.randrange(dont_use_all,4)
    return base_category + postfixes[(ch-1)*len(postfixes)/3]

#this gets a random cargo listed on the master part list.
def getItem (cat,parentcat=None):
    if not cat:
        return VS.Cargo("","",0,0,0,0)
    list=VS.getRandCargo(1,cat)#try to get a cargo from said category
    if (list.GetQuantity()<=0):#if no such cargo exists in this cateogry
        if (parentcat!=None):
            print "Python Upgrade Error: finding %s using %s instead" % (cat,parentcat)
            list=VS.getRandCargo(1,parentcat)#get it from the parent category
        if (list.GetQuantity()<=0):#otherwise get cargo from upgrades category
            print "Python UpgradeError: category %s -- getting random instead" % (cat)
            list=VS.getRandCargo(1,"upgrades")#this always succeeds
    return list

def BuildDiffCat (diff,basecat,prefixes,postfixes,use_all=1,dont_use_all=0):#gets random beam or mounted gun from master part list
    return GetDiffCargo(diff,basecat+"/"+prefixes[vsrandom.randrange(0,len(prefixes),1)],basecat,use_all,postfixes,dont_use_all)

def GetRandomWeapon (diff):#gets random beam or mounted gun from master part list
    cat=BuildDiffCat(diff,upgrades_weapons_category,upgrades_weapons_prefixes,upgrades_weapons_postfixes,1,0)
    debug.debug("Getting weapon from %s... " % cat)
    item=getItem(cat,upgrades_weapons_category)
    debug.debug("Got %s\n" % item)
    return item

def getRandIncDec (type):
    type += vsrandom.randrange (-1,2,2)
    if (type<0):
        type=0
    elif (type>5):
        type=5
    return type

def GetShieldLevelZero(faces):
    return shieldBuildName(faces,0)

def GetRandomShield (faces,type):#gets random shield system from master part list (returns filename)
    type = getRandIncDec (type)
    return shieldBuildName(faces,type)

def GetRandomAfterburner (diff):#get random afterburner from master part list (returns filename)
    return afterburnerBuildName(1+diff*(GetDiffInt(diff)*(afterburnerMaxLevel-1)/5))

def getRandomRadar (diff):
    cat=BuildDiffCat(diff,upgrades_sensors_category,upgrades_sensors_prefixes,upgrades_sensors_postfixes,1,0)
    debug.debug("Getting sensors from %s... " % cat)
    item=getItem(cat,upgrades_sensors_category)
    debug.debug("Got %s\n" % item)
    return item

def UpgradeRadar (un,diff):
    item = getRandomRadar(diff)
    if item:
        temp=un.upgrade(item.GetContent(),0,0,1,0)

def UpgradeAfterburner (un,diff):
    i=0
    while (i<diff*3.0):
        temp=un.upgrade (GetRandomAfterburner(diff),0,0,1,0)
        i=i+1

def getRandomEngine (diff): #get random engine from master part list
    myint=GetDiffInt(diff)
    cat=engineBuildName(myint*engineMaxLevel/5)
    dog=reactorBuildName(myint*reactorMaxLevel/5)
    return (myint,cat,dog)

def UpgradeEngine (un, diff):
    (type,cat,dog) = getRandomEngine (diff)
    if (type!=0):
        temp=un.upgrade (cat,0,0,1,0)
        temp=un.upgrade (dog,0,0,1,0)
        debug.debug("Upgrading Engine %s percent %f" % (cat,temp))
        if (temp>0.0):
            cat = GetRandomShield (2,type)
            temp=un.upgrade (cat,0,0,1,0)
            debug.debug("Upgrading Shield %s percent %f" % (cat,temp))
            cat = GetRandomShield (4,type)
            temp=un.upgrade (cat,0,0,1,0)
            debug.debug("Upgrading Shield4 %s percent %f" % (cat,temp))
            return True
    cat=GetShieldLevelZero(2)
    temp = un.upgrade(cat,0,0,1,0)
    debug.debug("Upgrading Shield2 level 0... percent="+str(temp))
    cat=GetShieldLevelZero(4)
    temp = un.upgrade(cat,0,0,1,0)
    debug.debug("Upgrading Shield4 level 0... percent="+str(temp))
    return False

def GetRandomHull ():
    return getItem(upgrades_hull_category)

def GetRandomTurret ():
    return getItem(upgrades_turrets_category,None) # No fallback

def GetRandomArmor (diff):
    return VS.GetMasterPartList().GetCargo(armorBuildName(1+diff*(GetDiffInt(diff)*(armorMaxLevel-1)/5)))

def UpgradeArmor (un, diff):
    return un.upgrade(GetRandomArmor(diff).GetContent(),0,0,1,0)

def GetRandomAmmo ():
    return getItem (upgrades_ammo_category_pri,upgrades_ammo_category_sec)

def GetRandomRepairSys ():
    return getItem (upgrades_repair_category_pri,upgrades_repair_category_sec)

#this function sets up a blank unit with some basic upgrades that are really a necessecity for any sort of figthing
def basicUnit (un, diff):
    i=0
    for i in range(upgrades_weapons_default_weapon_count): #no unarmed ships, please
        percent=un.upgrade(upgrades_weapons_default_weapon,i,0,0,1)
    UpgradeEngine (un,diff)
    UpgradeRadar (un,diff)
    #UpgradeArmor (un,diff)
    if ((vsrandom.random()<0.9) and (vsrandom.random()<(diff*5.0))):
        UpgradeAfterburner(un,diff)
        if ((vsrandom.random()<0.9) and (vsrandom.random()<(diff*5.0))):
            percent=un.upgrade("jump_drive",i,i,0,1)
    else:
        percent=un.upgrade("jump_drive",i,i,0,1)

#this function does the dirty work of the upgrade unit function... Given the list that contains a piece of cargo, it upgrades it, subtracts the price, and slaps it on your ship, and returns the new number of creds the computer player has.  It may well be negative cus we thought that these guys may go in debt or something
def upgradeHelper (un, mycargo, curmount,creds, force, cycle):
    newcreds=0.0
    if (mycargo.GetQuantity()<=0): #if somehow the cargo isn't there
        return 0.0 #and terminate the enclosing loop by saying we're out of cash
    else:
        str=mycargo.GetContent() #otherwise our name is the GetContent() function
        newcreds=mycargo.GetPrice() #and the price is the GetPrice() function
        newcreds = newcreds*un.upgrade(str,curmount,curmount,force,cycle)
        creds = creds -newcreds #we added some newcreds and subtracted them from credit ammt
    return creds #return new creds

def upgradeUnit (un, diff):
    creds=0.0
    curmount=0
    mycargo=VS.Cargo("","",0,0,0,0)
    str=""
    basicUnit(un,diff)
    mycargo = GetRandomHull() #ok now we get some hull upgrades
    creds =upgradeHelper (un,mycargo,0,creds,1,0)
    mycargo = GetRandomArmor(diff) #and some random armor
    creds =upgradeHelper (un,mycargo,0,creds,1,0)
    inc=0
    rndnum=vsrandom.random()*2
    if (rndnum<diff):
        mycargo = GetRandomRepairSys() #here there is a small chance that you will get a repair system.
        creds =upgradeHelper (un,mycargo,0,creds,1,0)
    turretz=un.getSubUnits()
    turretcount=0
    while (turretz.current()):
        turretz.advance()
        turretcount += 1
    turretcount-=1
    for i in range(turretcount):
        for j in range(4):
            mycargo=GetRandomTurret()#turrets as 3rd...
            creds = upgradeHelper (un,mycargo,i,creds,0,0)
    turretcount=diff*50
    if (turretcount>24):
        turretcount=24
    elif (turretcount<3):
        turretcount=3
    for i in range(int(turretcount)):
        for j in range (10):
            if (vsrandom.random()<0.66):
                mycargo=GetRandomWeapon(diff)#weapons go on as first two items of loop
            else:
                mycargo=GetRandomAmmo()
            cont = mycargo.GetContent()
            if (cont.find('tractor')==-1 and cont.find('repulsor')==-1 and cont.find("steltek_gun")==-1):
                creds =upgradeHelper (un,mycargo,curmount,creds,0,1)#we pass this in to the credits...and we only loop through all mounts if we're adding a weapon
                break
        curmount+=1#increase starting mounts hardpoint
