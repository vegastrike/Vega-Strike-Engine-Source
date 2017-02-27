import Base
import weapons_lib
import vsrandom
import dynamic_mission
import VS
import fixers
def MakeIndustrial(time_of_day='day',mybartender='bases/bartender_default.py'):
    dynamic_mission.CreateMissions()
    room = Base.Room ('Industrial_Landing_Pad')
    room0 = room
    Base.Texture (room, 'background', 'bases/industrial/industrial_landing'+time_of_day+'.spr', 0, 0)
    Base.Ship (room, 'my_ship', (0.409863,-0.50531,4), (0, 0.93, -0.34), (-1, 0, 0))
    
    room = Base.Room ('Industrial_Planet_Surface')
    room1 = room
    Base.Texture (room, 'background', 'bases/industrial/industrial_exterior2'+time_of_day+'.spr', 0, 0)
    
    room = Base.Room ('Trade_Center')
    room2 = room
    Base.Texture (room, 'background', 'bases/industrial/industrial_exterior4'+time_of_day+'.spr', 0, 0)
    
    room = Base.Room ('Main_Concourse')
    room3 = room
    Base.Texture (room, 'background', 'bases/industrial/industrial_concourse'+time_of_day+'.spr', 0, 0)
    
    room = Base.Room ('Upgrade_Starship')
    room4 = room
    Base.Texture (room, 'background', 'bases/industrial/industrial_mining_ocean_weaponroom'+time_of_day+'.spr', 0, 0)
    
    import  bar
    room5 = bar.MakeAgriculturalBar(room3,'Main_Concourse',"bases/industrial/industrial_bar","bases/bartender_default.py")
    
    room = Base.Room ('Factory_Production_Facility')
    room6 = room
    Base.Texture (room, 'background', 'bases/industrial/industrial_exterior3'+time_of_day+'.spr', 0, 0)
    
    room = Base.Room ('Industrial_Planet_Hills')
    room7 = room
    Base.Texture (room, 'background', 'bases/industrial/industrial_exterior1'+time_of_day+'.spr', 0, 0)
    
    Base.LaunchPython (room0, 'my_launch_id', 'bases/launch_music.py', -0.0820312, -0.653646, 0.632812, 0.413542, 'Launch')
    Base.Link (room0, 'my_link_id', -0.242188, -0.143229, 0.398438, 0.393229, 'Industrial_Planet_Surface', room1)
    Base.Link (room0, 'my_link_id', -1, -0.994792, 1.99805, 0.192708, 'Main_Concourse', room3)
    Base.Link (room0, 'my_link_id', -0.996094, -0.989583, 0.255859, 0.940104, 'Weapon_Dealer', room4)
    Base.Comp (room0, 'my_comp_id', -0.972656, -0.171875, 0.773438, 0.367188, 'Ship_Dealer', 'ShipDealer Info ')
    Base.Link (room1, 'my_link_id', -0.433594, -0.526042, 0.210938, 0.213542, 'Trade_Center', room2)
    Base.Link (room1, 'my_link_id', -0.150391, -0.971354, 0.708984, 0.195312, 'Landing_Pad', room0)
    Base.Link (room1, 'my_link_id', -0.917969, -0.630208, 0.330078, 0.388021, 'Main_Concourse', room3)
    Base.Link (room1, 'my_link_id', -0.390625, -0.145833, 0.474609, 1.10156, 'Examine_Factory', room6)
    Base.Link (room1, 'my_link_id', 0.873047, -0.460938, 0.101562, 0.739583, 'Explore_Planetary_Surface', room7)
    #Base.Link (room1, 'my_link_id', -0.0644531, -0.278646, 0.498047, 0.489583, 'Bar', room5)
    Base.Link (room2, 'my_link_id', -1.0, -1.0, 0.7075, 1.53125, 'Main_Concourse', room3)
    Base.Comp (room2, 'my_comp_id', 0.185547, -0.510417, 0.0410156, 0.658854, 'Trade_Cargo', 'Cargo News Info ')
    Base.Comp (room2, 'my_comp_id', 0.0820312, -0.565104, 0.544922, 0.632812, 'Trade_Cargo', 'Cargo News Info ')
    Base.Link (room2, 'my_link_id', -1, -0.994792, 1.99805, 0.0911459, 'Industrial_Planet_Surface', room1)
    Base.Link (room3, 'my_link_id', -1, -0.994792, 1.99609, 0.127604, 'Trade_Cargo', room2)
    Base.Link (room3, 'my_link_id', 0.559375, -0.15625, 0.209375, 0.296875, 'Weapon_Dealer', room4)
    Base.Link (room3, 'my_link_id', -0.978125, -0.169271, 1.030859, 0.333333, 'Bar', room5)
    Base.Link (room3, 'my_link_id', 0.0644531, -0.0807292, 0.290625, 0.200521, 'Landing_Pad', room0)
    Base.Comp (room3, 'my_comp_id', 0.759766, -0.0651042, 0.189453, 0.515625, 'Computer', 'Missions News Info ')
    Base.Link (room4, 'my_link_id', 0.916016, -0.869792, 0.0605469, 1.02865, 'Landing_Pad', room0)
    Base.Link (room4, 'my_link_id', -0.996094, -0.986979, 1.97656, 0.0625, 'Main_Concourse', room3)
    Base.Comp (room4, 'my_comp_id', -0.0605469, -0.268229, 0.761719, 0.307292, 'Upgrade_Weapons', 'Upgrade Info ')
    #Base.Link (room5, 'my_link_id', -1, -0.994792, 1.99805, 0.0859375, 'Main_Concourse', room3)
    Base.Link (room5, 'my_link_id', 0.173828, -0.0859375, 0.0625, 0.127604, 'Industrial_Planet_Exterior', room1)
    Base.Link (room6, 'my_link_id', -1, -0.997396, 1.99805, 0.1520833, 'Industrial_Planet_Surface', room1)
    Base.Link (room6, 'my_link_id', 0.927734, -0.866667, 0.1203125, 1.91406, 'Industrial_Planet_Hills', room7)
    Base.Link (room2, 'my_link_id', 0.927734, -0.866667, 0.1203125, 1.91406, 'Industrial_Planet_Hills', room7)
    Base.Link (room7, 'my_link_id', -0.0292969, -0.164062, 0.214844, 0.510417, 'Examine_Factory_Operation', room6)
    Base.Link (room7, 'my_link_id', -0.9375, -0.221354, 0.541016, 0.231771, 'Main_Complex_Exterior', room1)
    return (room0,room5,room4)

def MakeCorisc(time_of_day='day',mybartender='bases/bartender_default.py'):
    playlist = 'industrial.m3u'
    if (mybartender=='bases/bartender_union.py'):
        playlist='university.m3u'
    plist=VS.musicAddList(playlist)
    VS.musicPlayList(plist)    

    dynamic_mission.CreateMissions()
    room1 = Base.Room ('Landing Platform')
    Base.Texture (room1, 'tex', 'bases/industrial/landing'+time_of_day+'.spr', 0, 0)
    bar = Base.Room ('Bar')
    #Base.Texture (bar, 'tex', 'bases/industrial/bar'+time_of_day+'.spr', 0, 0)
    #Base.Link (bar, 'Exlink1', -1, -1, 0.466797, 2, 'Exit The Bar', 0)
    #Base.Link (bar, 'Exlink2', -1, -0.200521, 1, 1.200521, 'Exit The Bar', 0)
    #Base.Python (bar, 'talk', 0.46875, -0.151042, 0.4, 0.4, 'Talk to the Bartender', mybartender,0)
    #Base.Texture(bar,'bartender','bases/industrial/bartender%d.spr' % (vsrandom.randrange(0,11)),0.66875, 0.05)
    #fixers.CreateFixers(bar,[(.1,-0.15,.4,.4),])
    import bar_lib
    bar = bar_lib.MakeBar(room1,time_of_day,mybartender)
    weap = Base.Room ('Upgrade_Facilities/Ship_Dealer')
    Base.Texture (weap, 'background', 'bases/civilian/civilian_weaponroom.spr', 0, 0)
    Base.Ship (weap, 'my_ship', (-0.765918,-0.054948,3.5), (0, 0.93, -0.34), (-1, 0, 0))

    Base.Comp (weap, 'my_comp_id', -0.236328, -0.513021, 0.5, 0.53125, 'Upgrade_Computer', 'Upgrade ShipDealer Info ')
    Base.Link (weap, 'my_link_id', -1, -0.997396, 1.99805, 0.117188, 'Main_Concourse', room1)
    if (time_of_day=='_day'):
        Base.Comp (room1, 'CargoComputer', -0.476563, -0.705729, 0.0664063, 0.200521, 'Cargo Computer', 'Cargo')
        Base.Comp (room1, 'MissionComputer', 0.300781, -0.695313, 0.0800781, 0.195313, 'Mission Computer', 'News Missions')
        Base.Ship (room1, 'ship', (-0.02539065*2,-0.4254165*2,4.5), (0,.9,-.13), (.5,.13,.8))
        Base.Link (room1, 'BarLink', -0.347656, -0.335938, 0.144531, 0.195313, 'Bar', bar)
        Base.Link (room1, 'WeapLink', 0.152344, -0.40625, 0.294922, 0.361979, 'Weapons Room', weap)
        Base.LaunchPython (room1, 'launch','bases/launch_music.py', -0.369844, -0.997396, 0.638672, 0.40625, 'Launch Your Ship')
    else:
        Base.Comp (room1, 'comp', -0.900391, -0.848958, 0.136719, 0.411458, 'Computer', 'Cargo News Missions')
        Base.Ship (room1, 'ship', (0.0539065*2,-0.3354165*2,2.25), (0,1,0), (.5,0,.86))
        Base.Link (room1, 'BarLink', -0.558594, -0.151042, 0.275391, 0.242188, 'Bar', bar)
        Base.Link (room1, 'WeapLink', 0.3125, -0.0963542, 0.681641, 0.333333, 'Weapons Room', weap)
        Base.LaunchPython (room1, 'launch','bases/launch_music.py', -0.492188, -1, 0.982422, 0.625, 'Launch Your Ship')
    return (room1,bar,weap)
