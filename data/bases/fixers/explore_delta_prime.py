import Base
import VS
import Director
import quest
plyr=VS.getPlayer().isPlayerStarship()
if VS.numActiveMissions()<=1:
	if quest.findQuest(plyr,'gemini_sector/gamma_navpoint',1):
		VS.getPlayer().addCredits(10000)
		quest.removeQuest(plyr,'gemini_sector/gamma_navpoint',2)
	Base.Message('''Congratulations on your third mission! We have decided to go to the next system.
Based on the data that you have collected, it appears that the other jump point should go
to a system that we have named #55ffffdelta prime#000000. Again, collect info
on six objects there and report back to me with your info.  You will earn 25000 credits.''')
	VS.LoadMission('patrol/explore_delta_prime.mission')
else:
	Base.Message('''Hi. Our records indicate that you are already completing another mission.  Finish
that and then come back to me.''')
