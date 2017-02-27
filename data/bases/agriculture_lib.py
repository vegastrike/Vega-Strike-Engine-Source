import Base
import bar_lib
import weapons_lib
import dynamic_mission
import VS
def MakeAgri (time_of_day='_day'):
    plist=VS.musicAddList('agricultural.m3u')
    VS.musicPlayList(plist)
    dynamic_mission.CreateMissions()
    room1 = Base.Room ('Main Concourse')
    Base.Texture (room1, 'tex', 'bases/agriculture/landing'+time_of_day+'.spr', 0, 0)
    Base.Comp (room1, 'CargoComputer', -0.306641, -0.263021, 0.105469, 0.420833, 'Cargo Computer', 'Cargo')
    Base.Comp (room1, 'MissionComputer', 0.292969, -0.263021, 0.0878906, 0.420833, 'Mission Computer', 'News Missions')
    Base.Ship (room1, 'ship', (0.0253906*2,-0.148958*2,4), (0,.93,-.34), (-1,0,0))
    bar = bar_lib.MakeBar (room1,time_of_day)
    weap = weapons_lib.MakeWeapon (room1,time_of_day)
    Base.Link (room1, 'BarLink', 0.580078, -1, 0.419922, 1.09115, 'Bar', bar)
    Base.Link (room1, 'WeaponsRoom', -1, -1, 0.419922, 1.09115, 'Weapons Room', weap)
    Base.LaunchPython (room1, 'launch','bases/launch_music.py', -1, -1, 2, .8, 'Launch Your Ship')
    ### MUST BE LAST LINK ###
    return (room1,bar,weap)
