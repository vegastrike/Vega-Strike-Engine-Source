import Base
import dynamic_mission
import VS

time_of_day=''
weap=-1
room0=-1
plist=VS.musicAddList('industrial.m3u')
VS.musicPlayList(plist)
import industrial_lib
industrial_lib.MakeIndustrial(time_of_day,'bases/bartender_default.py')