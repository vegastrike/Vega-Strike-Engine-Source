#ifndef UPGRADEABLEUNIT_H
#define UPGRADEABLEUNIT_H

#include <string>
#include <vector>


class Unit;
class Mount;

// TODO: make this into a subclass of unit later

class UpgradeableUnit
{
public:
    UpgradeableUnit();
    void UpgradeUnit(const std::string &upgrades);
    bool UpgradeMounts(const Unit *up,
                  int subunitoffset,
                  bool touchme,
                  bool downgrade,
                  int &numave,
                  double &percentage);
};

#endif // UPGRADEABLEUNIT_H
