import Base

def MakeWeapon (room, time_of_day='_day'):
    weap = Base.Room ('Weapons Room')
    Base.Texture (weap, 'tex', 'bases/generic/weapon.spr', 0, 0)
    Base.Comp (weap, 'comp', 0.5, -0.4, 0.5, 0.765625, 'Upgrade your Ship', 'Upgrade Info ShipDealer')
    Base.Link (weap, 'Back', -0.998047, -0.997396, 2, 0.28125, 'Exit the Weapons Room', room)
    Base.Link (weap, 'Back', -1, -1, .5, 2, 'Exit the Weapons Room', room)

    return weap
