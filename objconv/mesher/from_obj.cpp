#include "from_obj.h"
#include <map>
using std::map;
struct MTL:public GFXMaterial {
  MTL() {
    blend_src=ONE;
    blend_dst=ZERO;
    reflect=true;
  }  
  bool usenormals;
  bool reflect;
   int blend_src;int blend_dst;
   vector<textureholder> textures;
   textureholder detail;   
  
   vector <float> detailplanei;
   vector <float> detailplanej;
   vector <float> detailplanek;
};
struct IntRef {
  int v;
  int n;
  int t;
  IntRef() {v=-1;n=-1;t=-1;}
  IntRef(int vv, int nn, int tt) {v=vv;n=nn;t=tt;}
};
vector<string> splitWhiteSpace(string inp) {
  int where;
  vector<string> ret;
  while ((where=inp.find_first_of("\t "))!=string::npos) {
    if (where!=0)
      ret.push_back(inp.substr(0,where));
    inp = inp.substr(where+1);
  }
  if (inp.length()&&where==string::npos)
    ret.push_back(inp);
  return ret;
}
IntRef parsePoly(string inp) {
  IntRef ret;
  const char * st=inp.c_str();
  if (3==sscanf(st,"%d/%d/%d",&ret.v,&ret.t,&ret.n)) {
    ret.v-=1;ret.t-=1;ret.n-=1;
    return ret;
  }
  if (2==sscanf(st,"%d/%d/",&ret.v,&ret.t)) {
    ret.v-=1;ret.t-=1;
    return ret;
  }
  if (2==sscanf(st,"%d/%d",&ret.v,&ret.t)) {
    ret.v-=1;ret.t-=1;
    return ret;
  }
  if (2==sscanf(st,"%d//%d",&ret.v,&ret.n)) {
    ret.v-=1;ret.n-=1;
    return ret;
  }
  sscanf(st,"%d",&ret.v);
    ret.v-=1;
  return ret;  
}
void charstoupper(char *buf){
   while(*buf) {
      *buf=toupper(*buf);
      ++buf;
   }
}
void wordtoupper(char *buf){
   while(*buf&&!isspace(*buf)) {
      *buf=toupper(*buf);
      ++buf;
   }
}
textureholder makeTextureHolder(std::string str, int which) {
  textureholder ret;
  ret.type=TEXTURE;
  if (str.find(".ani")!=string::npos) {
    ret.type=ANIMATION;
  }

  
  ret.index=which;
  ret.name.insert(ret.name.begin(),str.begin(),str.end());
  return ret;
}
void ObjToBFXM (FILE* obj, FILE * mtl, FILE * outputFile) {
   fseek (obj,0,SEEK_END);
   int osize=ftell(obj);
   fseek (obj,0,SEEK_SET);
   fseek (mtl,0,SEEK_END);
   {
      int msize=ftell(mtl);
      osize = osize>msize?osize:msize;
   }
   fseek(mtl,0,SEEK_SET);
   char * buf = (char*)malloc((osize+1)*sizeof(char));
   char * str = (char*)malloc((osize+1)*sizeof(char));
   char * str1 = (char*)malloc((osize+1)*sizeof(char));
   char * str2 = (char*)malloc((osize+1)*sizeof(char));
   char * str3 = (char*)malloc((osize+1)*sizeof(char));
   char * str4 = (char*)malloc((osize+1)*sizeof(char));
   map<string,MTL> mtls;
   XML xml;
   mtls["default"]=MTL();
   MTL * cur=&mtls["default"];
   while (fgets(buf,osize,mtl)) {
      if (buf[0]=='#'||buf[0]==0)
         continue;
      if (1==sscanf(buf,"newmtl %s\n",str)) {
         mtls[str]=MTL();
         cur=&mtls[str];
		 cur->textures.push_back(textureholder());
		 cur->textures.back().index=0;
         cur->textures.back().type=TEXTURE;
	     cur->textures.back().name.push_back('w');
	     cur->textures.back().name.push_back('h');
	     cur->textures.back().name.push_back('i');
	     cur->textures.back().name.push_back('t');
	     cur->textures.back().name.push_back('e');
	     cur->textures.back().name.push_back('.');
	     cur->textures.back().name.push_back('b');
	     cur->textures.back().name.push_back('m');
	     cur->textures.back().name.push_back('p');
         continue;
      }
      wordtoupper(buf);
      float tmpblend;
      if (1==sscanf(buf,"BLEND %f\n",&tmpblend)) {
        if (tmpblend==1) {
          cur->blend_src = ONE;
          cur->blend_dst = ONE;
        }else if (tmpblend==.5) {
          cur->blend_src = SRCALPHA;
          cur->blend_dst = INVSRCALPHA;
        }else {
          cur->blend_src=ONE;
          cur->blend_dst=ZERO;
        }
      }
      if (3==sscanf(buf,"KA %f %f %f\n",&cur->ar,&cur->ag,&cur->ab)) {
        cur->aa=1;
      }
      if (3==sscanf(buf,"KS %f %f %f\n",&cur->sr,&cur->sg,&cur->sb)) {
        cur->sa=1;
      }
      if (3==sscanf(buf,"KD %f %f %f\n",&cur->dr,&cur->dg,&cur->db)) {
        cur->da=1;
      }
      if (3==sscanf(buf,"KE %f %f %f\n",&cur->er,&cur->eg,&cur->eb)) {
        cur->ea=1;
      }
      if (1==sscanf(buf,"NORMALS %f\n",&tmpblend)) {
        if (tmpblend!=0)
          cur->usenormals=true;
      
        else 
          cur->usenormals=false;
      }
      if (1==sscanf(buf,"MAP_REFLECTION %f\n",&tmpblend)) {
        if (tmpblend!=0) {
          cur->reflect=1;
        }
        else {
          cur->reflect=0;
        }
      }
      sscanf(buf,"NS %f\n",&cur->power);
      float floate,floatf,floatg;
      if (3==sscanf(buf,"detail_plane %f %f %f\n",&floate,&floatf,&floatg)) {
         cur->detailplanei.push_back(floate);
         cur->detailplanej.push_back(floatf);
         cur->detailplanek.push_back(floatg);
      }
      if (1==sscanf(buf,"illum %f\n",&floate)) {
         cur->er=floate;
         cur->eg=floate;
         cur->eb=floate;
      }
      if (1==sscanf(buf,"MAP_KD %s\n",str)) {
         if (cur->textures.empty())
           cur->textures.push_back(textureholder());
         cur->textures[0]=makeTextureHolder(str,0);
      }
      if (1==sscanf(buf,"MAP_KA %s\n",str)) {
         while (cur->textures.size()<3)
           cur->textures.push_back(textureholder());         
         cur->textures[2]=makeTextureHolder(str,2);
      }
      if (1==sscanf(buf,"MAP_KS %s\n",str)) {
         while (cur->textures.size()<2)
           cur->textures.push_back(textureholder());         
         cur->textures[1]=makeTextureHolder(str,1);
      }
      if (1==sscanf(buf,"MAP_DETAIL %s\n",str)) {
        cur->detail=makeTextureHolder(str,0);
      }      
   }
   bool changemat=false;
   int textnum=0;
   vector <pair<float,float> > tex;
   while (fgets(buf,osize,obj)) {
      if (buf[0]=='#'||buf[0]=='g')
         continue;
      if (1==sscanf(buf,"usemtl %s\n",str)) {
         if (changemat) {
            //write out unit file;
           xmeshToBFXM(xml,outputFile,textnum==0?'c':'a');
           textnum++;
         }
         changemat=true;
         xml.tris.clear();
         xml.quads.clear();
         xml.lines.clear();
         xml.linestrips.clear();
         xml.tristrips.clear();
         xml.quadstrips.clear();
         xml.trifans.clear();
         
         xml.material=mtls[str];
         xml.textures=mtls[str].textures;
         xml.detailtexture=mtls[str].detail;
         for (int jjjj=0;jjjj<mtls[str].detailplanei.size();++jjjj) {
           Mesh_vec3f v;
           v.x=mtls[str].detailplanei[jjjj];
           v.y=mtls[str].detailplanej[jjjj];
           v.z=mtls[str].detailplanek[jjjj];
           xml.detailplanes.push_back(Mesh_vec3f(v));
         }
         xml.blend_src = mtls[str].blend_src;
         xml.blend_dst = mtls[str].blend_dst;
         xml.reflect=mtls[str].reflect;
         xml.usenormals=mtls[str].usenormals;
         continue;
      }
      GFXVertex v;

      memset(&v,0,sizeof(GFXVertex));
      if (3==sscanf(buf,"v %f %f %f\n",&v.x,&v.y,&v.z)) {
        xml.vertices.push_back(v);
        xml.num_vertex_references.push_back(0);
		continue;
      }
      if (3==sscanf(buf,"vn %f %f %f\n",&v.i,&v.j,&v.k)) {
        //ignore
		continue;
      }
      if (2==sscanf(buf,"vt %f %f\n",&v.s,&v.t)) {
        tex.push_back(pair<float,float>(v.s,v.t)); 
		continue;
      }
      GFXVertex temp;
      if (5== sscanf(buf,"f %s %s %s %s %s",str,str1,str2,str3,str4)) {
        vector<string> splitwhite = splitWhiteSpace(buf+1);        
        //        printf ("split white %d %s %s %s %s %s\n",splitwhite.size(),str,str1,str2,str3,str4);
        xml.trifans.push_back(strip());
        IntRef first=parsePoly(str);
        
        IntRef second=parsePoly(str1);        
        for (int i=2;i<splitwhite.size();++i) {
          IntRef cp = parsePoly(splitwhite[i]);
          SetNormal(temp,xml.vertices[first.v],
                    xml.vertices[second.v],
                    xml.vertices[cp.v]);
          if (i==2) {
            AddNormal(xml.vertices[first.v],temp);
            AddNormal(xml.vertices[second.v],temp);
          }
          AddNormal(xml.vertices[cp.v],temp);
        }        
        for (int i=0;i<splitwhite.size();++i) {
          IntRef cp = parsePoly(splitwhite[i]);
          xml.stripelementtemp=stripelement();
          xml.stripelementtemp.indexref=cp.v;
          if (cp.t>=0&&cp.t<tex.size()) {
            xml.stripelementtemp.s=tex[cp.t].first;
            xml.stripelementtemp.t=tex[cp.t].second;
          }
          xml.trifans.back().points.push_back(xml.stripelementtemp);
        }
        continue;
      }
      if (4== sscanf(buf,"f %s %s %s %s\n",str,str1,str2,str3)) {
        IntRef A = parsePoly(str);        
        IntRef B = parsePoly(str1);        
        IntRef C = parsePoly(str2);        
        IntRef D = parsePoly(str3);        
        SetNormal(temp,xml.vertices[A.v],xml.vertices[B.v],xml.vertices[C.v]);
        AddNormal(xml.vertices[A.v],temp);
        AddNormal(xml.vertices[B.v],temp);
        AddNormal(xml.vertices[C.v],temp);
        AddNormal(xml.vertices[D.v],temp);
        xml.quadtemp.indexref[0]=A.v;
        if (A.t>=0&&A.t<tex.size()) {
          xml.quadtemp.s[0]=tex[A.t].first;
          xml.quadtemp.t[0]=tex[A.t].second;
        }
        xml.quadtemp.indexref[1]=B.v;
        if (B.t>=0&&B.t<tex.size()) {
          xml.quadtemp.s[1]=tex[B.t].first;
          xml.quadtemp.t[1]=tex[B.t].second;
        }
        xml.quadtemp.indexref[2]=C.v;
        if (C.t>=0&&C.t<tex.size()) {
          xml.quadtemp.s[2]=tex[C.t].first;
          xml.quadtemp.t[2]=tex[C.t].second;
        }
        xml.quadtemp.indexref[3]=D.v;
        if (D.t>=0&&D.t<tex.size()) {
          xml.quadtemp.s[3]=tex[D.t].first;
          xml.quadtemp.t[3]=tex[D.t].second;
        }
        xml.quads.push_back(xml.quadtemp);
        continue;
      }
      if (2== sscanf(buf,"l %s %s\n",str,str1)) {
        IntRef A = parsePoly(str);        
        IntRef B = parsePoly(str1);        
        temp.i = xml.vertices[B.v].x-xml.vertices[A.v].x;
        temp.j = xml.vertices[B.v].y-xml.vertices[A.v].y;
        temp.k = xml.vertices[B.v].z-xml.vertices[A.v].z;
        float len = sqrt(temp.i*temp.i+temp.j*temp.j+temp.k*temp.k);
        temp.i/=len;temp.j/=len;temp.k/=len;
        AddNormal(xml.vertices[B.v],temp);
        temp.i*=-1;temp.j*=-1;temp.k*=-1;
        AddNormal(xml.vertices[A.v],temp);
        xml.linetemp.indexref[0]=A.v;
        if (A.t>=0&&A.t<tex.size()) {
          xml.linetemp.s[0]=tex[A.t].first;
          xml.linetemp.t[0]=tex[A.t].second;
        }
        xml.linetemp.indexref[1]=B.v;
        if (B.t>=0&&B.t<tex.size()) {
          xml.linetemp.s[1]=tex[B.t].first;
          xml.linetemp.t[1]=tex[B.t].second;
        }
        xml.lines.push_back(xml.linetemp);
      }
      if (3== sscanf(buf,"f %s %s %s\n",str,str1,str2)) {
        IntRef A = parsePoly(str);        
        IntRef B = parsePoly(str1);        
        IntRef C = parsePoly(str2);        
        SetNormal(temp,xml.vertices[A.v],xml.vertices[B.v],xml.vertices[C.v]);
        AddNormal(xml.vertices[A.v],temp);
        AddNormal(xml.vertices[B.v],temp);
        AddNormal(xml.vertices[C.v],temp);
        xml.triangletemp.indexref[0]=A.v;
        if (A.t>=0&&A.t<tex.size()) {
          xml.triangletemp.s[0]=tex[A.t].first;
          xml.triangletemp.t[0]=tex[A.t].second;
        }
        xml.triangletemp.indexref[1]=B.v;
        if (B.t>=0&&B.t<tex.size()) {
          xml.triangletemp.s[1]=tex[B.t].first;
          xml.triangletemp.t[1]=tex[B.t].second;
        }
        xml.triangletemp.indexref[2]=C.v;
        if (C.t>=0&&C.t<tex.size()) {
          xml.triangletemp.s[2]=tex[C.t].first;
          xml.triangletemp.t[2]=tex[C.t].second;
        }
        xml.tris.push_back(xml.triangletemp);
        continue;
      }
   }   
   xmeshToBFXM(xml,outputFile,textnum==0?'c':'a');
   textnum++;

}
