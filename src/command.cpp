#include "command.h"
#include <sstream>
#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include "vs_random.h"
#include "python/python_class.h"
/* *******************************************************************
Example Commands: 
class WalkControls {
	public:
		WalkControls() { 
 // ****************************** 1
			Functor<WalkControls> *ctalk = new Functor<WalkControls>(this, &WalkControls::talk);
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
//static HoldCommands rcCMD __attribute__ ((init_priority (543)));
HoldCommands *rcCMD = 0x0;
bool rcCMDEXISTS = false; //initialize to false
//We need the init_priority to be higher than default (default is 65535 or something
//so we can be assured the vector rc will exist before any commands
//are added to it, even if those commands are created before the
//command interpretor (If say they're created on global scope.)
//If a command is created on the global scope, it may not initialize
//properly unless some special care is taken to trick the compiler into thinking
//the object is used (If the compiler thinks it's unused, it may not bother
//to even initialize it). Probobly making a static object out of the class

// }}}

// {{{ command interpretor constructor
commandI::commandI() {
//ALL commands must be first defined as a VIRTUAL function in object.h
//Your command must overload the virtual function, you get to name it.
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
	addCommand(dhelp, "help", ARG_1CSTR);
	// }}}
	// set some local object variables {{{
	menumode = false;
	immortal = false;
	console = false;
	// }}}
};
// }}}
// {{{ command interpretor constructor
/*
commandI::commandI(mud *world_in, object *player_in) {
//ALL commands must be first defined as a VIRTUAL function in object.h
//Your command must overload the virtual function, you get to name it.
    World = world_in;
	player = player_in;
    // {{{ add some base commands
    Functor<commandI> *dprompt = new Functor<commandI>(this, &commandI::prompt);
         //fill with dummy function.
    dprompt->attribs.hidden = true;
    addCommand(dprompt, "prompt", ARG_NONE);

        Functor<commandI> *newFunct = new Functor<commandI>(this, &commandI::dummy);
         //fill with dummy function
        newFunct->attribs.hidden = true;
        addCommand(newFunct, "dummy", ARG_1STRVEC);
        //add it right at creation to ensure it's at 0

    Functor<commandI> *dcommands = new Functor<commandI>(this, &commandI::pcommands);
    addCommand(dcommands, "commands", ARG_NONE);

    Functor<commandI> *dhelp = new Functor<commandI>(this, &commandI::help);
    addCommand(dhelp, "help", ARG_1CSTR);
    // }}}
    // set some local object variables {{{
    menumode = false;
    immortal = false;
    // }}}
};
*/
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
void commandI::help(char *) {
        std::string buf;
        buf.append("<NORM>Sorry, there is no help system yet\n\r ");
	buf.append("But most commands are self supporting, just type them to see what they do.\n\r");
//        World->print1(this, &buf);

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
//	World->print1(this, &cmd2);
	//use print1 for color formatting.

}
// }}}
// {{{ addCommand - Add a command to the interpreter
void commandI::addCommand(TFunctor *com, char *name, int args){
		std::cout << "Adding command: " << name << std::endl;
		coms newOne(com); 
	//use new() to make sure it doesn't get deleted when addCommad
	//returns.
        newOne.Name.append(name);
        newOne.argtype = args;
	//push the new command back the vector.
		if(!rcCMDEXISTS) {
			if(rcCMD != 0x0) {
				std::cout << "Apparently rcCMD is not 0x0.. \n";
			}
			rcCMD = new HoldCommands(); 
			rcCMDEXISTS = true; }; 
        rcCMD->rc.push_back(newOne);
};
// }}}
// {{{ Remove a command remCommand(char *name)
void commandI::remCommand(char *name){ 
	std::cout << "Removing: " << name << std::endl;
	if(rcCMD->rc.size() < 1) return;
	for(std::vector<coms>::iterator iter = rcCMD->rc.end(); iter >= rcCMD->rc.begin();) { 
		iter--;
		if(iter < rcCMD->rc.begin()) { return; }; 
		if((*(iter)).Name.compare(name) == 0) {
			delete (*(iter)).functor;
			rcCMD->rc.erase(iter);
			return;
		}
	}
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
// {{{ external function, should NEVER be used! UNSAFE!!!!!
//strips up command, extracts the first word and runs
//findCommand on it,
//then tries to execute the member function.
//If one is not found, it will call commandI::dummy() .
char *newstring(char *s, int l);
// }}}

// {{{ main execute function

bool commandI::execute(std::string *incommand, bool isDown, int sock_in)
{
//	std::string wreplace;
	//use the menusystem ONLY if the sock_in is the same as socket{{{
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
	size_t ls, y;
	bool breaker = false;
    //************
	while(breaker == false) {
		ls = incommand->find(" ");
		if(ls > 2 || incommand->size() > 1 || ls == std::string::npos) {
			breaker = true;
		} else {
			incommand->erase(ls, 1);
		}
	}
	size_t ylast = 0;
	for(y = incommand->find("\r\n"); y != std::string::npos; y = incommand->find("\r\n", y+1)) {
        incommand->replace(y, 2, " ");
    }
	for(y = incommand->find("  "); y != std::string::npos; y = incommand->find("  ", y+1)) {
		incommand->replace(y, 1, "");
	}
	size_t args = 0;
	breaker = false; //reset our exit bool
    
	//now make what 1CSTRARRAY relies on
	std::string newincommand;
	//append incommand
	newincommand.append((*(incommand)));
    // {{{ ' to say
    //ok special case before we find the command, if the first arguement
    //in svec starts with ', we must seperate the first argument
    //and replace ' with say.
    {
        size_t x = newincommand.find("'");
        if(x == 0) {
			newincommand.replace(0, 1, "say ");
        }
		// }}}
		// {{{ ! to the last command typed
        x = newincommand.find("!");
        if(x == 0) {
            newincommand.replace(0, 1, lastcommand);
        }
		// }}}
		// {{{ : to python
        x = newincommand.find(":");
        if(x == 0) {
            newincommand.replace(0, 1, "python ");
        }
    }
    // }}}
    // {{{ / to gossip
    //ok special case before we find the command, if the first arguement
    //in svec starts with ', we must seperate the first argument
    //and replace ' with say.
    {
        size_t x = newincommand.find("/");
        if(x == 0) {
            newincommand.replace(0, 1, "gossip ");
        }
        
    }
    // }}}
	//make sure there's always at least 1 space at the end.
	newincommand.append(" ");
	std::vector<std::string*> stringvec;
	ylast = 0;
	size_t xasd = 0;
        
    //count arguments and create stringvec{{{
    {for(size_t striter= 0; striter < newincommand.size();striter++) {
        if(newincommand[striter]==32) {
            //			w[args] = incommand->substr(ylast, x-ylast);
            //			ylast = x;
            std::string *xs = new std::string();
			xs->append(newincommand.substr(ylast, xasd-ylast));
			ylast = xasd;
			stringvec.push_back(xs);
			args++;
		}
		xasd++;
    } }
	// }}}
	{
        std::vector<std::string *>::iterator iter = stringvec.end();
        iter--;
        if((*(iter))->compare(" ") == 0) {
            std::string *xs = (*(iter));
            delete xs;
            stringvec.erase(iter);
        }
        
	}
	if(args == 0) args = 1; //hack fix.
	vector<char*>w(args);
	size_t vargs = 0;
	ylast = 0;
	breaker = false;
	//put together a c style charactor array: char w[length] (1CSTRARRAY) {{{
    { for(std::vector<std::string *>::iterator iter = stringvec.begin(); iter < stringvec.end() ;iter++) {
            w[vargs] = "";
            w[vargs] = (char *)(*(iter))->c_str();//(char *)(*(iter)).c_str();
            vargs++;
        } 
    }
	// }}}
	ylast = 0;
	if(vargs == 0) 
		w[0] = (char *)incommand->c_str();
	w.push_back("");	
    //	wreplace.append(incommand->c_str());
    
    //now this stuff is kind of confusing.
    //******************Find the command.
    //	if(tryExit(&stringvec)) { Exits were a virtual function that tested against
		// a vector of objects in a different game
//		std::vector<std::string *>::iterator itera = stringvec.begin();
//		while(stringvec.size() > 0 ) {
//			std::string *s = (*(itera));
//			delete s;
//			stringvec.erase(itera);
//			itera=stringvec.begin();
//		}
//		return true; 
//	};

    
	try {
		coms theCommand = findCommand(w[0], sock_in);
        
//Now, we try to replace what was typed with the name returned.
//to autocomplete words (EX: translate gos into gossip so the gossip
//command only has to find it's access name and not all possible
//methods of accessing it.)
        size_t x = newincommand.find_first_of(w[0]);
        if(x != std::string::npos) {
            newincommand.replace(x, strlen(w[0]), theCommand.Name); 
            //	free(w[0]);
            //	memset(w[0], 0, sizeof(w[0]));
            //	strncpy(w[0], theCommand->Name.c_str(), theCommand->Name.size());
            w[0] = (char *)theCommand.Name.c_str();
        }
        std::string sendstring;
        // {{{ try to execute it now	
//*********************Switch based on the enumerated argument types
//the two that will likely be used the most are:
//1CSTRARRAY (becuase aruments in it are split up as: w[0] = command used
//w[1] = arguement 1, w[2] = argument 2, etc. etc. which makes it VERY
//easy to turn each argument into it's own string to be manipulated
//and 1STR (which returns everything in a regular c++ string, without
//any \r\n's, without multiple spaces.) which makes it easy
//to do general formatting on the string (though not as easy to 
//extract the seperate arguments, which is why CSTRARRAY is available
//as well.
// the default case should NEVER be called, and if it is it won't 
// call the functor assosiated with it(as this would be unsafe.)
//all argument memory is actually held on the localstack
//which is a copy of incommand, the variable: char *w[] uses
//pointers to charactors inside wreplace (a c++ string held on the local stack)
//and thus arn't copied, and don't need to be freed, because when 
//the local stack returns, wreplace will get deleted, effecively freeing
//what w[x] pointed to, after this function is finished calling
//whatever function is being held in the functor.
        lastcommand.erase();lastcommand.append(newincommand);
        //	if(webb && !theCommand->functor->attribs.webbcmd ) {
		bool printit = false;
		if(menumode) {
            //			if(menu_in->selected) {
            //				if(menu_in->iselected->inputbit || menu_in->iselected->inputbit2) printit = true;
            //			}
		} else printit = true;
		if(printit) {
			std::string webout;
			webout.append(incommand->c_str());
			webout.append("\n\r");
            //			World->print1(this, &webout);
		}
        
        //	}
        try {
            switch(theCommand.argtype) {
            case ARG_1INT:
                theCommand.functor->Call(atoi((const char *)newincommand.at(1)));
                break;
            case ARG_NONE:
                theCommand.functor->Call();
                break; //this
            case ARG_1CSTR:
                theCommand.functor->Call(w[1]);
                break;
            case ARG_1CSTRARRAY:
                theCommand.functor->Call(&w[0]);
                        break;
            case ARG_2CSTR:
                theCommand.functor->Call(w[1], w[2]);
                break;
                case ARG_1BOOL:
                    theCommand.functor->Call((bool *)isDown);
                    break; //oops
            case ARG_1STR: 
                //			std::string sptr;
                theCommand.functor->Call(newincommand);
                break;
            case ARG_1STRSPEC:
//                      std::string sptr;
                theCommand.functor->Call(newincommand, sock_in);
                break;
		case ARG_1STRVEC:
			theCommand.functor->Call(&stringvec);
			break;
            case ARG_1STRVECSPEC:
                theCommand.functor->Call(&stringvec, sock_in);
                break;
            default:
                std::string err1;
                err1.append("\n\rError, unsupported argument type!\n\r");
                err1.append("Try using 1CSTRARRAY or 1STR!\n\r");
                //			World->print1(this, &err1);
                //                                theCommand->functor->Call(wreplace);
                break;
            }
        } catch (std::exception e) {
            std::cout << "Command proccessor: Exception occurered: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Command processor: exception occurered: Unknown, most likely cause: Wrong Arg_type arguement sent with addCommand.\n\r";
        }
        
    // }}}
    } catch(const char *in) { //catch findCommand error
        std::cout << in;
    }
    
    //    delete[] w;
    
	//some cleanup {{{
	std::vector<std::string *>::iterator itera = stringvec.begin();
	while(stringvec.size() > 0 ) {
		std::string *s = (*(itera));
		delete s;
		stringvec.erase(itera);
		itera=stringvec.begin();
	}
	// }}}
    return true;
}	

// }}}

// {{{ menusystem 
//add a menu {{{
bool commandI::addMenu(menu *menu_in) {
    
//	menu *m = new menu;
//	m->Name.append(name);
//	m->escape.append(escape_in);
	menus.push_back(menu_in);
	lastmenuadded = menu_in;
return true;
};
// }}}
// {{{ display menu function
std::string commandI::displaymenu() {
    if(menumode) {
        std::ostringstream ps;
		ps << menu_in->Display << "\n\r";
        for(std::vector<mItem *>::iterator iter = menu_in->items.begin();
            iter < menu_in->items.end(); iter++) {
            ps << (*(iter))->Name << " " << (*(iter))->display;
			if((*(iter))->predisplay.size() > 0)
					 ps << " " << display((*(iter))->predisplay);
			ps << "\n\r";
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
		        buf.append(" to quit: " );
			} else {
				buf.append("Enter your selection: ");
			}
		}
//		if(webb) buf.append("\n\r");
		return buf;
//        World->print1(this, &buf);
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
//					World->print1(this, &l);
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
//						World->print1(this,  &x);
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
//						World->print1(this, &buf);
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
//                        World->print1(this, &x);
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
//						World->print1(this, &buf);
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

//						World->print1(this, &buf);
					} else if(menu_in->iselected->inputbit) {
 						std::string buf;
						buf.append(menu_in->iselected->selectstring);
						buf.append(": ");
//						if(webb) buf.append("\n\r");
//						World->print1(this, &buf);
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
//	                    World->print1(this, &l);
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
//		World->print1(this, &y);
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







// bad old method of allocating newstrings, shouldn't be used {{{
char *newstring(char *s, int l) {
	char *b = (char *)malloc(l+1);
	strncpy(b, s, l);
	b[l] = 0;
	return b;
}

// }}}
// {{{ path stuff from CUBE engine, shouldn't be used 
//char *path(char *s)
//{
//    for(char *t = s; (t = strpbrk(t, "/\\")); *t++ = PATHDIV);
//    return s;
//};
// }}}
// cube engines parseword, included for informational purposes only, shouldn't be used {{{
char* commandI::parseword(char *&p)// parse single argument, including expressions
{
        p += strspn(p, " \t\r");
        if(p[0]=='/' && p[1]=='/') p += strcspn(p, "\n\0");
        if(*p=='\"')
                {
                p++;
                char *word = p;
                p += strcspn(p, "\"\r\n\0");
                char *s = newstring(word, p-word);
                if(*p=='\"') p++;
                return s;
                };
//                      if(*p=='(') return parseexp(p, ')');
//                      if(*p=='[') return parseexp(p, ']');
        char *word = p;
        p += strcspn(p, "; \t\r\n\0");
        if(p-word==0) return NULL;
        return newstring(word, p-word);
};
// }}}
// This was an attempt at something stupid, DONT use this! {{{
bool commandI::execCommand(char *buf, bool isDown)
{
//        char *temp;
  //      if(!buf) return false;
//        std::string *executestring = new std::string();
//        while(1) {
//                temp = seperate_c_string(&buf, "\n");
//                if(!temp)
//                        break;
//                executestring->erase();
//                executestring->append(temp);
//                //strip out everything after //
//                std::string::size_type comment = executestring->find("//", 0);
//                if(comment != std::string::npos) {
//                        executestring->erase(comment, executestring->size()-comment);
//                };
//                if(executestring->substr(0,1) != "#")
//                        execute(executestring, isDown);
//		
//        };
//	delete executestring; //deleted  by command interpreter.
//	free(buf);
        return true;
};
//supporting function
bool commandI::execCommand(std::string *string, bool isDown)
{
//        if(execCommand((char *)string->c_str(), isDown))
//                 return true;
        return false;
}
// }}}
// {{{ don't use this. 
bool commandI::execfile(char *cfgfile)
{
//        char s[260];
  //      strncpy(s, cfgfile, 260);
//        char *buf = loadfile(path(s), NULL);
//        //char *temp;
//        if(!buf) return false;
//        if(execCommand(buf, true) ) {
//		free(buf); return true;
//	}
//	free(buf);
        return false;
};
// }}}
// don't use any of this old stuff{{{
void commandI::exec(char *cfgfile)
{
//    if(!execfile(cfgfile)) std::cout <<"could not read " << cfgfile <<std::endl;
};
// }}}
//unused commandI::display. Needs to be defined to return something... 
std::string commandI::display(std::string &in) {
//this is used to build strings containing values related to the string in
//For an Online Content Creation Menusystem
//such as SHIP_SHIELD
//        SHIPE_DESC
//        then it would build a string based on the the current ship or editing
//		if(in.compare("ship_desc") == 0){  return editing_ship->description; };
std::string f;
f.append("FAKE");
return f;
}

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
// 
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */

