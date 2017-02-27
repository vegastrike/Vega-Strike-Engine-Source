
import VS
import vsrandom
import string
import stardate
import debug

class SystemInformation:
  """A class the provides a query mechanism, for each
  instance, that will return the specified system's
  faction, name, or parent sector."""
  SECTOR_SYSTEM = 1
  SECTOR = 2
  SYSTEM = 3
  FACTION = 4

  def __init__(self, system=VS.getSystemFile()):
    self.system = system
    self.faction = VS.GetGalaxyFaction(self.system)

  def getInfo(self, inf_type=False):
    """Returns the information corresponding to the
    given class variable."""
    try:
      [sec, sys] = self.system.split('/')
    except:
      sec = self.system
      sys = self.system
    if not inf_type:
      return self.system
    elif inf_type == self.SECTOR:
      return sec
    elif inf_type == self.SECTOR_SYSTEM:
      return self.system
    elif inf_type == self.SYSTEM:
      return sys
    elif inf_type == self.FACTION:
      return self.faction
    else:
      raise ValueError("Invalid information type specified")

def LookupRealName(oldname,faction):
  import VS
  newname=VS.LookupUnitStat(oldname,faction,"Name")
  if len(newname)==0:
    return oldname
  return newname


class NewsTranslator:
  """Provides functions that handle the translation of a
  news item using a relevant event tuple."""
  STARDATE_TEXT = "\\\\\\\This story was first broadcast on: "

  def __init__(self, dynamic_data):
    self.dynamic_data = dynamic_data
    self.item = None
    self.vars = None
    self.rand_int = None

  def lookupInfo(self, var, tag):
    """Returns the information corresponding to the
    given var and tag pair."""
    try:
      return self.vars[var][tag]
    except:
      try:
        results = self.dynamic_data.getFactionData(self.vars[var]['faction'], tag)
      except:
        st = 'ERROR_%s_%s'%(var,tag)#this is in case there is a typo like 'docketat'
        debug.error(st)
        results=['ERROR_%s_%s'%(var,tag)]
      return results[self.rand_int % len(results)]

  def translateWord(self, word):
    """Determines if the word given is a variable set,
    and if so returns the substitute information."""
    if word.find('VAR_') == -1:
      return word
    try:
      [pre,var,tag] = word.split("_")
    except:
      return word
    pre = pre[:pre.find('VAR')]
    tagnopun = str()
    for letter in tag:
      if letter not in string.punctuation:
        tagnopun+=letter
      else:
        break
    return pre + self.formatText(self.lookupInfo(var, tagnopun)) + tag[len(tagnopun):]

  def translateItem(self, item, news_tuple, docked_faction):
    """Using the given item and information, returns a
    fully translated version of the item."""
    self.rand_int = news_tuple[NewsManager.RANDOM_INTEGER_INDEX]
    self.item = item[2].split()
    self.vars = dict()
    stardat_ = dict()
    if news_tuple[NewsManager.STARDATE_INDEX] == 0:
      stardat_['value'] = VS.getStarTime()
    else:
      stardat_['value'] = news_tuple[NewsManager.STARDATE_INDEX]
    self.vars['stardate'] = stardat_
    aggressor = dict()
    aggressor['faction'] = news_tuple[NewsManager.AGGRESSOR_INDEX]
    aggressor['FG'] = news_tuple[NewsManager.AGGRESSOR_FLIGHTGROUP_INDEX]
    aggressor['FGtype'] = LookupRealName(news_tuple[NewsManager.AGGRESSOR_SHIPTYPE_INDEX],aggressor['faction'])
    self.vars['aggressor'] = aggressor
    defender = dict()
    defender['faction'] = news_tuple[NewsManager.DEFENDER_INDEX]
    defender['FG'] = news_tuple[NewsManager.DEFENDER_FLIGHTGROUP_INDEX]
    defender['FGtype'] = LookupRealName(news_tuple[NewsManager.DEFENDER_SHIPTYPE_INDEX],defender['faction'])
    self.vars['defender'] = defender
    dockeda_ = dict()
    dockeda_['faction'] = docked_faction
    self.vars['dockedat'] = dockeda_
    system = SystemInformation(news_tuple[NewsManager.EVENT_SYSTEM_INDEX])
    syste_ = dict()
    syste_['system'] = system.getInfo(system.SYSTEM)
    syste_['sector'] = system.getInfo(system.SECTOR)
    self.vars['system'] = syste_
    for i in range(len(self.item)):
      self.item[i] = self.translateWord(self.item[i])
    return string.join(self.item) + self.STARDATE_TEXT + stardate.formatStarDate(self.vars['dockedat']['faction'],self.vars['stardate']['value']) + "\\INDY - Independent Daily Yarn"

  def formatText(self, text, punc=[' ' , '_' , '.'], capitalise=True):
    """Runs a quick formatting algorithm over the
    provided text, using the punc list as a guide to
    the markup."""
    for pun in punc:
      tex = text.split(pun)
      for i in range(len(tex)):
        if len(tex[i]) > 1:
          tex[i] = tex[i][0].capitalize() + tex[i][1:]
        else:
          tex[i] = tex[i].capitalize()
      text = string.join(tex)
    return text

class DynamicNewsData:
  """Each instance of this class acts as an accessor to
  the faction specific information stored for the purpose
  of translating news stories."""
  def __init__(self):
    import dynamic_news_content
    self.faction_dict = dynamic_news_content.allFactionNames()
    self.news_dict = dynamic_news_content.allNews()

  def getFactionData(self, faction, variable):
    """Return the variable information stored for this
    faction."""
    if variable in self.faction_dict["alltags"]:
      try:
        return self.faction_dict[faction][variable]
      except:
        # raise ValueError("Invalid Faction Specified")
        debug.error("ERROR: FACTION LOOKUP ERROR faction %s variable %s" % (faction, variable))
        return self.faction_dict['unknown'][variable]
    else:
      debug.error("ERROR: VARIABLE LOOKUP ERROR faction %s variable %s" % (faction, variable))
      return "VARIABLE LOOKUP ERROR"

  def translateKeyToDictionary(self, variable):
    """Translates the information from the stored
    values to those used to lookup items in the item
    dictionary."""
    replace = ""
    if variable == NewsManager.KEYWORD_DEFAULT:
      replace = "all"
    elif variable == NewsManager.TYPE_SIEGE:
      replace = "siege"
    elif variable == NewsManager.TYPE_EXPLORATION:
      replace = "exploration"
    elif variable == NewsManager.TYPE_BATTLE:
      replace = "battle"
    elif variable == NewsManager.TYPE_FLEETBATTLE:
      replace = "fleetbattle"
    elif variable == NewsManager.TYPE_DESTROYED:
      replace = "destroyed"
    elif variable == NewsManager.STAGE_BEGIN:
      replace = "start"
    elif variable == NewsManager.STAGE_MIDDLE:
      replace = "middle"
    elif variable == NewsManager.STAGE_END:
      replace = "end"
    elif variable == NewsManager.SUCCESS_WIN:
      replace = "success"
    elif variable == NewsManager.SUCCESS_DRAW:
      replace = "draw"
    elif variable == NewsManager.SUCCESS_LOSS:
      replace = "loss"
    elif variable == NewsManager.POV_GOOD:
      replace = "good"
    elif variable == NewsManager.POV_BAD:
      replace = "bad"
    elif variable == NewsManager.POV_NEUTRAL:
      replace = "neutral"
    else:
      raise TypeError("Unrecognised variable")
    return replace

  def makeNewsKeyList(self, news_list, news_faction, pov):
    """Creates a list of the structure used to store
    each news event."""
    key_list = list()
    key_list.append(news_faction)
    key_list.append(news_list[NewsManager.EVENT_TYPE_INDEX])
    key_list.append(news_list[NewsManager.EVENT_STAGE_INDEX])
    key_list.append(self.translateKeyToDictionary(news_list[NewsManager.AGGRESSOR_SUCCESS_INDEX]))
    key_list.append(self.translateKeyToDictionary(pov))
    return key_list

  def getNewsList(self, key_list, get_neutral=False):
    """Searches the item dictionary to find matching
    items for this given event."""
    story_list = self.news_dict
    try:
      if get_neutral:
        key_list[0] = "neutral"
      for key in key_list:
        story_list = story_list[key]
    except:
      return list()
    return story_list

  def getBestMatch(self, stories, varlist):
    """From the provided list of stories, return the
    item who's \"scale\" most closely matches that of
    the given event (minimise variance)."""
    kw_stories = list()
    for story in stories:
      if story[1] == varlist[NewsManager.EVENT_KEYWORD_INDEX]:
        kw_stories.append(story)
    if not len(kw_stories):
      debug.error("ERROR: NO KEYWORD STORIES AVAILABLE FOR "+str(varlist))
      return False
    if len(kw_stories) == 1:
      return kw_stories[0]
    scale_stories = list()
    scale_stories.append(kw_stories[0])
    diff = abs(int(1000*scale_stories[0][0]) - int(1000*varlist[NewsManager.EVENT_SCALE_INDEX]))
    kw_stories.pop(0)
    for story in kw_stories:
      if abs(int(1000*scale_stories[0][0]) - int(1000*varlist[NewsManager.EVENT_SCALE_INDEX])) < diff:
        scale_stories = list()
        scale_stories.append(story)
      elif abs(int(1000*scale_stories[0][0]) - int(1000*varlist[NewsManager.EVENT_SCALE_INDEX])) == diff:
        scale_stories.append(story)
    return scale_stories[varlist[NewsManager.RANDOM_INTEGER_INDEX] % len(scale_stories)]

class NewsManager:
  """This class is used to manage dynamic news, it is
  designed to be used as a global object, but can be used
  otherwise."""

  RANDOM_INTEGER_INDEX = 0
  STARDATE_INDEX = 1
  EVENT_TYPE_INDEX = 2
  EVENT_STAGE_INDEX = 3
  AGGRESSOR_INDEX = 4
  DEFENDER_INDEX = 5
  AGGRESSOR_SUCCESS_INDEX = 6
  EVENT_SCALE_INDEX = 7
  EVENT_SYSTEM_INDEX = 8
  EVENT_KEYWORD_INDEX = 9
  AGGRESSOR_FLIGHTGROUP_INDEX = 10
  AGGRESSOR_SHIPTYPE_INDEX = 11
  DEFENDER_FLIGHTGROUP_INDEX = 12
  DEFENDER_SHIPTYPE_INDEX = 13

  KEYWORD_DEFAULT = "all"

  TYPE_SIEGE = "siege"
  TYPE_EXPLORATION = "exploration"
  TYPE_BATTLE = "battle"
  TYPE_FLEETBATTLE = "fleetbattle"
  TYPE_DESTROYED = "destroyed"

  STAGE_BEGIN = "start"
  STAGE_MIDDLE = "middle"
  STAGE_END = "end"

  SUCCESS_WIN = '1'
  SUCCESS_DRAW = '0'
  SUCCESS_LOSS = '-1'

  POV_GOOD = 5
  POV_BAD = 6
  POV_NEUTRAL = 7

  POV_CUTOFF = 0.25

  def __init__(self):
    self.dockedat_faction = None
    self.updateDockedAtFaction()
    self.data = DynamicNewsData()
    self.translator = NewsTranslator(self.data)

  def translateDynamicString(self, strin):
    """Takes an argument, of type str (this is not checked),
    that is of the same format as that stored by
    self.writeDynamicString(varlist)"""
    varlist = self.sTovarlist(strin)
    if varlist[self.AGGRESSOR_FLIGHTGROUP_INDEX] == VS.getPlayer().getFlightgroupName():
      varlist[self.EVENT_KEYWORD_INDEX] = "player"
    keys = self.data.makeNewsKeyList(varlist, self.dockedat_faction, self.getPOV(varlist))
    stories = self.data.getNewsList(keys)
    if not len(stories):
      stories = self.data.getNewsList(keys, True)
    if not len(stories):
      return False
    item = self.data.getBestMatch(stories, varlist)
    if item:
      return self.translator.translateItem(item, varlist, self.dockedat_faction)
    else:
      return False

  def getPOV(self, varlist):
    """Returns the corresponding POV_* class variable
    for the reaction of the dockedat faction to the status
    of the event."""
    relatdef = VS.GetRelation(self.dockedat_faction,varlist[self.DEFENDER_INDEX])
    relatagg = VS.GetRelation(self.dockedat_faction,varlist[self.AGGRESSOR_INDEX])
    success = varlist[NewsManager.AGGRESSOR_SUCCESS_INDEX]
    if (relatdef <= -self.POV_CUTOFF and relatagg <= -self.POV_CUTOFF) or (relatdef >= self.POV_CUTOFF and relatagg >= self.POV_CUTOFF):
      return self.POV_NEUTRAL
    elif relatdef > relatagg:
      if success == self.SUCCESS_WIN:
        return self.POV_BAD
      elif success == self.SUCCESS_LOSS:
        return self.POV_GOOD
      elif success == self.SUCCESS_DRAW:
        return self.POV_GOOD
    elif relatdef < relatagg:
      if success == self.SUCCESS_WIN:
        return self.POV_GOOD
      elif success == self.SUCCESS_LOSS:
        return self.POV_BAD
      elif success == self.SUCCESS_DRAW:
        return self.POV_BAD
    else:
      debug.debug("ERROR:  VS is returning -0 for relationship relatagg number")
      return self.POV_NEUTRAL

  def sTovarlist(self, s):
    """Converts a stored dynamic news string into a
    variable list usable by other methods and classes."""
    varlist = s.split(',')
    varlist[self.RANDOM_INTEGER_INDEX] = int(varlist[self.RANDOM_INTEGER_INDEX])
    varlist[self.STARDATE_INDEX] = float(varlist[self.STARDATE_INDEX])
    varlist[self.EVENT_SCALE_INDEX] = float(varlist[self.EVENT_SCALE_INDEX])
    return varlist

  def updateDockedAtFaction(self):
    """Updates the current self.dockedat_faction to its
    current value.  Should be called before translating
    a batch of stores."""
    i = VS.getUnitList()
    playa=VS.getPlayer()
    while i.notDone():
      un = i.current()
      i.advance()
      if (un.isDocked(playa) or playa.isDocked(un)):
        if not (un.isPlanet() or (un.getFactionName() == "neutral")):
          fac = un.getFactionName()
          # debug.debug('returning '+un.getName()+' s faction as '+fac+' from flightgroup '+un.getFlightgroupName())
          self.dockedat_faction = fac
        break
    retfac = VS.GetGalaxyFaction(VS.getSystemFile())
    # debug.debug("Returning " + retfac + " as the systems faction")
    self.dockedat_faction = retfac

  def isStoryRelevant(self, strin):
    """Is the event in this string relevant to the current
    system and dockedat faction?"""
    varlist = self.sTovarlist(strin)
    limit = False
    if varlist[self.EVENT_TYPE_INDEX] in [self.TYPE_BATTLE, self.TYPE_DESTROYED]:
      limit = 1
    else:
      return True
    event_sys = varlist[self.EVENT_SYSTEM_INDEX]
    syslist = [VS.getSystemFile()]
    done_syslist = list()
    while limit >= 0:
      if event_sys in syslist:
        return True
      else:
        done_syslist+=syslist
        new_syslist = list()
        for syst in syslist:
          for i in range(VS.GetNumAdjacentSystems(syst)):
            sy = VS.GetAdjacentSystem(syst,i)
            if sy not in done_syslist:
              new_syslist.append(sy)
        syslist = new_syslist
        limit-=1
    return False

  def writeDynamicString(self, varlist):
    """Stores a news story list into the \"dynamic news\"
    key in the save game."""
    varlist = string.join([str(vsrandom.randrange(0,4194304))]+varlist,',')
    import Director        
    Director.pushSaveString(0,"dynamic_news",varlist)

