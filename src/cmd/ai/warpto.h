class Unit;
bool DistanceWarrantsWarpTo (Unit * parent, float sigdistance, bool following);// following == not worried about toodamnclose
bool DistanceWarrantsTravelTo (Unit * parent, float sigdistance, bool following);
bool TargetWorthPursuing(Unit * parent, Unit* target);
void WarpToP(Unit * parent, Unit * target, bool following);
void WarpToP(Unit * parent, const QVector &target, float radius, bool following);
