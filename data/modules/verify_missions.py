
import VS
import faction_ships

campaign_name={}

def verifyMission(name,args,campaign=None):
    mission=None
    if name=='directions_mission':
        mission=DirectionsVerifier(args)
    elif name=='ambush':
        mission=AmbushVerifier(args)
    elif name=='bounty_leader':
        mission=BountyLeaderVerifier(args)
    elif name=='bounty_troop':
        mission=BountyTroopVerifier(args)
    elif name=='bounty':
        mission=BountyVerifier(args)
    elif name=='cargo_mission':
        mission=CargoVerifier(args)
    elif name=='wrong_escort':
        mission=WrongEscortVerifier(args)
    elif name=='cleansweep':
        mission=CleansweepVerifier(args)
    elif name=='cleansweep_escort':
        mission=CleansweepEscortVerifier(args)
    elif name=='defend':
        mission=DefendVerifier(args)
    elif name=='escort_local':
        mission=EscortLocalVerifier(args)
    elif name=='escort_mission':
        mission=EscortVerifier(args)
    elif name=='patrol_ambush':
        mission=PatrolAmbushVerifier(args)
    elif name=='patrol_enemies':
        mission=PatrolEnemiesVerifier(args)
    elif name=='patrol':
        mission=PatrolVerifier(args)
    elif name=='plunder':
        mission=PlunderVerifier(args)
    elif name=='rescue':
        mission=RescueVerifier(args)
    elif name=='tail':
        mission=TailVerifier(args)
    elif name=='tripatrol':
        mission=TripatrolVerifier(args)
    elif name=='ambush_scan':
        mission=AmbushScan(args)
    if mission is None:
        print 'Unsupported mission type'
        return False
    else:
        return mission.isValid()

class Default:
    pass

class NoDefault:
    pass
class Argument:

    NAME="Argument"

    def __init__(self, nextarg=None, value=NoDefault()):
        self.warnings = []
        self.value = None
        self.nextarg = nextarg
        self.set(value)

    def set(self, newvalue):
        self.value = newvalue
        arg = self.nextarg
        if arg is not None:
            arg.set(self.value)

    def isValid(self):
#        print "valid %s?"%self.NAME
        if isinstance(self.value, NoDefault):
            self.warn("No default exists for unset argument")
            return False
        elif isinstance(self.value, Default):
            return True
        valid = self.checkValidity()
        arg = self.nextarg
        if arg is not None:
            valid = valid or arg.isValid()
        if not valid:
            self.printwarnings()
        return valid

    def checkValidity(self):
        return True

    def warn(self, text):
        self.warnings.append(text)

    def printwarnings(self):
        for warning in self.warnings:
            print "\'%s\' Argument Warning: "%self.NAME + warning

class PositiveInt(Argument):

    NAME="PositiveInt"

    def checkValidity(self):
        if isinstance(self.value,long) or isinstance(self.value,int):
            if self.value < 0:
                self.warn("Value %s is not a positive integer"%str(self.value))
                return False
        else:
            self.warn("Value %s is not an integer, much less positive."%str(self.value))
            return False
        return True

class PositiveIntList(Argument):

    NAME="PositiveIntList"

    def checkValidity(self):
        if isinstance(self.value,list):
            for val in self.value:
                if isinstance(val,long) or isinstance(val,int):
                    if self.value < 0:
                        self.warn("Value %s is not a positive integer"%str(val))
                        return False
                else:
                    self.warn("Value %s is not an integer, much less positive."%str(val))
                    return False
        else:
            self.warn("Value %s is not a list, much less a list of integers."%str(self.value))
            return False
        return True

class PositiveFraction(Argument):

    NAME="PositiveFraction"

    def checkValidity(self):
        if not isinstance(self.value,float) or self.value < 0 or self.value > 1:
            if isinstance(self.value,int) and (self.value == 0 or self.value == 1):
                return True
            self.warn("Value %s is not a positive fraction"%str(self.value))
            return False
        return True

class String(Argument):

    NAME="String"

    def checkValidity(self):
        if not isinstance(self.value,str):
            self.warn("Value "+str(self.value)+"is not a string")
            return False
        return True

class PositiveNumber(Argument):

    NAME="PositiveNumber"

    def checkValidity(self):
        if not (isinstance(self.value,float) or isinstance(self.value,int)) or self.value < 0:
            self.warn("Value is not a positive number")
            return False
        return True

class ZeroInt(Argument):

    NAME="ZeroInt"

    def checkValidity(self):
        if isinstance(self.value,long) or isinstance(self.value,int):
            if self.value != 0:
                self.warn("Value %s is not 0"%str(self.value))
                return False
        else:
            self.warn("Value %s is not an integer, much less 0"%str(self.value))
        return True

class System(Argument):

    NAME="System"

    def checkValidity(self):
        v = ( VS.universe.has_key(self.value) )
        if not v:
            self.warn("System %s does not exist in universe"%self.value)
            return False
        return True

class SystemTuple(Argument):

    NAME="SystemTuple"

    def checkValidity(self):
        for sys in self.value:
            v = ( VS.universe.has_key(sys) )
            if not v:
                self.warn("System %s does not exist in universe"%sys)
                return False
        return True

class Destination(Argument):

    NAME="Destination"

    def checkValidity(self):
        self.warn("No way to test destination exists")
        return True

class SaveVar(Argument):

    NAME="SaveVar"

    def checkValidity(self):
        global campaign_name
        campvar=self.value.find("_mission")
        if campvar!=-1 and self.value[0:campvar] in campaign_name:
             return True
        elif self.value.find("visited_") == 0:
            temparg=System(None,self.value[8:])#FIXME hellcatv added
            if temparg.isValid():
                return True
        elif self.value=="menesch_dead" or self.value=="jones_dead" or self.value=="awacs_escort":
            return True
        self.warn("%s is not valid"%self.value)
        return False

class Faction(Argument):

    NAME="Faction"

    def checkValidity(self):
        if self.value not in VS._factions:
            self.warn("'%s' is not a valid faction"%self.value)
            return False
        return True
class FactionTuple(Argument):

    NAME="Faction"

    def checkValidity(self):
        if type(self.value)!=type(()) and type(self.value)!=type([]):
            self.warn(str(self.value)+" is not a tuple")
            return False
        for value in self.value:
            if value not in VS._factions:
                self.warn("'%s' is not a valid faction"%value)
                return False
        return True

class DynFG(String):
    NAME="DynFG"
class DynFGTuple(String):
    NAME="DynFG"
    def checkValidity(self):
        if type(self.value)!=type(()) and type(self.value)!=type([]):
            self.warn(str(self.value)+" is not a tuple");
            return False;
        return True


class ShipFactionPair(Argument):

    NAME="ShipFactionPair"

    def checkValidity(self):
        if not isinstance(self.value,tuple):
            self.warn("is not a tuple")
            return False
        if len(self.value) != 2:
            self.warn("%s does not have a length of 2")
            return False
        if self.value[0] not in faction_ships.stattable.keys():
            self.warn("%s is not a valid shiptype")%str(self.value[0])
            return False
        if self.value[1] not in VS._factions:
            self.warn("%s is not a valid faction")%str(self.value[1])
            return False
        return True

class FactionList(Argument):

    NAME="FactionList"

    def checkValidity(self):
        if type(self.value)==type(""):
            self.value=[self.value]
        for val in self.value:
            if val not in VS._factions:
                self.warn("%s is not a valid faction"%val)
                return False
        return True

class ShipType(Argument):

    NAME="ShipType"

    def checkValidity(self):
        if self.value not in faction_ships.stattable.keys():
            self.warn(str(self.value)+" ship type unknown")
            return False
        return True

class ShipTypeList(Argument):

    NAME="ShipTypeList"

    def checkValidity(self):
        if type(self.value)==type(""):
            self.value=[self.value]
        for val in self.value:
            if val not in faction_ships.stattable.keys():
                self.warn("%s is not a valid shiptype"%val)
                return False
        return True
class TextListTuple(Argument):
    NAME="TupleTextList"
    def checkValidity(self):
        try:
            for value in self.value:
                temparg=TextList(None,value)
                if temparg.isValid():
                    return True
        except:
            self.warn (str(self.value)+" is not iterable")
            return False
class TextList(Argument):

    NAME="TextList"

    def checkValidity(self):
        if not isinstance(self.value,list):
            self.warn(str(self.value)+"is not a list type")
            return False
        for item in self.value:
            if not isinstance(item,str):
                if isinstance(item,tuple):
                    if len(item) == 2:
                        if not isinstance(item[0],str) and isinstance(item[1],type(bool(True))):
                            self.warn("item is not a (str,bool) pair")
                            return False
                    elif len(item) == 3:
                        if not (isinstance(item[0],str) and isinstance(item[1],type(bool(True))) and isinstance(item[2],str)):
                            self.warn("item %s is not a (str,bool,str) triplet"%str(item))
                            return False
                    elif len(item) == 0:
                        pass
                    else:
                        self.warn("item %s has too many entries"%str(item))
                        return False
                else:
                    self.warn("is not a list of strings")
                    return False
        return True

class Boolean(Argument):

    NAME="Boolean"

    def checkValidity(self):
        try:
            isbool = self.value == True or self.value == False
        except:
            isbool = False
        if not isbool:
            self.warn("is not a boolean type")
            return False
        return True

class Empty(Argument):

    NAME="Empty"

    def checkValidity(self):
        if self.value is list() or self.value is tuple() or self.value is str():
            return True
        return False

class MissionVerifier:

    MISSION_ARGS=[]

    def __init__(self, newargs):
        self.origargs = newargs
        self.args = self.MISSION_ARGS
        givenargs = False
        try:
            givenargs = not len(newargs) == 0
        except:
            pass
        if len(newargs) > len(self.args):
            raise RuntimeError("More arguments given than this object supports")
        if givenargs:
            for i in range(len(newargs)):
                self.args[i].set(newargs[i])

    def isValid(self):
        for a in self.args:
            if not a.isValid():
                print self.origargs
                return False
        return True

    def warn(self, text):
        print "Mission Warning: " + text

class AmbushVerifier(MissionVerifier):
    MISSION_ARGS=[SaveVar(),SystemTuple(System()),PositiveNumber(),FactionList(),PositiveInt(PositiveIntList()),ShipType(ShipTypeList(Empty()),Default()),DynFG(None,Default()),TextList(None,Default()),SystemTuple(None,Default()),Destination(None,Default()),Boolean(None,Default())]

class BountyLeaderVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(),PositiveInt(),PositiveInt(),Boolean(),PositiveInt(),Faction(),SystemTuple(None,Default()),SaveVar(None,Default()),DynFG(None,Default()),ShipType(Empty(),Default()),Boolean(None,Default()),DynFG(None,Default()),ShipType(ShipFactionPair(Empty()),Default()),TextList(None,Default()),Argument(None,Default())]#FIXME: last argument is leader_upgrades=[]

class BountyTroopVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(),PositiveInt(),PositiveInt(),Boolean(),PositiveInt(),Faction(),SystemTuple(None,Default()),SaveVar(None,Default()),DynFG(None,Default()),ShipType(Empty(),Default()),Boolean(None,Default()),DynFG(None,Default()),ShipType(ShipFactionPair(Empty()),Default()),TextList(None,Default()),Argument(None,Default()),PositiveInt(None,Default()),PositiveInt(None,Default()),PositiveInt(None,Default())]#FIXME: last argument is leader_upgrades=[]

class BountyVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(),PositiveInt(),PositiveInt(),Boolean(),PositiveInt(),Faction(),SystemTuple(None,Default()),SaveVar(None,Default()),DynFG(None,Default()),ShipType(Empty(),Default()),Boolean(None,Default()),TextList(None,Default())]

class CargoVerifier(MissionVerifier):
    MISSION_ARGS=[Faction(),PositiveInt(),PositiveInt(),PositiveInt(),PositiveInt(),Boolean(),PositiveInt(),Argument(),SystemTuple(None,Default()),SaveVar(None,Default())]#FIXME: Argument is cargo 'category'

class CleansweepVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(),PositiveInt(),PositiveNumber(),PositiveInt(),SystemTuple(),SaveVar(),PositiveInt(),PositiveInt(),PositiveFraction(),PositiveFraction(),Faction(FactionList()),Boolean(),Boolean()]

class CleansweepEscortVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(),PositiveInt(),PositiveNumber(),PositiveInt(),SystemTuple(),SaveVar(),PositiveInt(),PositiveInt(),PositiveFraction(),PositiveFraction(),Faction(),Boolean(),Boolean(),Faction(),TextList(None,Default()),PositiveInt(None,Default()),TextList(None,Default())]

class DefendVerifier(MissionVerifier):
    MISSION_ARGS=[Faction(),PositiveInt(),PositiveInt(),PositiveNumber(),PositiveNumber(),PositiveInt(),Boolean(),Boolean(),Faction(None,Default()),SystemTuple(None,Default()),SaveVar(None,Default()),DynFG(None,Default()),ShipType(Empty(),Default()),ShipType(Empty(),Default()),PositiveInt(None,Default()), TextList(None,Default())]
class DefendDroneVerifier(MissionVerifier):
    MISSION_ARGS=[ShipType(Empty()),Faction(),System(),ShipType(),Faction(), PositiveInt(),Faction(),SystemTuple(),SaveVar(Empty(),Default()),TextList(None,Default())]

class DirectionsVerifier(MissionVerifier):
    MISSION_ARGS=[SaveVar(),SystemTuple(None,Default()),Destination(None,Default())]

class EscortLocalVerifier(MissionVerifier):
    MISSION_ARGS=[Faction(),PositiveInt(),PositiveInt(),PositiveInt(),PositiveNumber(),PositiveInt(),Boolean(), Faction(Empty(),Default()),SystemTuple(Empty(),Default()),SaveVar(Empty(),Default()),DynFG(Empty(), Default()),ShipType(Empty(),Default()),DynFG(Empty(), Default()),ShipType(Empty(),Default()),TextList(String(),Default())]

class EscortVerifier(MissionVerifier):
    MISSION_ARGS=[Faction(),PositiveInt(), PositiveNumber(), PositiveNumber(), PositiveNumber(), PositiveInt(), ZeroInt(), PositiveInt(),SystemTuple(),SaveVar(Empty(), Default()),DynFG(Empty(), Default()), ShipType(Empty(), Default())]

class WrongEscortVerifier(MissionVerifier):
    MISSION_ARGS=[Faction(),PositiveInt(), PositiveNumber(), PositiveNumber(), PositiveInt(),SystemTuple(),SaveVar(Empty(), Default()),DynFG(Empty(), Default()), ShipType(None, Default()),SystemTuple(None,Default()),FactionTuple(None,Default()),DynFGTuple(None,Default()),ShipTypeList(None,Default()),TextListTuple(String(),Default()),SaveVar(Empty(),Default())]

class PatrolAmbushVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(), PositiveNumber(), PositiveInt(), SaveVar(),SystemTuple(),PositiveInt(),Faction(),PositiveInt(),ShipType(Empty(), Default()),DynFG(Empty(), Default()),TextList(String(), Default()),SystemTuple(Empty(), Default()),Destination(Empty(), Default())]

class PatrolEnemiesVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(), PositiveInt(), PositiveNumber(), PositiveInt(), SystemTuple(), SaveVar(),PositiveInt(),PositiveInt(),PositiveFraction(),PositiveFraction(),Faction(),Boolean()]

class PatrolVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(), PositiveInt(), PositiveNumber(), PositiveInt(), SystemTuple(), SaveVar()]

class PlunderVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(), Faction(), Argument(), PositiveNumber(), Argument(), SaveVar()]#FIXME: first Argument is cargo 'category', second is not used

class RescueVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(), PositiveInt(), Faction(), PositiveInt(), Faction(), DynFG(Empty(), Default()),SystemTuple(Empty(), Default()), SaveVar(Empty(), Default())]

class TailVerifier(MissionVerifier):
    MISSION_ARGS=[SaveVar(),PositiveInt(),SystemTuple(),PositiveNumber(),PositiveNumber(),Faction(),Faction(),DynFG(),DynFG(),PositiveInt(None,Default()),ShipType(Empty(),Default()),ShipType(Empty(),Default()),DynFG(None,Default()),DynFG(None,Default()),TextList(None,Default()),TextList(None,Default()),TextList(None,Default())]

class TripatrolVerifier(MissionVerifier):
    MISSION_ARGS=[PositiveInt(),PositiveInt(),PositiveNumber(),PositiveInt(),SystemTuple(),SaveVar(),PositiveInt(),PositiveInt(),PositiveFraction(),PositiveFraction(),Faction(FactionList()),Boolean(),Boolean(),TextList()]
class AmbushScan(MissionVerifier):
    MISSION_ARGS=[SaveVar(),SystemTuple(System()),PositiveNumber(),FactionList(),PositiveInt(PositiveIntList()),ShipType(ShipTypeList(Empty()),Default()),DynFG(None,Default()),TextList(None,Default()),SystemTuple(None,Default()),Destination(None,Default()),Boolean(None,Default()),String(),TextList(None,Default())]
