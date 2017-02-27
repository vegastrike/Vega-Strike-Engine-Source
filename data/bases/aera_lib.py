import Base
import bar_lib
import weapons_lib
import dynamic_mission
import VS

def MakeAera(time_of_day='day'):
    plist=VS.musicAddList('aera.m3u')
    VS.musicPlayList(plist)
    dynamic_mission.CreateMissions()
    
    room1 = Base.Room ('Aera Planet')
    Base.Texture (room1, 'tex', 'bases/aera/aera_planet.sprite', 0, 0)
    Base.Comp (room1, 'CargoComputer', 0.472656, -0.614583, 0.152344, 0.403646, 'Cargo Computer', 'Cargo')
    Base.Comp (room1, 'MissionComputer', -0.376953, -0.653646, 0.15625, 0.424479, 'Mission Computer', 'News Missions')
    Base.Ship (room1, 'ship', (0.1546875*2, -0.3645835*2,2), (0,1,0), (.5,0,.86))
    
    bar = bar_lib.MakeBar (room1,time_of_day) 
    weap = weapons_lib.MakeWeapon (room1,time_of_day)
    
    room2 = Base.Room ('Planet Exterior')
    Base.Texture (room2, 'background', 'bases/aera/biosimple.sprite', 0, 0)
    
    Base.Link (room1, 'BarLink1', 0.230469, -0.184896, 0.166016, 0.653646, 'Bar', bar)
    Base.Link (room1, 'ExtLink', 0.398438, 0.015625, 0.298828, 0.216146, 'Planet Exterior', room2)
    Base.Link (room1, 'Weapons1', -0.173828, -0.0338542, 0.429688, 0.653646, 'Weapons Room', weap)
    Base.Link (room1, 'Weapons2', -0.117188, -0.195313, 0.376953, 0.182292, 'Weapons Room', weap)
    Base.Link (room2, 'Landing', -0.5, -1, 1, 0.2, 'Landing Pad', room1)
    
    Base.LaunchPython (room1, 'launch','bases/launch_music.py', -0.152344, -0.869792, 0.523438, 0.382813, 'Launch Your Ship')
    ### MUST BE LAST LINK ###
    return (room1,bar,weap,room2)
