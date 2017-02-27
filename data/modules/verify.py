from campaign_lib import *
import campaigns
import verify_missions
campaigns.loadAll(0)
global traverser
traverser=[]
def isType(a,typ):
    return a.__init__.im_func==typ.__init__.im_func
exnonescript=None

class MyCondition:
	def __init__(self,var=[]):
		self.system=("","")
		self.vars=var
	def pre(self,precond):

		#FIXME
		return True
	def script(self,scriptname,trav):
		if (scriptname==None and not isType(trav.node,CampaignChoiceNode)):
			trav.warn("None script")
			return False
		return isinstance(scriptname,TrueSubnode)
	def post(self,postcond):
		#FIXME
		return True
def EndNode(node):
	return CampaignEndNode(node.campaign).subnodes[0]
def IsEnd(node):
	precond=CampaignEndNode(node.campaign).subnodes[0].preconditions[0]
	try:
		if (len(node.preconditions)!=1):
			return False
	except:
		print node
		print dir(node)
		print node.preconditions
# Ignore this, don't try and replace it with isinstance because it won't work!
	if (node.preconditions[0].__init__.im_func!=precond.__init__.im_func):
		return False
	return node.preconditions[0].system==precond.system and  node.preconditions[0].dockedshipname==precond.dockedshipname
def IsSuperimposition(node):
	precons=node.preconditions
	currnodesys=[]
	currnodeship=[]
	for pc in precons:
		if isinstance(pc,InSystemCondition):
			currnodesys.append(pc.system)
			currnodeship.append(pc.dockedshipname)
	if not len(currnodesys):
		return False
	if len(currnodesys)>1:
		print "Two system checks for the same node.  Impossible to evaluate as True."
		return True
	selector=node.script
	if selector is None:
		return False
	elif isinstance(selector,TrueSubnode):
		pass#print "TrueSubnode found ... evaluating."
	else:
		return False
	subnodes=node.subnodes
	snp=[]
	for i in range(len(subnodes)):
		snp=snp+subnodes[i].preconditions
	subnodesys=[]
	subnodeship=[]
	for pc in snp:
		if isinstance(pc,InSystemCondition):
			subnodesys.append(pc.system)
			subnodeship.append(pc.dockedshipname)
	if len(subnodesys)<1:
		return False
	valid=True
	for i in range(len(subnodesys)):
		if subnodesys[i] != currnodesys[0]:
			valid=False
			break
		elif not (currnodeship[0] is None or subnodeship[i] is None or currnodeship[0] == subnodeship[i]):
			valid=False
			break
	if valid:
		return False
	else:
		print currnodesys+currnodeship
		print subnodesys+subnodeship
		return True
def ValidMissions(node):
	missions=[]
	sc=node.script
	while True:
		if sc==None:
			break
		if isinstance(sc,LoadMission):
			missions.append(sc)
		sc=sc.nextscript
	if missions == []:
		return True
	valid = True
	for m in missions:
#		print m.mname + " :: " + str(m.args) + " :: " + m.name
		print " ++ Mission '%s'"%m.mname
		if not verify_missions.verifyMission(m.mname,m.args):
			valid = False
	return valid
class Traverser:
	def __init__(self,campaign):
		self.node=campaign.root
		self.system=[]
		self.visitednodes=[self.node]
	def dup(self):
		tmp=Traverser(self.node.campaign)
		tmp.system=self.system
		tmp.node=self.node
		tmp.visitednodes=self.visitednodes
		return tmp
	def warn(self,strin):
		print strin
	def cont(self,newnode):
		if newnode in self.visitednodes:
			self.node=EndNode(self.node)
		else:
			self.node=newnode
			self.visitednodes.append(newnode)
	def Update(self,var):
		if (IsEnd(self.node)):
			return None
		if (IsSuperimposition(self.node)):
			self.warn("Warning: Superimposition detected.")
		if not ValidMissions(self.node):
			self.warn("Warning: Invalid Mission Arguments.")
		cond=MyCondition(var)
		if not cond.pre(self.node.preconditions):
			return None
		if cond.script(self.node.script,self):
			#true subnode
			for sn in self.node.subnodes:
				if (cond.post(sn.preconditions)):
					tmp=self.dup()
					tmp.cont(sn)
					traverser.append(tmp)
			if (self.node.contingency):
				self.cont(self.node.contingency)
			else:
				self.cont(EndNode(self.node))
		elif self.node.contingency:
			for sn in self.node.subnodes:
				tmp=self.dup()
				tmp.cont(sn)
				traverser.append(tmp)
			if (not (isType(self.node,CampaignClickNode) or isType(self.node,CampaignChoiceNode))):
				self.warn("Warning: Contingency in non trueSubnode")
				global exnonescript
				exnonescript=self.node
			self.cont(self.node.contingency)
		elif (len(self.node.subnodes)):
			if (len(self.node.subnodes)>1):
				for sn in self.node.subnodes[1:]:
					tmp=self.dup()
					tmp.cont(sn)
					traverser.append(tmp)
			self.cont(self.node.subnodes[0])
		else:
			return None#cannot make progres
		return cond.vars;
def IsFinished():
	global traverser
	for i in range(len(traverser)-1,-1,-1):

		if IsEnd(traverser[i].node):
			del traverser[i]

	return len(traverser)>0
for iter in campaigns.campaigns:
	import verify_missions
	verify_missions.campaign_name[iter.name]=1
	traverser.append(Traverser(iter))
print len(traverser)
while len(traverser):
	IsFinished()
	progress=False
	variables=[]
	for i in range(len(traverser)):
		var=traverser[i].Update(variables)
		if (type(var)==type([])):
			variables=var
			progress=True
	if not progress and len(traverser):
		print "Deadlock "
		break
