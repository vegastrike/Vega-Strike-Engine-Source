import fixers
import mission_lib
import VS

fixers.DestroyActiveButtons ()
if VS.numActiveMissions()>1:
	Base.Message('You are already doing a mission. Finish that instead.')
else:
	mission_lib.BriefLastMission(0,1)
	mission_lib.LoadLastMission()
	#VS.LoadMission('internal_mission_lib.mission')
