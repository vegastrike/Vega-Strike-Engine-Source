
#use this to allow more interesting weightings than are feasible to manually enter
def weightedlist(tuples):
  rettuple=[]
  for i in xrange(0,len(tuples)):
    for j in xrange(tuples[i][1]):
      rettuple.append(tuples[i][0])
  return tuple(rettuple)

confed=0
aera=1
rlaan=2
merchant=3
merchant_guild=3
luddites=4
pirates=5
hunter=6
homeland_security=7
ISO=8
unknown=9
andolian=10
highborn=11
shaper=12
unadorned=13
purist=14
forsaken=15
LIHW=16
uln=17
dgn=18
klkk=19
mechanist=20
shmrn=21
rlaan_briin=22

aeran_merchant_marine=23
rlaan_citizen=24
merchant_citizen=25
merchant_guild_citizen=25
andolian_citizen=26
highborn_citizen=27
shaper_citizen=28
unadorned_citizen=29
purist_citizen=30
forsaken_citizen=31
LIHW_citizen=32
uln_citizen=33
dgn_citizen=34
klkk_citizen=35
mechanist_citizen=36
shmrn_citizen=37

fortress_systems={"Crucible/Cephid_17":1-.03625}
invincible_systems={}

max_flightgroups={"Gemini/Troy":25,"Gemini/Penders_Star":15,"Gemini/Junction":12,"Crucible/Cephid_17":22}
min_flightgroups={"Gemini/Troy":22,"Gemini/Penders_Star":10,"Gemini/Junction":4,"Crucible/Cephid_17":22}

factions = ("confed","aera","rlaan","merchant_guild","luddites","pirates","hunter","homeland-security","ISO","unknown","andolian","highborn","shaper","unadorned","purist","forsaken","LIHW","uln","dgn","klkk","mechanist","shmrn","rlaan_briin","aeran_merchant_marine","rlaan_citizen","merchant_guild_citizen","andolian_citizen","highborn_citizen","shaper_citizen","unadorned_citizen","purist_citizen","forsaken_citizen","LIHW_citizen","uln_citizen","dgn_citizen","klkk_citizen","mechanist_citizen","shmrn_citizen")
factiondict={}
for i in xrange(len(factions)):
    factiondict[factions[i]]=i
factiondict["retro"]=luddites
factiondict["militia"]=homeland_security
factiondict["merchant"]=merchant_guild_citizen

siegingfactions={"confed":10
                ,"andolian":10
                ,"highborn":10
                ,"shaper":10
                ,"unadorned":10
                ,"purist":10
                ,"forsaken":100
                ,"LIHW":50
                ,"aera":10
                ,"rlaan":10
                ,"ISO":40
                ,"luddite":100
                ,"uln":150
                ,"mechanist":9
                }

fightersPerFG=  {"confed":10
                ,"andolian":10
                ,"highborn":10
                ,"shaper":10
                ,"unadorned":10
                ,"purist":10
                ,"forsaken":3
                ,"LIHW":6
                ,"aera":8
                ,"rlaan":11
                ,"ISO":8
                ,"luddite":4
                ,"uln":2
                ,"merchant_guild":3
                ,"pirates":6
                ,"hunter":1
                ,"homeland-security":6
                ,"default":10
                ,"dgn":4
                ,"klkk":4
                ,"mechanist":8
                ,"shmrn":10
                ,"rlaan_briin":2

                ,"andolian_citizen":24*2
                ,"highborn_citizen":24*2
                ,"shaper_citizen":24*2
                ,"unadorned_citizen":24*2
                ,"purist_citizen":24*2
                ,"forsaken_citizen":6*2
                ,"LIHW_citizen":12*2
                ,"aeran_merchant_marine":24*2
                ,"rlaan_citizen":36*2
                ,"uln_citizen":12*2
                ,"merchant_guild_citizen":48*2
                ,"dgn_citizen":12*2
                ,"klkk_citizen":24*2
                ,"mechanist_citizen":12*2
                ,"shmrn_citizen":12*2
                }

capitalsPerFG=  {"confed":1
                ,"andolian":1
                ,"highborn":1
                ,"shaper":1
                ,"unadorned":1
                ,"purist":1
                ,"forsaken":1
                ,"LIHW":1
                ,"aera":1
                ,"rlaan":1
                ,"ISO":1
                ,"luddite":1
                ,"uln":1
                ,"merchant_guild":1
                ,"pirates":1
                ,"hunter":1
                ,"homeland-security":1
                ,"default":1
                ,"dgn":1
                ,"klkk":1
                ,"mechanist":1
                ,"shmrn":1
                ,"rlaan_briin":1
                ,"andolian_citizen":2
                ,"highborn_citizen":1
                ,"shaper_citizen":0
                ,"unadorned_citizen":0
                ,"purist_citizen":0
                ,"forsaken_citizen":0
                ,"LIHW_citizen":0
                ,"aeran_merchant_marine":1
                ,"rlaan_citizen":0
                ,"uln_citizen":0
                ,"merchant_guild_citizen":2
                ,"dgn_citizen":0
                ,"klkk_citizen":0
                ,"mechanist_citizen":0
                ,"shmrn_citizen":0
                }

staticFighterProduction={"luddites":3, "pirates":1}

fighterProductionRate=  {"confed":.01
                        ,"andolian":.1
                        ,"highborn":.15
                        ,"shaper":.1
                        ,"unadorned":.1
                        ,"purist":.1
                        ,"forsaken":.1
                        ,"LIHW":.05
                        ,"aera":.12
                        ,"rlaan":.11
                        ,"ISO":.14
                        ,"luddite":.04
                        ,"uln":.1
                        ,"merchant_guild":.1
                        ,"pirates":.1
                        ,"hunter":.1
                        ,"homeland-security":.05
                        ,"default":.1
                        ,"dgn":.1
                        ,"klkk":.1
                        ,"mechanist":.1
                        ,"shmrn":.08
                        ,"rlaan_briin":.05

                        ,"andolian_citizen":1
                        ,"highborn_citizen":1
                        ,"shaper_citizen":1
                        ,"unadorned_citizen":1
                        ,"purist_citizen":1
                        ,"forsaken_citizen":.3
                        ,"LIHW_citizen":.60
                        ,"aeran_merchant_marine":.80
                        ,"rlaan_citizen":1.10
                        ,"uln_citizen":1.00
                        ,"merchant_guild_citizen":3.00
                        ,"dgn_citizen":.40
                        ,"klkk_citizen":1.00
                        ,"mechanist_citizen":.80
                        ,"shmrn_citizen":.20
                        }

capitalProductionRate=  {"confed":.002
                        ,"andolian":.025
                        ,"highborn":.02
                        ,"shaper":.02
                        ,"unadorned":.02
                        ,"purist":.02
                        ,"forsaken":.02
                        ,"LIHW":.01
                        ,"aera":.024
                        ,"rlaan":.022
                        ,"ISO":.028
                        ,"luddite":.004
                        ,"uln":.02
                        ,"merchant_guild":.02
                        ,"pirates":.02
                        ,"hunter":.02
                        ,"homeland-security":.001
                        ,"default":.02
                        ,"dgn":.02
                        ,"klkk":.02
                        ,"mechanist":.02
                        ,"shmrn":.001
                        ,"rlaan_briin":.001

                        ,"andolian_citizen":.1
                        ,"highborn_citizen":.05
                        ,"shaper_citizen":.01
                        ,"unadorned_citizen":.01
                        ,"purist_citizen":.01
                        ,"forsaken_citizen":.003
                        ,"LIHW_citizen":.0060
                        ,"aeran_merchant_marine":.0080
                        ,"rlaan_citizen":.110
                        ,"uln_citizen":.0100
                        ,"merchant_guild_citizen":.200
                        ,"dgn_citizen":.0040
                        ,"klkk_citizen":.00500
                        ,"mechanist_citizen":.0080
                        ,"shmrn_citizen":.0020
                        }

#FIXME homeworlds should *exist*
homeworlds={"confed":"Sol/Sol"
                ,"aera":"enigma_sector/shelton"
                ,"rlaan":"enigma_sector/shanha"
                ,"ISO":"enigma_sector/defiance"
                }
production_centers={"confed":["Sol/Sol"]
                ,"aera":["enigma_sector/shelton"]
                ,"rlaan":["enigma_sector/shanha"]
                ,"ISO":["enigma_sector/defiance"]
                }
earnable_upgrades={} #tech tree (new)

def Precache():
	pass#fixme

useStock = (   0    ,  0   ,   0   ,     1    ,   0   ,       0     ,    0   ,         0    ,      0  ,    0,         0    ,    0     ,   0   ,     0      ,   0   ,     1     ,    1 ,  1  ,  0  ,   0 , 0 ,  1 , 1  ,      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0   )#close ones are all civvies

enemies =  ((aera,aera,luddites,pirates,ISO), #confed
            (confed,confed,confed,confed,confed,confed,homeland_security,rlaan,rlaan,rlaan,rlaan,rlaan,rlaan_citizen,rlaan_citizen,rlaan_citizen,rlaan_citizen,rlaan_citizen,pirates,hunter,merchant_guild,merchant_guild_citizen,ISO,andolian,highborn,shaper,unadorned,purist,forsaken_citizen,LIHW,andolian_citizen,highborn_citizen,shaper_citizen,unadorned_citizen,purist_citizen,forsaken_citizen,LIHW_citizen), #aera
            (aera,aera,aera,aera,aera,aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,luddites,luddites,luddites,luddites,luddites,hunter,highborn,highborn_citizen),#rlaan
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,aeran_merchant_marine,luddites,pirates,pirates,pirates,pirates,pirates,pirates,pirates,pirates,pirates,forsaken,forsaken_citizen), #merchant_guild
            (confed,confed,confed,homeland_security,rlaan,rlaan_citizen,rlaan_citizen,pirates,hunter,merchant_guild,merchant_guild,merchant_guild,merchant_guild,merchant_guild,merchant_guild,merchant_guild,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,ISO,ISO,ISO,ISO,hunter,hunter,hunter,hunter,hunter), #luddites
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,rlaan,rlaan,rlaan,rlaan_citizen,rlaan_citizen,rlaan_citizen,luddites,aera,aera,aera,aeran_merchant_marine,aeran_merchant_marine,aeran_merchant_marine,merchant_guild,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild_citizen,ISO,andolian,highborn,shaper,unadorned,purist,andolian_citizen,highborn_citizen,shaper_citizen,unadorned_citizen,purist_citizen), #pirates
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,aeran_merchant_marine,aeran_merchant_marine,luddites,luddites,luddites,rlaan,rlaan_citizen,pirates,pirates,ISO), #hunter
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,luddites,pirates,ISO,forsaken,forsaken_citizen), #homeland_security
            (confed,confed,confed,confed,confed,confed,confed,homeland_security,homeland_security,homeland_security,aera,aera,aera,pirates,luddites,luddites,luddites,hunter,highborn,highborn_citizen,shaper,shaper_citizen,purist,purist_citizen), #ISO
            (confed,aera,rlaan,merchant_guild,luddites,pirates,hunter,homeland_security,ISO,andolian,highborn,shaper,unadorned,purist,forsaken,LIHW,uln,dgn,aeran_merchant_marine,rlaan_citizen,merchant_guild_citizen,andolian_citizen,highborn_citizen,shaper_citizen,unadorned_citizen,purist_citizen,forsaken_citizen,LIHW_citizen,uln_citizen,dgn_citizen), #unknown
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,luddites,luddites,pirates), #andolian
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,luddites,pirates,ISO,ISO), #highborn
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,luddites,luddites,ISO), #shaper
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,luddites,luddites), #unadorned
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,ISO,ISO), #purist
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,luddites,homeland_security), #forsaken
            (aera,aera,aeran_merchant_marine,luddites), #LIHW
            (aera,rlaan,aeran_merchant_marine,rlaan_citizen,confed), #uln
            (aera,aeran_merchant_marine,pirates), #dgn
            (aera,aeran_merchant_marine,luddites,luddites,pirates), #klkk
            (luddites,luddites,pirates,aera,aera,pirates),#mechanist
            (aera,aeran_merchant_marine,rlaan,rlaan_citizen,confed), #shmrn
            (aera,aeran_merchant_marine,luddites,pirates), #rlaan_briin
            (confed,confed,confed,confed,confed,confed,homeland_security,rlaan,rlaan,rlaan,rlaan,rlaan,rlaan_citizen,rlaan_citizen,rlaan_citizen,rlaan_citizen,rlaan_citizen,pirates,hunter,merchant_guild,merchant_guild_citizen,ISO,andolian,highborn,shaper,unadorned,purist,forsaken_citizen,LIHW,andolian_citizen,highborn_citizen,shaper_citizen,unadorned_citizen,purist_citizen,forsaken_citizen,LIHW_citizen), #aeran_merchant_marine
            (aera,aera,aera,aera,aera,aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,luddites,luddites,luddites,luddites,luddites,hunter,highborn,highborn_citizen),#rlaan_citizen
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,aeran_merchant_marine,luddites,pirates,pirates,pirates,pirates,pirates,pirates,pirates,pirates,pirates,forsaken,forsaken_citizen), #merchant_guild_citizen
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,luddites,luddites,pirates), #andolian_citizen
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,luddites,pirates,ISO,ISO), #highborn_citizen
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,luddites,luddites,ISO), #shaper_citizen
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,luddites,luddites), #unadorned_citizen
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,pirates,ISO,ISO), #purist_citizen
            (aera,aera,aeran_merchant_marine,aeran_merchant_marine,luddites,homeland_security), #forsaken_citizen
            (aera,aera,aeran_merchant_marine,luddites), #LIHW_citizen
            (aera,rlaan,aeran_merchant_marine,rlaan_citizen,confed), #uln_citizen
            (aera,aeran_merchant_marine,pirates), #dgn_citizen
            (aera,aeran_merchant_marine,luddites,luddites,pirates), #klkk_citizen
            (luddites,luddites,pirates,aera,aera,pirates),#mechanist_citizen
            (aera,aeran_merchant_marine,rlaan,rlaan_citizen,confed) #shmrn_citizen
           )


rabble  =  ((luddites,pirates,ISO,pirates,ISO,pirates,ISO,pirates,pirates,pirates,pirates,ISO,forsaken,forsaken_citizen), #confed
            (pirates,pirates,pirates,hunter,hunter,pirates,pirates,pirates,hunter,hunter,confed,andolian,rlaan,uln,uln_citizen,uln_citizen), #aera
            (pirates,pirates,pirates,pirates,aera,aera,aeran_merchant_marine,confed,hunter,hunter,hunter,uln,uln_citizen,uln_citizen),#rlaan
            (pirates,luddites,pirates,luddites), #merchant_guild
            (homeland_security,homeland_security,ISO,hunter,pirates), #luddites
            (hunter,luddites,ISO,homeland_security), #pirates
            (pirates,luddites,ISO), #hunter
            (luddites,pirates,ISO,forsaken,forsaken_citizen), #homeland_security
            (homeland_security,homeland_security,homeland_security,pirates,luddites,luddites,luddites,hunter), #ISO
            (pirates,pirates,pirates,pirates,luddites,ISO,forsaken,forsaken_citizen,aera,aera,aeran_merchant_marine,rlaan,confed,uln,uln_citizen,uln_citizen,dgn), #unknown
            (luddites,luddites,pirates,luddites,pirates,pirates,pirates,aera,aera,aeran_merchant_marine,rlaan), #andolian
            (luddites,pirates,ISO,ISO,pirates,ISO,ISO,luddites,aera,aera,aeran_merchant_marine,rlaan), #highborn
            (pirates,luddites,luddites,ISO,pirates,luddites,luddites,ISO,aera,aera,aeran_merchant_marine,rlaan), #shaper
            (pirates,luddites,luddites,pirates,luddites,luddites,ISO,aera,aera,aeran_merchant_marine,rlaan), #unadorned
            (pirates,ISO,ISO,pirates,ISO,ISO,aera,aera,aeran_merchant_marine,rlaan), #purist
            (luddites,homeland_security), #forsaken
            (luddites,luddites,luddites,aera,aera,aeran_merchant_marine,rlaan), #LIHW
            (hunter,hunter,hunter,aera,aera,aeran_merchant_marine,rlaan,confed), #uln
            (pirates,pirates,pirates,), #dgn,dgn_citizen
            (luddites,luddites,pirates), #klkk
            (pirates,luddites,luddites,ISO,pirates,luddites,luddites,ISO,aera,aera,aeran_merchant_marine,rlaan), #mechanist
            (hunter,hunter,hunter,aera,aera,aeran_merchant_marine,rlaan), #shmrn
            (pirates,pirates,pirates,pirates,aera,aera,aeran_merchant_marine,confed,hunter,hunter,hunter,uln,uln_citizen,uln_citizen),#rlaan_briin
            (pirates,pirates,pirates,hunter,hunter,pirates,pirates,pirates,hunter,hunter,confed,andolian,rlaan,uln,uln_citizen,uln_citizen), #aeran_merchant_marine
            (pirates,pirates,pirates,pirates,aera,aera,aeran_merchant_marine,confed,hunter,hunter,hunter,uln,uln_citizen,uln_citizen),#rlaan_citizen
            (pirates,luddites,pirates,luddites), #merchant_guild_citizen
            (luddites,luddites,pirates,luddites,pirates,pirates,pirates,aera,aera,aeran_merchant_marine,rlaan), #andolian_citizen
            (luddites,pirates,ISO,ISO,pirates,ISO,ISO,luddites,aera,aera,aeran_merchant_marine,rlaan), #highborn_citizen
            (pirates,luddites,luddites,ISO,pirates,luddites,luddites,ISO,aera,aera,aeran_merchant_marine,rlaan), #shaper_citizen
            (pirates,luddites,luddites,pirates,luddites,luddites,ISO,aera,aera,aeran_merchant_marine,rlaan), #unadorned_citizen
            (pirates,ISO,ISO,pirates,ISO,ISO,aera,aera,aeran_merchant_marine,rlaan), #purist_citizen
            (luddites,homeland_security), #forsaken_citizen
            (luddites,luddites,luddites,aera,aera,aeran_merchant_marine,rlaan), #LIHW_citizen
            (hunter,hunter,hunter,aera,aera,aeran_merchant_marine,rlaan,confed), #uln_citizen
            (pirates,pirates,pirates,), #dgn_citizen
            (luddites,luddites,pirates), #klkk_citizen
            (pirates,luddites,luddites,ISO,pirates,luddites,luddites,ISO,aera,aera,aeran_merchant_marine,rlaan), #mechanist_citizen
            (hunter,hunter,hunter,aera,aera,aeran_merchant_marine,rlaan), #shmrn_citizen
           )

insysenemies  =  enemies

friendlies=((confed,confed,confed,confed,confed,confed,confed,confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,highborn,highborn_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,purist,purist_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen), #confed
            (aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,uln,uln_citizen,uln_citizen), #aera
            (uln,uln_citizen,uln_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen), #rlaan
            (ISO,confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,hunter,rlaan,rlaan_citizen,andolian,andolian_citizen,highborn,highborn_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,purist,purist_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen), #merchant_guild
            (luddites,luddites,luddites), #luddites
            (forsaken,forsaken_citizen,uln,uln_citizen,uln_citizen,LIHW,LIHW_citizen,pirates,pirates,pirates,pirates,pirates), #pirates
            (confed,confed,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,hunter,hunter,hunter,hunter,hunter,merchant_guild_citizen,merchant_guild_citizen,merchant_guild), #hunter
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,highborn,highborn_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,purist,purist_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen), #homeland_security
            (ISO,ISO,ISO,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,ISO,ISO,ISO,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,ISO,ISO,ISO,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild), #ISO
            (merchant_guild_citizen,merchant_guild_citizen,merchant_guild,), #unknown
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,unadorned,unadorned_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #andolian
            (confed,confed,confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,shaper,shaper_citizen,purist,purist_citizen,unadorned,unadorned_citizen,shaper,shaper_citizen,purist,purist_citizen,unadorned,unadorned_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #highborn
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,shaper,shaper_citizen,shaper,shaper_citizen,highborn,highborn_citizen,highborn,highborn_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #shaper
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #unadorned
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,highborn,highborn_citizen,purist,purist_citizen,highborn,highborn_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #purist
            (forsaken,forsaken_citizen,forsaken,forsaken_citizen,pirates,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen), #forsaken
            (forsaken,forsaken_citizen,forsaken,forsaken_citizen,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen), #LIHW
            (uln,uln_citizen,uln_citizen,uln,uln_citizen,uln_citizen,uln,uln_citizen,uln_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,pirates,rlaan,rlaan_citizen,forsaken,forsaken_citizen), #uln
            (dgn,dgn_citizen,dgn,dgn_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen), #dgn
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,unadorned,unadorned_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,klkk,klkk_citizen,klkk,klkk_citizen),  #klkk
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,uln,uln_citizen,uln_citizen),  #mechanist
            (uln,uln_citizen,uln_citizen,shmrn,shmrn_citizen,shmrn,shmrn_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,pirates,rlaan,rlaan_citizen,forsaken,forsaken_citizen), #shmrn
            (confed,confed,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,hunter,rlaan_briin,rlaan_briin,rlaan_briin), #rlaan_briin
            (aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,aera,aera,aeran_merchant_marine,uln,uln_citizen,uln_citizen), #aeran_merchant_marine
            (uln,uln_citizen,uln_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen,rlaan,rlaan_citizen), #rlaan_citizen
            (ISO,confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,hunter,rlaan,rlaan_citizen,andolian,andolian_citizen,highborn,highborn_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,purist,purist_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen), #merchant_guild_citizen
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,unadorned,unadorned_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #andolian_citizen
            (confed,confed,confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,shaper,shaper_citizen,purist,purist_citizen,unadorned,unadorned_citizen,shaper,shaper_citizen,purist,purist_citizen,unadorned,unadorned_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #highborn_citizen
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,shaper,shaper_citizen,shaper,shaper_citizen,highborn,highborn_citizen,highborn,highborn_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #shaper_citizen
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,unadorned,unadorned_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #unadorned_citizen
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,highborn,highborn_citizen,purist,purist_citizen,highborn,highborn_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,purist,purist_citizen,dgn,dgn_citizen,uln,uln_citizen,uln_citizen),  #purist_citizen
            (forsaken,forsaken_citizen,forsaken,forsaken_citizen,pirates,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen), #forsaken_citizen
            (forsaken,forsaken_citizen,forsaken,forsaken_citizen,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen,LIHW,LIHW_citizen), #LIHW_citizen
            (uln,uln_citizen,uln_citizen,uln,uln_citizen,uln_citizen,uln,uln_citizen,uln_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,pirates,rlaan,rlaan_citizen,forsaken,forsaken_citizen), #uln_citizen
            (dgn,dgn_citizen,dgn,dgn_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen), #dgn_citizen
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,andolian,andolian_citizen,unadorned,unadorned_citizen,forsaken,forsaken_citizen,LIHW,LIHW_citizen,klkk,klkk_citizen,klkk,klkk_citizen),  #klkk_citizen
            (confed,confed,confed,homeland_security,homeland_security,homeland_security,homeland_security,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,LIHW,LIHW_citizen,highborn,highborn_citizen,highborn,highborn_citizen,highborn,highborn_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,mechanist,mechanist_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,shaper,shaper_citizen,unadorned,unadorned_citizen,uln,uln_citizen,uln_citizen),  #mechanist_citizen
            (uln,uln_citizen,uln_citizen,shmrn,shmrn_citizen,shmrn,shmrn_citizen,merchant_guild_citizen,merchant_guild_citizen,merchant_guild,pirates,rlaan,rlaan_citizen,forsaken,forsaken_citizen), #shmrn_citizen
           )

fighters = (("Lancelot","Lancelot","Gawain","Lancelot","Gawain","Progeny","Progeny","Pacifier","Schroedinger","Pacifier","Schroedinger","Derivative","Convolution","Derivative","Convolution","Goddard","Franklin","Quicksilver"), #confed
                        ("Nicander","Ariston","Areus"), #aera
                        ("Shizu","Zhuangzong","Taizong"), #rlaan
                        ("Mule","Plowshare"), #merchant_guild
                        ("Redeemer",), #luddites
                        ("Hyena","Plowshare"), #pirates
                        ("Hyena","Robin","Hyena","Robin","Sickle","Hammer","Vendetta.hunter","Entourage"), #hunter
                        ("Admonisher",), #homeland_security
                        ("Hammer","Sickle","Hammer","Sickle","Hammer","Sickle","Hammer","Sickle","Hammer","Sickle","Sickle","Sickle","Franklin","Entourage"), #ISO
                        ("Beholder",), #unknown
                        ("Schroedinger","Schroedinger","Schroedinger","Schroedinger","Schroedinger","Goddard","Goddard","Franklin","Kierkegaard"),#andolian
                        ("Gawain","Lancelot"),#highborn
                        ("Ancestor","Progeny","Progeny"),#shaper
                        ("Derivative","Determinant","Convolution","Derivative","Determinant","Convolution","Franklin"),#unadorned
                        ("Pacifier","Admonisher","Plowshare"),#purist
                        ("Hyena",),#forsaken
                        ("Sickle","Robin","Robin","Robin","Robin","Robin","Hammer"),#LIHW
                        ("Ancestor","Llama","Hyena","Entourage"),#uln
                        ("Dodo","Dodo","Dodo","Quicksilver"), #dgn
                        ("Dostoevsky","Dostoevsky","Dostoevsky","Dostoevsky","Dostoevsky","Kierkegaard"), #klkk
                        ("Llama","Convolution"),#mechanist
                        ("Dirge","Dirge","Regret"),#shmrn
                        ("Zhuangzong","Zhuangzong","Zhuangzong","Gaozong","Shizong","Shizong","Shizong"), #rlaan_briin
                        ("Nicander.escort","Nicander.escort","Ariston"), #aeran_merchant_marine
                        ("Shizu.civvie","Shizu.civvie","Shizu.civvie","Shizu.civvie","Gaozong","Shizong","Shizong","Shizong","Shizong","Shizong"), #rlaan_citizen
                        ("Mule.civvie","Plowshare.civvie","Llama.civvie","Quicksilver.civvie","Entourage"), #merchant_guild_citizen
                        ("Franklin.civvie","Sartre.civvie","Sartre.civvie","Kafka.civvie","Kafka.civvie","Llama.civvie","Quicksilver.civvie","MacGyver"),#andolian_citizen
                        ("Hidalgo.civvie","GTIO.civvie","GTIO.civvie","H496","H496","H496","Entourage","Entourage","Entourage","Entourage","Entourage"),#highborn_citizen
                        ("Mule.civvie","Plowshare.civvie","Llama.civvie","Quicksilver.civvie","Entourage"),#shaper_citizen
                        ("Mule.civvie","Plowshare.civvie","Llama.civvie"),#unadorned_citizen
                        ("Mule.civvie","Llama.civvie","Plowshare.civvie","Mule.civvie","Llama.civvie","Plowshare.civvie","GTIO.civvie","Quicksilver.civvie","Diligence","H496","Entourage"),#purist_citizen
                        ("Koala.civvie","Koala.civvie","Hyena.civvie","Llama.civvie","H496"),#forsaken_citizen
                        ("Koala.civvie","Llama.civvie","Llama.civvie","Llama.civvie","Quicksilver.civvie","H496"),#LIHW_citizen
                        ("Koala.civvie","Koala.civvie","Koala.civvie","Dodo.civvie","Llama.civvie","Entourage"),#uln_citizen
                        ("Koala.civvie","Dodo.civvie","Quicksilver.civvie"), #dgn_citizen
                        ("Kafka.civvie","Kafka.civvie","Sartre.civvie","Llama.civvie",), #klkk_citizen
                        ("Koala.civvie","Kafka.civvie","Llama.civvie"),#mechanist_citizen
                        ("Koala.civvie","Koala.civvie","Kafka.civvie","Kafka.civvie","Sartre.civvie"),#shmrn_citizen

           )
isBomber = {"Areus":6,"Taizong":8,"Pacifier":5,"Goddard":4,"Kierkegaard":5,"Hammer":16,"Admonisher":10,"Areus.blank":6,"Taizong.blank":8,"Pacifier.blank":5,"Goddard.blank":4,"Hammer.blank":16,"Admonisher.blank":10}
unescortable = {"Tesla":"Ox",
	"Kahan":"Mule",
	"Clydesdale":"Ox",
	"Shundi":"Zhuangzong",
	"Ruizong":"Taizong",
	"Agesipolis":"Agasicles",
	"Watson":"Mule",
	"Leonidas":"Agasicles",
	"Anaxidamus":"Agasicles"}

capitals = (("Clydesdale","Watson","Archimedes","Kahan","Hawking"), #confed
            ("Agasicles","Agasicles","Agasicles","Agasicles","Agasicles","Agasicles","Agasicles","Agasicles","Agasicles","Agasicles","Agesipolis","Leonidas","Anaxidamus","Anaxidamus","Anaxidamus","Anaxidamus","Anaxidamus",), #aera
            ("Ruizong","Ruizong","Ruizong","Shundi"), #rlaan
            ("Ox","Ox","Clydesdale"), #merchant_guild
            ("Mule",), #luddites
            ("Thales","Thales","Thales","Thales","Gleaner","Gleaner","Yeoman"), #pirates
            ("Mule",), #hunter
            ("Clydesdale",), #homeland_security
            ("Thales","Mule","Gleaner"), #ISO
            ("Beholder",), #unknown
            ("Kahan","Watson","Archimedes","Tesla","Hawking"),#andolian
            ("Clydesdale",),#highborn
            ("Clydesdale","Midwife","Midwife","Midwife"),#shaper
            ("Watson","Kahan"),#unadorned
            ("Clydesdale","Vigilance","Vigilance","Vigilance"),#purist
            ("Thales",),#forsaken
            ("Ox",),#LIHW
            ("Yeoman","Yeoman","Gleaner","Gleaner","Gleaner"),#uln
            ("Dodo",), #dgn
            ("Kahan",), #klk
            ("Watson",),#mechanist
            ("Yeoman","Gleaner"), #shmrn
            ("Ruizong","Ruizong","Shenzong",), #rlaan_briin
            ("Agasicles","Charillus","Charillus","Charillus","Charillus","Charillus","Charillus","Charillus"), #aeran_merchant_marine FIXME
            ("Shenzong",), #rlaan_citizen FIXME
            ("Mule","Mule","Mule","Mule","Mule","Mule","Ox","Ox","Clydesdale"), #merchant_guild_citizen
            ("Ox","Mule"), #andolian_citizen #FIXME - all citizens a bit b0rken
            ("Ox","Mule"), #highborn_citizen
            ("Ox","Mule","Mule","Mule","Cultivator","Cultivator"), #shaper_citizen
            ("Ox","Mule"), #unadorned_citizen
            ("Ox","Mule"), #purist_citizen
            ("Yeoman","Gleaner","Mule"), #forsaken_citizen
            ("Ox","Mule"), #LIHW_citizen
            ("Gleaner","Gleaner","Gleaner","Gleaner","Mule"), #uln_citizen
            ("Ox","Mule"), #dgn_citizen
            ("Ox","Mule"), #klk_citizen
            ("Ox","Mule"), #mechanist_citizen
            ("Ox","Mule"), #shmrn_citizen
           )

stattableexp={
        #SHIPNAME:(CHANCE_TO_HIT,CHANCE_TO_DODGE,DAMAGE,SHIELDS,ORDINANCE_DAMAGE),
        "Admonisher":(0.38,0.32,100,1410,2000),
        "Ancestor":(0.48,0.58,160,410,400),
        "Archimedes":(1,0.18,60000,2292530,1000000),
        "Ariston":(0.54,0.32,500,1190,800),
        "Areus":(0.64,0.34,400,1300,300000),
        "Mk32":(1,0.18,60000,2292530,1000000), #dupe of Archimedes FIXME
        "Beholder":(1,1,5000,6940,0),
        "Convolution":(0.54,0.7,500,620,50000),
        "Thales":(0.32,0.02,1000,10000,10000),
        "Clydesdale":(1,0.14,40000,1683740,300000),
        "Cultivator":(0.68,0.16,300,286770,0),
        "Derivative":(0.5,0.46,500,1030,400),
        "Determinant":(0.5,0.62,300,590,400),
        "Diligence":(0.52,0.14,200,18720,400), # dupe of Mule FIXME
        "Dirge":(0.38,0.38,180,290,200),
        "Dodo":(0.4,0.16,10,2500,0),
        "Dostoevsky":(0.6,0.68,200,540,2000),
        "Franklin":(0.76,0.78,200,2590,2000),
        "Gaozong":(0,0.9,0,40,0), 
        "Gawain":(0.67,0.7,500,400,400),
        "Gleaner":(0.52,0.14,200,18720,400), #dupe of Mule FIXME
        "Goddard":(0.86,0.24,800,5200,500000),
        "GTIO":(0.4,0.16,10,2500,0),
        "H496":(0.4,0.16,10,2500,0),
        "Hammer":(0.36,0.28,600,550,50000),
        "Hawking":(1,0.2,80000,1887640,0),
        "Hidalgo":(0.52,0.14,200,18720,400),
        "Ruizong":(1,0.19,25000,1800000,400000),
        "Hyena":(0.44,0.52,150,300,200),
        "Kafka":(0.4,0.16,10,2500,0),
        "Koala":(0.4,0.16,10,2500,0), # dupe of Kafka FIXME
        "Kahan":(1,0.18,25000,1400000,500000),
        "Kierkegaard":(0.86,0.24,800,5200,500000), #dupe of Goddard FIXME
        "Lancelot":(0.5,0.44,540,1250,600),
        "Llama":(0.34,0.22,200,4630,400),
        "MacGyver":(0.52,0.52,40,320,0),
        "Midwife":(1,0.16,2000,269400,3210), #dupe of Watson FIXME
        "Mule":(0.52,0.14,200,18720,400),
        "Nietzsche":(1,0.18,20000,1564400,100000),
        "Nicander":(0.52,0.46,300,910,300),
        "Nicander.escort":(0.52,0.46,300,910,300),
        "Ox":(0.68,0.16,300,286770,0),
        "Pacifier":(0.3,0.2,400,1890,100000),
        "Plowshare":(0.3,0.2,100,1380,400),
        "Progeny":(0.68,0.86,200,470,400),
        "Quicksilver":(0.52,0.52,40,320,0),
        "Redeemer":(0.38,0.38,180,290,200),
        "Robin":(0.44,0.48,300,350,200),
        "Sartre":(0.3,0.2,100,1380,400),
        "Schroedinger":(0.8,0.91,120,790,400),
        "Seaxbane":(0.44,0.48,300,350,200), # dupe of Robin FIXME
        "Shenzong":(0.52,0.52,40,320,0),
        "Shizu":(0.52,0.52,40,320,0),
        "Shizu.civvie":(0.52,0.52,40,320,0),
        "Shundi":(1,0.18,50000,2017640,3210),        
        "Sickle":(0.34,0.34,480,390,800),
        "Taizong":(0.78,0.42,440,1150,100000),
        "Tesla":(1,0.22,100000,1887640,0),
        "Tridacna":(0.68,0.16,300,286770,0), #dupe of Ox FIXME
        "Agasicles":(1,0.22,20000,1366420,600000),
        "Vendetta":(0.52,0.5,440,450,0), #dupe of Zhuangzong FIXME
        "Agesipolis":(1,0.16,50000,5738710,3210),
        "Watson":(1,0.16,2000,269400,3210),
        "Leonidas":(1,0.12,300000,8138400,2000000),
        "Yeoman":(0.68,0.16,300,286770,0), # dupe of Ox FIXME
        "Anaxidamus":(1,0.24,50000,2495160,1000000),
        "Zhuangzong":(0.52,0.5,440,450,0),
        "Shizong":(0.52,0.5,440,450,0),
        "Agricultural_Station":(1,0,10,21841060,0), #dupe of Commerce_Center FIXME
        "AsteroidFighterBase":(0.52,0,200,1512400,3210), #dupe of Refinery FIXME
        "Asteroid_Refinery":(0.4,0,10,33071210,0),
        "Asteroid_Shipyard":(0.4,0,10,33071210,0), #dupe of Asteroid_Refinery FIXME
        "Commerce_Center":(1,0,10,21841060,0),
        "Diplomatic_Center":(1,0,10,21841060,0), #dupe of Commerce_Center FIXME
        "Factory":(.02,0.02,10,13987040,0),
        "Shaper_Bio_Adaptation":(0.12,0,10,9050760,3210),
        "Fighter_Barracks":(0.12,0,100,9050760,3210),
        "Gasmine":(.02,0.02,10,13987040,0), #dupe of Factory FIXME
        "Medical":(1,0,0,2230130,0),
        "MiningBase":(1,0,100,715750,0),
        "Outpost":(0.12,0,100,9050760,3210), #dupe of Fighter_Barracks FIXME
        "Refinery":(0.4,0,10,33071210,0),
        "Relay":(0.24,0,10,3228510,0),
        "Research":(0.12,0,0,5497290,0),
        "Shipyard":(0.12,0,100,9050760,3210), #dupe of Fighter_Barracks FIXME
        "Starfortress":(1,0,750000,475993990,4000000)
        }

# stattable is generated dynamically at module initialization
# Modify by adding your ship/base to stattableexp

def GetStats ( name):
    try:
        return stattable[name]
    except:
        import debug
        debug.error( 'cannot find '+name)
        return (.5,.5,1,1,1)


capitols=capitals
capitaldict={}
for i in capitols:
    for j in i:
        capitaldict[j]=1
for i in capitols:
    for j in i:
        capitaldict[j+'.blank']=1

def isCapital(type):
    return type in capitaldict

generic_bases = ("Starfortress","Starfortress",
                 "Research","Research",
                 "Medical","Medical","Medical",
                 "Commerce_Center","Commerce_Center","Commerce_Center",
                 "Diplomatic_Center","Diplomatic_Center",
                 "Agricultural_Station","Agricultural_Station","Agricultural_Station",
                 "Factory","Factory","Factory",
                 "Shipyard","Shipyard",
                 "Gasmine","Gasmine",
                 "AsteroidFighterBase",
                 "Outpost","Outpost","Outpost","Outpost",
                 "Fighter_Barracks","Fighter_Barracks","Fighter_Barracks","Fighter_Barracks",
                 "Relay","Relay","Relay","Relay","Relay",
                 "Refinery","Refinery","Refinery","Refinery","Refinery",
                 "MiningBase","MiningBase","MiningBase","MiningBase","MiningBase","MiningBase")

bases = (generic_bases,
                 generic_bases, #aera
                 generic_bases, #rlaan
                 generic_bases, #merchant_guild
                 generic_bases, #luddites
                 generic_bases, #pirates
                 generic_bases, #hunter
                 generic_bases, #homeland_security
                 generic_bases, #ISO
                 generic_bases, #unknown
                 generic_bases, #andolian
                 generic_bases, #highborn
                 generic_bases+("Shaper_Bio_Adaptation","Shaper_Bio_Adaptation"), #shaper
                 generic_bases, #unadorned
                 generic_bases, #purist
                 generic_bases, #forsaken
                 generic_bases, #LIHW
                 generic_bases+("Asteroid_Refinery","Asteroid_Refinery"), #uln
                 generic_bases, #dgn
                 generic_bases, #klkk
                 generic_bases, #mechanist
                 generic_bases, #shmrn
                 generic_bases, #rlaan_briin
                 generic_bases, #aeran_merchant_marine
                 generic_bases, #rlaan_citizen
                 generic_bases, #merchant_guild_citizen
                 generic_bases, #andolian_citizen
                 generic_bases, #highborn_citizen
                 generic_bases, #shaper_citizen
                 generic_bases, #unadorned_citizen
                 generic_bases, #purist_citizen
                 generic_bases, #forsaken_citizen
                 generic_bases, #LIHW_citizen
                 generic_bases, #uln_citizen
                 generic_bases, #dgn_citizen
                 generic_bases, #klkk_citizen
                 generic_bases, #mechanist_citizen
                 generic_bases, #shmrn_citizen

                 )
basedict={}
for i in bases:
    for j in i:
        basedict[j]=1

def appendName(faction):
    from difficulty import usingDifficulty
    if (useStock[faction] and usingDifficulty()):
	# DON'T USE .blanks directly if possible-- preserve as templates. Use .stock where possible
        return ".stock"
    else:
        return ""

def factionToInt  (faction):
    try:
        return factiondict[faction]
    except:
        return 0
    return 0

def intToFaction (faction):
    return factions[faction]

def getMaxFactions ():
    return len(factions)

def get_non_citizen_X_of (mylist, index):
    import vsrandom
    import VS
    enemylist = mylist[index]
    newindex = vsrandom.randrange(0,len(enemylist))
    rez=intToFaction(enemylist[newindex])
    if VS.isCitizen(rez):
        while (newindex>0):
          newindex-=1
          rez=intToFaction(enemylist[newindex])
          if not VS.isCitizen(rez):
              return rez
        while (newindex+1<len(enemylist)):
          newindex+=1
          rez=intToFaction(enemylist[newindex])
          if not VS.isCitizen(rez):
              return rez
    return rez

def get_X_of (mylist, index):
    import vsrandom
    enemylist = mylist[index]
    newindex = vsrandom.randrange(0,len(enemylist))
    return intToFaction(enemylist[newindex])


def get_enemy_of (factionname):
    return get_X_of (enemies, factionToInt(factionname))

def get_insys_enemy_of (factionname):
    return get_X_of (insysenemies, factionToInt(factionname))

def get_friend_of (factionname):
    return get_X_of (friendlies, factionToInt(factionname))

def get_rabble_of (factionname):
    return get_X_of (rabble, factionToInt(factionname))

def get_enemy_of_no_citizen (factionname):
    return get_X_of (enemies, factionToInt(factionname))
    #return get_non_citizen_X_of (enemies, factionToInt(factionname))

def get_insys_enemy_of_no_citizen (factionname):
    return get_X_of (insysenemies, factionToInt(factionname))
    #return get_non_citizen_X_of (insysenemies, factionToInt(factionname))

def get_friend_of_no_citizen (factionname):
    return get_X_of (friendlies, factionToInt(factionname))
    #return get_non_citizen_X_of (friendlies, factionToInt(factionname))

def get_rabble_of_no_citizen (factionname):
    return get_X_of (rabble, factionToInt(factionname))
    #return get_non_citizen_X_of (rabble, factionToInt(factionname))

def getRandomShipType(ship_list):
    import vsrandom
    index=vsrandom.randrange(0,len(ship_list))
    return ship_list[index]

def getFigher(confed_aera_or_rlaan, fighter):
    fighterlist = fighters[confed_aera_or_rlaan]
    fighterlist = fighterlist[fighter]
    return fighterlist+appendName(confed_aera_or_rlaan)

def getRandomFighterInt(confed_aera_or_rlaan):
    return getRandomShipType(fighters[confed_aera_or_rlaan])+appendName(confed_aera_or_rlaan)

def getNumCapitol (confed_aera_or_rlaan):
    return len(capitols[confed_aera_or_rlaan])

def getNumFighters (confed_aera_or_rlaan):
    lst = fighters[confed_aera_or_rlaan]
    return len(lst)

def getCapitol(confed_aera_or_rlaan, fighter):
    caplist = capitols[confed_aera_or_rlaan]
    caplist = caplist[fighter]
    return caplist

def getRandomCapitolInt(confed_aera_or_rlaan):
    lst = capitols[confed_aera_or_rlaan]
    return getRandomShipType(lst)

def getRandomFighter(faction):
    return getRandomFighterInt (factionToInt (faction))

def getRandomCapitol (faction):
    return getRandomCapitolInt (factionToInt (faction))

launch_distance_factor=1
max_radius=10000
min_forward_distance=100000 
min_distance=10000 
#print len(fightersPerFG)
#print len (fighterProductionRate)
#print len(capitalProductionRate)
#print len(enemies)
#print len(rabble)
#print len (friendlies)
#print len(fighters)
#print len(capitals)
#print len(bases)
#print len(useStock)

import math

def make_stattable(stattableexp):
    stattable={}

    def lg (num, log=math.log, ilog2=1.0/math.log(2)):
        return log(1+num)*ilog2

    for i,t in stattableexp.iteritems():
        stattable[i]=(t[0],t[1],lg(t[2]),lg(t[3]),lg(t[4]))
        stattable[i+'.rgspec']=(t[0],t[1]*.8,lg(t[2])*.8,lg(t[3])*.8,lg(t[4])*.8)
        stattable[i+'.milspec']=(t[0],t[1]*.65,lg(t[2])*.65,lg(t[3])*.65,lg(t[4])*.65)
        stattable[i+'.stock']=(t[0],t[1]*.5,lg(t[2])*.5,lg(t[3])*.5,lg(t[4])*.5)
        stattable[i+'.blank']=(t[0],t[1]*.1,lg(t[2])*.1,lg(t[3])*.1,lg(t[4])*.1)
        stattable[i+'.civvie']=(t[0],t[1]*.2,lg(t[2])*.2,lg(t[3])*.2,lg(t[4])*.2)
    
    return stattable

stattable = make_stattable(stattableexp)
