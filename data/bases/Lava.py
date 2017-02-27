import Base
import VS

shipsize = VS.getPlayer().rSize()/35
#print "Ship Size: " + str(shipsize) #debug

landing = Base.Room ('Landing Pad')    
Base.Texture (landing, 'tex', 'bases/Lava/landing.sprite', 0, 0)
Base.Ship (landing, 'ship', (-0.3, -0.3, 10/shipsize), (0, 0.93, -0.34), (-1, 0, -1))

landscape = Base.Room ('Planet Exterior')
Base.Texture (landscape, 'tex', 'bases/Lava/landscape.sprite', 0, 0)

Base.Link (landing, 'ExtLink', 0.40, 0.02, 0.30, 0.22, 'Planet Exterior', landscape)
Base.Link (landscape, 'Landing', -0.5, -1, 1, 0.2, 'Landing Pad', landing)
Base.LaunchPython (landing, 'launch','bases/launch_music.py', -0.5, -0.5, 0.5, 0.3, 'Launch Your Ship')
