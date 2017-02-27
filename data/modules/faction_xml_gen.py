fp = open ('factions.xml','w')
from faction_ships import factions
import faction_ships


def GetRelation(fac,fnum,enfac,enfnum):
    if (fac=='privateer'):
        return 0
    if fnum==enfnum:
        return 1
    if enfnum in faction_ships.enemies[fnum]:
        return -1
    elif (enfnum in faction_ships.friendlies[fnum]):
        return 1
    return 0

fp.write('<Factions>\n')
fp.write('\t<Faction name="neutral" logoRGB="logos/confedPri.png" logoSec="logos/confedSec.png"/>\n')
for i in  range (len(faction_ships.factions)+1):
    if (i<len(factions)):
        f = factions[i]
    else:
        f='privateer'
    fp.write ('\t<Faction name="'+f+'" logoRGB="logos/'+f+'Pri.png" secLogoRGB="logos/'+f+'Sec.png" contraband="contraband">\n')
    fp.write('\t\t<stats name="privateer" relation="0"/>\n')
    for j in range (len(faction_ships.factions)):
        e=factions[j]
        fp.write('\t\t<stats name="'+e+'" relation="'+ str(GetRelation(f,i,e,j)) + '"/>\n')
    fp.write('\t\t<Explosion name="explosion_orange.ani"/>\n<!--\t\t<CommAnimation>  \n')
    fp.write('\t\t\t<MoodAnimation name="explosion_orange.ani" sex="0"/>  \n')
    fp.write('\t\t</CommAnimation>  -->\n')
    fp.write('\t</Faction>\n')
fp.write('\t<Faction name="upgrades" logoRGB="logos/confedPri.png" logoSec="logos/confedSec.png"/>\n')
fp.write('\t<Faction name="planets" logoRGB="logos/confedPri.png" logoSec="logos/confedSec.png"/>\n')
fp.write ('</Factions>\n')
fp.close()
