import Base
import VS
import Director
import quest
plyr=VS.getPlayer().isPlayerStarship()
if VS.numActiveMissions()<=1:
	if quest.findQuest(plyr,'gemini_sector/delta_navpoint',1):
		VS.getPlayer().addCredits(5000)
		quest.removeQuest(plyr,'gemini_sector/delta_navpoint',2)
	Base.Message('''Congratulations on your first mission! We have decided to go to the next system.
Based on the data that you have collected, it appears that the other jump point should go
to a system that we have named #55ffffbeta#000000. Again, collect info
on six objects there and report back to me with your info.  You will earn 10000 credits.''')
	VS.LoadMission('patrol/explore_beta.mission')
else:
	Base.Message('''Hi. Our records indicate that you are already completing another mission.  Finish
that and then come back to me.''')
