#include "unit_generic.h"
#include "csv.h"
#include "xml_serializer.h"
#include "gfx/sphere.h"
#include "unit_collide.h"
#include "collide/cs_compat.h"
#include "collide/rapcol.h"
#include "gfx/bsp.h"
#include "unit_factory.h"
using namespace std;
extern int GetModeFromName (const char * input_buffer);
extern void pushMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg, int startframe, double texturestarttime);
void addShieldMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addRapidMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addBSPMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
static void UpgradeUnit (Unit * un, std::string upgrades) {
  while (upgrades.length()) {
    unsigned int where = upgrades.find(":");
    unsigned int mountoffset=0;
    unsigned int subunitoffset=0;
    string upgrade = upgrades.substr(0,where);
    unsigned int where1 = upgrade.find(";");
    unsigned int where2 = upgrade.rfind(";");
    if (where1!=string::npos) {
      mountoffset=XMLSupport::parse_int(upgrade.substr(where1+1,where2!=where1?where2:upgrade.length()));      
      if (where2!=where1&&where2!=string::npos) {
        subunitoffset=XMLSupport::parse_int(upgrade.substr(where2+1));
      }
    }
    upgrade = upgrade.substr(0,where1);
    if (upgrade.length()==0) 
      continue;
    Unit *upgradee =UnitFactory::createUnit(upgrade.c_str(),
                                            true,
                                            FactionUtil::
                                            GetFaction("upgrades"));
    double percent=1.0;
    un->Unit::Upgrade (upgradee,
                       mountoffset,
                       subunitoffset,
                       GetModeFromName (upgrade.c_str()),true,percent,NULL);
    upgradee->Kill();    
    if (where!=string::npos) {
      upgrades=upgrades.substr(where+1);
    }else break;
  }  
}
static void AddMeshes(Unit::XML& xml, std::string meshes,string meshStartFrame, string textureStartTime,int faction,Flightgroup *fg){
  unsigned int where,wheresf,wherest;
  while (meshes.length()) {
    where=meshes.find(":");
    wheresf = meshStartFrame.find(":");
    wherest = textureStartTime.find(":");
    string startf = strtoupper(meshStartFrame.substr(0,wheresf));
    string startt = strtoupper(textureStartTime.substr(0,wherest));
    int startframe = startf=="RANDOM"?-1:(startf=="ASYNC"?-1:atoi(startf.c_str()));
    float starttime = startt=="RANDOM"?-1.0f:atof(startt.c_str());
    string mesh = meshes.substr(0,where);
    pushMesh(&xml,mesh.c_str(),xml.unitscale,faction,fg,startframe,starttime);
    if (wheresf!=string::npos) 
      meshStartFrame.substr(wheresf+1);
    if (wherest!=string::npos) 
      textureStartTime.substr(wherest+1);
    if (where!=string::npos)
      meshes=meshes.substr(where+1);
    else
      break;
  }
}

void Unit::LoadRow(CSVRow &row,string modification, string * netxml) {
  Unit::XML xml;
  csvRow = row[0];
  //
  image->cargo_volume = atof(row["CargoVolume"].c_str());


  //begin the geometry (and things that depend on stats)
  UpgradeUnit(this,row["Upgrade"]);
  xml.unitscale = atoi(row["Scale"].c_str());
  if (!xml.unitscale) xml.unitscale=1;
  image->unitscale=xml.unitscale;
  AddMeshes(xml,row["Mesh"],row["MeshStartFrame"],row["MeshTextureStartFrame"],faction,getFlightgroup());
  meshdata= xml.meshes;
  meshdata.push_back(NULL);
  corner_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
  corner_max = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  calculate_extent(false);
  xml.shieldmesh_str = row["ShieldMesh"];
  if (xml.shieldmesh_str.length()){
    addShieldMesh(&xml,xml.shieldmesh_str.c_str(),xml.unitscale,faction,getFlightgroup());
    meshdata.back()=xml.shieldmesh;
  }else {
    static int shieldstacks = XMLSupport::parse_int (vs_config->getVariable ("graphics","shield_detail","16"));
    meshdata.back()= new SphereMesh (rSize(),shieldstacks,shieldstacks,vs_config->getVariable("graphics","shield_texture","shield.bmp").c_str(), NULL, false,ONE, ONE);
  }
  meshdata.back()->EnableSpecialFX();
  //Begin the Pow-w-w-war Zone Collide Tree Generation
  {
    xml.bspmesh_str = row["BSPMesh"];
    xml.rapidmesh_str = row["RapidMesh"];
    vector<bsp_polygon> polies;
    std::string collideTreeHash = VSFileSystem::GetHashName(string(modification)+"#"+row[0]);
    this->colTrees = collideTrees::Get(collideTreeHash);
    if (this->colTrees) {
      this->colTrees->Inc();
    }
    BSPTree * bspTree=NULL;
    BSPTree * bspShield=NULL;
    csRapidCollider *colShield=NULL;
    csRapidCollider *colTree=NULL;
    string tmpname = row[0];//key
    if (!this->colTrees) {
      string val;
      xml.hasBSP=1;
      xml.hasColTree=1;
      if ((val=row["HasBSP"]).length()) {
        xml.hasBSP = XMLSupport::parse_bool(val);
      }
      if ((val=row["HasRapid"]).length()) {
        xml.hasColTree= XMLSupport::parse_bool(val);
      }
      if (xml.shieldmesh) {
        if (!CheckBSP ((tmpname+"_shield.bsp").c_str())) {
          BuildBSPTree ((tmpname+"_shield.bsp").c_str(), false, meshdata.back());
        }
        if (CheckBSP ((tmpname+"_shield.bsp").c_str())) {
          bspShield = new BSPTree ((tmpname+"_shield.bsp").c_str());
        }
        if (meshdata.back()) {
          meshdata.back()->GetPolys(polies);
          colShield = new csRapidCollider (polies);
        }
      }
      if (xml.rapidmesh_str.length())
        addRapidMesh(&xml,xml.rapidmesh_str.c_str(),xml.unitscale,faction,getFlightgroup());
      if (xml.bspmesh_str.length())
        addBSPMesh(&xml,xml.bspmesh_str.c_str(),xml.unitscale,faction,getFlightgroup());
      if (xml.hasBSP) {
        tmpname += ".bsp";
        if (!CheckBSP (tmpname.c_str())) {
          BuildBSPTree (tmpname.c_str(), false, xml.bspmesh);
        }
        if (CheckBSP (tmpname.c_str())) {
          bspTree = new BSPTree (tmpname.c_str());
        }	
      } else {
        bspTree = NULL;
      }
      polies.clear();
      if (xml.rapidmesh) {
        xml.rapidmesh->GetPolys(polies);
      }
      csRapidCollider * csrc=NULL;
      if (xml.hasColTree) {
        csrc=getCollideTree(Vector(1,1,1),
                            xml.rapidmesh?
                            &polies:NULL);
      }
      this->colTrees = new collideTrees (collideTreeHash,
                                         bspTree,
                                         bspShield,
                                         csrc,
                                         colShield);
      if (xml.rapidmesh&&xml.hasColTree) {
        //if we have a special rapid mesh we need to generate things now
        for (int i=1;i<collideTreesMaxTrees;++i) {
          if (!this->colTrees->rapidColliders[i]) {
            unsigned int which = 1<<i;
            this->colTrees->rapidColliders[i]= 
              getCollideTree(Vector (1,1,which),
                             &polies);
          }
        }
      }
      if (xml.bspmesh) {
        delete xml.bspmesh;
        xml.bspmesh=NULL;
      }
      if (xml.rapidmesh) {
        delete xml.rapidmesh;
        xml.rapidmesh=NULL;
      }
    }
  }
}
CSVRow GetUnitRow(string filename, bool subu, int faction, bool readlast, bool &rread) {
  std::string hashname = filename+"__"+FactionUtil::GetFactionName(faction);
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
    bool bad=false;
    if (!modifications)bad=true;
    if (!bad) {
      if(!strlen(modifications)) {
        bad=true;
      }
    }
    if (bad) {
      fprintf(stderr,"Cannot Write out unit file %s %s that has no filename\n",name.c_str(),csvRow.c_str());
      return;
    }
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
using XMLSupport::tostring;
string Unit::WriteUnitString () {
  static bool UNITTAB = XMLSupport::parse_bool(vs_config->getVariable("physics","UnitTable","false"));
  string ret="";
  if (UNITTAB) {
    //this is the fillin part
    //fixme

    for (int i=unitTables.size()-1;i>=0;--i) {
      unsigned int where;
      string val;
      if (unitTables[i]->RowExists(csvRow,where)) {
        CSVRow row(unitTables[i],where);
        vector<string> keys;
        vector<string> values;
        keys.push_back("Key");
        values.push_back(csvRow);
        keys.push_back("Directory");
        values.push_back(row["Directory"]);

        // mutable things
        keys.push_back("CargoVolume");
        values.push_back(XMLSupport::tostring(image->cargo_volume));
        
        // immutable things
        keys.push_back("Scale");
        values.push_back(row["Scale"]);
        keys.push_back("Mesh");
        values.push_back(row["Mesh"]);       
        if ((val=row["MeshStartFrame"]).length()) {
          keys.push_back("MeshStartFrame");
          values.push_back(val);
        }
        if ((val=row["MeshTextureStartTime"]).length()) {
          keys.push_back("MeshTextureStartTime");
          values.push_back(val);
        }
        if ((val=row["ShieldMesh"]).length()) {
          keys.push_back("ShieldMesh");
          values.push_back(val);
        }
        if ((val=row["RapidMesh"]).length()) {
          keys.push_back("RapidMesh");
          values.push_back(val);
        }
        if ((val=row["BSPMesh"]).length()) {
          keys.push_back("BSPMesh");
          values.push_back(val);
        }
        if ((val=row["HasRapid"]).length()) {
          keys.push_back("HasRapid");
          values.push_back(val);
        }        
        if ((val=row["HasBSP"]).length()) {
          keys.push_back("HasBSP");
          values.push_back(val);
        }        
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

