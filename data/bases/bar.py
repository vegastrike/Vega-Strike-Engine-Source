import Base
import VS
import vsrandom

bartenders=['0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','18','19']
		
def MakeBar(concourse, concoursetext, bartext, BaseTexture, createCampaignFixers=True,  defaultbtr=None, forcedefbtr=False, bartenderprefix="", bartenderloc=(.89725,.0813), patrons=[],fixerlocations=[(-0.80025, -1.0088, 0.776, 1.2416),(-0.0725, -0.4058125, 0.1758125, 0.5385)]):
	if vsrandom.random()<.6:
		forcedefbtr=True # remove me if you want random bartenders at random bars
	import bartender
	bartender.speaktimes=0
	room0 = Base.Room ('Bar')
	x=0
	y=0
	Base.Texture (room0, 'background', BaseTexture+'.spr', x, y)
	for p in patrons:
		if vsrandom.random()<.85:
			Base.Texture(room0,str(p[0]),'bases/generic/'+str(p[0])+'.spr',float(p[1]),float(p[2]))
	if defaultbtr:
		if not forcedefbtr:
			defaultbtr=bartenders[vsrandom.randrange(len(bartenders))]
		Base.Texture(room0,'btr','bases/generic/'+bartenderprefix+'bartender'+str(defaultbtr)+'.spr', float(bartenderloc[0]),float(bartenderloc[1]))
	if VS.networked():
		Base.Python (room0, 'newscomp', -1, .7, .3, .3, 'Public Terminal', '#\nimport custom\ncustom.run("computer",[],None)',True)
	else:
		Base.Comp (room0, 'newscomp', -1, .7, .3, .3, 'Public Terminal', 'News')
	Base.Link (room0, 'my_link_id', -0.998047, -0.997396, 1.99414, 0.119792, concoursetext ,concourse)
	Base.Python (room0, 'talk', bartenderloc[0]-.16, bartenderloc[1]-.15,.3,.3,'Talk to the Bartender',bartext,0)
	import fixers
	func=fixers.CreateFixers
	if not createCampaignFixers:
		func=fixers.CreateMissionFixers
	func(room0,[(fixerlocations[0][0],fixerlocations[0][1],fixerlocations[0][2],fixerlocations[0][3],"_1"),(fixerlocations[1][0],fixerlocations[1][1],fixerlocations[1][2],fixerlocations[1][3],"_2")])#add more locations?
	return room0;


def MakeMiningBar(concourse,concoursetext,background,bartext):
	import bar
	return bar.MakeBar(concourse,
		concoursetext,
		bartext,
		background,
		True,
		'0',
		False,
		'',
		(.87735,-.018),
		[],
		[
		(-0.8025, -1.055 , 0.5, .8),
		(-0.1725, -0.4558125, 0.1758125, 0.5385),
		#(-0.0325, -0.3558125, 0.1758125, 0.5385),
		])
		
def MakeAgriculturalBar(concourse,concoursetext,background,bartext):
	import bar
	room= bar.MakeBar(concourse,
		concoursetext,
		bartext,
		background,
		True,
		'0',
		False,
		'big_',
		(-.86,-.047),
		[],
		[(0.4905, -.7195 , 0.5, .8),(0.050, -0.41858125, 0.1758125, 0.5385)])
	import Base
	Base.Texture(room,'counter',background+'_counter.spr',-.665,-.137)
	return room
def MakeOceanBar(concourse,concoursetext,background,bartext):
	import bar
	return bar.MakeBar(concourse,
		concoursetext,
		bartext,
		background,
		True,
		'0',
		False,
		'small_',
		(.45,.109),
		[],
		[(0.4725, -.735 , 0.5, .8),(-0.875, -0.38858125, 0.1758125, 0.5385)])
