

class VegaConfig {
 public:
  VegaConfig(char *configfile);

  void getColor(char *name, float color[4]);
  char *getVariable(char *section,char *name);

};
