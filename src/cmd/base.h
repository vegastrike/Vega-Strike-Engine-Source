#ifndef __BASE_H__
#define __BASE_H__
#include <vector>
#include <string>
#include "unit_interface.h"
#include "gfx/hud.h"
#include <stdio.h>

#define BASE_MAKER

class Texture;

class Base {
	struct Room {
		struct Link {
			float x,y,wid,hei;
			std::string text;
			virtual void Click (::Base* base,float x, float y, int button, int state);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		struct Goto : public Link {
			int index;
			virtual void Click (::Base* base,float x, float y, int button, int state);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		struct Comp : public Link {
			vector <UpgradingInfo::BaseMode> modes;
			virtual void Click (::Base* base,float x, float y, int button, int state);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		struct Launch : public Link {
			virtual void Click (::Base* base,float x, float y, int button, int state);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		struct BaseObj {
			virtual void Draw ();
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BaseObj () {}
			BaseObj () {}
		};
		struct BaseShip : public BaseObj {
			virtual void Draw ();
			Matrix mat;
			virtual ~BaseShip () {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			BaseShip () {}
			BaseShip (float r0, float r1, float r2, float r3, float r4, float r5, float r6, float r7, float r8, QVector pos)
				:mat (r0,r1,r2,r3,r4,r5,r6,r7,r8,pos) {}
		};
		struct BaseSprite : public BaseObj {
			virtual void Draw ();
			Sprite spr;
#ifdef BASE_MAKER
			std::string texfile;
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BaseSprite () {}
			BaseSprite (const char *spritefile) 
				:spr(spritefile) {}
		};
		std::string deftext;
		std::vector <Link*> links;
		std::vector <BaseObj*> objs;
#ifdef BASE_MAKER
		void EndXML(FILE *fp);
#endif
		void Draw ();
		void Click (::Base* base,float x, float y, int button, int state);
		int MouseOver (float x, float y);
		Room ();
		~Room ();
	};
	int curlinkindex;
	friend Room;
	bool drawlinkcursor;
	std::vector <Room*> rooms;
	TextPlane curtext;
	int curroom;
	int unitlevel;
public:
	static Base *CurrentBase;
	static bool CallComp;
	Unit *caller;
	Unit *baseun;
#ifdef BASE_MAKER
	void EndXML(FILE *fp);
#endif
	void GotoLink(int linknum);
	void InitCallbacks ();
	void CallCommonLinks (std::string name, std::string value);
	static void Base::beginElement(void *userData, const XML_Char *names, const XML_Char **atts);
	void Base::beginElement(const string &name, const AttributeList attributes);
	static void Base::endElement(void *userData, const XML_Char *name);
	void Base::LoadXML(const char * filename);
	static void ClickWin (int x, int y, int button, int state);
	void Click (float x, float y, int button, int state);
	static void MouseOverWin (int x, int y);
	void MouseOver (float x, float y);
	Base (const char *basefile, Unit *base, Unit *un);
	~Base ();
	static void DrawWin ();
	void Draw ();
};

#endif
