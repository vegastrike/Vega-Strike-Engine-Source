#include "command.h"
#include <sstream>
#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>

#include "vs_random.h"
#include "python/python_class.h"

#ifdef HAVE_SDL
#   include <SDL/SDL.h>
#endif

// Introduction Comments {{{
// The {{{ and }}} symbols are VIM Fold Markers.
// They FOLD up the page so a user only needs to see a general outline of the entire huge file
// and be able to quickly get to exactly what part they need.
// It helps when a 300 line function is wrapped up to a one line comment
/* *******************************************************************
Example Commands: 
class WalkControls {
	public:
		WalkControls() { 
 // ****************************** 1
			Functor<WalkControls> *ctalk = new Functor<WalkControls>(this, &WalkControls::talk);
// Please notice the NEW, do _not_ delete this. It will be deleted for
// you when remCommand is run, or when the command processor destructor
// is called :)





			CommandInterpretor.addCommand(ctalk, "say", ARG_1STR); //1 c++ string argument,
			// CommandInterpretor is a global (defined in vs_globals.h or 
			// vegastrike.h (don't remember which) and created in main.cpp
 // ******************************* 2
			Functor<WalkControls> *ctalk = new Functor<WalkControls>(this, &WalkControls::talk);
			CommandInterpretor.addCommand(ctalk, "order", ARG_1STRVEC); 
// easy way to scroll through arguments to make logical desicions aboot them.
// use std::vector<std::string *>::iterator iter = d->begin(); 
// and (*(iter))->c_str() or (*(iter))->compare etc. iter++ to go up 
// iter-- to go down, and if(iter >= d->end()) to check and see if it's at the end.
  // ******************************* 3
			Functor<WalkControls> *dWalkLeft = new Functor<WalkControls>(this, &WalkControls::WalkLeft);
			CommandInterpretor.addCommand(dWalkLeft, "left", ARG_1BOOL);
			//to use this, there'd need to be a mechanism to bind
			//a single charactor to a full command, then when that
			//charactor is passed alone to execute it should translate it to 
			//whatever command it's bound to, then it can pass it to findCommand
			//and call findCommand("left", 0) instead of findCommand("a", 0);
  // ******************************** That's enuf.
	// Full list of supported argument types can be seen in commands.h (bottom)
	// for information about the actual supported callback methods
	// (all the functions below are be compatible, and then some)
	// see functors.h, commandI::fexecute, and the enum at the bottom of command.h
		}
		void talk(std::string &in);
		void order(std::vector<std::string *> *d);
		void WalkLeft(bool KeyIsDown);
		void WalkRight(bool KeyIsDown);
		void Jump(bool KeyIsDown);
		void setGravity(int &amount);
}
Then to get the commands to initiate, simply:
static WalkControls done; // and when the program starts, this variable will be made.
******************************************************************* */

/* *******************************************************************
	Steps to adding a different argument types
	Usually:
	std::string &entirecommandtyped (1STR)
	std::vector<std::string *> *CommandsTypedBrokenUpAtSpaces (1STRVEC)
	bool istheKeyPressedDownOrUp(True for down, false for up) (1BOOL)
	are enough.


	First, add a method of passing what you need to be included to execute
	(Such as an object pointer or something)
	Open Functors.h
	add a new callback functor
	(4 parts, 1 pure virtual function like the others in the base class
	with the argument type you need, 1 private variable for holding
	the function pointer on the second class, 1 constructor for
	creating the callback like any other, and 1 overloaded call()
	method to make it be called right)

	Add a new type to the ENUM in command.h (TYPE_MYSPECIALARGUMENTS)
	go to the execute function in this file, figure out how to pass
	your extra data down through execute, past the menusystem, to fexecute,
	down to the switch()
	At the switch statement add your enum, and the proper method of calling
	the functor.


	There are already quite a few types supported, so code examples are
	readily availble

	Valid command examples: 
	//Note: CMDOBJ stands for the Command Processor Object Name
		class myClass() {
			public:
			myClass() {
				 Functor<myClass> *newcommand = Functor<myClass>(this, &myClass::myFunction);
				CMDOBJ.addCommand(newcommand, "access-word", 1STR);
				Functor<myClass> *anotherone = Functor<myClass>(this, &myCommand::mySecondFunction);
				CMDOBJ.addCommand(newcommand, "access-word2", 1STRVEC);
			}
			void myFunction(std::string &in) {
				std::cout << in << "\n";
			}
			void mySecondFunction(std::vector<std::string *> *d) {
				for(std::vector<std::string *>::iterator iter = d->begin();
					iter < d->end(); iter++) {
					std::cout << (*(iter))->c_str() << "\n";
				}
			}
		};
	There currently is no way of removing commands from a command proccessor
	but it should be pretty easy, so something like:
	~myClass() {  CMDOBJ.remCommand("access-word"); }; could be done
	and the command proccessor could change depending on what objects exist.
	Command Processor Object Name(for reference) (Named: FILL_IN_NAME)
	

******************************************************************* */
/* *******************************************************************
PolyMorphic Behaviors:
If you have the command "left" on the object "flight-mode" 
the flight-mode object may always be in memory, but imagine you want to land
on a planet, where the command "left" might need to toggle a different vector
 to get it to perform different physics equations.
You could create a new object: (psuedo)
	class walkOnPlanet {
		Functor<WOP> *leftCommand;
		walkOnPlanet() { 
			Functor<WOP> *leftCommand = new Functor<WOP>(This, &walkOnPlanet::left);
			addCommand(leftCommand, "left", 1BOOL); //adding the second left command will automagically override the first
	}
	~walkOnPlanet() {
		remCommand(leftCommand); //by passing it by pointer we can be assured the right one will be removed, in case commands are added/removed out of order
	}
	void left(bool isDown) {
		perform different ops
	}

Then 

******************************************************************* */

/* A quick comment on Multiple Arguments 
	Imagine you have:
	void MyClass::myFunction(const char *arg1, const char *arg2) 
	If you set it to 2CSTR it will work, it also combines everything in a "" to a single arg.
	
	(In fact argument type does this.)

	In other words, if you do:
	myFunction "a four word name" "some arguments"
	it will send "a four word name" as arg1,
	and "some arguments" as arg2 (With the quotes edited out.)
	
	Everything except 1STR (C++ style std::string &in) behaves as so.
	1STR should still contain the quotes if they are needed. Just something to think about.

	
*/

/* ********************
Finally the last comments for way up here
BUG WARNING WITH 1STRARRAY
Do NOT try to modify anything in your 1STRARRAY argument!
if you have
void myClass::myFunction(char *array_in[]) {...}
do NOT modify array_in, you can COPY it to an std::string or to
a different charactor pointer then modify that.
The reason for this, the array passed to your function, is actually built inside
an std::vector<std::string *>, and is a CONST pointer to the data inside the vector
********************* */
// }}}

//Coms object {{{
coms::coms(TFunctor *t_in) {functor = t_in;};
coms::coms(coms *oldCom) {
	if(oldCom->Name.size() > 0)
		Name.append(oldCom->Name);
	argtype = oldCom->argtype;
	functor = oldCom->functor;
};
coms::coms(const coms &in) {
	if(in.Name.size() > 0) 
		Name.append(in.Name);
	argtype = in.argtype;
	functor = in.functor;
}
coms::~coms() {
//	std::cout << "Destroying coms object\n";
};
// }}}

// {{{ Set up the command vector
class HoldCommands {
	private:
		friend class commandI; 
		std::vector<coms> rc; //real commands vector
}; 
HoldCommands *rcCMD = 0x0;
bool rcCMDEXISTS = false; //initialize to false
// We use a pointer so we can initialize it in addCommand, which can, and does 
// run before the command interpretor constructor, and before all local variables
// on the command interpretor itself might be initialized.
// This object could and probobly should be used for menu's as well, IF 
// anybody ever uses the menusystem 
// }}}

// {{{ command interpretor constructor
commandI::commandI() {
	std::cout << "Command Interpretor Created\n\r";
	// {{{ add some base commands 

	Functor<commandI> *dprompt = new Functor<commandI>(this, &commandI::prompt);
    //fill with dummy function.
	dprompt->attribs.hidden = true;
	addCommand(dprompt, "prompt", ARG_NONE);

	Functor<commandI> *newFunct = new Functor<commandI>(this, &commandI::dummy);
	newFunct->attribs.hidden = true;
	addCommand(newFunct, "dummy", ARG_1STRVEC);

	Functor<commandI> *dcommands = new Functor<commandI>(this, &commandI::pcommands);
	addCommand(dcommands, "commands", ARG_NONE);

	Functor<commandI> *dhelp = new Functor<commandI>(this, &commandI::help);
	addCommand(dhelp, "help", ARG_1STR);
	// }}}
	// set some local object variables {{{
	menumode = false;
	immortal = false;
	console = false;
	// }}}
};
// }}}
// {{{ command interpretor destructor 
commandI::~commandI() {
		{
	        std::vector<coms>::iterator iter = rcCMD->rc.begin();// = rcCMD->rc.end();
	        while(rcCMD->rc.size() > 0) {
					TFunctor *f = ((*(iter)).functor);
					delete f;
        	        rcCMD->rc.erase(iter);
					iter = rcCMD->rc.begin();
	       	 };
		}
		{
			std::vector<menu *>::iterator iter;
			while(menus.size() > 0) {
				iter = menus.begin();
				menu *m = (*(iter));
				delete m;
				menus.erase(iter);
			};
		}
		if(rcCMDEXISTS) delete rcCMD;
};
// }}}
// {{{ Menu object destructor
menu::~menu() {
	for(std::vector<mItem *>::iterator iter = items.begin();
		iter < items.end(); iter++ ) {
			mItem *m = (*(iter));
			delete m;
			items.erase(iter);
	};
};
// }}}

// {{{ UNFINISHED HELP COMMAND
void commandI::help(std::string &helponthis) {
        std::string buf;
        buf.append("Sorry, there is no help system yet\n\r ");
	buf.append("But most commands are self supporting, just type them to see what they do.\n\r");
//        conoutf(this, &buf);

};
// }}}
// {{{ send prompt ONLY when 0 charactors are sent with a newline
void commandI::prompt() {
	std::string l;
	l.append("Wooooooooooo\n");
	conoutf(l);
//	std::cout << "Prompt called :)\n";
};
// }}}
// {{{ dummy function
void commandI::dummy(std::vector<std::string *> *d) {
    // {{{
	std::string outs;
	int rand = vsrandom.genrand_int32();
	if(rand % 2 == 0) {
		outs.append("Wtf?\n\r");
	} else
		outs.append("Try: commands\n\r");
	conoutf(outs);
	// }}}
}
// }}}
//list all the commands {{{
#include <iomanip>
void commandI::pcommands() {
	int x = 0;
	std::ostringstream cmd;
	cmd << "\n\rCommands available:\n\r";
	std::vector<coms>::iterator iter;
//	if(webb) cmd << "<PRE>";
	for(iter = rcCMD->rc.begin(); iter < rcCMD->rc.end(); iter++) {
		if(!(*(iter)).functor->attribs.hidden && !(*(iter)).functor->attribs.webbcmd) {
			if((*(iter)).functor->attribs.immcmd == true) {
				if(immortal) {
					if(x != 5)  cmd << std::setiosflags(std::ios::left) <<std::setw(10);

						cmd << (*(iter)).Name.c_str() ;

					x++;
				} //we don't want to add the command if we arn't immortal
			} else {
				if(x != 5) cmd << std::setiosflags(std::ios::left) <<std::setw(10);

				cmd <<(*(iter)).Name.c_str();
				x++;
			}
			if(x == 5)  {
//				if(webb) cmd << "<br>"; //lowercase <br> 
				cmd << "\n\r";
				x = 0;
			}
		}
	}
	if(x != 5) {
//		if(webb) {
//			cmd << "<br>"; //lowercase <br> again
//		} else 
			cmd << "\n\r";
	}
//	if(webb) cmd << "</PRE>";
//	cmd.append("\n\r");
	std::string cmd2;
	cmd2.append(cmd.str());
	conoutf(cmd2);
//	conoutf(this, &cmd2);
	//use print1 for color formatting.

}
// }}}
// {{{ addCommand - Add a command to the interpreter
void commandI::addCommand(TFunctor *com, char *name, int args){
		std::cout << "Adding command: " << name << std::endl;
		coms newOne = new coms(com); 
	// See the very bottom of this file for comments about possible optimization
        newOne.Name.append(name);
        newOne.argtype = args;
	//push the new command back the vector.
		if(!rcCMDEXISTS) {
			if(rcCMD != 0x0) {
				std::cout << "Apparently rcCMD is not 0x0.. \n";
			}
			rcCMD = new HoldCommands(); 
			rcCMDEXISTS = true; }; 
			rcCMD->rc.insert(rcCMD->rc.begin(), newOne);
//        rcCMD->rc.push_back(newOne);
};
// }}}
// {{{ Remove a command remCommand(char *name)
void commandI::remCommand(char *name){ 
	if(rcCMD->rc.size() < 1) return;
	for(std::vector<coms>::iterator iter = rcCMD->rc.begin(); iter < rcCMD->rc.end();iter++) { 
		if((*(iter)).Name.compare(name) == 0) {
			std::cout << "Removing: " << name << std::endl;
			delete (*(iter)).functor;
			rcCMD->rc.erase(iter);
			return;
		}
	}
	std::cout << "Error, command " << name << " not removed, try using the TFunctor *com version instead. Also, this is case sensitive ;)\n";
}
void  commandI::remCommand(TFunctor *com) {
    if(rcCMD->rc.size() < 1) return;
    for(std::vector<coms>::iterator iter = rcCMD->rc.begin(); iter < rcCMD->rc.end();iter++) { 
        if((*(iter)).functor == com) {
			std::cout << "Removing: " << (*(iter)).Name << std::endl;
            delete (*(iter)).functor;
            rcCMD->rc.erase(iter);
            return;
        }
    }
	std::cout << "Error, couldn't find the command that owns the memory area: " << com << std::endl;
}
// }}}
// {{{ Find a command in the command interpretor 
coms commandI::findCommand(const char *comm, int &sock_in) {
	if(rcCMD->rc.size() < 1) throw "Error, commands vector empty, this shouldn't happen!\n";

	std::ostringstream in_s;

	if(!comm) ;
	else in_s << comm;//this is actually a hack
	//comm shouldn't ever be null if it gets this far.
	//but for some fucking reason it is sometimes..
	std::string name;
	name.append(in_s.str());
	size_t x;

// remove \n and \r's (4 possible network input) {{{
	for(x = name.find(" "); x != std::string::npos; x = name.find(" ", x+1)) name.replace(x, sizeof((int)"\n"), "");
	for(x = name.find("\n"); x != std::string::npos; x = name.find("\n", x+1)) name.replace(x, sizeof((int)"\n"), "");
	for(x = name.find("\r"); x != std::string::npos; x = name.find("\r", x+1)) name.replace(x, sizeof((int)"\r"), "");
// }}}
//if the input is less than one return prompt function{{{
	if(name.size() < 1) {
		std::vector<coms>::iterator iter = rcCMD->rc.begin();
	    bool breaker = true;
	    while(breaker == true) {
	        if(iter >= rcCMD->rc.end()) {iter--; breaker = false;continue;}
    	    else if((*(iter)).Name.compare("prompt") == 0) { return (*(iter));}
			else iter++;
		}
		return((*(iter))); //assign testCom to the iterator
	} 
// }}}
//transform name (the word in) to lowercase {{{
	bool golower = true;
	if(golower)
		std::transform(name.begin(), name.end(), name.begin(),static_cast < int(*)(int) > (tolower));
// }}}
// Start testing command names against the command entered {{{
	std::vector<coms>::iterator iter;
	for(iter = rcCMD->rc.begin();iter < rcCMD->rc.end(); iter++) {
	//set the test variable to the iterator of something in the command vector
		coms testCom((*(iter)));
		//clear the temporary buffer used for holding the name of this command
		std::string temp;
		//define a string to possibly print something to the user
		std::string printer;
		//if the length of the commands name is larger than what was entered {{{
		if(testCom.Name.length() >= name.length() ) {
			//append the size of the command entered of the test commands name
			//to the temporary test string
			temp.append(testCom.Name, 0, name.size());
			//transform the partial name to lowercase
			bool golower = true;
			if(golower)
				std::transform(temp.begin(), temp.end(), temp.begin(),static_cast < int(*)(int) > (tolower));
			//compare them
			if(temp.compare(name) == 0 && name.size() > 0) {
				//they match {{{
				//If it is an immortal command 
				bool returnit = true;
				if(testCom.functor->attribs.immcmd == true) {
					//if we are immortal all's good, go on
					if(immortal);
					else { 
						//if we arn't immortal move on to the next command
						//this allows commands to have immortal/mortal versions
						//that call different functions.
						returnit = false;
//						iter = rcCMD->rc.begin();
//						iter++;
//						testCom = (*(iter));
						
					} 
				}
				//if it's an immortal command and we are an immortal simply don't return it.
				if(returnit) 
					return testCom;
				// }}}
			}	
//} }}}
// else {{{}
		//the command entered is larger than the commands length
		//if it's at most 1 larger try shaving off the last 1  
		//try fuzzy match
		} else if(testCom.Name.length() < name.length() && testCom.Name.length() >= name.length()-1 ) {
//			if(webb && !p->canSend(sock_in)) continue;
			temp.append(testCom.Name);
			std::string commandentered2; 
			commandentered2.append(name, 0, testCom.Name.size());
			//transform them to lowercase
			std::transform(temp.begin(), temp.end(), temp.begin(),static_cast <int(*)(int) > (tolower));
			std::transform(commandentered2.begin(), commandentered2.end(), commandentered2.begin(),static_cast <int(*)(int) > (tolower));
			if(temp.compare(commandentered2) == 0) {
                //they match {{{
                //If it is an immortal command
                bool returnit = true;
                if(testCom.functor->attribs.immcmd == true) {
                    //if we are immortal all's good, go on
                    if(immortal);
                    else {
                        //if we arn't immortal move on to the next command
                        //this allows commands to have immortal/mortal versions
                        //that call different functions.
                        returnit = false;
//                      iter = rcCMD->rc.begin();
//                      iter++;
//                      testCom = (*(iter));

                    }
                }
                //if it's an immortal command and we are an immortal simply don't return it.
                if(returnit)
                    return testCom;
                // }}}

			}
			
		}
		// }}}
	}
	
// }}}
// No command was found, so we find and return the dummy command {{{
	{
	std::vector<coms>::iterator iter = rcCMD->rc.begin();
	bool breaker = true;
	for(;breaker == true;) {
		if(iter >= rcCMD->rc.end()) { iter--;breaker = false; break;}
		else if((*(iter)).Name.compare("dummy") == 0) { return (*(iter)); } //here
		else iter++;
	}
	coms testCom( (*(iter)) ); //we should never get here
	return testCom; //or here, it exists on the first else above
	}
// }}}
};
/// }}}

// {{{ main execute function

bool commandI::execute(std::string *incommand, bool isDown, int sock_in)
{
//	std::string wreplace;
	// use the menusystem ONLY if the sock_in is the same as socket{{{
	// All talk about sockets is due to the  the command processor
	// being used in the heart of a (soon to be GPL) http/1.1 webserver
	{
		if(menumode ) {
			std::string l;
			std::string y;
			size_t x = incommand->find(" ");
			if(x < std::string::npos) {
				l.append(incommand->substr(0, x));
			} else {
				l.append(incommand->c_str());
			}
			std::string t;
			t.append((*(incommand)));
			if(x < std::string::npos)
				y.append(incommand->substr(x, incommand->size()-1));
			else 
				y.append(incommand->c_str());
			if(l.compare("\r\n") == 0) ;
			else {
				size_t lv = l.find("\r");
				while(lv < std::string::npos) {
					l.replace(lv, 1, "");
					lv = l.find("\r");
				}
	            lv = l.find("\n");
	            while(lv < std::string::npos) {
	                l.replace(lv, 1, "");
	                lv = l.find("\n");
	            }
	            lv = y.find("\r");
	            while(lv < std::string::npos) {
	                y.replace(lv, 1, "");
	                lv = y.find("\r");
	            }
	            lv = y.find("\n");
    	        while(lv < std::string::npos) {
	                y.replace(lv, 1, "");
	                lv = y.find("\n");
	            }

			}
			char *name_out = NULL;
			if(l.size() > 0) name_out = (char *)l.c_str();
			if(callMenu(name_out, (char *)y.c_str(), t) ) return false;
			incommand->erase();
			incommand->append(t); //t may have changed if we got this far
		}

	}
	// }}}
	return fexecute(incommand, isDown, sock_in);
};
//broken up into two execute functions
//the one below is the real execute, the one above uses the menusystem
//it's broken up so the menusystem can call fexecute themself at the right
//time
//sock_in was used for passing commands around for an HTTP server

bool commandI::fexecute(std::string *incommand, bool isDown, int sock_in) {
	size_t ls, y; //, args = 0;
	bool breaker = false;
    //************ try to replace erase leading space if there is one
	//eg, someone types: " do_something" instead of "do_something"
	while(breaker == false) {
		ls = incommand->find(" ");
		if(ls > 2 || incommand->size() < 2 || ls == std::string::npos) {
			breaker = true;
		} else {
			incommand->erase(ls, 1);
		}
	}

// Print back what the user typed.. {{{
// .. Sometimes people believe they typed python print "hello world\n"
// (and saw what they typed when they typed it)
// but may have actually typed oython print "hello world\n"
// and don't want to admit it, so they blame the system.
// So the system must sometimes politely tell the user what they typed
		{
			bool printit = false;
			if(menumode) {
//				if(menu_in->selected) {
//					if(menu_in->iselected->inputbit || menu_in->iselected->inputbit2) printit = true;
//				}
			} else if(console) printit = true;
			if(printit) {
				std::string webout;
				webout.append(incommand->c_str());
				webout.append("\n\r");
				conoutf(webout);
			}
		}
// }}}
	//replace \r\n with a space {{{

	for(y = incommand->find("\r\n"); y != std::string::npos; y = incommand->find("\r\n", y+1)) {
        incommand->replace(y, 2, " ");
    }
	// }}}
	// remove multiple spaces {{{
	for(y = incommand->find("  "); y != std::string::npos; y = incommand->find("  ", y+1)) {
		incommand->replace(y, 1, "");
	}
	// }}}
    // {{{ ' to say

    {
        size_t x = incommand->find("'");
        if(x == 0) {
            incommand->replace(0, 1, "say ");
        }
        // }}}
        // {{{ ! to the last command typed
        x = incommand->find("!");
        if(x == 0) {
            incommand->replace(0, 1, lastcommand);
        }
        // }}}
        // {{{ : to python
        x = incommand->find(":");
        if(x == 0) {
            incommand->replace(0, 1, "python ");
        }
    }
    // }}}
    // {{{ / to gossip
    {
        size_t x = incommand->find("/");
        if(x == 0) {
            incommand->replace(0, 1, "gossip ");
        }

    }
    // }}}

	breaker = false; //reset our exit bool 
    
	//done with formatting
	//now make what our std::vector<std::string> {{{
	std::vector<std::string> strvec; //to replace newincommand
								// to reduce data replication by one;
    {
	std::string::const_iterator scroller = incommand->begin();
	size_t last = 0, next = 0;
	bool quote = false;
	next=incommand->find(" ");
	for(next = incommand->find("\"\"", 0); (next=incommand->find("\"\"",last),(last!=std::string::npos)); last=(next!=std::string::npos)?next+1:std::string::npos) {
		if(next < std::string::npos)
			incommand->replace(next, 2, "\" \""); //replace "" with " "
	}
	std::string starter("");
	strvec.push_back(starter);
	for(scroller = incommand->begin(); scroller < incommand->end(); scroller++)
	{
		if(*scroller=='\"') {
			if(quote) {
			quote = false;
			} else {
			quote = true;
			}
			continue;
		}
		if(*scroller==' ' && !quote) {
			strvec.push_back(starter);
		continue;
		}
		strvec[strvec.size()-1] += *scroller;
	}

	}
    // }}}
    {
		// if the last argument is a space, erase it. {{{
        std::vector<std::string>::iterator iter = strvec.end();
        iter--;
        if((*(iter)).compare(" ") == 0) {
            strvec.erase(iter);
        }
		// }}}
    }
	try {
	coms theCommand = findCommand((char *)strvec[0].c_str(), sock_in);
	

//	if(args == 0) args = 1; //hack fix. 
//	else args += 1; 
//
//Now, we try to replace what was typed with the name returned by findCommand {{{
//to autocomplete words (EX: translate gos into gossip so the gossip
//command only has to find it's access name and not all possible
//methods of accessing it.)
		size_t x = incommand->find_first_of(strvec[0]);
		if(x != std::string::npos) {
			incommand->replace(x, strvec[0].size(), theCommand.Name); 
			strvec[0] = theCommand.Name;
		}
// }}}
        // {{{ try to execute it now - Switch based on the enumerated argument types
		lastcommand.erase();lastcommand.append(*incommand); //set the
		//last command entered.. Maybe this should only be set IF 
		//the callback works, and is not dummy(). (Use ! to trigger)
		try {
			//maybe if/else if would be more efficient, if this ever
			//gets really large.
			switch(theCommand.argtype) {
			case ARG_1INT:
				if(strvec.size() > 1)
					theCommand.functor->Call(atoi(strvec.at(1).c_str()));
				else theCommand.functor->Call(0);
				break;
			case ARG_NONE:
				theCommand.functor->Call();
				break; 
			case ARG_1CSTR:
				if(strvec.size() > 1)
					theCommand.functor->Call(strvec.at(1).c_str());
				else theCommand.functor->Call((const char *)NULL);
				break;
			case ARG_1CSTRARRAY: 
				{
					vector<const char*>w; 
					size_t vargs = 0;
					//put together a c style charactor array: char w[length] (1CSTRARRAY) {{{
				{ 
						for(std::string::size_type x = 0; x < strvec.size(); x++)
						{
	//						w[vargs] = "";
							w.push_back(strvec.at(x).c_str());
							vargs++;
						}
				}
		// }}}
					if(vargs == 0)
						w[0] = incommand->c_str();
					w.push_back("");
					theCommand.functor->Call(&w[0]);
					break;
				}
			case ARG_2CSTR:
				if(strvec.size() > 2)
					theCommand.functor->Call(strvec.at(1).c_str(),strvec.at(2).c_str());
				else {
					if(strvec.size() == 2) theCommand.functor->Call(strvec.at(1).c_str(), (const char *)NULL);
					else theCommand.functor->Call((const char *)NULL, (const char *)NULL);
				}

				break;
			case ARG_1BOOL:
				theCommand.functor->Call((bool *)isDown);
				break; 
			case ARG_1STR: 
				{
					theCommand.functor->Call(*incommand);
					break;
				}
			case ARG_1STRSPEC:
				theCommand.functor->Call(*incommand, sock_in);
				break;
			case ARG_1STRVEC: 
				{
					std::vector<std::string *> stringvec;
					{
						for(std::string::size_type x = 0; x < strvec.size(); x++) 
						{
							stringvec.push_back(&strvec.at(x));
						}
					}
					theCommand.functor->Call(&stringvec);
					break;
				}
			case ARG_1STRVECSPEC:
				{
					std::vector<std::string *> stringvec;
					{
						for(std::string::size_type x = 0; x < strvec.size(); x++) 
						{
							stringvec.push_back(&strvec.at(x));
						} 
					}
					theCommand.functor->Call(&stringvec, sock_in);
					break;
				}
			default:
				std::string err1;
				err1.append("\n\rError, unsupported argument type!\n\r");
				err1.append("Try using 1CSTRARRAY or 1STR!\n\r");
				conoutf(err1);
				break;
			}
		//try to catch any errors that occured while executing
		} catch (std::exception e) {
			std::string l; 
			l.append("Command processor: Exception occured: ");
			l.append(e.what());
			l.append("\n\r");
			std::cout << l;
			conoutf(l);
		} catch (...) {
			std::string y;
			y.append("Command processor: exception occurered: Unknown, most likely cause: Wrong Arg_type arguement sent with addCommand.\n\r");
			std::cout << y;
			conoutf(y);
		}
        
    // }}}
	} catch(const char *in) { //catch findCommand error
		std::cout << in;
	}
	return true;
}	

// }}}

// {{{ menusystem 
/* ***************************************
An example of how the menusystem is used: 
(the very first menu when a player logs onto the ANT-Engine http://daggerfall.dynu.com:5555/player1/index.html OR telnet://daggerfall.dynu.com:5555 )

    {
    menu *m = new menu("newuser", "Welcome to the <GREEN>ANT<NORM> engine", "\r\n");
    m->autoselect = true; //automatically select a menuitem, MUST BE SET
    m->noescape = true; //no escaping this menu except by forcing it
    addMenu(m); //add the menu to the command processor
    mItem *mi = new mItem; //make a new menuitem 
    mi->Name.append(" "); //argument to access menu  //must have a name
    mi->action.append("UNAME "); //adds this to the function 2 call as the argument
    mi->action.append(seccode); //add the security code.
    mi->display.append(" "); // menu's display name
    mi->func2call.append("loginfunc"); //function 2 call
    mi->inputbit = true; // set single-line input mode
    mi->selectstring.append("Enter a username"); //string to display when this menuitem is selected
    addMenuItem(mi);// add the menuitem to the command processor, by default
					// added to the last menu added, can be overredden by passing
					// a menu * pointer as the second argument, eg:
					// addMenuItem(mi, m);
    m->aselect = mi; //this is the menu item to automatically select
    }

*************************************** */
//add a menu {{{
bool commandI::addMenu(menu *menu_in) {
	menus.push_back(menu_in);
	lastmenuadded = menu_in;
	return true;
};
// }}}
// {{{ display menu function
std::string commandI::displaymenu() {
    if(menumode) {
        std::ostringstream ps;
		ps << menu_in->Display << "\n";
        for(std::vector<mItem *>::iterator iter = menu_in->items.begin();
            iter < menu_in->items.end(); iter++) {
            ps << (*(iter))->Name << " " << (*(iter))->display;
			if((*(iter))->predisplay.size() > 0)
					 ps << " " << display((*(iter))->predisplay);
			ps << "\n";
        }
        std::string buf;
        buf.append(ps.str());
		if(menu_in->autoselect == true) {
			if(menu_in->selected == true) {
				buf.append(menu_in->iselected->selectstring);
			buf.append(": ");
//				if(webb == true) buf.append("\n\r");
			}
		} else {
			if(!menu_in->noescape) {
		
		        buf.append("Use: ");
		        if(menu_in->escape.compare("\r\n") == 0)
		            buf.append("enter");
		        else
		            buf.append(menu_in->escape);
		        buf.append(" to quit: \n" );
			} else {
				buf.append("Enter your selection: \n");
			}
		}
//		if(webb) buf.append("\n\r");
		return buf;
//		conoutf(buf);
	}
	std::string buf;
	buf.append("Error, not in menumode!");
	return buf;
};
// }}}
//menuitem to be appended to the last menu appended, or an existing menu if {{{
//the menu2use is specified
bool commandI::addMenuItem(mItem *mi, menu *menuin) {
	menu *menu2use;
	if(menuin == NULL) 
		menu2use = lastmenuadded;
	else 
		menu2use = menu_in;
	//if the command isn't found it will return dummy or prompt.
	for(std::vector<menu *>::iterator iter = menus.begin(); iter < menus.end(); iter++) {
		if(menu2use == (*(iter))) {
			menu2use->items.push_back(mi); //doh! :)
			return true;
		}
	}
return false;
};
// }}}
//call a menu with arguements {{{
bool commandI::callMenu(char *name_in, char *args_in, std::string &d) {
	//if there is a menu operation return true;
	std::string name;
	if(name_in != NULL)
	name.append(name_in);
//	bool freturnfalse = false; //force return false
	//{{{ if the name_in is the menu_in's escape charactor
		//change the menu_in to the last menu on menustack if there is
		//one, and pop the stack. If there is no menustack, set menumode
		//off.
		if(menumode) {
			if(!menu_in->selected) {
				if(!menu_in->noescape) {
					if(name.compare(menu_in->escape) == 0) {
						if(menustack.size() > 0 ) {
							std::vector<menu *>::iterator iter = menustack.end();
							iter--;
							menu_in = (*(iter));
							menustack.pop_back();
		//					return true;
						} else {
							menu_in = NULL;
							menumode = false;
							return true;
						}
					}
				}
			}
		}
	// }}}
	if(menumode) {
		if(menu_in->selected) {
			// Input mode 1  {{{
			if(menu_in->iselected->inputbit == true && menu_in->iselected->inputbit2 == false) {
				menu_in->selected = false;
				std::string arg;
				arg.append(menu_in->iselected->action);
				std::string funcn;
				funcn.append(menu_in->iselected->func2call);
				std::string dreplace;
				dreplace.append(d);
				d.erase();
				d.append(funcn);
				d.append(" ");
				d.append(arg);
				d.append(" ");
				d.append(dreplace);
			//setMenus {{{
				if(funcn.compare("setMenu") == 0) {
					std::string l;
					l.append(setMenu((char *)arg.c_str()));
					conoutf(l);
					return true;
				}
			// }}}
				size_t ylast = 0, xasd = 0;
				//login function {{{
				if(funcn.compare("loginfunc") == 0) {
					std::vector<std::string *> d_out;
					d.append(" ");
					for(size_t x = d.find("\r\n"); x < std::string::npos; x = d.find("\r\n", x+3)) {
						d.replace(x, 1, " \r\n");
					}
					for(size_t iter= 0; iter < d.size();iter++) {
						if(d[iter]==32) {
							std::string *xs = new std::string();
							xs->append(d.substr(ylast, xasd-ylast));
							ylast = xasd;
							d_out.push_back(xs);
						}
						xasd++;
					}
//					loginfunc(&d_out); //login function 
					std::vector<std::string *>::iterator itera = d_out.begin();
					while(d_out.size() > 0 ) {
						std::string *s = (*(itera));
						delete s;
						d_out.erase(itera);
						itera=d_out.begin();
					}
					return true;
				}
			// }}}
			//autoreprint {{{
				if(menu_in->iselected->autoreprint == true) {
						fexecute(&d, true, 0);
						std::string x;
						x.append(displaymenu());
						conoutf(x);
						return true;
				}
		// }}}
				return false;
			}
			// }}}
			//input mode 2 {{{
            if(menu_in->iselected->inputbit == false && menu_in->iselected->inputbit2 == true) {
				//wait until we find an escape seqence alone {{{
				if( name.compare(menu_in->escape) == 0 ) {
					menu_in->selected = false;
					std::string arg;
					arg.append(menu_in->iselected->action);
					std::string funcn;
					funcn.append(menu_in->iselected->func2call);
					d.erase();
					d.append(funcn);
					d.append(" ");
					d.append(arg);
					d.append(" ");
					{ 
						size_t l = 0;
						bool y = false;
						for(size_t x = menu_in->iselected->menubuf.find("\r\n"); x < std::string::npos; x = menu_in->iselected->menubuf.find("\r\n", x+1)) {
							menu_in->iselected->menubuf.replace(x, 2, "<BR>");
							l = x;
							y = true;
						}
						if(y)
						menu_in->iselected->menubuf.replace(l, 4, ""); //replace the last <BR>
					}
					d.append(menu_in->iselected->menubuf);
					d.append(" ");
					menu_in->iselected->menubuf.erase();
									
					if(funcn.compare("setMenu") == 0) {
						std::string buf;
						buf.append(setMenu((char *)arg.c_str()));
						conoutf(buf);
						return true;
	                }
					if(funcn.compare("loginfunc") == 0) {
						std::vector<std::string *> d_out;
						d.append(" ");
						for(size_t x = d.find("\r\n"); x < std::string::npos; x = d.find("\r\n", x+1)) {
							d.replace(x, 2, "<BR>");
						}
						size_t ylast = 0, xasd = 0;
						for(size_t iter= 0; iter < d.size();iter++) {
							if(d[iter]==32) {
								std::string *xs = new std::string();
								xs->append(d.substr(ylast, xasd-ylast));
								ylast = xasd;
								d_out.push_back(xs);
							}
						xasd++;
						}
//						loginfunc(&d_out); //login function
						std::vector<std::string *>::iterator itera = d_out.begin();
						while(d_out.size() > 0 ) {
						 	std::string *s = (*(itera));
							delete s;
							d_out.erase(itera);
							itera=d_out.begin();
						}
						return true;
					}
                    if(menu_in->iselected->autoreprint == true) {
						fexecute(&d, true, 0);
                        std::string x;
                        x.append(displaymenu());
                        conoutf(x);
						return true;
                    }

					return false;
				// }}} 
				// or we append the input to the buffer  {{{
				} else {
					menu_in->iselected->menubuf.append(d);
//					if(webb) {
//						std::string buf;
//						buf.append(d);
//						size_t x = buf.find("\r\n");
//						buf.replace(x, 2, "\n\r");
//						conoutf(this, &buf);
//					}
				}
				// }}}

                return true;
            }
			// }}}
		}

		// if we don't have anything selected, select one.. {{{
		if(!menu_in->selected) {
			for(std::vector<mItem*>::iterator iter = menu_in->items.begin();
			iter < menu_in->items.end(); iter++)  {
				if((*(iter))->Name.compare(name) == 0) {
					menu_in->selected = true;
					menu_in->iselected = (*(iter));
//					if(menu_in->iselected->predisplay.size() > 0) {
//						display(menu_in->iselected->predisplay);
//					}
					if(menu_in->iselected->inputbit2) {
						std::string buf;
						buf.append(menu_in->iselected->selectstring);
						buf.append("\n\r");
						buf.append("Use: ");
						if(menu_in->escape.compare("\r\n") == 0)
							buf.append("enter");
						else
							buf.append(menu_in->escape);
						buf.append(" to confirm: " );
//						if(webb) buf.append("\n\r");

						conoutf(buf);
					} else if(menu_in->iselected->inputbit) {
 						std::string buf;
						buf.append(menu_in->iselected->selectstring);
						buf.append(": ");
//						if(webb) buf.append("\n\r");
						conoutf(buf);
					}
				}
			}
			if(menu_in->selected) {
				if(!menu_in->iselected->inputbit && !menu_in->iselected->inputbit2) 
				{
					menu_in->selected = false;
					std::string arg;
					arg.append(menu_in->iselected->action);
					std::string funcn;
					funcn.append(menu_in->iselected->func2call);
					std::string dreplace;
					dreplace.append(d);
					d.erase();
					d.append(funcn);
					d.append(" ");
					d.append(arg);
					d.append(" ");
					d.append(dreplace);
	                if(funcn.compare("setMenu") == 0) {
	                    std::string l;
	                    l.append(setMenu((char *)arg.c_str()));
	                    conoutf(l);
	                    return true;
	                }
	                if(funcn.compare("loginfunc") == 0) {
	                    std::vector<std::string *> d_out;
	                    d.append(" ");
	                    for(size_t x = d.find("\r\n"); x < std::string::npos; x = d.find("\r\n", x+3)) {
	                        d.replace(x, 1, " \r\n");
	                    }
						size_t ylast = 0, xasd = 0;
	                    for(size_t iter= 0; iter < d.size();iter++) {
	                        if(d[iter]==32) {
	                            std::string *xs = new std::string();
	                            xs->append(d.substr(ylast, xasd-ylast));
	                            ylast = xasd;
	                            d_out.push_back(xs);
	                        }
	                        xasd++;
	                    }
//	                    loginfunc(&d_out); //login function
	                    std::vector<std::string *>::iterator itera = d_out.begin();
	                    while(d_out.size() > 0 ) {
	                        std::string *s = (*(itera));
	                        delete s;
	                        d_out.erase(itera);
	                        itera=d_out.begin();
	                    }
	                    return true;
	                }

					return false;
				}
				return true;
			} else {
				if(menu_in->defaultInput) {
					menu_in->selected = true;
					menu_in->iselected = menu_in->idefaultInput;
					execute(&d, true, 0);
					return true;
				}
			}
		}
		// }}}
	} 
	if(menumode && !menu_in->selected) {
	//we're in a menu but don't have anything selected {{{
		std::string y;
		y.append(displaymenu());
		conoutf(y);
		return true;
	}
	// }}}
	return false;
};
// }}}
// set a menu {{{
std::string commandI::setMenu(char *name_in) {
	std::string name;
	name.append(name_in);
	if(name[0] == 32) name.replace(0, 1, "");
	for(std::vector<menu *>::iterator iter = menus.begin();
		iter < menus.end(); iter++ ){
		if((*(iter))->Name.compare(name) == 0) {
			if(!menumode) {
			menumode = true;
			} else {
				menustack.push_back(menu_in);
			}
			menu_in = (*(iter));
			menu_in->selected = false;
			if(menu_in->autoselect == true) {
				menu_in->selected = true;
				menu_in->iselected = menu_in->aselect;
			}
			iter = menus.end();
		}
	}
	return displaymenu();

};
// }}}
void commandI::breakmenu() {
	while(menustack.size() > 0 ) {
		menustack.pop_back();
	}
	menu_in = NULL;
	menumode = false;
};
// }}}

// commandI::display, find and display an existing value for an item on the menusystem {{{
std::string commandI::display(std::string &in) {
//this is used to build strings containing values related to the string in
//For an Online Content Creation Menusystem
//such as SHIP_SHIELD
//        SHIPE_DESC
//        then it would build a string based on the the current ship or editing
//		if(in.compare("ship_desc") == 0){  return editing_ship->description; };
	std::string f;
	f.append("Bad commandI::Display value (predisplay value on menuitems) \n");
	f.append(in);
	return f;
}
// }}}



//This whole python object could be moved to it's own cpp file and expanded to make a nice big python command that does stuff like... take multiple lines of input before executing, edit existing input (emacs/vim/notepad like or something)
extern commandI CommandInterpretor;
// {{{ Python object
RegisterPythonWithCommandInterp::RegisterPythonWithCommandInterp() {
	Functor<RegisterPythonWithCommandInterp> *l = new Functor<RegisterPythonWithCommandInterp>(this, &RegisterPythonWithCommandInterp::runPy);
	CommandInterpretor.addCommand(l, "python", ARG_1STR);
}

//run a python string
void RegisterPythonWithCommandInterp::runPy(std::string &argsin) {
	std::string pyRunString;
	pyRunString.append(argsin); //append the arguments in to the string to run
	size_t x = pyRunString.find("python "); //strip out the name of the command
	//and the first space
	if(x == 0)
		pyRunString.replace(x, 7, ""); //replace here
//this method was copied from somewhere else in the vegastrike source
	char *temppython = strdup(pyRunString.c_str()); //copy to a char *
	PyRun_SimpleString(temppython); //run it
	Python::reseterrors();
	free (temppython); //free the copy char *
}

// }}};


/* ***************************************************************
 Possible Optimizations:

	Optimizations discussed here arn't the tiny little save 2 or 3 cpu ops by reforming a for loop.
	These optimizations may make an impact on very very slow machines, or
	when ram is limited for copying objects, or when certain copies or types
	arn't ever needed.

	Possible optimization for findCommand (small optimization, less game-time copying overhead (after boot, while playing))
		copy a coms object when adding a command to the real command vector, (as it is now in addCommand)
		
		return a reference to the coms object from findCommand, to avoid
		copying every time a key is pressed or a command is entered.
		(change      coms findCommand   to    coms *findCommand)
		



	 Possible optimization for the main execute function ( medium optimization, less unneeded allocated variables in the execute function when not needed)
		Move findCommand higher up, before the string vector and 1str array are
		built, and build those depending on the argument type, to avoid
		excessive string copying when it's not needed, such as when not in
		console mode but in game mode, when 1 bool is enough to tell
		the function being called wether the key is pressed or not.
	- This might make it a little more difficult to read the execute function


*************************************************************** */

namespace ConsoleKeys {

    void BringConsole(const KBData&,KBSTATE newState)
    {
        //this way, keyboard state stays synchronized
        if(newState==RELEASE){
            CommandInterpretor.console = true;
#if HAVE_SDL
            SDL_EnableUNICODE(true);
#endif
        }
    }

}



// footer, leave at bottom
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
