import quest
import VS
import fixers
from fixers import Conversation, Node, RootNode, SubNode
import Director

SAVE_KEY = "bobo_fixer"

REJECTED = 2
STARTED = 3
DELIVERED = 4
FAILED = 5
WARNEDOFF = 6
SUCCESS = 7

AMOUNTTOPAY = 5000

playa = VS.getPlayer()

def putSaveValue(value, key=SAVE_KEY):
    Director.eraseSaveData(playa.isPlayerStarship(),key,0)
    Director.pushSaveData(playa.isPlayerStarship(),key,value)
    print "Changing Save Value for key " + SAVE_KEY
    return 1

def getSaveValue(key=SAVE_KEY):
    if Director.getSaveDataLength(playa.isPlayerStarship(),key) > 0:
        return Director.getSaveData(playa.isPlayerStarship(),key,0)
    return 0

def payMe():
    playa.addCredits(AMOUNTTOPAY)
    putSaveValue(SUCCESS)
    return 1

def takeCargo():
    c = playa.GetCargo("PolyDuranium")
    if c.GetQuantity() > 0:
        c.SetQuantity(0)
        putSaveValue(DELIVERED)
        return 1
    return 0

def loadCargo():
    playa.addCargo(VS.GetMasterPartList().GetCargo("PolyDuranium"))

def isAt(sysstring):
    syss = sysstring.split('/')
    sy = syss[0] + '/' + syss[1]
    un = ''
    if len(syss)>2:
        un = syss[2]
    if sy != VS.getSystemFile():
        return 0
    if un:
        iter = VS.getUnitList()
        while iter.notDone():
            if (playa.isDocked(iter.current()) or iter.current().isDocked(playa)):
                return iter.current().getName() == un
            iter.advance()
        return 0
    return 1

def cargoIterate(ref="Root"):
    if ref == "accept":
        loadCargo()
        putSaveValue(STARTED)
    elif ref == "reject":
        putSaveValue(REJECTED)
    getCargoCon().iterate(ref)
        

def getCargoCon():
    con = Conversation( SAVE_KEY,
                        ['#\nimport quest_intro2\nresult = (quest_intro2.isAt(\"Crucible/Cephid_17/Dakar\") or quest_intro2.isAt(\"Crucible/Enyo\"))'],
                        "#\nimport quest_intro2\nquest_intro2.cargoIterate()" )
    roo = RootNode()
    roo.addSubNode(SubNode( "The mission robot.",
                            ['#\nimport quest_intro2\nresult = quest_intro2.isAt(\"Crucible/Cephid_17/Dakar\")'],
                            ['#\nimport quest_intro2\nquest_intro2.cargoIterate(\"irobo\")'],
                            "bases/fixers/hunter.spr",
                            "Talk to iRobo" ))
    roo.addSubNode(SubNode( "The receiving merchant.",
                            ['#\nimport quest_intro2\nresult = quest_intro2.isAt(\"Crucible/Enyo\")'],
                            ['#\nimport quest_intro2\nquest_intro2.cargoIterate(\"merchant\")'],
                            "bases/fixers/merchant.spr",
                            "Talk to the Merchant." ))
    con.addNode(roo)

    mer = Node()
    mer.addSubNode(SubNode( "Thankyou!  I've been waiting hours for this!",
                            [str(SAVE_KEY) + '#' + str(STARTED),
                             '#\nimport quest_intro2\nresult = quest_intro2.takeCargo()'] ))
    mer.addSubNode(SubNode( "You lost it!!!",
                            [str(SAVE_KEY) + '#' + str(STARTED),
                             '#\nimport quest_intro2\nresult = quest_intro2.putSaveValue(%s)'%str(FAILED)] ))
    mer.addSubNode(SubNode( "Filthy privateer, get out of my sight!",
                            [str(SAVE_KEY) + '#' + str(FAILED)] ))
    mer.addSubNode(SubNode( "Filthy privateer, get out of my sight!",
                            [str(SAVE_KEY) + '#' + str(WARNEDOFF)] ))
    mer.addSubNode(SubNode( "Thank you again!",
                            [str(SAVE_KEY) + '#' + str(DELIVERED)] ))
    mer.addSubNode(SubNode( "Thank you again!",
                            [str(SAVE_KEY) + '#' + str(SUCCESS)] ))
    mer.addSubNode(SubNode( "Hello!  I'm sorry I can't talk, I'm waiting for a very important package." ))
    con.addNode(mer, "merchant")

    rob = Node()
    rob.addSubNode(SubNode( "Go away before I change my mind.",
                            [str(SAVE_KEY) + '#' + str(WARNEDOFF)] ))
    rob.addSubNode(SubNode( "You did not complete the mission!  Leave now and you will not be penalised.",
                            [str(SAVE_KEY) + '#' + str(FAILED),
                             '#\nimport quest_intro2\nresult = quest_intro2.putSaveValue(%s)'%str(WARNEDOFF)] ))
    rob.addSubNode(SubNode( "Thanks you again.",
                            [str(SAVE_KEY) + '#' + str(SUCCESS)] ))
    rob.addSubNode(SubNode( "Thanks you.  Here's %s."%str(AMOUNTTOPAY),
                            [str(SAVE_KEY) + '#' + str(DELIVERED),
                             '#\nimport quest_intro2\nresult = quest_intro2.payMe()'] ))
    rob.addSubNode(SubNode( "Get going!",
                            [str(SAVE_KEY) + '#' + str(STARTED)] ))
    rob.addSubNode(SubNode( "You feel up to the mission yet?",
                            [str(SAVE_KEY) + '#' + str(REJECTED)],
                            ["bases/fixers/no.spr|#\nimport quest_intro2\nquest_intro2.cargoIterate(\"reject\")|Sorry, still sick.",
                             "bases/fixers/yes.spr|#\nimport quest_intro2\nquest_intro2.cargoIterate(\"accept\")|Much better."] ))
    rob.addSubNode(SubNode( "I need someone to deliver a package to Enyo, can you be of assistance?",
                            list(),
                            ["bases/fixers/no.spr|#\nimport quest_intro2\nquest_intro2.cargoIterate(\"reject\")|Sorry, I don't feel well.",
                             "bases/fixers/yes.spr|#\nimport quest_intro2\nquest_intro2.cargoIterate(\"accept\")|Yes."] ))
    con.addNode(rob, "irobo")

    acc = Node()
    acc.addSubNode(SubNode( "Great, take this package to the merchant in the Enyo system.  You'll find him easy enough, he pops up all over the place.",
                            list(),
                            list() ))
    con.addNode(acc,"accept")
    rej = Node()
    rej.addSubNode(SubNode( "Then quit bothering me!",
                            list(),
                            list() ))
    con.addNode(rej,"reject")
    return con
