#include "base.h"
#ifdef BASE_MAKER
#include <stdio.h>

void begintag(FILE * Fp,const char *tag,int indent) {
	int i;
	for (i=0;i<indent;i++) {
		fwrite("\t",sizeof(char),1,Fp);
	}
	fwrite("<",sizeof(char),1,Fp);
	fwrite(tag,sizeof(char)*strlen(tag),1,Fp);
}
void midxmltag(FILE * Fp,const char *tag,const float value) {
	char value2 [100]={0};//new char [100];
	sprintf(value2,"%g",value);
	fwrite(" ",sizeof(char),1,Fp);
	fwrite(tag,sizeof(char)*strlen(tag),1,Fp);
	fwrite("=\"",sizeof(char)*2,1,Fp);
	fwrite(value2,sizeof(char)*strlen(value2),1,Fp);
	fwrite("\"",sizeof(char)*1,1,Fp);
//	delete value2;
}
void midxmlchar(FILE * Fp,const char *tag,const char * value) {
	fwrite(" ",sizeof(char),1,Fp);
	fwrite(tag,sizeof(char)*strlen(tag),1,Fp);
	fwrite("=\"",sizeof(char)*2,1,Fp);
	fwrite(value,sizeof(char)*strlen(value),1,Fp);
	fwrite("\"",sizeof(char)*1,1,Fp);
}
void midxmlint(FILE * Fp,const char *tag,const int value) {
	char value2 [100]={0};
	sprintf(value2,"%d",value);
	fwrite(" ",sizeof(char),1,Fp);
	fwrite(tag,sizeof(char)*strlen(tag),1,Fp);
	fwrite("=\"",sizeof(char)*2,1,Fp);
	fwrite(value2,sizeof(char)*strlen(value2),1,Fp);
	fwrite("\"",sizeof(char)*1,1,Fp);
}
void midxmlbool(FILE * Fp,const char *tag,const bool value) {
	char value2 [6];
	strcpy(value2,value==true?"true":"false");
	fwrite(" ",sizeof(char),1,Fp);
	fwrite(tag,sizeof(char)*strlen(tag),1,Fp);
	fwrite("=\"",sizeof(char)*2,1,Fp);
	fwrite(value2,sizeof(char)*strlen(value2),1,Fp);
	fwrite("\"",sizeof(char)*1,1,Fp);
}
void endtag(FILE * Fp, bool end=false) {
	if (end==true) {
		fwrite("/",sizeof(char),1,Fp);
	}
	fwrite(">\n",sizeof(char)*2,1,Fp);
}

void Base::Room::Link::EndXML (FILE *fp) {
	midxmlchar(fp,"Text",text.c_str());
	midxmltag(fp,"x",x);
	midxmltag(fp,"y",y);
	midxmltag(fp,"wid",wid);
	midxmltag(fp,"hei",hei);
}

void Base::Room::Goto::EndXML (FILE *fp) {
	begintag(fp,"Link",2);
	Link::EndXML(fp);
	midxmlint(fp,"index",index);
	endtag(fp,true);
}

void Base::Room::Launch::EndXML (FILE *fp) {
	begintag(fp,"Launch",2);
	Link::EndXML(fp);
	endtag(fp,true);
}

void Base::Room::Comp::EndXML (FILE *fp) {
	begintag(fp,"Comp",2);
	Link::EndXML(fp);
	for (int i=0;i<modes.size();i++) {
		char *mode=NULL;
		switch(modes[i]) {
		case UpgradingInfo::NEWSMODE:
			mode="NewsMode";
			break;
		case UpgradingInfo::SHIPDEALERMODE:
			mode="ShipMode";
			break;
		case UpgradingInfo::UPGRADEMODE:
			mode="UpgradeMode";
			break;
		case UpgradingInfo::DOWNGRADEMODE:
			mode="DowngradeMode";
			break;
		case UpgradingInfo::BRIEFINGMODE:
			mode="BriefingMode";
			break;
		case UpgradingInfo::MISSIONMODE:
			mode="MissionMode";
			break;
		case UpgradingInfo::SELLMODE:
			mode="SellMode";
			break;
		case UpgradingInfo::BUYMODE:
			mode="BuyMode";
			break;
		}
		if (mode)
			midxmlchar(fp,mode,"");
	}
	endtag(fp,true);
}

void Base::Room::BaseObj::EndXML (FILE *fp) {
//		Do nothing
}

void Base::Room::BaseShip::EndXML (FILE *fp) {
	begintag(fp,"Ship",2);
	midxmltag(fp,"x",mat.p.i);
	midxmltag(fp,"y",mat.p.j);
	midxmltag(fp,"z",mat.p.k);
	midxmltag(fp,"ri",mat.getR().i);
	midxmltag(fp,"rj",mat.getR().j);
	midxmltag(fp,"rk",mat.getR().k);
	midxmltag(fp,"qi",mat.getQ().i);
	midxmltag(fp,"qj",mat.getQ().j);
	midxmltag(fp,"qk",mat.getQ().k);
	endtag(fp,true);
}

void Base::Room::BaseSprite::EndXML (FILE *fp) {
	float x,y;
	begintag(fp,"Texture",2);
	spr.GetPosition(x,y);
	midxmlchar(fp,"File",texfile.c_str());
	midxmltag(fp,"x",x);
	midxmltag(fp,"y",y);
	endtag(fp,true);
}

void Base::Room::EndXML (FILE *fp) {
	begintag(fp,"Room",1);
	midxmlchar(fp,"Text",deftext.c_str());
	endtag(fp,false);
	int i;
	for (i=0;i<links.size();i++) {
		links[i]->EndXML(fp);
	}
	for (i=0;i<objs.size();i++) {
		objs[i]->EndXML(fp);
	}
	begintag(fp,"/Room",1);
	endtag(fp,false);
}

void Base::EndXML (FILE *fp) {
	begintag(fp,"Base",0);
	endtag(fp,false);
	for (int i=0;i<rooms.size();i++) {
		rooms[i]->EndXML(fp);
	}
	begintag(fp,"/Base",0);
	endtag(fp,false);
}


#endif