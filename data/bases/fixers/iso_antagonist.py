import Base
import fixers
import Director
import quest
import faction_ships
import launch
def AssignMission ():
	fixers.DestroyActiveButtons ()
	fixers.CreateChoiceButtons(Base.GetCurRoom(),[
		fixers.Choice("bases/fixers/yes.spr","bases/fixers/iso_antagonist_mission.py","Accept This Agreement"),
		fixers.Choice("bases/fixers/no.spr","bases/fixers/iso_antagonist_decline.py","Decline This Agreement")])

playa=VS.getPlayer()
playernum = playa.isPlayerStarship()
#only want this var if you agreed to the quest at some point--so we want it to return fasle if len is 0
if (not Director.getSaveDataLength(playernum,"iso_mission1")):
	Base.Message("Good day, stranger.  I do not know you, and I do not wish to know you. However, in the face of our mutual situation, perhaps you could come to an agreement with Rowenna of the ISO.  Talk to her. But before you leave, talk to me. I may have an offer that you simply cannot refuse.")
else:
	fixers.DestroyActiveButtons ()
	if (fixers.checkSaveValue(playernum,"iso_mission1",0)):
		Base.Message("Excellent! You accepted the mission.  I will offer you 38,000 credits to do one thing: destroy that vessel. You should have help from a few of my elite force; however, I expect the ISO to put up a fight, so be wary and beware. I trust you will accept my offer, my young....friend")
	elif (fixers.checkSaveValue(playernum,"iso_mission1",1) or fixers.checkSaveValue(playernum,"iso_evil2",-1) or fixers.checkSaveValue(playernum,"iso_evil3",-1) or fixers.checkSaveValue(playernum,"iso_evil4",-1)):
		Base.Message("There is only one thing I despise more than a member of the Interplanetary Socialist Organization: and that is one of their mercenary lapdogs. You are not worth the breath I have wasted on you. Leave now, if you dare.  My forces will destroy your starship and leave your corpse to boil in vacuum.")
		type = faction_ships.getRandomFighter ("confed")
		fgname="IntelSecElite"
		fixers.setSaveValue (playernum,"decided_iso_good",1)
		launch.launch_wave_around_unit (fgname,"confed",type,"default",1,80,100,playa).SetTarget(playa)
		launch.launch_wave_around_unit (fgname,"confed",type,"default",1,80,100,playa).SetTarget(playa)
		launch.launch_wave_around_unit (fgname,"confed",type,"default",1,80,100,playa).SetTarget(playa)
		launch.launch_wave_around_unit (fgname,"confed",type,"default",1,80,100,playa).SetTarget(playa)
		launch.launch_wave_around_unit (fgname,"confed",type,"default",1,80,100,playa).SetTarget(playa)
	elif (fixers.checkSaveValue(playernum,"iso_mission1",-1) and fixers.checkSaveValue(playernum,"iso_evil2",0)):
		msg =''
		if (fixers.checkSaveValue(playernum,"iso_mission1_paid",0)):
			msg+='Excellent! Excellent! Without him, the ISO cannot stand against us! Soon Defiance itself will be in my grasp! Here is your pay. '
			playa.addCredits(38000)
			fixers.setSaveValue(playernum,"iso_mission1_paid",1)
		if (VS.numActiveMissions()>1):
			msg += 'My next job will be most fit when you have no other contracts. We need your services then.'
		else:
			msg += 'My next job will be a simple one.  A high ranking ISO official will be enroute to the Adams Sector from this one.  I would like you to eliminate her before she leaves this system. This is critical to our success... that is 25000 credits critical--will you do it?'	
			AssignMission()
		Base.Message (msg)
	else:
		if (VS.numActiveMissions()>1):
			Base.Message ("your other contracts interfere with me assigning you a new mission. Finish them, then talk to me.")			
		elif (fixers.checkSaveValue(playernum,"iso_mission1",-1) and fixers.checkSaveValue(playernum,"iso_evil2",1) and fixers.checkSaveValue(playernum,"iso_evil3",0)):
			Base.Message("So far you have served our interests well. If you accept this contract you will go now to the Adams sector and destroy a cargo ship enroute to the Defiance system. It is loaded with political prisoners who have been illegally freed. These felons must not make it to Defiance alive or they will spark revolutionary fervor there. Eliminate them for me, and I will grant you 28000 credits. Will you crush them?")
			AssignMission()
		elif (fixers.checkSaveValue(playernum,"iso_mission1",-1) and fixers.checkSaveValue(playernum,"iso_evil2",1) and fixers.checkSaveValue(playernum,"iso_evil3",1) and fixers.checkSaveValue(playernum,"iso_evil4",0)):
			Base.Message("Now the stakes are higher. My elite force had planned to assault a ISO stronghold in the Defiance system, but we did not anticipate the number of ISO ships we found to be there. Therefore, we will send in a larger ship that will assist you in destroying the last vestiges of the only truly substantial ISO defense force in any system.  Once this is secured, this system, and all others like it, will be more amenable to control via the more subtle applications of IntelSec diplomacy. Your assistance is worth 40000 credits to us.")
			AssignMission()
		elif (fixers.checkSaveValue(playernum,"iso_mission1",-1) and fixers.checkSaveValue(playernum,"iso_evil2",1) and fixers.checkSaveValue(playernum,"iso_evil3",1) and fixers.checkSaveValue(playernum,"iso_evil4",1)):
			Base.Message ("Excellent, my young hunter--your work has ensured that IntelSec's clean and sweep program will be a success when it is deployed.  Not only have you removed the key figures from the ISO's leadership but you have crushed their morale and their defenses with a sweeping blow.  Your heroism will be duly detailed in what must, unfortunately for your fame, remain the classified annals of the Confederacy.")
		else:
			Base.Message ("your other contracts interfere with me assigning you a new mission. Finish them, then talk to me.")





