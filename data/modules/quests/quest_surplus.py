import quest
import Vector
import VS
import unit
import vsrandom
import universe
class quest_surplus (quest.quest):
    def __init__ (self,categories,pr,qr,ships,planets):
        print "surplus %f %f" % (pr, qr)
        print categories
        universe.systemCargoDemand(categories,pr,qr,ships,planets)
    def Execute (self):
        self.removeQuest()
        return 0

class quest_surplus_factory (quest.quest_factory):
    def __init__ (self,categories,priceratio,quantratio,ships,planets,precondition=()):
        print "initin"
        self.ships=ships
        self.planets=planets
        self.priceratio=priceratio
        self.quantratio=quantratio
        self.categories=categories
        self.precond=precondition
        quest.quest_factory.__init__ (self,"quest_surplus")
    def precondition(self,playernum):
        for blah in self.precond:
            print blah
            if (not quest.findQuest (playernum,blah)):
                return 0
        print "suc"
        return 1
    def create (self):
        return quest_surplus(self.categories,self.priceratio,self.quantratio,self.ships,self.planets)

def makeSurplusShortage():
    (sys,mylist)=universe.getAdjacentSystems(VS.getSystemFile(),vsrandom.randrange(1,3))
    adj=vsrandom.uniform (-.5,.5)
    myquest=0
    cats = tuple(set([VS.getRandCargo(1,"").GetCategory(), VS.getRandCargo(1,"").GetCategory()]))
    news=""
    if (adj<0):
        news = "SURPLUS IN %s:  There are reports of surpluses of %s items in the %s system.  Wise Privateers have been purchasing the surpluses in hopes of future gains elsewhere." % (sys," and ".join(cats),sys)
    else:
        news = "DEMAND FOR %s in %s:  Citizens in the %s system have been frusterated by the expensive prices and low quantities of %s goods.  Privateers are currently banding together to try to bring supply to meet the demand.  But while prices are still high, some big traders could certainly capitalize on this shortage." % (" AND ".join(cats),sys,sys," and ".join(cats))
    return (sys,quest_surplus_factory(cats,1+adj,1-adj,1,1),news)

