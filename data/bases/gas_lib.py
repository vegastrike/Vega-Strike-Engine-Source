import Base
import VS
import dynamic_mission

def MakeGas (time_of_day='_day'):
	plist=VS.musicAddList('agricultural.m3u')
	VS.musicPlayList(plist)
	dynamic_mission.CreateMissions()
	
	room1 = Base.Room ('Landing Pad')    
	Base.Texture (room1, 'tex', 'bases/gas/landing.spr', 0, 0)
	Base.Ship (room1, 'ship', (-0.15,0.3,4), (0, 0.93, -0.34), (-1, 0, 0))
	#Base.Texture (room1, 'tex', 'bases/generic/holo.spr', -.5, -.4)
	Base.Comp (room1, 'Computer', -1, -0.8, .2578906, 0.80833, 'Computer', 'Cargo Upgrade Info ShipDealer News Missions')
	Base.LaunchPython (room1, 'launch','bases/launch_music.py', -0.4, 0.1, 0.5, 0.3, 'Launch Your Ship') #minx, miny, width, height
	return room1

