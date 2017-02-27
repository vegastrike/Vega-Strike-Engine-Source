import VS
import custom

class PlayerInfo:
	def __init__(self):
		self.myUnit = VS.getPlayer()
		self.myTarget = VS.Unit()
		self.sendingShipinfoRequest = False

plr = None

def getTargetShipInfo(plr):
	print 'running getTargetShipInfo'
	newTarget = plr.myTarget
	fg = newTarget.getFlightgroupName()
	def targetResponse(args):
		plr.sendingShipinfoRequest=False;
		if (newTarget == plr.myUnit.GetTarget()):
			VS.setTargetLabel('\n'.join(args))
		else: # Our target changed... try again:
			getTargetShipInfo(plr)
	if fg and fg!='Base' and not plr.sendingShipinfoRequest:
		custom.run("shipinfo",[fg],targetResponse)
		plr.sendingShipinfoRequest=True
	
def Execute():
	global plr
	if not VS.networked():
		return
	if not plr:
		plr = PlayerInfo()
	if VS.getPlayer() != plr.myUnit:
		plr = PlayerInfo()
	if not plr.myUnit:
		return
	if plr.myUnit.GetTarget() != plr.myTarget:
		plr.myTarget = plr.myUnit.GetTarget()
		print 'plr.myUnit.GetTarget() changed!'
		getTargetShipInfo(plr)

