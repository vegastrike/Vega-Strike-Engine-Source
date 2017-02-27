import string

# Independent of the date system used, this will scale how fast time progresses
# in VS.
#  0.1 will mean 10000 (real) seconds (~3h) = 1 (VS) year
#  1 will mean 1000 (real) seconds (~17min) = 1 (VS) year
#  2 will mean  500 (real) seconds (~ 8min) = 1 (VS) year ... ie time is twice as fast
SCALEFACTOR = 0.02 # ~14h gameplay = 1 VS year

def formatStarDate(fac,stdt):
    """Formats the stardate for news"""
    stdtf=getFacCal(fac,stdt)
    if fac == "confed":
        return string.join([fillZeros(fac,stdtf[2]),str(stdtf[1]),str(stdtf[0]),formatTime(fac,stdt)])
    else:
        return string.join([fillZeros(fac,stdtf[2]),str(stdtf[1]),str(stdtf[0]),formatTime(fac,stdt)])

def formatTime(fac,stdt):
    """Formate the stardate time for news"""
    stdtf=getFacCal(fac,stdt)
    if fac == "confed":
        return string.join([fillZeros(fac,stdtf[3]),fillZeros(fac,stdtf[4]),fillZeros(fac,stdtf[5])],":")
    else:
        return string.join([fillZeros(fac,stdtf[3]),fillZeros(fac,stdtf[4]),fillZeros(fac,stdtf[5])],":")

def fillZeros(fac,tim):
    """Returns a string with the required number of zeros for each faction"""
    zer=2
    tim=str(tim)
    if fac == "confed":
        zer=2
    for i in range(zer):
        if len(tim) < i+1:
            tim='0'+tim
    return tim

def getFacCal(fac,stdt): #FIXME: add some other factions date systems....all ready to go :-)
    """takes a stardate string and returns a list of ints with
    [year,month,date,hour,minute,second]"""
    datesys = getDateSystem(fac)
    facstdt = (float(stdt)*SCALEFACTOR+float(getZeroStarDate(fac)))*datesys[3]
    incyear = int(facstdt)/1000
    return [incyear] + getMDDHMS(facstdt-(incyear*1000),datesys,incyear,fac)

def daysinMonth(monthsystem,month):
    for mon in monthsystem:
        if mon[0] == month:
            return mon[1]
    return 1

def daysinYear(monthsystem):
    count = 0
    for mon in monthsystem:
        count+=mon[1]
    return count

def addMonthDays(monthsys,leap):
    for i in range(len(monthsys)):
        for lmon in leap:
            if monthsys[i][0] == lmon[0]:
                monthsys[i] = (monthsys[i][0],monthsys[i][1] + lmon[1])
    return monthsys

def getMDDHMS(frac,system,year,fac):
    monthsystem = addMonthDays(system[0],getLeap(year,fac))
    numdays = getStarToDay(monthsystem) * frac
    remdays = numdays
    countdays = 0
    mon = monthsystem[0][0]
    for i in range(len(monthsystem)):
        countdays+=monthsystem[i][1]
        if countdays >= numdays:
            mon = monthsystem[i][0]
            break
        else:
            remdays = numdays - countdays    #FIXME: not sure exactly why the 1 is required, I think
                            #it is so that a fractional day would be counted as
                                        #the 1st, 3.5 days into the month the 4th day etc
    days = int(remdays)
    remainder = remdays - days

    htemp = remainder * system[1][0]
    hours = int(htemp)
    mintemp = (htemp - hours) * system[1][1]
    minutes = int(mintemp)
    sectemp = (mintemp - minutes) * system[1][2]
    seconds = int(sectemp)
    return [mon,days+1,hours,minutes,seconds]


def getDateSystem(faction):
    """returns a particlar races standard date system (not including leap years)"""
    if facDateSystems().has_key(faction):
        return facDateSystems()[faction]
    else:
        return facDateSystems()["standard"]


def getStarToDay(monthsystem):
    """returns a particlar races stardate to day ratio"""
    return daysinYear(monthsystem) / 1000.0

def getZeroStarDate(faction):
    """Returns the VS stardate at which each faction has the zero date."""
    if faction == "confed":
        return 3276800
    else:
        return 3276800

def getLeap(year,faction):
    if faction == "confed":
        if year%100 == 0:
            return list()
        elif year%4 == 0:
            return [("February",1)]
        else:
            return list()
    else:
        if year%100 == 0:
            return list()
        elif year%4 == 0:
            return [("February",1)]
        else:
            return list()

def facDateSystems():
    """returns the date systems for all the factions with special ones.
    It is a tuple, with the first item a list of (month,#days) tuples, the second
    a tuple with (#hoursperday,#minutesperhour,#secondsperminute), the third is
    a list of the names for the time divisions, and the last is the number of
    cycles (years) per standard kilostardate."""
    return {
        "standard" :
        ([("January",31),("February",28),("March",31),("April",30),("May",31),("June",30),("July",31),("August",31),("September",30),("October",31),("November",30),("December",31)],(24,60,60),["year","month","week","day","hour","minute","second"],1)
    }

