import quest
import VS
import fixers
from fixers import Conversation, Node, RootNode, SubNode
import Director

TRIGGER_SAVE = "IntroJP"
TRIGGER_VALUE = 1
TRIGGER_MESSAGE = ["[Reminder]","Jenek on Wiley may have a jump drive going cheap."]

DEFAULT_VALUE = 0
REFUSED_VALUE = 2
DONE_VALUE = 3

JP_DRV_PRICE = 7000

DRV_SUCCESS = 1
DRV_FAIL = 2

JP_DISTANCE = 10000

FIXER_NAME = "IntroFixer"
FIXER_TEXT = "#\nimport quest_intro\nconversation = quest_intro.getJenekConversation()"

class quest_introduction_factory(quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_introduction")
    def create (self):
        return quest_introduction()
        
class quest_introduction(quest.quest):

    def __init__(self):
        self.playa = VS.getPlayer()
        self.jps = list()
        self.refreshJumpPoints()
        Director.eraseSaveData(self.playa.isPlayerStarship(),TRIGGER_SAVE,0)
        Director.pushSaveData(self.playa.isPlayerStarship(),TRIGGER_SAVE,DEFAULT_VALUE)
#I work on the assumption that only one value exists in the TRIGGER_SAVE var,
# thus before writing to it, you must delete the first value :-)

    def refreshJumpPoints(self):
        """Updates the jumppoint list \'self.jps\' used by \'self.checkDistances()\'."""
        self.jps = list()
#This is how to use the unit iterator.  *Very useful*
        iter = VS.getUnitList()
        while iter.notDone():
            if iter.current().isJumppoint():
                self.jps.append(iter.current())
            iter.advance()
    
    def checkDistances(self):
        """Determines whether \'self.playa\' (the player) is close enough to
        a jumppoint to trigger the IOMessage."""
#You will find yourself doing this a lot :-)
        for jp in self.jps:
            if self.playa.getDistance(jp) <= JP_DISTANCE:
                return 1
        return 0
    
    def Execute(self):
        if self.checkDistances() and quest.checkSaveValue(self.playa.isPlayerStarship(),TRIGGER_SAVE,DEFAULT_VALUE):
            VS.IOmessage (0,TRIGGER_MESSAGE[0],"all",TRIGGER_MESSAGE[1])
            Director.eraseSaveData(self.playa.isPlayerStarship(),TRIGGER_SAVE,0)
            Director.pushSaveData(self.playa.isPlayerStarship(),TRIGGER_SAVE,TRIGGER_VALUE)
        elif quest.checkSaveValue(self.playa.isPlayerStarship(),TRIGGER_SAVE,DONE_VALUE):
            Director.eraseSaveData(self.playa.isPlayerStarship(),TRIGGER_SAVE,0)
            fixers.eraseCFixer(self.playa.isPlayerStarship(), FIXER_NAME)
            self.removeQuest()
#You only want to remove the quest once the fixer has finished doing his thing :-)
            return 0
        return 1


def buyDrive():
    if haveDrive():
        return DRV_FAIL
    elif VS.getPlayer().getCredits()>=JP_DRV_PRICE:
#deducts the required sum
        VS.getPlayer().addCredits(-JP_DRV_PRICE)
#adds the jumpdrive temporarily (will be removed if player upgrades)
        VS.getPlayer().upgrade("jump_drive",0,0,1,0)
#adds the jumpdrive to cargo, making it a permenant upgrade
        VS.getPlayer().addCargo(VS.GetMasterPartList().GetCargo("jump_drive"))
        print "TEST: Bought Jump Drive"
        Director.eraseSaveData(VS.getPlayer().isPlayerStarship(),TRIGGER_SAVE,0)
        Director.pushSaveData(VS.getPlayer().isPlayerStarship(),TRIGGER_SAVE,DONE_VALUE)
        return DRV_SUCCESS
    return 0

def haveDrive():
    """Does the player already have a jumpdrive?"""
    if VS.getPlayer().GetJumpStatus()>-2:
# when -2 is returned by GetJumpStatus, the player doesn't have a jumpdrive
        print "TEST: You have a jump drive already!"
        return 1
    return 0

def okayDrawJenek():
    """Checks if the player is docked to the right planet, and the fixer is still \'around\'."""
    if not quest.checkSaveValue(VS.getPlayer().isPlayerStarship(),TRIGGER_SAVE,DONE_VALUE):
        iter = VS.getUnitList()
        while iter.notDone():
            if iter.current().isPlanet() and (VS.getPlayer().isDocked(iter.current()) or iter.current().isDocked(VS.getPlayer())):
#Not sure why both have to be checked, it seems to second gives a more consistantly correct response
                return iter.current().getName() == 'Wiley'
            iter.advance()
    return 0

def getJenekConversation():
    """Returns the conversation object for the Jenek fixer."""
    con = Conversation(FIXER_NAME, ['#\nimport quest_intro\nresult = quest_intro.okayDrawJenek()'])

    roo = RootNode()
    roo.addSubNode(SubNode( "knownjenek",
                            [str(TRIGGER_SAVE) + '#' + str(TRIGGER_VALUE)],
                            ["#\nimport quest_intro\nquest_intro.interactWithJenek(\"jenek\")"],
                            "bases/fixers/merchant.spr",
                            "Talk to Jenek about the jump drive." ))
    roo.addSubNode(SubNode( "unknownjenek",
                            list(),
                            ["#\nimport quest_intro\nquest_intro.interactWithJenek(\"jenek\")"],
                            "bases/fixers/merchant.spr",
                            "Talk to Jenek." ))
    con.addNode(roo)

    jen = Node()
    jen.addSubNode(SubNode( "Ahh, that's right.  You're here about the jump drive!",
                            [str(TRIGGER_SAVE) + '#' + str(TRIGGER_VALUE)],
                            ["bases/fixers/no.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"nojump\")|Sorry, what else do you have?",
                             "bases/fixers/yes.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"yesjump\")|Yeah, I'm interested."],
                            "bases/fixers/merchant.spr",
                            "Talk to Jenek about the jump drive." ))
    jen.addSubNode(SubNode( "So you're interested in the jump drive now?",
                            [str(TRIGGER_SAVE) + '#' + str(REFUSED_VALUE)],
                            ["bases/fixers/no.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"nojump\")|Sorry.",
                             "bases/fixers/yes.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"yesjump\")|Yeah, I'm interested."],
                            "bases/fixers/merchant.spr",
                            "Talk to Jenek about the jump drive." ))
    jen.addSubNode(SubNode( "Well, what are you waiting for?  Go and give it a spin!",
                            ["#\nimport quest_intro\nresult = quest_intro.haveDrive()"] ))
    jen.addSubNode(SubNode( "You're back!  Quite a party that was the other night, if it wasn't for my reminder I would have forgotten you!  You still interested in that jump drive I've got?",
                            list(),
                            ["bases/fixers/no.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"forgetjump\")|Sorry, I have no idea what you are talking about!",
                             "bases/fixers/yes.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"yesjump\")|I completely forgot myself!  Of course I'm interested."],
                            "bases/fixers/merchant.spr",
                            "Talk to Jenek." ))
    con.addNode(jen,"jenek")

    forg = Node()
    forg.addSubNode(SubNode(    "Well, I can't say I'm surprised ... not with the amount of Tripezian Premium you downed!  Anyway, I've come across a few \'spare\' jump drives, and your ship, last you spoke about it, was one short.  Are you still as interested as you were then?",
                            list(),
                            ["bases/fixers/no.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"nojump\")|Sorry, don't suppose you've got anything else?",
                             "bases/fixers/yes.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"yesjump\")|Yeah, I'm interested."] ))
    con.addNode(forg, "forgetjump")

    yes = Node()
    yes.addSubNode(SubNode( "Great!  I can let you have one for just %s.  Do you want it now?"%str(JP_DRV_PRICE),
                            list(),
                            ["bases/fixers/no.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"rejectdrive\")|Maybe later.",
                             "bases/fixers/yes.spr|#\nimport quest_intro\nquest_intro.interactWithJenek(\"takedrive\")|Okay."] ))
    con.addNode(yes, "yesjump")

    no = Node()
    no.addSubNode(SubNode(  "If it's not about the jump drive I don't have the time.  Come back later." ))
    con.addNode(no, "nojump")

    tak = Node()
    tak.addSubNode(SubNode( "I'll send some mechanics to fit it for you now.  Nice to see you again!",
                            ["#\nimport quest_intro\nresult = quest_intro.buyDrive()==%s"%str(DRV_SUCCESS)] ))
    tak.addSubNode(SubNode( "You idiot, you already have a jumpdrive!",
                            ["#\nimport quest_intro\nresult = quest_intro.buyDrive()==%s"%str(DRV_FAIL)] ))
    tak.addSubNode(SubNode( "Sorry, it looks like your account is a little dry.  Come back when you've got %s credit."%str(JP_DRV_PRICE) ))
    con.addNode(tak, "takedrive")

    rej = Node()
    rej.addSubNode(SubNode( "Fine.  I may not be around for much longer though." ))
    con.addNode(rej, "rejectdrive")

    return con

def interactWithJenek(ref="Root"):
    """Iterates the conversation, allowing for extra events to be launched
    during the conversation."""
    if ref == "nojump" or ref == "rejectdrive":
        Director.eraseSaveData(VS.getPlayer().isPlayerStarship(),TRIGGER_SAVE,0)
        Director.pushSaveData(VS.getPlayer().isPlayerStarship(),TRIGGER_SAVE,REFUSED_VALUE)
    getJenekConversation().iterate(ref)
