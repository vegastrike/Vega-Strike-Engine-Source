#ifndef __BASE_H__
#define __BASE_H__
#include <vector>
#include <string>
#include "unit_interface.h"
#include "gfx/hud.h"
#include <stdio.h>

//#define BASE_MAKER
//#define BASE_XML //in case you want to write out XML instead...

#define BASE_EXTENSION ".py"

class Base {
	int curlinkindex;
	bool drawlinkcursor;
	TextPlane curtext;
public:
	class Room {
	public:
		class Link {
		public:
			std::string pythonfile;
			float x,y,wid,hei;
			std::string text;
			const std::string index;
			virtual void Click (::Base* base,float x, float y, int button, int state);
			explicit Link (std::string ind,std::string pfile) : pythonfile(pfile),index(ind) {}
			virtual ~Link(){} 
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Goto : public Link {
		public:
			int index;
			virtual void Click (::Base* base,float x, float y, int button, int state);
			virtual ~Goto () {}
			explicit Goto (std::string ind, std::string pythonfile) : Link(ind,pythonfile) {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Comp : public Link {
		public:
			vector <UpgradingInfo::BaseMode> modes;
			virtual void Click (::Base* base,float x, float y, int button, int state);
			virtual ~Comp () {}
			explicit Comp (std::string ind, std::string pythonfile) : Link(ind,pythonfile) {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Launch : public Link {
		public:
			virtual void Click (::Base* base,float x, float y, int button, int state);
			virtual ~Launch () {}
			explicit Launch (std::string ind, std::string pythonfile) : Link(ind,pythonfile) {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Talk : public Link {
		public:
			//At the moment, the Base::Room::Talk class is unused... but I may find a use for it later...
			std::vector <std::string> say;
			std::vector <std::string> soundfiles;
			int index;
			int curroom;
			virtual void Click (::Base* base,float x, float y, int button, int state);
			explicit Talk (std::string ind, std::string pythonfile);
			virtual ~Talk () {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class Python : public Link {
		public:
			std::string file;
			virtual void Click (::Base* base,float x, float y, int button, int state);
			Python(std::string ind,std::string pythonfile);
			virtual ~Python () {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
		};
		class BaseObj {
		public:
			const std::string index;
			virtual void Draw (::Base *base);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BaseObj () {}
			explicit BaseObj (std::string ind) : index(ind) {}
		};
		class BaseShip : public BaseObj {
		public:
			virtual void Draw (::Base *base);
			Matrix mat;
			virtual ~BaseShip () {}
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp);
#endif
			explicit BaseShip (std::string ind) : BaseObj (ind) {}
			BaseShip (float r0, float r1, float r2, float r3, float r4, float r5, float r6, float r7, float r8, QVector pos, std::string ind)
				:BaseObj (ind),mat (r0,r1,r2,r3,r4,r5,r6,r7,r8,QVector(pos.i/2,pos.j/2,pos.k))  {}
		};
		class BaseSprite : public BaseObj {
		public:
			virtual void Draw (::Base *base);
			Sprite spr;
#ifdef BASE_MAKER
			std::string texfile;
			virtual void EndXML(FILE *fp);
#endif
			virtual ~BaseSprite () {}
			BaseSprite (const char *spritefile, std::string ind) 
				: BaseObj(ind),spr(spritefile,BILINEAR,GFXTRUE) {}
		};
		class BaseTalk : public BaseObj {
		public:
			static bool hastalked;
			virtual void Draw (::Base *base);
//			Talk * caller;
//			int sayindex;
			int curchar;
			float curtime;
			virtual ~BaseTalk () {}
			std::string message;
//			BaseTalk (Talk *caller) : caller (caller),  sayindex (0),curchar(0) {}
			BaseTalk (std::string msg,std::string ind, bool only_one_talk);
#ifdef BASE_MAKER
			virtual void EndXML(FILE *fp) {}
#endif
		};
		std::string soundfile;
		std::string deftext;
		std::vector <Link*> links;
		std::vector <BaseObj*> objs;
#ifdef BASE_MAKER
		void EndXML(FILE *fp);
#endif
		void Draw (::Base *base);
		void Click (::Base* base,float x, float y, int button, int state);
		int MouseOver (::Base *base,float x, float y);
		Room ();
		~Room ();
	};
	friend class Room;
	friend class Room::BaseTalk;
	int curroom;
	std::vector <Room*> rooms;
	TextPlane othtext;
	static Base *CurrentBase;
	static bool CallComp;
	UnitContainer caller;
	UnitContainer baseun;
#ifdef BASE_MAKER
	void EndXML(FILE *fp);
#endif
	void Terminate ();
	void GotoLink(int linknum);
	void InitCallbacks ();
	void CallCommonLinks (std::string name, std::string value);
//	static void Base::beginElement(void *userData, const XML_Char *names, const XML_Char **atts);
//	void Base::beginElement(const string &name, const AttributeList attributes);
//	static void Base::endElement(void *userData, const XML_Char *name);
	void Base::Load(const char * filename, const char * time_of_day, const char * faction);
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
