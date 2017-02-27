import fixers
import Base
fixers.DestroyActiveButtons ()
playernum = VS.getPlayer().isPlayerStarship()
if (fixers.checkSaveValue(playernum,"iso_mission1",0) and VS.numActiveMissions()<=1):
	fixers.setSaveValue (playernum,"iso_mission1",0)
	#load mission 1
	VS.LoadMission ("defend/iso/defend_iso_mission1.mission")
elif (fixers.checkSaveValue(playernum,"iso_mission2",0) and VS.numActiveMissions()<=1 and fixers.checkSaveValue (playernum,"iso_mission1",1)):
	fixers.setSaveValue (playernum,"iso_mission2",0)
	VS.LoadMission ("escort/iso/escort_iso_mission2.mission")
	#load mission 2
elif (fixers.checkSaveValue(playernum,"iso_mission3",0) and VS.numActiveMissions()<=1 and fixers.checkSaveValue (playernum,"iso_mission2",1)):
	fixers.setSaveValue (playernum,"iso_mission3",0)
	VS.LoadMission ("cargo/iso/cargo_contraband_mission3.mission")
	#load mission 3
elif (fixers.checkSaveValue(playernum,"iso_mission4",0) and VS.numActiveMissions()<=1 and fixers.checkSaveValue (playernum,"iso_mission3",1)):
	fixers.setSaveValue (playernum,"iso_mission4",0)
	VS.LoadMission ("defend/iso/defend_iso_mission4.mission")
	#load mission 4
else:
	Base.message ("Our records indicate that you are already completing a mission. Come back to me when you're done with that business first.")
