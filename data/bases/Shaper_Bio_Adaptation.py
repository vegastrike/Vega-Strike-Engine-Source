import Base
import VS

shipsize = VS.getPlayer().rSize()/35
#print "Ship Size: " + str(shipsize) #debug

# rooms
landing = Base.Room ('Landing Pad')    
Base.Texture (landing, 'tex', 'bases/Shaper_Bio_Adaptation/landing.sprite', 0, 0)
# ship size is inverse proportional!
Base.Ship (landing, 'ship_l', (-0.3, -0.5, 5/shipsize), (0.00, 0.90, -0.20), (-0.7, 0, -0.7))

Base.LaunchPython (landing, 'launch','bases/launch_music.py', -0.5, -0.5, 0.5, 0.3, 'Launch Your Ship')

# computer terminals
Base.Comp (landing, 'my_comp_id', -0.75, -0.35, 0.30, 0.22, 'Computer', 'Info Missions News Upgrade Cargo ShipDealer')

