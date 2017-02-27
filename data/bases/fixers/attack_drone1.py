import Base
import VS
import Director
import quest
import fixers
plyr=VS.getPlayer().isPlayerStarship()
if quest.findQuest(plyr,'quest_drone',-1):
	Base.Message("""There is an alien vessel let loose on confed territory. We are offering 30,000 credits to anyone who manages to take it out. Be warned, however, our scans reveal armor and shielding that we do not know how to penetrate. Good luck and godspeed!""")
elif quest.findQuest(plyr,'quest_drone',1):
	Base.Message("""Thank you! Your effort saved the lives of trillions of civilians. That drone was terrorizing our people and our space force!""")
	if fixers.checkSaveValue(plyr,'quest_drone_cash',0):
		VS.getPlayer().addCredits(30000)
		quest.removeQuest(plyr,'gemini_sector/quest_drone_cash',1)
