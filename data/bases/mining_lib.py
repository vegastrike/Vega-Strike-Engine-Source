import Base
import dynamic_mission
import VS
def MakeMining(repairbay,shipbay,bartendertext='bases/bartender_default.py',time_of_day=''):
	dynamic_mission.CreateMissions()
	room = Base.Room ('Landing_Bay')
	room0 = room
	Base.Texture (room, 'background', 'bases/mining/mining_landing'+time_of_day+'.spr', 0, 0)
	Base.Ship (room, 'my_ship', (-0.818262,-0.632005,5), (0, 0.93, -0.34), (-1, 0, 0))
	
	room = Base.Room ('Trade_Center')
	room1 = room
	Base.Texture (room, 'background', 'bases/mining/mining_traderoom'+time_of_day+'.spr', 0, 0)
	
	room = Base.Room ('Upgrade_Facilities')
	room2 = room
	Base.Texture (room, 'background', 'bases/mining/mining_exterior2'+time_of_day+'.spr', 0, 0)
	Base.Ship (room, 'my_ship', (0.0317383,-0.447396,2), (0, 1, 0), (-0.85, 0.03, -0.89))

	room = Base.Room ('View_To_Trade_Center')
	room3 = room
	Base.Texture (room, 'background', 'bases/mining/mining_exterior3'+time_of_day+'.spr', 0, 0)
	
	room = Base.Room ('Main_Concourse')
	room4 = room
	Base.Texture (room, 'background', 'bases/mining/mining_concourse'+time_of_day+'.spr', 0, 0)
	import bar
	room5 = bar.MakeMiningBar(room4, 'Main_Concourse', 'bases/mining/mining_bar'+time_of_day,bartendertext)
	#Base.Texture (room5, 'background', 'bases/mining/mining_bar'+time_of_day+'.spr', 0, 0)
	
	room = Base.Room ('Shipyards')
	room6 = room
	Base.Texture (room, 'background', 'bases/mining/mining_exterior1'+time_of_day+'.spr', 0, 0)
	
	Base.LaunchPython (room0, 'my_launch_id', 'bases/launch_music.py', -0.660156, -0.765625, 0.339844, 0.4375, 'Launch')
	Base.Link (room0, 'my_link_id', 0.476562, -0.565104, 0.361328, 0.302083, 'Trade_Center', room1)
	if (repairbay):
		Base.Link (room0, 'my_link_id', -1, -0.190104, 0.384766, 0.356771, 'Upgrade_Facilities', room2)
		Base.Link (room1, 'my_link_id', 0.410156, 0.0104167, 0.556641, 0.471354, 'Upgrade_Facilities', room2)
		Base.Link (room3, 'my_link_id', -0.423828, -0.473958, 0.292969, 0.132812, 'Upgrade_Center', room2)
		Base.Link (room4, 'my_link_id', -0.998047, -0.997396, 1.99414, 0.0494791, 'Upgrade_Facilities', room2)
		Base.Link (room6, 'my_link_id', 0.148438, -0.981771, 0.84375, 0.119792, 'Upgrade_Center', room2)
	Base.Link (room0, 'my_link_id', -0.941406, 0.541667, 0.652344, 0.104167, 'View_From_Platform', room3)
	Base.Link (room0, 'my_link_id', -0.617188, 0.174479, 0.324219, 0.242187, 'Main_Concourse', room4)
	if (shipbay):
		Base.Link (room0, 'my_link_id', 0.125, 0.5625, 0.484375, 0.40625, 'Shipyards', room6)
		Base.Link (room3, 'my_link_id', -0.708984, 0.239583, 0.541016, 0.408854, 'Shipyards', room6)
		Base.Link (room4, 'my_link_id', -0.541016, -0.164062, 0.201172, 0.317708, 'Shipyards', room6)

	Base.Link (room1, 'my_link_id', -0.833984, -0.997396, 1.03906, 0.463542, 'Landing_Bay', room0)
	Base.Comp (room1, 'my_comp_id', -0.970703, -0.104167, 0.484375, 0.432292, 'Trade_Cargo', 'Cargo News Info ')
	Base.Comp (room2, 'my_comp_id', -0.568359, -0.804688, 0.875, 0.398438, 'Upgrade_Starship', 'Upgrade Info ')
	Base.Link (room2, 'my_link_id', -0.0683594, -0.364583, 1.04492, 0.882812, 'Landing_Pad', room0)
	Base.Link (room2, 'my_link_id', -0.283203, -0.307292, 0.283203, 0.197917, 'Main_Concourse', room4)
	Base.Link (room2, 'my_link_id', -0.998047, -0.994792, 1.99609, 0.09375, 'Trade_Cargo', room1)
	Base.Link (room3, 'my_link_id', -0.267578, -0.0416667, 0.3125, 0.177083, 'Trade_Center', room1)
	Base.Link (room3, 'my_link_id', 0.224609, -0.661458, 0.261719, 0.182292, 'Landing_Pad', room0)

	Base.Link (room4, 'my_link_id', 0.421875, -0.205729, 0.257812, 0.557292, 'Hangar', room0)
	Base.Comp (room4, 'my_comp_id', -0.976562, 0.0416667, 0.597656, 0.479167, 'Mission_Computer', 'Missions News Info ')
	Base.Link (room4, 'my_link_id', -0.208984, -0.0286458, 0.199219, 0.174479, 'Trade_Center', room1)
	Base.Link (room4, 'my_link_id', 0.117188, -0.106771, 0.195312, 0.302083, 'Bar', room5)
	Base.Comp (room6, 'my_comp_id', -0.849609, 0.179688, 0.970703, 0.492188, 'Ship_Dealer', 'ShipDealer Info ')
	Base.Comp (room6, 'my_comp_id', 0.335938, 0.328125, 0.630859, 0.575521, 'Ship_Dealer', 'ShipDealer Info ')
	Base.Link (room6, 'my_link_id', 0.0546875, -0.0807292, 0.566406, 0.276042, 'Trade_Center', room1)
	Base.Link (room6, 'my_link_id', 0.205078, -0.638021, 0.416016, 0.106771, 'Landing_Pad', room0)

	Base.Link (room6, 'my_link_id', -0.423828, -0.994792, 0.525391, 0.143229, 'Main_Concourse', room4)
	Base.Link (room6, 'my_link_id', -0.978516, 0.458333, 0.232422, 0.523438, 'View_From_Platform', room3)
