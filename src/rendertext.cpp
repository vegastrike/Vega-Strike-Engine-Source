// rendertext.cpp: based on Don's gl_text.cpp
// Based on Aardarples rendertext and menus.. Memleak somewhere
#include "command.h"
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#include "vegastrike.h"
#include "cg_global.h"
#include "SDL/SDL.h"
#include "gfx/hud.h"
#include <sstream>
// ****************
// Console Rendering System by Rogue
// 2005-08-a-few-days
//
// ****************
RText::RText() {
	ndraw = 15;
	WORDWRAP = 85;
	conskip = 0;
//	histpos = 0;
	saycommand("");
}

RText::~RText() {

}


int RText::text_width(char *str)
{
return 0;
};


void RText::draw_textf(std::string  &fstr, int left, int top, int gl_num, int arg)
{
	draw_text(fstr, left, top, gl_num);
};
#define VIRTW 2400
#define VIRTH 1800
#define PIXELTAB (VIRTW/12)
#define FONTH 64
int verts = 0;
void RText::draw_text(std::string &str, float left, float top, int gl_num)
{
    int x = left;
    int y = top;

    int i;
    std::string::iterator iter = str.begin();
    GFXColor foreground(1, 1, 1, 1);
    TextPlane newTextPlane(foreground);
    newTextPlane.SetPos(x, y);
    newTextPlane.SetCharSize(.8, .12);
    newTextPlane.Draw(str);
};


//From cube, unused
void RText::draw_envbox_aux(float s0, float t0, int x0, int y0, int z0,
                     float s1, float t1, int x1, int y1, int z1,
                     float s2, float t2, int x2, int y2, int z2,
                     float s3, float t3, int x3, int y3, int z3,
                     int texture)
{

};
//unused
void RText::draw_envbox(int t, int w)
{
};


void RText::renderconsole()// render buffer
{

    int nd = 0;
    std::vector<std::string> refs;
	bool breaker = false;
//	int i = 0;
//	int lastmillis = 0;
//	int length = 0;
//	for(std::vector<cline>::iterator iter = conlines.begin();
//	iter < conlines.end(); iter++) length++;
	{for(std::vector<cline>::iterator iter = conlines.begin(); iter < conlines.end(); iter++)  {
	if(nd < ndraw) 
		refs.push_back((*(iter)).cref);
	else iter = conlines.end();
	nd++;
	}}
    size_t j = 0;
    float x = -1;
    float y = -0.5;
    std::string workIt;
    workIt.append("\n");
    bool breakout = true;
	std::vector<std::string>::iterator iter = refs.end();
	if(iter == refs.begin()) breakout = false;
    for(; breakout;) {
	iter--;
	if(iter == refs.begin()) breakout = false;
	workIt.append((*(iter)));
	workIt.append("\n");
    };
    y = 1;
    std::ostringstream drawCommand;
    drawCommand << workIt << "#000066> " << "#FF1100" << getcurcommand() << "#00000";
    std::string Acdraw;
    Acdraw.append(drawCommand.str());
    draw_text(Acdraw, x, y, 2);

};

void RText::conline(std::string &sf, bool highlight)        // add a line to the console buffer
{
    cline cl;
    int lastmillis = 0;
    cl.outtime = lastmillis;                        // for how long to keep line on screen
    if(highlight)        // show line in a different colour, for chat etc.
    {
        cl.cref.append("\f");
        cl.cref.append(sf);
    }
    else
    {
        cl.cref.append(sf);
    };
    conlines.insert(conlines.begin(), cl);
//    puts(cl.cref.c_str());

};

void RText::conoutf(std::string &s, int a, int b, int c)
{
	{for(int x = WORDWRAP; x < s.size(); x = x+WORDWRAP) {
		s.insert(x, "\n");
	}}

	size_t x = s.find("\n");
	if(x < std::string::npos) {
		size_t xlast = 0;
		for(; x < std::string::npos; x = s.find("\n", x+1)) {
			std::string newone;
			newone.append(s.substr(xlast, x-xlast));
			conline(newone, 1);
			xlast = x+1;
		}
		
	} else {
		conline(s, 1);
	}

};

void RText::conoutn(std::string &s, int a, int b, int c) {
	size_t x = s.find("\n");
	size_t xlast = 0;
	if(x >= std::string::npos) {
		conoutf(s);
	}
	std::string::iterator iter = s.end();
	if(iter != s.begin() ) { 
		iter--;
		if(strcmp(&(*(iter)),"\n") != 0) { s.append("\n"); };
	}
	while(x < std::string::npos) {
		std::string part;
		part.append(s.substr(xlast, x-xlast));
		xlast = x+1;
		x = s.find("\n", x+1);
		conoutf(part, a, b, c);
	}
}

void RText::saycommand(char *init)/// 
{ //actually, it appends "init" to commandbuf
//Unused.
//  SDL_EnableUNICODE((init!=NULL));
//    if(!editmode) keyrepeat(saycommandon);
//  if(!init) init = "";
//  commandbuf.append(init);

};

//char *RText::getComBuf() { was depricated
//	return (char *) commandbuf.c_str();
//}
void RText::ConsoleKeyboardI(int code, bool isdown, int cooked)
{
	if(isdown) {
		switch(code){
//pop teh back of commandbuf
	                case SDLK_BACKSPACE:
				{
				std::string::iterator iter = commandbuf.begin();
				if(iter < commandbuf.end()) {
					iter = commandbuf.end();
					iter--;
					commandbuf.erase(iter);
				}
				break;
				}
	                case SDLK_LEFT:
//this should move a put pointer for commandbuf
//right should move it the other way.
//		                for(int i = 0; commandbuf[i]; i++) if(!commandbuf[i+1]) commandbuf[i] = 0;
		        break;

			case SDLK_RETURN:
				if(commandbuf[0])
				{
					
					std::vector<std::string>::iterator iter = vhistory.end();
					bool noSize = false;
					if(iter <=vhistory.begin() && iter >= vhistory.end()) noSize = true;
					if(!noSize) {
						iter--;
						if(commandbuf.compare((*(iter))) != 0 && !noSize)
						{
//store what was typed into a vector for a command history 
//to scroll up and down through what was typed
//This "feature" isn't finished
							vhistory.push_back(commandbuf);  // cap this?
						}
					} else if(noSize)vhistory.push_back(commandbuf);
					
//					histpos = vhistory.end();
//					histpos--;
				//commands beginning with / are executed
				//in localPlayer.cpp just before this is called
				};
				if(commandbuf.size() > 0) {
//print what was typed
					conoutf(commandbuf);
//clear the buffer
					commandbuf.erase();
				}
				break;
			default:
//add it to the command buffer
				if(cooked) {
					char add[] = { cooked, 0};
					std::cout << "Appending: " << add<< std::endl;
					std::ostringstream l;
					l << add;
					commandbuf.append(l.str());
				};
				break;
		}
	}

};

std::string RText::getcurcommand()
{
    return commandbuf;
};
// Begin Menus.. Need to re-work the whole menu system now.
/* // inspired by Cube, written for NAMC
void RText::newmenu(char *name) {
//	aMenu *newmenu = new aMenu();
//	newmenu->Name.append(name);
//	newmenu->menusel = 0;
//	menus.push_back(newmenu);
//	return;
}
//
//
//#include <sstream>
int RText::gvmenu() {
//	std::ostringstream m;
//	m << "getVMenu called: " << hmenus;
//	conoutf((char *)m.str().c_str());
	return hmenus;
}
void RText::menuset(int menu_in)
{
//    if((vmenu = menu)>=1) resetmovement(player1);
//    if(vmenu==1) menus[1].menusel = 0;
//	conoutf("Menuset called.");
//	hmenus = menu_in;
	return;
};

void RText::showmenu(char *name_in)
{

	std::string name;
	name.append(name_in);
//	std::cout << name << std::endl;
	for(size_t i = 0; i < menus.size(); i++)
	{
		aMenu *d = menus.at(i);
		if(d->Name.compare(name) == 0) {
//			std::cout << name << " " << i << std::endl;
			menuset(i); 
		};
	};
	return;

};

int RText::menucompare(mitemo *a, mitemo *b)
{

    int x = atoi(a->text.c_str());
    int y = atoi(b->text.c_str());
    if(x>y) return -1;
    if(x<y) return 1;

    return 0;

};

void RText::sortmenu(int start, int num)
{
//    qsort(&menus[0].items[start], num, sizeof(mitemo), (int (RText::)(RText::mitem*,RText::mitem*))menucompare);
};

bool RText::rendermenu()
{
//    if(vmenu<0) { menustack.setsize(0); return false; };
//    if(vmenu==1) refreshservers();
//    gmenu &m = menus[vmenu];

	if(gvmenu()<0 || menus.size() <= 0) { menustack.setsize(0); return false;};
	
	aMenu *m = menus.at(gvmenu());
	if (!m) return false;

    sprintf_sd(title)(gvmenu()>0 ? "[ %s menu ]" : "%s", m->Name.c_str());
    int mdisp = m->items.size();
    int w = 0;
    loopi(mdisp)
    {
        int x = text_width((char *)m->items.at(i)->text.c_str());
        if(x>w) w = x;
    };
    int tw = text_width(title);
    if(tw>w) w = tw;
    int step = FONTH/4*5;
    int h = (mdisp+2)*step;
    int y = (VIRTH-h)/2;
    int x = (VIRTW-w)/2;
    blendbox(x-FONTH/2*3, y-FONTH, x+w+FONTH/2*3, y+h+FONTH, true);
    draw_text(title, x, y,2);
    y += FONTH*2;
    if(gvmenu()>-1)
    {
        int bh = y+m->menusel*step;
        blendbox(x-FONTH, bh-10, x+w+FONTH, bh+FONTH+10, false);
    };
    loopj(mdisp)
    {
        draw_text((char *)m->items.at(j)->text.c_str(), x, y, 2);
        y += step;
    };
    return true;

};

//void RText::newmenu(char *name)
//{
//	std::string *newstr = new std::string();
//	newstr->append(name);
//	gmenu &menu = menus.add();
//	menu.name = (char *)newstr->c_str();
//	menu.menusel = 0;
//};

void RText::menumanual(int m, int n, char *text)
{
//  if(!n) menus[m].items.setsize(0);
//  mitemo &mitem = menus[m].items.add();
 // mitemo.text = text;
//  mitemo.action = "";

	mitemo *mi = new mitemo();
	mi->text.append(text);
	mi->action = "";
	menus.at(m)->items.push_back(mi);

};

void RText::menuitem(char *text, char *action)
{
//    gmenu &menu = menus.last();
  //  mitemo &mi = menu.items.add();
//    mi.text = newstring(text);
//    mi.action = action[0] ? newstring(action) : mi.text;

	mitemo *m = new mitemo();
	m->text.append(text);
	m->action.append(action);
	menus.at(menus.size()-1)->items.push_back(m); //add to the last
	//menu made with newmenu

};

bool RText::menukey(int code, bool isdown)
{

    if(gvmenu()<0) return false;
    int menusel = menus.at(gvmenu())->menusel;
    if(isdown)
    {
        if(code==SDLK_ESCAPE)
        {
            menuset(-1);
            if(!menustack.empty()) menuset(menustack.pop());
            return true;
        }
        else if(code==SDLK_UP || code==-4) menusel--;
        else if(code==SDLK_DOWN || code==-5) menusel++;
        int n = menus.at(gvmenu())->items.size();
        if(menusel<0) menusel = n-1;
        else if(menusel>=n) menusel = 0;
        menus.at(gvmenu())->menusel = menusel;
    }
    else
    {
        if(code==SDLK_RETURN)
        {
            menuAction = (char *)menus.at(gvmenu())->items.at(menusel)->action.c_str();
		//because it can't execute commands here, it must store
		//the action in menuAction, which can be returned
		//below with getMenuAction(), to be executed in localPlayer
            menustack.add(gvmenu());
            menuset(-1);
        };
    };
    return true;

return true;
};

char *RText::getMenuAction() {
	return menuAction;
}


void RText::blendbox(int x1, int y1, int x2, int y2, bool border)
{

    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    glBegin(GL_QUADS);
    if(border) glColor3d(0.5, 0.3, 0.4);
    else glColor3d(1.0, 1.0, 1.0);
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
    glColor3d(0.2, 0.7, 0.4);
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    verts += 8;
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);

};
*/


