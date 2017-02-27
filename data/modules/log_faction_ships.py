import math
import faction_ships
stattable={}
def lg (num):
    return math.log(1+num)/math.log(2)
for i in faction_ships.stattableexp:
    tuple = faction_ships.stattableexp[i]
    stattable[i]=(tuple[0],tuple[1],lg(tuple[2]),lg(tuple[3]),lg(tuple[4]))
    stattable[i+'.rgspec']=(tuple[0],tuple[1]*.8,lg(tuple[2])*.8,lg(tuple[3])*.8,lg(tuple[4])*.8)
    stattable[i+'.milspec']=(tuple[0],tuple[1]*.65,lg(tuple[2])*.65,lg(tuple[3])*.65,lg(tuple[4])*.65)
    stattable[i+'.stock']=(tuple[0],tuple[1]*.5,lg(tuple[2])*.5,lg(tuple[3])*.5,lg(tuple[4])*.5)
    stattable[i+'.blank']=(tuple[0],tuple[1]*.1,lg(tuple[2])*.1,lg(tuple[3])*.1,lg(tuple[4])*.1)
    stattable[i+'.civvie']=(tuple[0],tuple[1]*.2,lg(tuple[2])*.2,lg(tuple[3])*.2,lg(tuple[4])*.2)
print stattable
