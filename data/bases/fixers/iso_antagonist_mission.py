import fixers
import Base
fixers.DestroyActiveButtons ()
playernum = VS.getPlayer().isPlayerStarship()
if (fixers.checkSaveValue(playernum,"iso_evil2",0) and VS.numActiveMissions()<=1):
	fixers.setSaveValue (playernum,"iso_evil2",0)
	#load mission 2
	VS.LoadMission ("bounty/iso/attack_iso_freighter.mission")
elif (fixers.checkSaveValue(playernum,"iso_evil3",0) and VS.numActiveMissions()<=1):
	fixers.setSaveValue (playernum,"iso_evil3",0)
	VS.LoadMission ("bounty/iso/attack_iso_prisoners.mission")
	#load mission 2
elif (fixers.checkSaveValue(playernum,"iso_evil4",0) and VS.numActiveMissions()<=1):
	fixers.setSaveValue (playernum,"iso_evil4",0)
	VS.LoadMission ("defend/iso/attack_iso_force.mission")
	#load mission 3
else:
	Base.message ("Ahh but you are already completing a mission. Come back to me when you're done with that contract first.")
