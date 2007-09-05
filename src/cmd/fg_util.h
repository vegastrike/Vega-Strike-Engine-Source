namespace fg_util {
  std::string MakeFactionKey(int faction);
  std::string MakeFGKey(const std::string &fgname, int faction);
  std::string MakeStarSystemFGKey(const std::string&starsystem);
  unsigned int ShipListOffset();
  unsigned int PerShipDataSize();
  bool IsFGKey(const std::string&fgcandidate);
  bool CheckFG(std::vector<std::string> &data);
  void PurgeZeroShips (SaveGame*sg, unsigned int faction);
  void PurgeZeroShips (SaveGame*sg);

}
