import Base

room = Base.Room ('Landing Platform')
Base.Launch (room, 'launch', -0.841797, -0.859375, 1.5625, 0.565104, 'Launch Your Ship')
Base.Link (room, 'Conc', -0.453125, -0.291667, 0.767578, 0.770833, 'Main Concourse', 2)
Base.Texture (room, 'tex', 'bases/generic/base_entrance.spr', 0, 0)
Base.Ship (room, 'ship', (0,-0.3,4), (-1, 0, 0), (0, 0.93, -0.34))

room = Base.Room ('Mission Room')
Base.Comp (room, 'missioncomp', 0.2775, -0.28, 0.7175, 0.503333, 'Mission Computer', 'Missions News')
Base.Link (room, 'Conc', -0.9975, -0.996667, 1.9925, 0.373333, 'Main Concourse', 2)
Base.Texture (room, 'tex', 'bases/commerce/missioninside.spr', 0, 0)

room = Base.Room ('Main Concourse')
Base.Link (room, 'launchplat', -1, -0.993333, 1.9875, 0.363333, 'Landing Platform', 0)
Base.Comp (room, 'buysell', -0.0625, -0.62, 0.7575, 0.3, 'Cargo Computer', 'Cargo')
Base.Link (room, 'mall', -0.6675, -0.623333, 0.2375, 0.28, 'Shopping Mall', 3)
Base.Link (room, 'mission', -0.8925, -0.34, 1.89, 0.446667, 'Mission Room', 1)
Base.Texture (room, 'tex', 'bases/commerce/concourse.spr', 0, 0)

room = Base.Room ('Shopping Mall')
Base.Link (room, 'Conc', -0.7025, -0.756667, 0.3375, 1.75667, 'Main Concourse', 2)
Base.Comp (room, 'upg', -0.3625, -0.726667, 0.2925, 1.72, 'Upgrade Ship', 'Upgrade')
Base.Comp (room, 'shd', 0.06, -0.68, 0.345, 0.74, 'Ship Dealer', 'ShipDealer')
Base.Texture (room, 'tex', 'bases/commerce/shoppingmall.spr', 0, 0)

