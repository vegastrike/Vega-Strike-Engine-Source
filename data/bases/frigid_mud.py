import Base
import VS
import dynamic_mission
import vsrandom
import fixers

shipsize = VS.getPlayer().rSize()/35
#print "Ship Size: " + str(VS.getPlayer().rSize()) #debug
dynamic_mission.CreateMissions()
time_of_day='_day'

# ROOMS
landing = Base.Room ('Landing Pad')    
if (VS.getPlayer().rSize()<=100):
    Base.Texture (landing, 'tex', 'bases/frigid_mud/AridLandingSmall2.sprite', 0, 0)
else:
    Base.Texture (landing, 'tex', 'bases/frigid_mud/AridLandingCapship.sprite', 0, 0)
fuelbay = Base.Room ('Fuel Bay')    
Base.Texture (fuelbay, 'tex', 'bases/frigid_mud/AridFuelBay.sprite', 0, 0)
panorama = Base.Room ('Panorama')    
Base.Texture (panorama, 'tex', 'bases/frigid_mud/AridPanorama.sprite', 0, 0)
entrance = Base.Room ('Concourse')    
Base.Texture (entrance, 'tex', 'bases/frigid_mud/AridConcourse2.sprite', 0, 0)
exit = Base.Room ('Concourse')    
Base.Texture (exit, 'tex', 'bases/frigid_mud/AridConcourse.sprite', 0, 0)
bar1 = Base.Room ('Bar')    
Base.Texture (bar1, 'tex', 'bases/frigid_mud/AridBar2.sprite', 0, 0)
bar2 = Base.Room ('Bar')    
Base.Texture (bar2, 'tex', 'bases/frigid_mud/AridBar1.sprite', 0, 0)
balcony = Base.Room ('Balcony')    
Base.Texture (balcony, 'tex', 'bases/frigid_mud/AridBalcony.sprite', 0, 0)

# INTERLINKS
if (VS.getPlayer().rSize()<=100):
    Base.Link (landing, 'l_f',  -0.70, 0.30, 0.30, 0.22, 'Fuel Bay', fuelbay)
    Base.Link (landing, 'l_e',  0.40, -0.10, 0.30, 0.22, 'Concourse', entrance)
    Base.Link (landing, 'l_p',  -0.97, -0.20, 0.30, 0.22, 'Panorama', panorama)
else:
    Base.Link (landing, 'l_f',  0.00, -0.30, 0.30, 0.22, 'Fuel Bay', fuelbay)
    Base.Link (landing, 'l_e',  0.80, -0.40, 0.30, 0.22, 'Concourse', entrance)
    Base.Link (landing, 'l_p',  -0.97, -0.20, 0.30, 0.22, 'Panorama', panorama)
Base.Link (entrance, 'e_b', -0.10, 0.00, 0.30, 0.22, 'Bar', bar1)
Base.Link (entrance, 'e_e', 0.00, -0.90, 0.30, 0.22, 'To Landing Pad', exit)
Base.Link (bar1, 'b1_b2', -0.20, -0.10, 0.30, 0.22, 'Bar', bar2)
Base.Link (bar1, 'b1_e', 0.00, -0.90, 0.30, 0.22, 'Concourse', exit)
Base.Link (exit, 'e_l', -0.50, -0.30, 0.30, 0.22, 'Landing Pad', landing)
Base.Link (exit, 'e_e', 0.00, -0.90, 0.30, 0.22, 'To Bar', entrance)
Base.Link (bar2, 'b2_b', -0.10, 0.00, 0.30, 0.22, 'Balcony', balcony)
Base.Link (bar2, 'b2_b1', 0.00, -0.90, 0.30, 0.22, 'Bar', bar1)
Base.Link (balcony, 'b_b2', -0.50, -0.30, 0.30, 0.22, 'Bar', bar2)
Base.Link (balcony, 'b_l', 0.20, -0.10, 0.30, 0.22, 'Landing Pad', landing)
Base.Link (balcony, 'b_p', 0.50, 0.20, 0.30, 0.22, 'Panorama', panorama)
Base.Link (fuelbay, 'f_l', 0.00, -0.95, 0.30, 0.22, 'Landing Pad', landing)
Base.Link (fuelbay, 'f_p', -0.70, -0.10, 0.30, 0.22, 'Panorama', panorama)
Base.Link (panorama, 'p_l', 0.10, -0.60, 0.30, 0.22, 'Landing Pad', landing)
Base.Link (panorama, 'p_b', 0.00, -0.30, 0.30, 0.22, 'Balcony', balcony)
Base.Link (panorama, 'p_c', 0.25, -0.40, 0.30, 0.22, 'Concourse', entrance)
Base.Link (panorama, 'p_f', -0.35, -0.30, 0.30, 0.22, 'Fuel Bay', fuelbay)

# SHIP PLACEMENT (size is inverse proportional!)
if (VS.getPlayer().rSize()<=100):
    Base.Ship (landing, 'ship_l', (-0.3, -0.5, 2/shipsize), (0.00, 0.90, -0.20), (-0.7, 0, -0.7))
else:
    Base.Ship (landing, 'ship_l', (-0.4, -0.5, shipsize/20), (0.00, 1.0, 0.00), (0.7, 0, -0.4))
Base.LaunchPython (landing, 'launch','bases/launch_music.py', -0.5, -0.5, 0.5, 0.3, 'Launch Your Ship')

# COMPUTER TERMINALS (News Missions Info Cargo Upgrades ShipDealer)
if (VS.getPlayer().rSize()<=100):
    Base.Comp (landing,  'my_comp_id', 0.17, -0.15, 0.30, 0.22, 'Computer', 'News Missions Info')
else:
    Base.Comp (landing,  'my_comp_id', 0.17, -0.45, 0.30, 0.22, 'Computer', 'News Missions Info')
Base.Comp (fuelbay,  'my_comp_id', 0.20, -0.10, 0.30, 0.22, 'Computer', 'Upgrades Info')
Base.Comp (entrance, 'my_comp_id', -0.90, -0.30, 0.30, 0.22, 'Computer', 'News Missions Upgrades Info Cargo ShipDealer')
Base.Comp (exit,     'my_comp_id', 0.20, -0.30, 0.30, 0.22, 'Computer', 'News Missions Upgrades Info Cargo ShipDealer')

# FIXERS
bartender = vsrandom.randrange(0,19)
Base.Texture (bar1,'bartender','bases/generic/bartender%d.spr' % (bartender), -0.47, 0.15)
Base.Python (bar1, 'talk', -0.67, -0.12, 0.4, 0.4, 'Talk to the Bartender', 'bases/bartender_frigidmud.py',0)
Base.Texture (bar2,'bartender','bases/generic/bartender%d.spr' % (bartender), -0.47, 0.15)
Base.Python (bar2, 'talk', -0.67, -0.12, 0.4, 0.4, 'Talk to the Bartender', 'bases/bartender_frigidmud.py',0)
