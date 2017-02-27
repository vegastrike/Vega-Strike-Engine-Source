import Base
import vsrandom
import fixers
def MakeBar (room,time_of_day='_day',bartext='bases/bartender_default.py', BaseTexture="bases/generic/bar"):
    bar = Base.Room ('Bar')
    Base.Texture (bar, 'tex', BaseTexture+'.spr', 0, 0)
    Base.Python (bar, 'talk', -0.138672, -0.127604, 0.306641, 0.341146, 'Talk to the Bartender',bartext,0)
    Base.Link (bar, 'Back', -0.998047, -0.997396, 2, 0.28125, 'Exit the Bar', room)
    Base.Comp (bar, 'newscomp', 0.505859, 0.153646, 0.248047, 0.328125, 'GNN News', 'News')
    i=vsrandom.randrange(0,20)
    file='bases/generic/bartender%d.spr' % (i)
    Base.Texture(bar,'bartender',file,0,-0.0035)
    fixers.CreateFixers(bar,[(-.65,-.815,.25,.667),
			(.525,-.815,.25,.667),
			#(-1.05,-.974,.25,.667),
			])
    return bar
