import Base
import dynamic_mission
import VS
def MakeLandingAndConcourse(time_of_day):
	time_of_day=''#not used
	plist=VS.musicAddList('ocean.m3u')
	VS.musicPlayList(plist)
	dynamic_mission.CreateMissions()
	room = Base.Room ('Landing_Center')
	room0 = room
	Base.Texture (room, 'background', 'bases/ocean/ocean_landing'+time_of_day+'.spr', 0, 0)
	Base.Ship (room, 'my_ship', (-0.122559,-0.146484,4), (0, 0.93, -0.34), (-1, 0, 0))
	
	room = Base.Room ('ocean_concourse')
	room1 = room
	Base.Texture (room, 'background', 'bases/ocean/ocean_concourse'+time_of_day+'.spr', 0, 0)
	import bar
	room2 = bar.MakeOceanBar(room1,'Main_Concourse','bases/ocean/ocean_bar'+time_of_day,'bases/bartender_default.py')
	
	room = Base.Room ('Tube_To_Trade_Room')
	room3 = room
	Base.Texture (room, 'background', 'bases/ocean/ocean_exterior2'+time_of_day+'.spr', 0, 0)
	
	room = Base.Room ('Trade_Facilities')
	room4 = room
	Base.Texture (room, 'background', 'bases/ocean/ocean_exterior1'+time_of_day+'.spr', 0, 0)
	
	room = Base.Room ('Tube_To_Shipyards')
	room5 = room
	Base.Texture (room, 'background', 'bases/ocean/ocean_exterior3'+time_of_day+'.spr', 0, 0)
	
	room = Base.Room ('Upgrade_Facilities_and_Shipyard')
	room6 = room
	Base.Texture (room, 'background', 'bases/ocean/ocean_exterior4'+time_of_day+'.spr', 0, 0)
	
	Base.LaunchPython (room0, 'my_launch_id', 'bases/launch_music.py', -0.609375, -0.502604, 0.712891, 0.65625, 'Launch')
	Base.Link (room0, 'my_link_id', -1, -0.997396, 1.99805, 0.315104, 'Main_Concourse', room1)
	Base.Link (room1, 'my_link_id', 0.125, -0.132812, 0.273125, 0.372396, 'Bar', room2)
	Base.Link (room1, 'my_link_id', 0.400391, -0.276042, 0.322266, 0.669271, 'Hangar', room0)
	Base.Comp (room1, 'my_comp_id', -0.998047, -0.03125, 0.550391, 0.681771, 'Mission_Computer', 'Missions News Info ')
	Base.Link (room1, 'my_link_id', -0.292969, -0.0963542, 0.404297, 0.296875, 'Tube_To_Trade_Center', room3)
	Base.Link (room1, 'my_link_id', -0.450234, -0.1875, 0.154297, 0.377604, 'Tube_To_Shipyards', room5)
	Base.Link (room3, 'my_link_id', -0.681641, 0.507812, 0.234375, 0.479167, 'Continue_To_Trade_Facilities', room4)
	Base.Link (room3, 'my_link_id', -0.851562, -0.973958, 0.847656, 0.817708, 'Return_To_Concourse', room1)
	Base.Link (room3, 'my_link_id', -0.216797, 0.684896, 0.179688, 0.294271, 'Divert_To_Shipyards', room5)
	Base.Link (room3, 'my_link_id', -0.507812, -0.179688, 0.492188, 0.544271, 'Return_To_Main_Concourse', room1)
	Base.Comp (room4, 'my_comp_id', -0.443359, -0.960938, 1.25586, 1.0651, 'Trade_Cargo', 'Cargo Info ')
	Base.Link (room4, 'my_link_id', -0.257812, 0.276042, 0.275391, 0.669271, 'Return_To_Main_Concourse', room3)
	Base.Link (room5, 'my_link_id', 0.287109, 0.479167, 0.388672, 0.359375, 'Return_To_Main_Concourse', room1)
	Base.Link (room5, 'my_link_id', 0.0625, 0.046875, 0.228516, 0.403646, 'Divert_To_Cargo_Facilities', room3)
	Base.Link (room5, 'my_link_id', -0.988281, 0.0833333, 0.726562, 0.359375, 'Continue_To_Shipyards', room6)
	Base.Link (room6, 'my_link_id', -0.015625, 0.375, 0.998047, 0.502604, 'Tube_To_Main_Concourse', room5)
	Base.Comp (room6, 'my_comp_id', -0.382812, -0.679688, 0.486328, 0.507812, 'Upgrade_Ship', 'Upgrade Info ')
	Base.Comp (room6, 'my_comp_id', 0.261719, -0.757812, 0.708984, 0.677083, 'Shipyards', 'ShipDealer Info ')
	return (room0,room1,room2,room6)