import Base
import VS

shipsize = VS.getPlayer().rSize()/35
#print "Ship Size: " + str(shipsize) #debug

# rooms
landing = Base.Room ('Landing Pad')    
Base.Texture (landing, 'tex', 'bases/forest/landing.sprite', 0, 0)
Base.Ship (landing, 'ship_l', (-0.3, -0.3, 5/shipsize), (0.20, 1, -0.20), (-1, 0, -1))

landingexterior = Base.Room ('Landing Exterior')    
Base.Texture (landingexterior, 'tex', 'bases/forest/landingexterior.sprite', 0, 0)
#ship placement in differrent screens is not working
#Base.Ship (landingexterior, 'ship_e', (0.8, -0.9, 3/shipsize), (0, 1, -0.20), (1, 0, -0.5))

concourse = Base.Room ('Concourse')
Base.Texture (concourse, 'tex', 'bases/forest/concourse.sprite', 0, 0)

upgrades = Base.Room ('Maintenance')
Base.Texture (upgrades, 'tex', 'bases/forest/upgrades.sprite', 0, 0)

excursion1 = Base.Room ('Excursion')
Base.Texture (excursion1, 'tex', 'bases/forest/excursion1.sprite', 0, 0)

excursion2 = Base.Room ('Farther Excursion')
Base.Texture (excursion2, 'tex', 'bases/forest/excursion2.sprite', 0, 0)

# links to rooms
Base.Link (landing, 'l_c', 0.00, -0.95, 0.30, 0.22, 'Main Concourse', concourse)
Base.Link (landing, 'l_u', -0.90, -0.50, 0.30, 0.22, 'Maintenance', upgrades)
Base.Link (landing, 'l_le', -0.80, 0.50, 0.30, 0.22, 'Landing Exterior', landingexterior)
Base.Link (landingexterior, 'le_l', 0.40, -0.70, 0.30, 0.22, 'Landing Pad', landing)
Base.Link (landingexterior, 'le_u', -0.30, -0.80, 0.30, 0.22, 'Maintenance', upgrades)
Base.Link (landingexterior, 'le_c', 0.00, -0.90, 0.30, 0.22, 'Main Concourse', concourse)
Base.Link (upgrades, 'u_l', 0.80, -0.10, 0.30, 0.22, 'Landing Pad', landing)
Base.Link (upgrades, 'u_l', 0.00, -0.90, 0.30, 0.22, 'Main Concourse', concourse)
Base.Link (upgrades, 'u_le', -0.95, -.10, 0.30, 0.22, 'Landing Exterior', landingexterior)
Base.Link (excursion1, 'e1_c', -0.9, -0.5, 1, 0.2, 'Main Concourse', concourse)
Base.Link (excursion1, 'e1_e2', 0.9, -0.3, 1, 0.2, 'Farther Excursion', excursion2)
Base.Link (excursion2, 'e2_e1', -0.9, -0.4, 1, 0.2, 'Return', excursion1)
Base.Link (concourse, 'c_l', -0.50, -0.90, 0.30, 0.22, 'Landing Pad', landing)
Base.Link (concourse, 'c_e1', -0.00, 0.20, 0.30, 0.22, 'Visit Planet', excursion1)
Base.LaunchPython (landing, 'launch','bases/launch_music.py', -0.5, -0.5, 0.5, 0.3, 'Launch Your Ship')

# computer terminals
Base.Comp (concourse, 'my_comp_id', -0.60, 0.00, 0.30, 0.22, 'Terminal', 'News Missions Info Cargo ShipDealer')
Base.Comp (upgrades, 'my_comp_id', 0.30, -0.60, 0.30, 0.22, 'Terminal', 'Cargo ShipDealer')

