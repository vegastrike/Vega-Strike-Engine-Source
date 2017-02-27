import Base
import dynamic_mission
import VS
time_of_day=''
bar=-1
weap=-1
room0=-1
plist=VS.musicAddList('unit.m3u')
VS.musicPlayList(plist)
import unit_lib
(room1,room2,bar,weap)=unit_lib.MakeUnit()
