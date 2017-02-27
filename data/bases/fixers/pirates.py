import Base
import fixers
import Director
import VS
def AssignMission ():
	fixers.DestroyActiveButtons ()
	fixers.CreateChoiceButtons(Base.GetCurRoom(),[
		fixers.Choice("bases/fixers/yes.spr","bases/fixers/pirates_mission.py","Accept This Agreement"),
		fixers.Choice("bases/fixers/no.spr","bases/fixers/no.py","Decline This Agreement")])

playa = VS.getPlayer();
playernum = playa.isPlayerStarship()
if (VS.numActiveMissions()>1):
	Base.Message("I can't give your cargo and money until your current mission is complete.");
else:
	if (fixers.checkSaveValue (playernum,"pirate_mission1",0)):
		print "START1"
		AssignMission()
		Base.Message("We are in need of a pilot who can safely transport this cargo to one of our bases in the #55ffffrigel#000000 system. It is highly important that no confed or militia know of these goods. Will you accept this unavoidable offer?")
#And then if you fail.......
	elif (fixers.checkSaveValue (playernum,"pirate_mission1",-1) or fixers.checkSaveValue (playernum,"pirate_mission2",-1) or fixers.checkSaveValue (playernum,"pirate_mission3",-1) or fixers.checkSaveValue (playernum,"pirate_mission4",-1)):
		Base.Message ("How could you let our precious cargo be destroyed. It has cost us millions of credits and you trashed it like it was nothing. Prepare to be nailed,pal!")
		type = faction_ships.getRandomFighter ("pirates")
		fgname="shadow"
		launch.launch_wave_around_unit (fgname,"pirates",type,"default",1,80,300,playa).SetTarget(playa)
		launch.launch_wave_around_unit (fgname,"pirates",type,"default",1,80,300,playa).SetTarget(playa)
	else:
		if (fixers.checkSaveValue (playernum,"pirate_mission1",1) and fixers.checkSaveValue(playernum,"pirate_mission2",0)):
			Base.Message ("Thanks for the cargo that we needed, pal. We now need some cargo delivered to the #55fffftingvallir#000000 system. Can you do that without geting caught?")#assign mis 2
			fixers.payCheck(playernum,'paidpiratemission',0,20000)
			AssignMission()
		elif (fixers.checkSaveValue (playernum,"pirate_mission2",1) and fixers.checkSaveValue(playernum,"pirate_mission3",0)):			
			Base.Message ("Hey, pal. You got back here all right. Now I have a special mission for you that requires a tractor beam. This will require you to destroy a merchant transport in this system. You must have a tractor beam equipped to your ship...we want the ship to blow, but the cargo...")#assign mis 3:
			fixers.payCheck(playernum,'paidpiratemission',1,20000)
			AssignMission()
		elif (fixers.checkSaveValue (playernum,"pirate_mission3",1) and fixers.checkSaveValue(playernum,"pirate_mission4",0)):
			fixers.setSaveValue(playernum,"pirate_mission3",2)
			fixers.payCheck(playernum,'paidpiratemission',2,25000)
			Base.Message ("It looks like you got some nice cargo from that guy. Go to the #55ffffrigel#000000 system immediately. There is something very important that must be done. Meet my friend who will tell you what to do.")
		elif ((fixers.checkSaveValue (playernum,"pirate_mission3",2) or fixers.checkSaveValue (playernum,"pirate_mission3",1)) and VS.getSystemFile()=='enigma_sector/rigel' and fixers.checkSaveValue(playernum,"pirate_mission4",0)):
			Base.Message ("Our base is under attack! Defend it from the militia! They have heard about illegal contraband transferring and are not happy.")#assign mis 4
			AssignMission()
		elif (fixers.checkSaveValue(playernum,"pirate_mission4",1)):
			fixers.setSaveValue(playernum,"pirate_mission4",2)
			Base.Message ("Thanks, pal. You've earned our trust. You're a recognized friend of the pirates now. I don't know if you fully grok this yet, but you have saved many of our lives here in the Rigel system. Here are your 80000 credits that we promised. Go out, take what's yours, and don't let anyone give you flak.")
			fixers.payCheck(playernum,'paidpiratemission',3,80000)
		else:
			pass

