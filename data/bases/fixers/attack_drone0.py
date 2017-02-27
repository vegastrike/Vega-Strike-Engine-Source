import Base
import VS
import Director
import quest

plyr=VS.getPlayer().isPlayerStarship()
if quest.findQuest(plyr,'gemini_sector/delta_prime_navpoint',1):
	VS.getPlayer().addCredits(20000)
	quest.removeQuest(plyr,'gemini_sector/delta_prime_navpoint',2)
Base.Message("Congratulations on giving us the data on the last sector! We seem to have found a passage deep into Aera territory. Perhaps even the aera haven't completely civilized that sector of space.  Rumor has it that an unknown vessel has emerged from the jump point you discovered.  If you have any scans or information please take them to one of my former associates in the Enigma system.  Thank you for working with me, and I hope you have found it to be a profitable and worthwhile venture.")
