import Director

def getallchar ():
    rez = ""
    for i in range(256):
        rez += "%c"% i
    return rez
allchar = getallchar()
def loadStringList (playernum,mykey):
    lengt = Director.getSaveDataLength (playernum,mykey)
    if (lengt<1):
        return []
    rez = []
    curstr = ""
    lengt = Director.getSaveData(playernum,mykey,0)
    for j in range (lengt):
        i=j+1
        myint=Director.getSaveData (playernum,mykey,i)
        if (myint != 0):
            curstr += "%c"%myint
        else:
            rez +=[curstr,]
            curstr=""
    return rez
def saveStringList (playernum,mykey,names):
    length = Director.getSaveDataLength (playernum,mykey)
    k=1
    tot=0
    for i in range (len (names)):
        tot += len (names[i])+1
    if (length==0):
        Director.pushSaveData(playernum,mykey,tot)
    else:
        Director.putSaveData(playernum,mykey,0,tot)
    for i in range (len (names)):
        for j in range (len (names[i])):
            if (k < length):
                Director.putSaveData(playernum,mykey,k,allchar.find (names[i][j]))
            else:
                Director.pushSaveData(playernum,mykey,allchar.find (names[i][j]))
            k+=1
        if (k < length):
            Director.putSaveData(playernum,mykey,k,0)
        else:
            Director.pushSaveData(playernum,mykey,0)
        k+=1
