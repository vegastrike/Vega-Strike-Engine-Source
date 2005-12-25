// rendertext.cpp: based on Don's gl_text.cpp
// Based on Aardarples rendertext 
#include "command.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vegastrike.h"
#include "cg_global.h"
#include "SDL/SDL.h"
#include "gfx/hud.h"
#include "gldrv/winsys.h"
#include <sstream>
// ****************
// Console Rendering System by Rogue
// 2005-08-a-few-days
//
// ****************

//Render Text (Console) Constructor {{{
RText::RText() {
	ndraw = 15;
	WORDWRAP = 85;
	conskip = 0;
//	histpos = 0;
	saycommand("");
}
// }}}
// Render Text (Console) Destructor {{{
RText::~RText() {

}
// }}}
// Set the text width, not used  .. yet{{{
int RText::text_width(char *str)
{
return 0;
};
// }}}
// Should be unused defines {{{
#define VIRTW 2400
#define VIRTH 1800
#define PIXELTAB (VIRTW/12)
#define FONTH 64
// }}}
// Draw text, used by the console, should be private, use conoutf to print to the console {{{
void RText::draw_text(std::string &str, float left, float top, int gl_num)
{
    int x = left;
    int y = top;

    int i;
    std::string::iterator iter = str.begin();
    GFXColor foreground(1, 1, 1, 1);
    GFXColor background(0.05f, 0.05f, 0.2f, 0.5f);
    TextPlane newTextPlane(foreground,background);
    newTextPlane.SetPos(x, y);
    newTextPlane.SetCharSize(.8, .12);
    newTextPlane.Draw(str);
};
// }}}
// render the console, only call if bool console == true {{{
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
	std::string shorter;
	shorter.append(getcurcommand() );
	while (shorter.size() > 80) { 
		shorter.erase(shorter.begin()); 
	} //erase the front of the current command while it's larger than 80
	// charactors, as to not draw off the screen
    drawCommand << workIt << "#FF1100> " << "#FF1100" << shorter << "#00000";
    std::string Acdraw; //passing .str() straight to draw_text produces an 
		//error with gcc 4, because it's constant I believe
    Acdraw.append(drawCommand.str());
    draw_text(Acdraw, x, y, 2);

};
// }}}
//append a line to the console, optional "highlight" method , untested {{{
void RText::conline(std::string &sf, bool highlight)        // add a line to the console buffer
{
	{
		unsigned int search =0;
		unsigned int lastsearch = 0;
		for(; (search = sf.find("/r"))!=std::string::npos ; ) {
			sf.replace(lastsearch, search-lastsearch, "");
			lastsearch = search;
		}
	}
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
// }}}
// print a line to the console, broken at \n's {{{
void RText::conoutf(char *in) {
	std::string foobar(in);
	conoutf(foobar);
	return;
}
void RText::conoutf(std::string &s, int a, int b, int c)
{
	SDL_mutex * mymutex = SDL_CreateMutex();
	SDL_mutexP(mymutex);
	std::cout << s << std::endl;
// Old {{{
//	{
//		for(int x = WORDWRAP; x < s.size(); x = x+WORDWRAP) {
//			s.insert(x, "\n");
//		}
//	}

//	size_t x = s.find("\n");
//	if(x < std::string::npos) {
//		size_t xlast = 0;
//		for(; x < std::string::npos; x = s.find("\n", x+1)) {
//			std::string newone;
//			newone.append(s.substr(xlast, x-xlast));
//			conline(newone, 1);
//			xlast = x+1;
//		}
//		
//	} else {
//		conline(s, 1);
//	}
// }}}
	unsigned int fries = s.size();
	std::string customer;
	for(unsigned int burger = 0; burger < fries; burger++) {
		if(s[burger] == '\n' || burger == fries-1) {
			if(burger == fries-1) 
				if(s[fries-1] != '\n' && s[fries-1] != '\r')
					customer += s[burger];
			conline(customer, 1);
			customer.erase();
		} else if( customer.size() >= WORDWRAP) {
			customer += s[burger];
			std::string fliptheburger;
			while( customer[customer.size()-1] != ' ') {
				fliptheburger += customer[customer.size()-1];
				std::string::iterator oldfloormeat = customer.end();
				oldfloormeat--; 
				customer.erase(oldfloormeat);
			}
			conline(customer, 1);
			customer.erase();
			{
				std::string spatchula;
				for(int salt = fliptheburger.size()-1; salt >= 0; salt--) {
					spatchula += fliptheburger[salt];
				}
				fliptheburger.erase();
				fliptheburger.append(spatchula);
			}
			customer.append(fliptheburger);
		} else if( s[burger] != '\r') { 
			customer += s[burger]; // get fat
		}
	}
	SDL_mutexV(mymutex);
	SDL_DestroyMutex(mymutex);
};
// }}}
//same as above, but I think it works better {{{
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
// }}}
// saycommand(char *), should "say" something, will be useful only with network enabled {{{
//does nothing now
void RText::saycommand(char *init)/// 
{ //actually, it appends "init" to commandbuf
//Unused.
//  SDL_EnableUNICODE((init!=NULL));
//    if(!editmode) keyrepeat(saycommandon);
//  if(!init) init = "";
//  commandbuf.append(init);

};
// }}}
// Console Keyboard Input {{{
void RText::ConsoleKeyboardI(int code, bool isdown)
{
	if(isdown) {
		switch(code){
//pop teh back of commandbuf
	                case WSK_BACKSPACE:
				{
				std::string::iterator iter = commandbuf.begin();
				if(iter < commandbuf.end()) {
					iter = commandbuf.end();
					iter--;
					commandbuf.erase(iter);
				}
				break;
				}
	                case WSK_LEFT:
//this should move a put pointer for commandbuf
//right should move it the other way.
//		                for(int i = 0; commandbuf[i]; i++) if(!commandbuf[i+1]) commandbuf[i] = 0;
		        break;

			case WSK_RETURN:
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
//print what was typed - Now done in the command processor
//clear the buffer
					commandbuf.erase();
				}
				break;
			default:
//add it to the command buffer
				if (code>0&&code<256) {
					unsigned char k = (unsigned char)code;
					commandbuf+=k;
				};
				break;
		}
	}

};
// }}}
// get the current command buffer, to execute at enter {{{
std::string RText::getcurcommand()
{
    return commandbuf;
};
// }}}




//footer, leave at bottom
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */

