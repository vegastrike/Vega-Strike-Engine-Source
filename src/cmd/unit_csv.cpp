#include "unit_generic.h"
#include "csv.h"
#include "xml_serializer.h"
using namespace std;
extern void pushMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg, int startframe, double texturestarttime);
void Unit::LoadRow(CSVRow &row,string modification, string * netxml) {
  Unit::XML xml;
  csvRow = row[0];
  string meshes = row["Mesh"];
  unsigned int where;
  while (meshes.length()) {
    where=meshes.find(":");
    std::string mesh = meshes.substr(0,where);
    pushMesh(&xml,mesh.c_str(),1.0f/*fixme*/,faction,getFlightgroup(),0,0);
    if (where!=string::npos)
      meshes=meshes.substr(where);
    else
      break;
  }
  meshdata= xml.meshes;
  meshdata.push_back(NULL);
  corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
  corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);

}
CSVRow GetUnitRow(string filename, bool subu, int faction, bool readlast, bool &rread) {
  std::string hashname = filename+":"+FactionUtil::GetFactionName(faction);
  for (int i=((int)unitTables.size())-(readlast?1:2);i>=0;--i) {
    unsigned int where;
    if (unitTables[i]->RowExists(hashname,where)) {
      rread=true;
      return CSVRow(unitTables[i],where);
    }else if (unitTables[i]->RowExists(filename,where)) {
      rread=true;
      return CSVRow(unitTables[i],where); 
    }
  }
  rread=false;
  return CSVRow();
}

void Unit::WriteUnit (const char * modifications) {
  static bool UNITTAB = XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));
  if (UNITTAB) {
    assert(modifications);
    assert(strlen(modifications));
    std::string savedir = modifications;
    VSFileSystem::CreateDirectoryHome( VSFileSystem::savedunitpath+"/"+savedir);
    VSFile f;
    //string filepath( savedir+"/"+this->filename);
    //cerr<<"Saving Unit to : "<<filepath<<endl;
    VSError err = f.OpenCreateWrite( savedir+"/"+name+".csv", UnitFile);
    if (err>Ok) {
      fprintf( stderr, "!!! ERROR : Writing saved unit file : %s\n", f.GetFullPath().c_str() );
      return;
    }
    std::string towrite = WriteUnitString();
    f.Write(towrite.c_str(),towrite.length());
    f.Close();
  }else {
    if (image->unitwriter)
      image->unitwriter->Write(modifications);
    for (un_iter ui= getSubUnits();(*ui)!=NULL;++ui) {
      (*ui)->WriteUnit(modifications);
    } 
  }
}
string Unit::WriteUnitString () {
  static bool UNITTAB = XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));
  string ret="";
  if (UNITTAB) {
    //this is the fillin part
    //fixme

    for (int i=unitTables.size()-1;i>=0;--i) {
      unsigned int where;
      if (unitTables[i]->RowExists(csvRow,where)) {
        CSVRow row(unitTables[i],where);
        vector<string> keys;
        vector<string> values;
        keys.push_back("Key");
        values.push_back(csvRow);
        keys.push_back("Directory");
        values.push_back(row["Directory"]);
        keys.push_back("SubDirectory");
        values.push_back(row["SubDirectory"]);
        keys.push_back("Mesh");
        values.push_back(row["Mesh"]);        
        return writeCSV(keys,values);
      }
    }
    fprintf (stderr,"Failed to locate base mesh for %s %s %s\n",csvRow.c_str(),name.c_str(),fullname.c_str());
  }else {
    if (image->unitwriter)
      ret = image->unitwriter->WriteString();
    for (un_iter ui= getSubUnits();(*ui)!=NULL;++ui) {
      ret = ret + ((*ui)->WriteUnitString());
    }
  }
  return ret;
}

