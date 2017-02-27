import Base
import bar_lib
import weapons_lib
import dynamic_mission

def MakeUnit (bartendername='bases/bartender_default.py',time_of_day=''):
	dynamic_mission.CreateMissions()
	room = Base.Room ('Landing_Bay')
	room0 = room
	Base.Texture (room, 'background', 'bases/civilian/civilian_landing'+time_of_day+'.spr', 0, 0)
	Base.Ship (room, 'my_ship', (0.065918,-0.454948,3), (0, 0.93, -0.34), (-1, 0, 0))

	room = Base.Room ('Main_Concourse')
	room1 = room
	Base.Texture (room, 'background', 'bases/civilian/civilian_concourse'+time_of_day+'.spr', 0, 0)
	import bar
	room=bar.MakeMiningBar(room1,'Main_Concourse','bases/mining/mining_bar'+time_of_day,bartendername)
	room2 = room

	room = Base.Room ('Upgrade_Facilities/Ship_Dealer')
	room3 = room
	Base.Texture (room, 'background', 'bases/civilian/civilian_weaponroom'+time_of_day+'.spr', 0, 0)
	Base.Ship (room, 'my_ship', (-0.765918,-0.054948,3.5), (0, 0.93, -0.34), (-1, 0, 0))


	room = Base.Room ('Trade_Room')
	room6 = room
	Base.Texture (room, 'background', 'bases/civilian/civilian_traderoom'+time_of_day+'.spr', 0, 0)

	Base.LaunchPython (room0, 'my_launch_id', 'bases/launch_music.py', -0.308594, -0.703125, 0.669922, 0.520833, 'Launch')
	Base.Link (room0, 'my_link_id', -1.00391, -0.997396, 0.316406, 1.99219, 'Main_Concourse', room1)
	Base.Link (room0, 'my_link_id', -1, -0.997396, 1.99805, 0.135417, 'Main_Concourse', room1)

	Base.Link (room1, 'my_link_id', -1, -0.997396, 1.99805, 0.135417, 'Landing_Pad', room0)
	Base.Link (room1, 'my_link_id', 0.417969, -0.460938, 0.158203, 0.263021, 'Bar', room2)
	Base.Link (room1, 'my_link_id', -0.806641, -0.473958, 0.201172, 0.289062, 'Upgrade_Facilities/Ship_Dealer', room3)
	Base.Link (room1, 'my_link_id', -0.337891, -0.539062, 0.357422, 1.51562, 'Trade_Room', room6)
	Base.Comp (room3, 'my_comp_id', -0.236328, -0.513021, 0.5, 0.53125, 'Upgrade_Computer', 'Upgrade ShipDealer Info ')
	Base.Link (room3, 'my_link_id', -1, -0.997396, 1.99805, 0.117188, 'Main_Concourse', room1)
	Base.Link (room6, 'my_link_id', -1, -0.994792, 1.99609, 0.101562, 'Main_Concourse', room1)
	Base.Comp (room6, 'my_comp_id', -0.457031, -0.164062, 0.617188, 0.572917, 'Trade', 'Cargo Missions News Info ')

	return (room0, room1, room2, room3)
