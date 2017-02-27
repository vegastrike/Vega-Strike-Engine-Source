import Base
import VS
import bar_lib
import weapons_lib
import dynamic_mission

list=VS.musicAddList ('unit.m3u')
VS.musicPlayList(list)
import military_lib
(room1, room2, bar, weap) = military_lib.MakeMilitaryBase('bases/bartender_military.py')