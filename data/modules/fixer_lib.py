import Base
import Director
#import code #Doesn't seem to be needed...
import VS
from quest import checkSaveValue
global b
b=1
# To be merged with fixers.py???
#
# How to di it all:
# + first add the fixer to the (unique) queue, this should be the string to be
#   executed to create the \'Fixer\' object.
# + when getting fixers, first get campaign fixers, then add quest fixers...
#   print (or throw?)error if too many
# + each node should have seperate preconditions, each 'ref' value in iterateConversations
#   will then be a hash seperated string of various possible nodes, with preconditions evaluated
#   linearly until the last...which is accepted without preconditions evaluated.
# + The 'Root' node is in fact just the preconditions for displaying the fixer and, where
#   the text would be, just the reference list of possible starting points for the conversation.
#   The rest of the data in this string is ignored.
# Library to help in the creation of fixers

#from fixers import Fixer


def getNewInfo(conversation, ref="Root"):
    """Returns two strings, the first is the text for the node, the second is
    the choices."""
    text, choices, sprite, motext = getAllNewInfo(conversation, ref)
    return text, choices

def getNode(conversation, ref="Root"):
    """Gets the node in the conversation corresponding to \'ref\'"""
    for node in conversation:
        if node.split('^')[0].split('|')[0] == ref:
            return node
    raise RuntimeError("Node with reference \'%s\' not found in conversation."%ref)

def getPreconditions(conversation):
    """Gets the preconditions for the conversation."""
    pre = getNode(conversation).split('^')[0].split('|')
    if len(pre) < 4:
        return str()
    else:
        return pre[3]

def getAllNewInfo(conversation, ref="Root"):
    """Retrieves the suitable text and choices, as well as the sprite and
    mouseover text."""
    segs = getNode(conversation, ref).split('^')[1:]
    default = segs.pop()
    use = False
    for seg in segs:
        sseg = seg.split('|')
        if evaluateConditions(sseg[0]):
            use = sseg
            break
    if not use:
        use = default.split('|')
    while len(use) < 5:
        use.append(str())
    return use[1], use[2], use[3], use[4]

def getInitialInfo(conversation):
    """Retrieves the initial starting objects required for the CFixer class.
    This is the starting text, choices, and preconditions.  text and choices
    are both strings, but the preconditions returned may be a list."""
    name = getNode(conversation).split('^')[0].split('|')[1]
    whenclicked = getNode(conversation).split('^')[0].split('|')[2]
    text, choices, sprite, motext = getAllNewInfo(conversation)
    precon = getPreconditions(conversation)
    return text, whenclicked, sprite, motext, name, precon

def evaluateCondition(condition):
    """Takes a string representing the condition, and returns whether it is
    met or not."""
    if type(condition) != str:
        raise TypeError("The condition is not a valid type.")
    elif condition == str():
        return 1
    elif condition[0] == '#':
        tempd = dict()
        exec condition[1:] in tempd
        print "Custom condition code, returning: " + str(tempd["result"])
        return tempd["result"]
    else:
        con = condition.split('#')
        print "Checking \'%s : %s\'"%(con[0],con[1])
        print "Returning: " + str(checkSaveValue(VS.getCurrentPlayer(),con[0],int(con[1])))
        return checkSaveValue(VS.getCurrentPlayer(),con[0],int(con[1]))

def evaluateConditions(conditions):
    """Takes a list of conditions, and evaluates them."""
    for cond in conditions:
        if not evaluateCondition(cond):
            return 0
    return 1

def setB(value):
    global b
    b=value

def sampleNode():
    return "Node2|#a==2^#b==1|\"Aar!  This ale is mighty fine.  You there, walk the plank!\"|choices|sprite|mouseover^#b==2|\"Land Ahoy!\"|\"bases/fixers/no.spr\"~\"bases/fixers/iso_decline.py\"~\"F Off!\"^\"bases/fixers/yes.spr\"~\"bases/fixers/iso_accept.py\"~\"Sure!\"|sprite|mouseover"

def sampleCon():
    return ["Node2^#b==1|\"Aar!  This ale is mighty fine.  You there, walk the plank!\"|choices^#b==2|\"Land Ahoy!\"|choices","Root|#a==2^#b==1|\"Aar!  This RootBeer is mighty fine.  You are scummvm.\"|choices|sprite|mouseover^#b==2|\"Land AhoyRoot!\"|choices2|sprite2|mouseover2"]





#Root Node:
# + Conversation preconditions
# + Text contains strin reference to possible starting nodes
# + 
#Root|pirate.spr|"A Pirate"^preconditions^n1|n2|n3^
#
#
#Node:
# + Primary preconditions (is this node possible)
# + Secondary preconditions (what specific text makes sense...ie, no sense talking about an event that hasn't happened yet)
# + Each set of secondary precons has a related text node and choices set (and maybe sprite?)
#
#n1|pirate_angry.spr|"An Angry Pirate"^ ......... the segments would only occur for the topmost nodes, where it is possible to alter the initial appearance still.
#
#you would normally have:
#
#The node structure:
#
#n2|prim_preconditions^sec_precons|"Aar!  This ale is mighty fine.  You there, walk the plank!"|choices|sprite|mouseover^sec_precons|"Land Ahoy!"|choices|sprite|mouseover
#
#Segments are seperated by the '^' character, elements (within each segment) by the '|'
#The first segment has two elements, the first is the reference, the second the precondition for node activation.  This precondition, if it exists, will be ignored for references not equal to 'Root'.
#Each additional segment (minimum of 1) contains a possible value for this node.  Each (in the order the are written) have their preconditions evaluated.  The first to get the 'all clear' becomes this node, the text and choices are evaluated (and if the reference of the node is 'Root', then so are the sprite and mouseover values) and used.
#
#
#class Conversation:
#    """Takes a string of the content making up a fixer converstion, and
#    provides various methods to access and display the interaction."""
#    
#    def __init__(self, content):
#do error check for type 'content' etc
#        self.content = content.split()
#        self.content.sort()
#        pass
#
#    def getNode(ref="Root"):
#    """Gets the conversation node with a reference of \'ref\'."""
#do error check for type etc
#
#    def getNextNode(ref, result):
#    """Gets the next conversation node after \'ref\' given the result
#    of \'result\'."""
