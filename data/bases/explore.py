import Base
import VS
import Director
import fixers
import quest

def exploremission(part,filename,sysfile,creditnum):
    fixers.DestroyActiveButtons ()
    if part==1:
        plyr=VS.getPlayer().isPlayerStarship()
        if quest.findQuest(plyr,(sysfile+"_nav"),1) or quest.findQuest(plyr,(sysfile+"_nav"),2):
            part=3
    if part==1:
        fixers.CreateChoiceButtons(Base.GetCurRoom(),[
            fixers.Choice("bases/fixers/yes.spr",filename,"Accept This Mission"),
            fixers.Choice("bases/fixers/no.spr","bases/fixers/no.py","Decline This Mission")])
        Base.Message("""Hi. Our researchers have found a new signal similar to that of a jump point coming from an area in the
#55ffff%s#000000 system. This mission will require that you own a jump drive.
We hope that you will accept this mission. You will earn %.2f credits when you explore the area."""%(sysfile.split('/')[-1],creditnum))
    elif part==2:
        Base.Message("""Excellent! Go to the unknown energy source that we have uploaded to your ship computer
as a navpoint in #55ffff%s#000000. Once next to the energy source, jump
into the energy source and get close to all of the planets in the connecting system."""%sysfile.split('/')[-1])
        Base.EnqueueMessage("""You will earn %.2f credits when this is completed"""%creditnum)
        plyr=VS.getPlayer().isPlayerStarship()
        if Director.getSaveDataLength(plyr,(sysfile+"_nav"))==0:
            Director.pushSaveData(plyr,(sysfile+"_nav"),1)
        else:
            Director.putSaveData(plyr,(sysfile+"_nav"),0,1)
    else:
        Base.Message("""Go to the unknown energy source that we have uploaded to your ship computer
as a navpoint in #55ffff%s#000000. Once next to the energy source, jump
into the energy source and get close to all of the planets in the connecting system."""%sysfile.split('/')[-1])
