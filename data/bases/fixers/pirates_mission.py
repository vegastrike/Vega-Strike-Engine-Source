import fixers
import Base
fixers.DestroyActiveButtons ()
playernum = VS.getPlayer().isPlayerStarship()
if (fixers.checkSaveValue(playernum,"pirate_mission1",0)):
	fixers.setSaveValue (playernum,"pirate_mission1",0)
	Base.Message("Great! We will give you four missions to complete and once they are all done, you will earn a lump sum of 80000 credits. The first mission is a cargo mission. Your cargo has already been transported to your ship. See my friend in the #55ffffrigel#000000 system after the cargo has been transported.")
	#load mission 1
	VS.LoadMission ("cargo/pirates/pirates_cont_1.mission")
elif (fixers.checkSaveValue(playernum,"pirate_mission2",0) and fixers.checkSaveValue (playernum,"pirate_mission1",1)):
	fixers.setSaveValue (playernum,"pirate_mission2",0)
	Base.Message("Great! Your cargo has already been transported to your ship. Fly carefully to the #55fffftingvallir#000000 system and talk to him when the contraband run is complete.")
	VS.LoadMission ("cargo/pirates/pirates_cont_2.mission")
	#load mission 2
elif (fixers.checkSaveValue(playernum,"pirate_mission3",0) and fixers.checkSaveValue (playernum,"pirate_mission2",1)):
	fixers.setSaveValue (playernum,"pirate_mission3",0)
	Base.Message("We have uploaded the coordinates to where we have last seen the merchant docked. Your nav computer will automatically autopilot to those coordinates around the planet.  Once he is destroyed, tractor in all of the cargo and bring it to a base that will jump into this system soon.")
	VS.LoadMission ("cargo/pirates/plunder.mission")
	#load mission 3
elif (fixers.checkSaveValue(playernum,"pirate_mission4",0) and (fixers.checkSaveValue (playernum,"pirate_mission3",1) or fixers.checkSaveValue (playernum,"pirate_mission3",2))):
	fixers.setSaveValue (playernum,"pirate_mission4",0)
	Base.Message("Destroy all of the enemies attacking this base and come back to me when the battle is over. After that, I will give you all of your long deserved money")
	VS.LoadMission ("defend/pirates/pirate_msision4.mission")
	#load mission 4
else:
	Base.message ("Yes about what?")
