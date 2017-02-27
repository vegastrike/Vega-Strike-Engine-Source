import VS
import vsrandom
import debug
BATTLELIST=0
THREATLIST=BATTLELIST
PEACELIST=1
PANICLIST=2
VICTORYLIST=3
LOSSLIST=4

BATTLESITUATION=0
THREATSITUATION=1
PEACESITUATION=2
PANICSITUATION=3
VICTORYSITUATION=4
LOSSSITUATION=5

situation=PEACELIST

factionmap={
    "confed":"terran",
    "aera":"aera",
    "rlaan":"rlaan",
    "merchant":"terran",
    "luddites":"iso",
    "pirates":"iso",
    "hunter":"iso",
    "homeland-security":"terran",
    "ISO":"iso",
    "unknown":"unknown",
    "andolian":"terran",
    "highborn":"terran",
    "shaper":"terran",
    "unadorned":"terran",
    "purist":"terran",
    "forsaken":"terran",
    "LIHW":"iso",
    "uln":"rlaan",
    "dgn":"terran",
    "klkk":"terran",
    "mechanist":"terran",
    "shmrn":"terran",
    "rlaan_briin":"rlaan"
    }

HOSTILE_AUTODIST=10000
HOSTILE_NEWLAUNCH_DISTANCE=20000
SIGNIFICANT_DISTANCE_OFFSET=10000
SIGNIFICANT_DISTANCE_MULTIPLY=2
peaceawayfriendlylist={
    "aera":VS.musicAddList('playlists/peace/away/aera_friend.m3u'),
    "rlaan":VS.musicAddList('playlists/peace/away/rlaan_friend.m3u'),
    "terran":VS.musicAddList('playlists/peace/away/terran_friend.m3u'),
    "iso":VS.musicAddList('playlists/peace/away/iso_friend.m3u'),
    None:PEACELIST
    }
peaceawayhostilelist={
    "aera":VS.musicAddList('playlists/peace/away/aera_hostile.m3u'),
    "rlaan":VS.musicAddList('playlists/peace/away/rlaan_hostile.m3u'),
    "terran":VS.musicAddList('playlists/peace/away/terran_hostile.m3u'),
    "iso":VS.musicAddList('playlists/peace/away/iso_hostile.m3u'),
    None:PEACELIST
    }
peacearoundsiglist={
    "aera":VS.musicAddList('playlists/peace/around_sig/aera.m3u'),
    "rlaan":VS.musicAddList('playlists/peace/around_sig/rlaan.m3u'),
    "terran":VS.musicAddList('playlists/peace/around_sig/terran.m3u'),
    "iso":VS.musicAddList('playlists/peace/around_sig/iso.m3u'),
    None:PEACELIST
    }
threatlist={
    "aera":VS.musicAddList('playlists/threat/aera.m3u'),
    "rlaan":VS.musicAddList('playlists/threat/rlaan.m3u'),
    "terran":VS.musicAddList('playlists/threat/terran.m3u'),
    "iso":VS.musicAddList('playlists/threat/iso.m3u'),
    None:THREATLIST
    }
battlelist={
    "aera":VS.musicAddList('playlists/battle/aera.m3u'),
    "rlaan":VS.musicAddList('playlists/battle/rlaan.m3u'),
    "terran":VS.musicAddList('playlists/battle/terran.m3u'),
    "iso":VS.musicAddList('playlists/battle/iso.m3u'),
    None:BATTLELIST
    }
paniclist={
    "aera":VS.musicAddList('playlists/battle/aera.m3u'),
    "rlaan":VS.musicAddList('playlists/battle/rlaan.m3u'),
    "terran":VS.musicAddList('playlists/battle/terran.m3u'),
    "iso":VS.musicAddList('playlists/battle/iso.m3u'),
    None:PANICLIST
    }
asteroidlist=VS.musicAddList('asteroids.m3u')

def LookupTable(list,faction):
    if faction in factionmap:
        faction_name = factionmap[faction]
    else:
        faction_name = faction
    if faction_name in list:
        if (list[faction_name]!=-1):
            return list[faction_name]
        else:
            return list[None]
    else:
        return list[None]

__enabled = True

def enable():
    global __enabled
    if not __enabled:
        debug.debug('DJ enabled.')
    __enabled = True

def disable():
    global __enabled
    if __enabled:
        debug.debug('DJ disabled.')
    __enabled = False

def mpl (list,newsituation,forcechange):
    global situation
    debug.debug("SITUATION IS "+str( situation)+" force change "+str(forcechange) + " bool "+ str(forcechange or newsituation!=situation))
    if (forcechange or newsituation!=situation):
        debug.debug("SITUATION IS RESET TO "+str( newsituation))
        situation=newsituation
        VS.musicPlayList(list) 

def PlayMusik(forcechange=1,hostile_dist=0):
    un = VS.getPlayer()
    if not __enabled:
        return
    elif not un:
        mpl (PEACELIST,PEACELIST,forcechange)
        debug.debug("Ppeace")
    elif un.DockedOrDocking() not in [1,2]:
        perfect=1
        threat=0
        around_sig=0
        iter = VS.getUnitList()
        target = iter.current()
        unfcount={}
        siglist=[]
        asteroid=0
        while (iter.notDone()):
            if (target):
                ftmp = 2*target.getRelation(un)
                nam=target.getName().lower()
                fact=target.getFactionName()
                if un.getSignificantDistance(target)<=2*target.rSize() and ('afield'==nam[:6] or 'asteroid'==nam[:8]):
                    asteroid=1
                hdis = HOSTILE_AUTODIST
                if (hostile_dist!=0):
                    hdis = hostile_dist
                if (target.GetTarget()==un or (ftmp<0 and un.getDistance(target)<hdis)):
                    unfcount[fact] = unfcount.get(fact,0)+1
                    perfect=0
                    if ((target.GetTarget()==un) and (ftmp<0 and un.getDistance(target)<hdis)):
                        threat=1
                if (target.isSignificant() and (un.getSignificantDistance(target)<(SIGNIFICANT_DISTANCE_OFFSET+SIGNIFICANT_DISTANCE_MULTIPLY*target.rSize()))):
                    around_sig=1
            iter.advance()
            target=iter.current()
        if (perfect):
            if asteroid and asteroidlist!=-1 and vsrandom.random()<.7:
                mpl(asteroidlist,PEACELIST,forcechange)
                return
            sys=VS.getSystemFile()
            fact=VS.GetGalaxyFaction(sys)
            ufact=un.getFactionName()
            if (around_sig):
                mpl(LookupTable(peacearoundsiglist,fact),PEACESITUATION,forcechange)
                debug.debug("peace-around-significant " + fact)
            else:
                if (VS.GetRelation(ufact,fact)>=0):
                    mpl(LookupTable(peaceawayfriendlylist,fact),PEACESITUATION,forcechange)
                    debug.debug("peace-away-frienly " + fact)
                else:
                    mpl(LookupTable(peaceawayhostilelist,fact),PEACESITUATION,forcechange)
                    debug.debug("peace-away-hostile " + fact)
        else:
            ftmp = un.FShieldData()+2*un.GetHullPercent()+un.RShieldData()-2+0.5
            mfname=None
            mfcount=0
            for (fname,fcount) in unfcount.iteritems():
                if fcount>mfcount:
                    mfname=fname
                    mfcount=fcount            
            if (threat):
                if (ftmp<0):
                    mpl(LookupTable(paniclist,mfname),PANICSITUATION,forcechange)
                    debug.debug("panic " + mfname)
                else:
                    mpl(LookupTable(battlelist,mfname),BATTLESITUATION,forcechange)
                    debug.debug("battle " + mfname)
            else:
                mpl(LookupTable(threatlist,mfname),THREATSITUATION,forcechange)
                debug.debug("threat " + mfname)
                
class DJ:
    def __init__(self, check_frequency, hostile_dist=0):
        self.hostile_dist = hostile_dist
        self.check_frequency = check_frequency
        self.last_check_time = VS.GetGameTime()
    def Execute(self):
        if ((VS.GetGameTime() - self.last_check_time) >= self.check_frequency):
            PlayMusik(0,self.hostile_dist)
            self.last_check_time = VS.GetGameTime()
