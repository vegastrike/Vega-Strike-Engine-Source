#ifndef __PYTHON_MISSION_H__
#define __PYTHON_MISSION_H__
#include <string>

class PythonMissionBaseClass{
protected:
  virtual void Destructor();
public:
  PythonMissionBaseClass ();
  virtual void Destroy(){Destructor();}
  ~PythonMissionBaseClass();
  virtual void Execute ();
  virtual void callFunction (std::string s);
  virtual std::string Pickle();
  virtual void UnPickle(std::string s);
};

#endif
