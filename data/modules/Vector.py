def Vector (x,y,z):
    return (x,y,z)
def Add (a,b):
    return (a[0]+b[0],a[1]+b[1],a[2]+b[2])
def Sub (a,b):
    return (a[0]-b[0],a[1]-b[1],a[2]-b[2])
def Neg (a):
    return (-a[0],-a[1],-a[2])
def Cross(a,b):
    return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])
def Scale(a,f):
    return (a[0]*f,a[1]*f,a[2]*f)
def Dot (a,b):
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
def Transform (a,p,q,r):
    return (Dot (a,p),
            Dot (a,q),
            Dot (a,r))
def Mag(a):
    import VS
    return VS.sqrt(Dot(a,a))
def Norm (a):
    return Scale(a,1.0/Mag(a))
def SafeNorm (a):
    mag = Mag(a)
    if (mag):
        return Scale(a,1.0/mag)
    else:
        return Vector(0,0,0)
def ScaledCross (a,b):
    return Norm(Cross(a,b))
def Max(a,b):
    return ((a[0]>b[0])*a[0]+(a[0]<=b[0])*b[0],
            (a[1]>b[1])*a[1]+(a[1]<=b[1])*b[1],
            (a[2]>b[2])*a[2]+(a[2]<=b[2])*b[2])
def Min(a,b):
    return ((a[0]<b[0])*a[0]+(a[0]>=b[0])*b[0],
            (a[1]<b[1])*a[1]+(a[1]>=b[1])*b[1],
            (a[2]<b[2])*a[2]+(a[2]>=b[2])*b[2])
