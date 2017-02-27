import Base
import VS
#import sys
#import dynamic_mission
import fixers

#plist=VS.musicAddList('agricultural.m3u')
#VS.musicPlayList(plist)
#dynamic_mission.CreateMissions()

room = Base.Room ('Hangar')
room0 = room
Base.Texture (room, 'background', 'bases/Snow/landing.spr', 0, 0)
Base.Ship (room, 'my_ship', (0.65,-0.3,4), (0, 0.93, -0.34), (-1, 0, 0))

room = Base.Room ('Main_Concourse')
room1 = room
Base.Texture (room, 'background', 'bases/Snow/concourse.spr', 0, 0)

import bar
room2 = bar.MakeMiningBar(room1,'Main_Concourse','bases/Snow/bar', 'bases/bartender_default.py')

room = Base.Room ('Weapons Room')
room3 = room
Base.Texture (room, 'background', 'bases/generic/weaponroom.spr', 0, 0)

Base.LaunchPython (room0, 'my_launch_id', 'bases/launch_music.py', 0.1,-0.5, 0.6, 0.4, 'Launch')
Base.Link (room0, 'my_link_id', -1, -1, 0.3, 0.3, 'Main Concourse', room1)
Base.Comp (room0, 'my_comp_id', -0.765, -0.35, .2578906, 0.80833, 'Mission Computer', 'Missions News Info')
Base.Comp (room0, 'my_comp_id', -0.5, -0.6, 0.3, 0.3, 'Trade Cargo', 'Cargo Info ')

Base.Link (room1, 'my_link_id', -0.15, -1, 0.3, 0.3, 'Hangar', room0)
Base.Link (room1, 'my_link_id', 0.5, -0.153333, 0.1, 0.1, 'Bar', room2)
Base.Link (room1, 'my_link_id', -0.1, -0.1, 0.1, 0.2, 'Upgrade Ship', room3)

Base.Link (room3, 'my_link_id', -0.15, -1, 0.3, 0.3, 'Main Concourse', room1)
Base.Comp (room3, 'my_comp_id', 0, 0.3, 0.3, 0.3, 'Upgrade Weapons', 'Upgrade Info')
Base.Comp (room3, 'my_comp_id', -0.0605469, -0.268229, 0.761719, 0.307292, 'Ship Dealer', 'ShipDealer Info')
