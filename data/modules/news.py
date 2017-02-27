import VS
import vsrandom
import universe
import Director
import dynamic_news
import stardate

global dnewsman_
dnewsman_ = dynamic_news.NewsManager()
def saveVal(str):
  return Director.getSaveData(VS.getMissionOwner(),str,0)

class NotZero:
  def __init__ (self,str):
    self.str = str
  def __nonzero__ (self):
    print 'nonzeroing'
    return saveVal(self.str)!=0
class IsZero:
  def __init__ (self,str):
    self.str = str
  def __nonzero__ (self):
    print 'nonzeroing'
    return saveVal(self.str)==0
class GreaterZero:
  def __init__ (self,str):
    self.str = str
  def __nonzero__ (self):
    print 'nonzeroing'
    return saveVal(self.str)>0
class LessZero:
  def __init__ (self,str):
    self.str = str
  def __nonzero__ (self):
    print 'nonzeroing'
    return saveVal(self.str)<0

news =( ( 'kinneas',(IsZero('kinneas'),),"TEENAGE BOY OUTSMARTS SYSTEM:  A teenager from the Draul Bisa Habitat was caught redhanded on New Poona Mining Base last week, as he was trying to sneak past security without proper identification. The young human male, identified as Kinneas Pinman, somehow managed to make it past spaceport security on Draul Bisa and stow away on a passanger transport bound for the mining base in the Celeste System. While the motives behind Pinman's actions remain unknown, redfaced spaceport officials are hard-pressed to explain how the cheeky computer whizkid managed to elude all their security precautions. Pinman was detained by police on New Poona and will be returned to Draul Bisa within the week."),
        ('congress_convenes',(IsZero('congress_convenes'),),"CONFEDERATION SENATE CONVENES TO PONDER BORDER SECURITY:  The Senate of the Confederation of Inhabited Worlds convened earlier today at the Confederation Center on Mars. On their agenda, possible military intervention to protect human interests in Uln space because of the increased number of pirate raids launched from behind Uln borders. No decisions were made in the heated debate that followed, with the representatives from several LIHW worlds leaving the Confederation Center in disgust."),#
        ('news_slaver_guild',(IsZero('news_slaver_guild'),NotZero("quest_slaver"),),"SLAVER GUILD EXPOSED:  An operation lead by the HCS Illustrious working in conjunction with Confederate Special Forces uncovered a Slaver Guild in the Rigel System last week. The slavers were based off an outpost inside an asteroid field, and though they had managed to avoid detection by Navy patrols up till now, they had no such luck this time thanks to intelligence forwarded by the local militia. After the Illustrious overcame the outpost's defenses, the Special Forces unit moved in and secured the station, liberating over a hundred slaves, killing close to forty slavers and capturing twice as many. A large number of Uln were discovered on the outpost, and while they will be turned over to Uln authorities, a formal complaint is expected to be issued by the General Secretary to the Uln government within the week."),#
        ('missing_california',(IsZero('missing_california'),),"MISSING SHIP STILL MISSING:   The mysterious disappearance of the HCS California continues to puzzle naval investigators. The Confederate missile cruiser's last known position was in the Axis System, but all contact was lost with it two days ago. Fighter patrols dispatched to search the area, that was reported to be clear of pirates and other hostile elements after extensive sweeps by the military two months ago, came up with nothing. Mutiny or alien intervention cannot be ruled out, according to sources inside the Navy."),#
        ('missing_grizzlebane',(IsZero('missing_grizzlebane'),),"FREIGHTER MISSING:  The freighter Grizzlebane disappeared earlier this week in the Axis system.  The freighter was scanned in Calimanchus by the HCS Isengard as it entered the system, but it never arrived at its destination in Rigel.  The Grizzlebane was reported missing by the Targas, a reputedly shady trading group based in the Rigel system.  While Confederation authorities have written it off as another pirate attack, the Targas insist that the Grizzlebane had enough armaments to stave off any attacks or at least fire off a distress signal before losing integrity.  No debris was found at the site."),
        ('drone_sighting',(IsZero('drone_sighting'),NotZero('missing_grizzlebane'),),"STARSHIP SIGHTING:  The Targas trading clan has reported a sighting of an alien craft in the Axis system where numerous craft have been reported missing in the recent months.  The alien starship of unknown design apparently appeared from within a bluish cloud and destroyed several craft before the clan had the ability to repower their jump drives and exit the system.  Scans of the area reveal the debris of the destroyed starships. Confederation officials remain silent on the issue, and neither confirm nor deny the Targas report.  Interestingly, no debris has been discovered from previous starship disappearences, so whether this is or is not linked to the previous disappearences in the Axis system remains to be seen."),
        ('racene_star_intro',(IsZero('racene_star_intro'),),"RACENE STAR SOON SECURED:  The battle across the single continent of the habitable moon rages on, as the forces of the Confederate Army continue to push forward towards the 'Devil's Fort', the Aera's largest military outpost in the system. Last month, relieved military human strategists removed a significant Aera army from their battle maps, after its positions were broken on the anvil of the 93rd hovertanks under the command of General Sulimani Abdullah. The Army expresses hope that the moon, a strategic target in the push against Aera space, will be secured before the end of next month."),#
        ('racene_star_tide',(NotZero('racene_star_intro'),IsZero('quest_racene'),IsZero('racene_star_progress'),IsZero('racene_star_tide'),),"TIDE TURNS ON RACENE STAR - FOR THE WORSE:   After a large battle in the space surrounding the moon of racene Star B, the human fleet in orbit was forced to surrender its position and flee back towards friendly space, taking heavy losses in the process and effectively stranding over 75,000 soldiers on the surface below. In the last couple of weeks, things were looking up as Confederate forces began advancing through the lines of the Aera, but military analysts are certain that the troops will be butchered without space support. Scouts also reported that Aera capital ships immediately entered a close orbit around the moon and began bombarding the human positions below with heavy photon disruptor fire and space-to-surface missiles."),#
        ('racene_star_victory',(NotZero('racene_star_intro'),NotZero('quest_racene'),IsZero('racene_star_defeat'),IsZero('racene_star_victory'),),"VICTORY AT RACENE STAR:   Troops have taken Devil's Fort after a number of privateers and other civilians heroically stopped a key recon wing that would other wise have discovered the Terran Relief Fleet.  With the cover of the far moon, the terran fleet swooped in and delivered a close range bombardment to the fort.  With the TRF support from the air, Terran Marines flattened the military outposts, causing remaining Aera civilians to abandon much of the planet and retreat to Aera Space"),#
        ('racene_star_defeat',(NotZero('racene_star_intro'),NotZero('racene_star_tide'),IsZero('racene_star_quest'),IsZero('racene_star_progress'),IsZero('racene_star_defeat'),IsZero('racene_star_victory'),),"CONFEDERATION ARMY LOSES RACENE STAR:   After days of intense fighting, the Terran marines failed to hold off an advancing Aeran drone army and lost their main command center.  It was not long before the scattered, disorganized band of remaining marines were singly tracked down and destroyed.  Coordination between the offworld starbases and onworld establishments spelled disaster for these stranded marines in enemy turf."),#
        ('rlaan_rescue',(IsZero('rlaan_rescue'),),"RLAAN RESCUE OPERATION ON HUMAN COLONY:   The human settlers on Keppler Colony experienced a strange incident three days ago, when two Rlaan fighters seemingly on a reconnaissance mission swept in low over the settlement. Then, as they started to gain altitude again, one of them suffered what seemed to be engine trouble and proceeded to crash into the ground, but not before its pilot managed to eject. Less than two hours later, just as the colonists were organizing a rescue party to search the dense forest, a Rlaan assault shuttle appeared and dropped a heavily armed commando team equipped with hoverbelts. While six of the alien troopers kept the humans at bay by aiming their weapons at them, the others managed to extract the downed pilot and within fifteen minutes the Rlaan shuttle was on its way again. No shots were fired, and while the condition of the Rlaan pilot remains unknown, no one else was hurt."),#
        ('abu_dhabi_return',(IsZero('abu_dhabi_return'),),"ABU DHABI EMERGES:   The HCS Abu Dhabi, presumed lost 89 years ago when it failed to emerge in Sirius after having jumped from Sol, baffled the galaxy yesterday by suddenly appearing again. The crew of the now-obsolete frigate were in perfect health, but were naturally shocked to learn that almost a century had passed since they entered jumpspace, in what they experienced as a perfectly normal instantaneous jump. They have been transfered over to facilities on Holton Station where the General Secretary will meet with them tomorrow. Scientists and engineers are currently busy studying the Abu Dhabi's jump drive in an attempt to figure out what went wrong. This is the only recorded incident of its kind in the history of the Confederation."),
        ('iso_demonstration',(IsZero('iso_demonstration'),),"ISO DEMONSTRATION ON NEW BREMEN:  The ISO (Interstellar Socialist Organization) organized a demonstration at the University of Terra, Berkeley today in the Defiance system, protesting against the planet's economic policies and for being 'an Earth puppet'. A large number of police monitored the demonstration which never escalated into violence. The ISO calls itself a political organization, but it has been branded as paramilitary by the CSP. Though it deploys a significant fleet of mostly civilian spacecraft, the ISO has rarely used them in any recorded assaults against Confederate military or civilian targets. The ISO enjoys a strong following primarily from students and workers on the frontier colonies."),
        ('skull_pirate_arrest',(IsZero('skull_pirate_arrest'),),"THE SKULL ARRESTED!  Christopher 'The Skull' Thorne was finally arrested today in the Draul Bisa Habitat after an interstellar manhunt spanning seven systems. For three years, Thorne was the leader of one of the cruelest and most vicious band of pirates on the frontier, charged with over a hundred counts of murder and space piracy. In light of the Navy's failure to capture Thorne and his lackeys, a group of wealthy merchants banded together and with the help of a powerful mercenary squadron, they were able to decimate Thorne's armada. Thorne himself managed to elude his hunters, until today. He will be transfered to a prison ship until a trial can be scheduled."),#
        ('rlaan_mining_spy',(IsZero('rlaan_mining_spy'),),"MINERS EXPOSE RLAAN SPY:  Miners in the Novaya Kiev System were in for a surprise earlier today when they drilled their way into the core of a recently discovered asteroid. The rock wall collapsed only to reveal a small communications and sensor room and a lone Rlaan operating the controls inside it. Before the miners could react, the startled alien scrambled into a small escape pod and launched into space through a narrow tube. The slow mining ship was unable to intercept the pod before a Rlaan fighter de-stealthed and retrieved it. The Navy has now invoked a policy to thoroughly scan all suspicious asteroids near military installations and important jump routes in the space adjacent to the Rlaan border, should they contain Rlaan sensor posts.  In addition a hefty price of 200,000 credits has been placed on this spy's head should he be discovered before escaping Confederate space."),#
        ('callimanchius_disaster',(IsZero('callimanchius_disaster'),),"ASTEROID STRIKES CALLIMANCHIUS COLONY:  Disaster struck the Callimanchius colony yesterday when an asteroid the size of a small freighter somehow managed to make it past the planet's satellite defenses. The asteroid hit ground not far from Port Bernard, the second largest settlement on the planet. Casualties are expected to be tallied in the hundereds of thousands. The asteroid was large enough to have a profound impact on the enviroment of the entire planet, and as a result, a complete evacuation has been ordered by the local authorities. The Confederation Navy is diverting ships to assist in this endeavour.  The colonies in Callimanchius are relatively young, and it does not enjoy the same satellite protection from interstellar projectiles as the more heavily populated inner worlds do. Still, an investigation will be launched to determine why the two defense satellites in orbit did not react to the approaching danger."),
        ('callimanchius_survivors',(NotZero('callimanchius_disaster'),NotZero('callimanchius_survivors'),),"SURVIVORS FROM CALLIMANCHIUS COLONY:\\\\With the death count from the Callimanchius disaster still climbing, it is reassuring to hear of some of the more remarkable survival tales that are beginning to emerge from the debris.\\\\All settlements near Port Bernard were expected to have been obliterated.  So the Confederation Navy was quite surprised when it started picking up a distress signal identifying residents of the Port Bernard regional area who had been unable to escape the planet before the impact.\\\\The survivors, three humans and their assistant droid, had been unable to start their antique \"Azimuth\" class puddle jumper in time to escape. With some very lucky timing on their part, they were able to activate their shields just as the shockwave hit. While the shields, as expected, rapidly failed when activated planetside, they survived just long enough to protect against the shockwave."),
        ('bert_tribunal',(IsZero('bert_tribunal'),),"MARINE COLONEL TO FACE MILITARY TRIBUNAL:  Colonel Bert Thompson of the Confederate Marine Corps was detained by the CSP earlier this week. He is to stand before a military tribunal where he will be put to trial for his actions on the HCS Coral Sea two months ago. As stated in reports, the Coral Sea had managed to disable and tractor in an Aera transport after having destroyed its escorts. Colonel Thompson and his marines proceeded to board and secure the alien ship, encountering only light resistance. They took 22 prisoners, which Thompson himself interrogated. After Thompson had deemed the interrogations to be finished, he had the prisoners placed in one of the airlocks and then ejected them into space."),#
        ('holman_population',(IsZero('holman_population'),),"HOLMANS REACHES TWENTY-FIVE BILLION:   In the Alpha Centauri System, the population of the planet of Holman is estimated to have exceeded the 25 billion mark today. Holmans was the third planet to be colonized outside of the Sol System, and has been the human planet with the largest population aside from Earth for over five decades now. This rapid increase of population has caused housing to be ever more important.  With Holman's second moon, Hart, housing close to three billion people, it is a densely populated area of space, and standards of living have not improved since the onset of the Aeran conflict."),
        ('privateer_revenge',(IsZero('privateer_revenge'),),"CORPORATION SUFFERS PRIVATEER ATTACK:   The Cruyff Corporation suffered a minor setback today, when it lost two transports and eleven employees at the hands of an enraged privateer. The mercenary had recently fulfilled a contract for the corporation, and authorities speculate that he attacked the transports when Cruyff agents refused to give him the payment he felt he rightfully deserved. The two transports were part of a convoy of four, but fortunately Militia fighters managed to destroy his ship before the mercenary could cause any more damage. Representatives of the Cruyff Corporation have refused to comment the incident."),
        ('aera_prisoner_revolt',(IsZero('aera_prisoner_revolt'),),"AERAN PRISONERS ESCAPE:   Four weeks ago, the prison ship HCS Zhu-Jiang which was scheduled to transport Aeran P.O.W.s from the frontlines, failed to report in to the local Confederate Tracking Station as it entered the Zelmeny System. The ship then deviated from its assigned route and continued out of sensor range, not to be heard from again. Today the Navy finally released the details concerning the mysterious behavior of the Zhu-Jiang. The vessel did relay a brief message to the Tracking Station, indicating that the Aeran prisoners had somehow managed to seize the ship's armory and were fighting to reach the bridge. Since nothing was heard from the Zhu-Jiang again, it is assumed that the Aerans were successful in their goal, and flew the ship back into their own space. There are no details available regarding the fates of the 174 humans that were on the ship at the time of the rebellion. The Navy is launching a full investigation into the matter."),
        ('birrhes_tree_attack',(IsZero('birrhes_tree_attack'),),"SETTLEMENT UNDER ATTACK BY LOCAL FLORA:   A small settlement on a remote island on the moon of Xugen was the scene of unspeakable carnage last week. After contact had been lost with the village, a detachment of soldiers from the local garrison flew over to investigate, only to find the entire population lying butchered on the ground. Some of the victims appeared to be half-eaten, so the troopers immediately suspected that a local, previously unknown, predator was behind the murders, but thermal cameras deployed from the air did not show anything unusual. Then, without warning, the Birrhes Trees surrounding the settlement suddenly leapt at the soldiers, emitting high-pitched shrieks and slashing at them with their sharp branches. Two soldiers were killed, but the rest managed to retreat off the island, and though the Birrhes Tree is native to that island alone, biologists will conduct an investigation to determine if there is any more lethal flora turned fauna on the planet."),#
        ('bugle_bank_heist',(IsZero('bugle_bank_heist'),),"BUGLE BROTHERS STRIKE AGAIN:   Infamous for their bank heist on New Bremen two years ago, when they made off with over fifty million credits in a daring coup, the 'Bugle Brothers' selected the headquarters of the Cruyff Corporation as their target this time around. The enigmatic brothers somehow managed to infiltrate the corporation's vast office complex located on Nova Jolandria, and then proceeded to crack the main computer system where they used their superior programming skills to rewrite security protocols for the entire building. After having transfered sixty million credits into an unknown account, they vanished without a trace. Some people speculate that the brothers are the result of a genetic mutation project instigated by the Army, but military officials deny all involvement."),#
        ('hoffman_blobs',(IsZero('hoffman_blobs'),),"HOFFMAN'S BLOBS SPOTTED:   A flock of the strange space creatures known as 'Hoffman's Blobs' were observed in the Galileo System, resulting in the sixth sighting of these bizarre interstellar beings since they were first seen by Bruno Hoffman in Barnard's Star 53 years ago. Scientists are now flocking to study the questionablly non-sentient creatures before they leave, attempting to determine how it is that they are able to sustain themselves in the void of space. From previous observations, it appears they travel in flocks of about a dozen units, often varying in size with the largest being as large as a space cruiser, and the smallest only the size of a shuttlecraft. The creatures migrations from system to system take thousands of years, and the creatures are presumed to be dormant during the interstellar portions of such voyages."),
        ('holland_protests',(IsZero('holland_protests'),),"PROTESTS IN HOLLAND:   Anti-war protestors on Earth staged a large demonstration today in Amsterdam, Holland. The protestors marched up the streets of the city, demanding that the Confederate Senate immediately call an end to all hostilities on the front in Forsaken space and explore diplomatic solutions to end the war against the Aera. Furthermore, they accused the Rlaan of having forced a human entry into the war by a secret pact signed by the Senate to aid them as cannon fodder in their already senseless interstellar conflict. Several members of the Senate dismissed the accusations as 'preposterous', stating that 'there was not, and has never been' any pacts or military agreements between the human race and the Rlaan. Senatur Mueller was particularly critical, noting that a proper military alliance between the two groups would have them invading Aeran space and not the other way around."),#
        ('man_bites_droid',(IsZero('man_bites_droid'),),"ROBOT ATTACKS HUMANS:   An experimental Robot Maintenance Unit went rogue after it became self-aware near the University of Polusand on the planet of Feldham earlier today. The robot attacked several unsuspecting pedestrians on the street by ramming them with its 500 kg heavy frame before it was destroyed by local authorities. Two women were severly injured and had to be taken to a nearby hospital for treatment, but their condition is now reported as stable. Computer technicians are hard at work trying to determine what made the unit disregard from the fail-safe programming designed to prevent it from ever harming a human being, and subsequently all units of the series have been shut down until the investigation is finished."),#
        ('beagle_exploration',(IsZero('beagle_exploration'),),"THE BEAGLE RETURNS:   The HCS Beagle returned to a human port this afternoon after having finished its six-year exploration mission in an unknown region of space. The Beagle's 68 crewmembers expressed great relief at being back in human space, and it will be some time before the ship leaves on another mission to map unexplored systems. The Department of Space Exploration revealed that the Beagle had not made contact with any new sentient races, but that it had gathered an 'impressive' amount of data, mostly regarding habitable planets and the locations of new jump-points. "),
        )

def newNews():
  print "Adding news"
  if (vsrandom.randrange(0,2)!=0):
      return
  newsitem = vsrandom.randrange (0,len(news))
  newsitem = news[newsitem]
  player = VS.getMissionOwner()
  for conditional in newsitem[1]:
      print 'conditioning'
      if (not conditional):
          return
  universe.setFirstSaveData(player,newsitem[0],1)
  import Director
  newsfooter = "\\\\\This story was first broadcast on: "
  newsfooter += stardate.formatStarDate("confed",VS.getStarTime())
  newsfooter += "\\GNN - Galactic News Network"
  Director.pushSaveString(player,"dynamic_news",'#'+newsitem[2]+newsfooter)

def eraseNews(plr):
  import Director
  len = Director.getSaveStringLength(plr,"news")
  for i in range(len):
      Director.eraseSaveString(plr,"news",len-i-1)

def processNews(plr):
  eraseNews(plr)
  import Director
  howmuchnews=Director.getSaveStringLength(plr,"dynamic_news")
  minnews=0
  print "Processing News"
  global dnewsman_
  dnewsman_.updateDockedAtFaction()
  if (howmuchnews>4000):
      minnews=howmuchnews-4000
  for i in range (minnews,howmuchnews):
      noos=Director.getSaveString(plr,"dynamic_news",i)
      if (len(noos)):
          if (noos.startswith('#')):
              Director.pushSaveString(plr,"news",noos[1:])
          elif dnewsman_.isStoryRelevant(noos):
              noos = dnewsman_.translateDynamicString(noos)
              if noos:
                  Director.pushSaveString(plr,"news",noos)

def eraseNewsItem(plr,item):
  """removes the first news item matching the given item from
  plr's "dynamic_news" save variable"""
#    print "FIXME: someone please write a function to this spec! Every thing I try seems to produce some random result :-/"
  import Director
  for i in range (Director.getSaveStringLength(plr,"dynamic_news")):
      noos=Director.getSaveString(plr,"dynamic_news",i)
      if noos == item:
          Director.eraseSaveString(plr,"dynamic_news",i)
          return

def publishNews(text):
# publishes the news text with star date
  player = VS.getPlayer().isPlayerStarship()
  STARDATE_TEXT = "\\\\\This story was first broadcast on: "
  datetext = stardate.formatStarDate("confed",VS.getStarTime())
  newstext = "#" + text + STARDATE_TEXT + datetext + "\\GINA - Galactic Independent News Association"
  Director.pushSaveString(player,"dynamic_news",newstext)
