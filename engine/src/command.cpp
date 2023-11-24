/*
 * command.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Nachum Barcohen,
 * Stephen G. Tuggy, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include "command.h"
#include <Python.h>
#include <pyerrors.h>
#include <pythonrun.h>
#include "gldrv/winsys.h"
#include "main_loop.h"
#include "vs_random.h"
#include "python/python_class.h"
#include "vega_py_run.h"
#include <vector>

#ifdef HAVE_SDL
#   include <SDL2/SDL.h>
#endif

#include <string>
#include <iomanip>
#include <sstream>

using std::string;
using std::cout;
using std::endl;
using std::ostringstream;
using std::ios;
using std::setiosflags;
using std::setw;
using std::exception;

//Introduction Comments {{{
//The {{{ and }}} symbols are VIM Fold Markers.
//They FOLD up the page so a user only needs to see a general outline of the entire huge file
//and be able to quickly get to exactly what part they need.
//It helps when a 300 line function is wrapped up to a one line comment
/*
 *       Index:
 *               1) Example Commands
 *               2) Argument Types
 *               3) PolyMorphic Behavior
 *               4) Multiple Arguments
 *               5) Multiple Command Processors made easy with Inheritence
 */
/* *******************************************************************
*   1) Example Commands:
*   class WalkControls {
*        public:
*                WalkControls() {
*   // ****************************** 1
*                        Functor<WalkControls> *ctalk = new Functor<WalkControls>(this, &WalkControls::talk);
*   // Please notice the NEW, do _not_ delete this. It will be deleted for
*   // you when remCommand is run, or when the command processor destructor
*   // is called :)
*
*
*
*
*
*                        CommandInterpretor->addCommand(ctalk, "say", ARG_1STR); //1 c++ string argument,
*                        // CommandInterpretor is a global (defined in vs_globals.h or
*                        // vegastrike.h (don't remember which) and created in main.cpp
*   // ******************************* 2
*                        Functor<WalkControls> *ctalk = new Functor<WalkControls>(this, &WalkControls::talk);
*                        CommandInterpretor->addCommand(ctalk, "order", ARG_1STRVEC);
*   // easy way to scroll through arguments to make logical desicions aboot them.
*   // use std::vector<std::string *>::iterator iter = d->begin();
*   // and (*(iter))->c_str() or (*(iter))->compare etc. iter++ to go up
*   // iter-- to go down, and if(iter >= d->end()) to check and see if it's at the end.
*   // ******************************* 3
*                        Functor<WalkControls> *dWalkLeft = new Functor<WalkControls>(this, &WalkControls::WalkLeft);
*                        CommandInterpretor->addCommand(dWalkLeft, "left", ARG_1BOOL);
*                        //to use this, there'd need to be a mechanism to bind
*                        //a single charactor to a full command, then when that
*                        //charactor is passed alone to execute it should translate it to
*                        //whatever command it's bound to, then it can pass it to findCommand
*                        //and call findCommand("left", 0) instead of findCommand("a", 0);
*   // ******************************** That's enuf.
*        // Full list of supported argument types can be seen in commands.h (bottom)
*        // for information about the actual supported callback methods
*        // (all the functions below are be compatible, and then some)
*        // see functors.h, commandI::fexecute, and the enum at the bottom of command.h
*                }
*                void talk(std::string &in);
*                void order(std::vector<std::string *> *d);
*                void WalkLeft(bool KeyIsDown);
*                void WalkRight(bool KeyIsDown);
*                void Jump(bool KeyIsDown);
*                void setGravity(int &amount);
*   }
*   Then to get the commands to initiate, simply:
*   static WalkControls done; // and when the program starts, this variable will be made.
******************************************************************* */

/* *******************************************************************
*   2) Argument type details
*
*        Usually:
*        std::string &entire_command_typed
*        std::vector<std::string *> *Commands_Typed_BrokenUp_At_Spaces
*        bool is_the_Key_Pressed_Down_Or_Up(True for down, false for up)
*        int first argument translated through atoi, or 0 if first word is not entered
*        const char * first argument, if there is no first argument, it sends NULL
*        const char *, const char * first two arguments, sends NULl if not defined
*
*        are enough.
*
*        If you use std::string &, the name of your command will be at
*        string &in[0]; followed by a space
*
*        if you use std::vector<std::string *>* you can use:
*        std::vector<std::string *>::iterator iter = in->begin();
*        the very first iterator will point to the name of the command entered
*        not what the user inputed, findCommand will auto-finish commands entered
*        then use in->size() to see how many arguments were passed, and do whatever
*
*        1 Bool is for single keys bound to commands. An external layor to translate
*        the number of a key when pressed from sdl to a command is needed to
*        use this practically
*
*
*
*        A note about const char * types. If the user doesn't sent input, it
*        will pass NULL. So if you have two of them, and the user sends no
*        input, it will send them both as NULL. If the user types one
*        argument, the second one will be NULL, the first will be their argument.
*        If your function needs an argument, and NULL is passed, you can safely
*        throw an error message, either an std::exception or a const char *
*        so you can do: if(argument1 == NULL && argument2 == NULL) throw "I need at least one argument!";
*                                else if(argument2 == NULL) do_something_with_argument1, or maybe throw an error
*                                else we_have_both_arg1_and_arg2
*        const char *'s are the prefered method, std::exceptions get other error
*        text appended to them, and is mostly for detecting bugs in the std library
*
*
*
*        If you need to add a new argument type, or callback type, see functors.h
*        (Callbacks don't have to be on objects, there just is no support for that
*        in the functor class, and can be added as needed to make callbacks to existing
*        just do the 4 steps to makeing a new argument type in the functor, ignore the object part, it should be fairly trivial.
*
*        Sometimes this is useful, like with servers when passing a socket arond
*        to functions to complete requests.
*
*
*        To use a return value if support is added, the functor Call method returns
*        a void *, which by default is a casted reference to the return_type object
*        (see functors.h again) so it can be casted back with the string named "s"
*        extracted, which could have data if someone made a function that returned
*        an std::string and set it.
*
*
*   )
******************************************************************* */
/* *******************************************************************
*   (****** 3 ******)
*   PolyMorphic Behaviors:
*   If you have the command "left" on the object "flight-mode"
*   the flight-mode object may always be in memory, but imagine you want to land
*   on a planet, where the command "left" might need to toggle a different vector
*   to get it to perform different physics equations.
*   You could create a new object: (psuedo)
*        class walkOnPlanet  {
*                Functor<WOP> *leftCommand;
*                walkOnPlanet() {
*                        leftCommand = new Functor<WOP>(This, &walkOnPlanet::left);
*                        CommandInterpretor->addCommand(leftCommand, "left"); //adding the second left command will automagically override the first
*        }
*        ~walkOnPlanet() {
*                CommandInterpretor->remCommand(leftCommand); //by passing it by pointer we can be assured the right one will be removed, in case commands are added/removed out of order
*        }
*        void left(bool isDown) {
*                perform different ops
*        }
*
*   then create it, and it will ovverride the existing command with the access word "left"
*
******************************************************************* */

/*
 *  (***** 4 *****)
 *  A quick comment on Multiple Arguments
 *       Imagine you have:
 *       void MyClass::myFunction(const char *arg1, const char *arg2)
 *
 *       if you do:
 *       myFunction "a four word name" "some arguments"
 *       it will send "a four word name" as arg1,
 *       and "some arguments" as arg2 (With the quotes edited out.)
 *
 *       Everything except std::string does this, std::string passess the entire input string
 *       If you need a quote to pass through the command processor to a function, use \" the same way you'd pass a quote to a variable in c++ ;)
 *
 *
 */
/* ********************
 *  (****** 5 ******)
 *  Multiple Command Processors:
 *       Creating a custom command processor to do job X.
 *       Example abstract usage: Imbedded HTTP servers, objects which can be placed in game that can execute their own commands (then "taken-over" or "possessed", or given somehow to the player to be used)
 *
 *       Example with psuedo:
 *       class HTTPserver : public commandI {
 *               class clients {
 *                       friend class HTTPserver;
 *                       friend class std::vector<clients>;
 *                       clients(int &sock, char *addy) {
 *                               socket = sock;
 *                               ipaddress.append(addy);
 *                       };
 *                       clients(const clients &in) {
 *                               socket = in.socket;
 *                               ipaddress.append(in.ipaddress);
 *                       }
 *                       int socket;
 *                       std::string ipaddress;
 *               };
 *               class Packet {
 *                       public:
 *                               int socket;
 *                               char *data;
 *               };
 *               public:
 *               HTTPserver(char *port) : commandI() {
 *                       setupserveronport(atoi(port));
 *                       Functor<HTTPserver> *get = new Functor<HTTPserver>(this, &HTTPserver::GET);
 *                       commandI::addCommand(get, "GET");
 *
 *                       Functor<HTTPserver> *post = new Functor<HTTPserver(this, &HTTPserver::POST);
 *                       commandI::addCommand(post, "POST");
 *
 *
 *                       fork() {
 *                               while(SOMEBREAKVARIABLE)
 *                               runserver();
 *                       }
 *               }
 *               std::vector<clients> myclients;
 *               runServer() {
 *                       bool incfound = socketlayor->listen4new();
 *                       if(incfound) {
 *                               while(clients newclient(socketlayor->getANewOne() ) != NULL)
 *                               {
 *                                       myclients.push_back(newclient);
 *                               }
 *                       }
 *                       Packet *incomingpacket = socketlayor->listen();
 *                       if(incomingpacket != NULL) {
 *                               std::string ConvertDataToString(incomingpacket->data);
 *                               std::string buffer;
 *                               for(unsigned int counter = 0; counter < CDTS.size(); counter++)
 *                               {
 *                                       if(CDTS[counter] == '\n') {
 *                                               commandI::execute(buffer, true, socket);
 *                                               buffer.erase();
 *                                       } else {
 *                                               buffer += CDTS[counter];
 *                                       }
 *                               } if(buffer.size() > 0) { //POSTS don't end post data with anything, so if we want to process it, we do it now. Headers all terminate with \r\n.
 *                                       commandI::execute(buffer, true, socket);
 *                               }
 *
 *                               delete incomingpacket;
 *                       }
 *               }
 *               void GET(std::string &page, int socket) {
 *                       securityformat(page);
 *                       std::string getPage = Utilities.loadFile(page);
 *                       std::string buildHeader(page, getPage.size(), option1, option2, etc);
 *                       socketlayor->send(buildheader, socket);
 *                       socketlayor->send(getPage, socket);
 *               }
 *               void POST(std::string &page, int socket) {
 *                       setMode(POST) ;// do whatever we want posts to do on the server
 *               }
 *       }
 ********************* */
//}}}

//Coms object {{{


coms::coms(TFunctor *t_in) {
    functor = t_in;
}

coms::coms(coms *oldCom) {
    if (oldCom->Name.size() > 0) {
        Name.append(oldCom->Name);
    }
    functor = oldCom->functor;
}

coms::coms(const coms &in) {
    if (in.Name.size() > 0) {
        Name.append(in.Name);
    }
    functor = in.functor;
}

coms::~coms() {
//std::cout << "Destroying coms object\n";
}

//}}}
class HoldCommands;
HoldCommands *rcCMD = 0x0;
bool rcCMDEXISTS = false; //initialize to false

class HoldCommands   //Hold the commands here{{{
{
/*
 *  // Large comment about why and what {{{
 *  what:
 *  It creates "procs" objects which hold a pointer to a memory address(of the command processor) and the list of commands
 *  that processor has. It holds this list of "procs" in a global variable, which has a single bool to notify command
 *  interpretors when it needs to be created (So if there are 0 command interpretors in memory while the program is running,
 *  HoldCommands won't take up any space as a running object)
 *
 *  It contains 3 utility functions which take the "this" pointer
 *  addCMD: adds a command and takes a this pointer, if the command processor is not found in HoldCommands vector, it
 *               creates a new one, adds the command to the new one. Otherwise it adds it to the proper one.
 *  popProc: Pop a processor off the list (Not the usual pop_back type, it actually calls std::vector::erase(iterator))
 *  getProc: returns the procs object owned by the command interpretor calling it
 *
 *  why:
 *       To support multiple command processors, while still allowing commands to be added to a _Global_ command processor or two
 *       before they have allocated their internal variables and run their constructors. (If you add something to a vector
 *       on a global object before the object has initialized, that thing will get lost.)
 *       class B;
 *       extern B globalB;
 *       class A {
 *               public:
 *               A() { cout << "A created\n"; globalB.list.push_back(1); };
 *       }
 *       class B {
 *               public:
 *               B() { cout << "B created\n"; };
 *               vector<int> list;
 *       }
 *       A anA(); //A is defined BEFORE B. Very important
 *       B globalB();
 *       int main() {
 *               cout << globalB.list.size(); // gives me zero. Should be one.
 *       }
 *       (Normally, this wouldn't happen, IF we were working with just one file.
 *       But because we are working with multiple files that can be compiled in pretty much any order
 *       we have limited control of what order our globals are declared in. So we take advantage of the fact we can
 *       still run functions on a global object before it's initialized, and initialize what we need by hand at the
 *       exact time it's needed, which would be at any commandI::addCommand call, at any time.)
 *  // }}} I love folds so I can see only what I need ;) (ViM)
 */
    friend class commandI;
    bool finishmeoff;
    class procs {
    public:
        virtual ~procs() {
            while (rc.size() > 0) {
                rc.pop_back();
            }
        }

        procs(commandI *processor, coms *initcmd) {
            proc = processor;
            rc.push_back(initcmd);
        }

        procs(const procs &in) {
            procs *blah = const_cast< procs * > (&in);
            proc = blah->proc;
            for (vector<coms>::iterator iter = blah->rc.begin(); iter < blah->rc.end(); iter++) {
                rc.push_back((*(iter)));
            }
        }

        commandI *proc;
        vector<coms> rc;
    };

    HoldCommands() {
        if (rcCMD != 0x0) {
            cout << "Error, there shouldn't be 2 holdCommands objects!\n";
        }
        rcCMD = this;
        finishmeoff = false;
    }

    vector<procs> cmds;            //for multiple command processors.
    void addCMD(coms &commandin, commandI *proc2use) {
        bool found = false;
        for (vector<procs>::iterator iter = cmds.begin(); iter < cmds.end(); iter++) {
            if ((*(iter)).proc == proc2use) {
                found = true;
                (*(iter)).rc.insert((*(iter)).rc.begin(), commandin);
                iter = cmds.end();
            }
        }
        if (!found) {
            procs newproc(&(*proc2use), &commandin);
            cmds.push_back(newproc);
        }
    }

    void popProc(commandI *proc2use) {
        auto i = cmds.begin();
        while (i != cmds.end()) {
            if (proc2use == (*(i)).proc) {
                i = cmds.erase(i);
            } else {
                ++i;
            }
        }
        if (cmds.size() == 0) {
            finishmeoff = true;
        }
    }

    procs *getProc(commandI *in) {
        for (vector<procs>::iterator iter = cmds.begin(); iter < cmds.end(); iter++) {
            if (in == (*(iter)).proc) {
                return &(*(iter));
            }
        }
        return NULL;
    }
};

//mmoc initclientobject;

//Formerly RegisterPythonWithCommandInterp f***ingsonofat***w***lioness;

//We use a pointer so we can initialize it in addCommand, which can, and does
//run before the command interpretor constructor, and before all local variables
//on the command interpretor itself might be initialized.

//}}}

//{{{ command interpretor constructor

commandI::commandI() {
    cout << "Command Interpretor Created\n\r";
    //{{{ add some base commands

    Functor<commandI> *dprompt = new Functor<commandI>(this, &commandI::prompt);
    //fill with dummy function.
    dprompt->attribs.hidden = true;
    addCommand(dprompt, "prompt");

    Functor<commandI> *newFunct = new Functor<commandI>(this, &commandI::dummy);
    newFunct->attribs.hidden = true;
    addCommand(newFunct, "dummy");

    Functor<commandI> *dcommands = new Functor<commandI>(this, &commandI::pcommands);
    addCommand(dcommands, "commands");

    Functor<commandI> *dhelp = new Functor<commandI>(this, &commandI::help);
    addCommand(dhelp, "help");
    //}}}
    //set some local object variables {{{
    menumode = false;
    immortal = false;
    console = false;
    new RegisterPythonWithCommandInterpreter(this);     //mem leak - not cleaned up at end of program.
    //}}}
}

//}}}
//{{{ command interpretor destructor

commandI::~commandI() {
    {
        HoldCommands::procs *findme = rcCMD->getProc(this);
        if (findme->rc.size() > 0) {
            coms *iter = &findme->rc.back();
            while (findme->rc.size() > 0) {
                iter = &findme->rc.back();
                delete iter->functor;
                findme->rc.pop_back();
            }
        }
    }
    {
        menu *iter;
        while (menus.size() > 0) {
            iter = menus.back();
            delete iter;
            menus.pop_back();
        }
    }
    if (rcCMDEXISTS) {
        rcCMD->popProc(this);
        if (rcCMD->finishmeoff) {
            rcCMDEXISTS = false;
            delete rcCMD;
        }
    }
}

//}}}
//{{{ Menu object destructor

menu::~menu() {
    for (mItem *iter;
            items.size() > 0;) {
        iter = items.back();
        delete iter;
        items.pop_back();
    }
}

//}}}

//{{{ UNFINISHED HELP COMMAND

void commandI::help(string &helponthis) {
    string buf;
    buf.append("Sorry, there is no help system yet\n\r ");
    buf.append("But most commands are self supporting, just type them to see what they do.\n\r");
//conoutf(this, &buf);
}

//}}}
//{{{ send prompt ONLY when 0 charactors are sent with a newline

void commandI::prompt() {
    string l;
    l.append("Wooooooooooo\n");
    conoutf(l);
//std::cout << "Prompt called :)\n";
}

//}}}
//{{{ dummy function

void commandI::dummy(vector<string *> *d) {
    //{{{
    string outs;
    int rand = vsrandom.genrand_int32();
    if (rand % 2 == 0) {
        outs.append("Wtf?\n\r");
    } else {
        outs.append("Try: commands\n\r");
    }
    conoutf(outs);
    //}}}
}

//}}}
//list all the commands {{{

void commandI::pcommands() {
    int x = 0;
    ostringstream cmd;
    cmd << "\n\rCommands available:\n\r";
    vector<coms>::iterator iter;
    HoldCommands::procs *commands = rcCMD->getProc(this);
    for (iter = commands->rc.begin(); iter < commands->rc.end(); iter++) {
        if (!(*(iter)).functor->attribs.hidden && !(*(iter)).functor->attribs.webbcmd) {
            if ((*(iter)).functor->attribs.immcmd == true) {
                if (immortal) {
                    if (x != 5) {
                        cmd << setiosflags(ios::left) << setw(19);
                    }
                    cmd << (*(iter)).Name.c_str();
                    x++;
                }                 //we don't want to add the command if we arn't immortal
            } else {
                if (x != 5) {
                    cmd << setiosflags(ios::left) << setw(10);
                }
                cmd << (*(iter)).Name.c_str();
                x++;
            }
            if (x == 5) {
                cmd << "\n\r";
                x = 0;
            }
        }
    }
    if (x != 5) {
        cmd << "\n\r";
    }
    string cmd2;
    cmd2.append(cmd.str());
    conoutf(cmd2);
}

//}}}
//{{{ addCommand - Add a command to the interpreter

void commandI::addCommand(TFunctor *com, const char *name) {
    cout << "Adding command: " << name << endl;
    coms *newOne = new coms(com);
    //See the very bottom of this file for comments about possible optimization
    newOne->Name.append(name);
    //push the new command back the vector.
    if (!rcCMDEXISTS && rcCMD == 0x0) {
        if (rcCMD != 0x0) {
            cout << "Apparently rcCMD is not 0x0.. \n";
        }
        rcCMD = new HoldCommands();
        rcCMDEXISTS = true;
    }
    rcCMD->addCMD(*newOne, this);
//rcCMD->rc.push_back(newOne);
}

//}}}
//{{{ Remove a command remCommand(char *name)

void commandI::remCommand(char *name) {
    HoldCommands::procs *findme = rcCMD->getProc(this);
    if (findme->rc.size() < 1) {
        return;
    }
    for (vector<coms>::iterator iter = findme->rc.begin(); iter < findme->rc.end(); iter++) {
        if ((*(iter)).Name.compare(name) == 0) {
            cout << "Removing: " << name << endl;
            delete (*(iter)).functor;
            findme->rc.erase(iter);
            return;
        }
    }
    cout << "Error, command " << name
            << " not removed, try using the TFunctor *com version instead. Also, this is case sensitive ;)\n";
}

void commandI::remCommand(TFunctor *com) {
    HoldCommands::procs *findme = rcCMD->getProc(this);
    if (findme->rc.size() < 1) {
        return;
    }
    for (vector<coms>::iterator iter = findme->rc.begin(); iter < findme->rc.end(); iter++) {
        if ((*(iter)).functor == com) {
            cout << "Removing: " << (*(iter)).Name << endl;
            delete (*(iter)).functor;
            findme->rc.erase(iter);
            return;
        }
    }
    cout << "Error, couldn't find the command that owns the memory area: " << com << endl;
}

//}}}
//{{{ Find a command in the command interpretor

coms *commandI::findCommand(const char *comm, int &sock_in) {
    HoldCommands::procs *findme = rcCMD->getProc(this);
    if (findme->rc.size() < 1) {
        throw "Error, commands vector empty, this shouldn't happen!\n";
    }
    ostringstream in_s;
    if (!comm) {
    } else {
        in_s << comm;
    }        //this is actually a hack
    //comm shouldn't ever be null if it gets this far.
    //but for some fucking reason it is sometimes..
    string name;
    name.append(in_s.str());
    size_t x;
//remove \n and \r's (4 possible network input) {{{
    for (x = name.find(' '); x != string::npos; x = name.find(' ', x + 1)) {
        name.erase(name.begin() + x);
    }
    for (x = name.find('\n'); x != string::npos; x = name.find('\n', x + 1)) {
        name.erase(name.begin() + x);
    }
    for (x = name.find('\r'); x != string::npos; x = name.find('\r', x + 1)) {
        name.erase(name.begin() + x);
    }
//}}}
//if the input is less than one return prompt function{{{
    if (name.size() < 1) {
        vector<coms>::iterator iter = findme->rc.begin();
        bool breaker = true;
        while (breaker == true) {
            if (iter >= findme->rc.end()) {
                iter--;
                breaker = false;
                continue;
            } else if ((*(iter)).Name.compare("prompt") == 0) {
                return &(*(iter));
            } else {
                iter++;
            }
        }
        return &(*(iter));           //assign testCom to the iterator
    }
//}}}
//transform name (the word in) to lowercase {{{
    bool golower = true;
    if (golower) {
        transform(name.begin(), name.end(), name.begin(), static_cast< int (*)(int) > (tolower));
    }
//}}}
//Start testing command names against the command entered {{{
    coms *fuzzymatch = NULL;
    vector<coms>::iterator iter;
    for (iter = findme->rc.begin(); iter < findme->rc.end(); iter++) {
        //set the test variable to the iterator of something in the command vector
        coms &testCom = ((*(iter)));
        //clear the temporary buffer used for holding the name of this command
        string temp;
        //define a string to possibly print something to the user
        string printer;
        //if the length of the commands name is larger than what was entered {{{
        if (testCom.Name.length() >= name.length()) {
            //append the size of the command entered of the test commands name
            //to the temporary test string
            temp.append(testCom.Name, 0, name.size());
            //transform the partial name to lowercase
            bool golower = true;
            if (golower) {
                transform(temp.begin(), temp.end(), temp.begin(), static_cast< int (*)(int) > (tolower));
            }
            //compare them
            if (temp.compare(name) == 0 && name.size() > 0) {
                //they match {{{
                //If it is an immortal command
                bool returnit = true;
                if (testCom.functor->attribs.immcmd == true) {
                    //if we are immortal all's good, go on
                    if (immortal) {
                    } else {
                        //if we arn't immortal move on to the next command
                        //this allows commands to have immortal/mortal versions
                        //that call different functions.
                        returnit = false;
                    }
                    //iter = findme->rc.begin();
//iter++;
//testCom = (*(iter));
                }
                //if it's an immortal command and we are an immortal simply don't return it.
                if (returnit) {
                    if (name.size() == testCom.Name.size()) {
                        return &testCom;
                    }
                    if (fuzzymatch == NULL) {
                        fuzzymatch = &testCom;
                    }
                }
                //}}}
            }
//} }}}
//else {{{}
            //the command entered is larger than the commands length
            //if it's at most 1 larger try shaving off the last 1
            //try fuzzy match
        } else if (testCom.Name.length() < name.length() && testCom.Name.length() >= name.length() - 1) {
            temp.append(testCom.Name);
            string commandentered2;
            commandentered2.append(name, 0, testCom.Name.size());
            //transform them to lowercase
            transform(temp.begin(), temp.end(), temp.begin(), static_cast< int (*)(int) > (tolower));
            transform(commandentered2.begin(), commandentered2.end(), commandentered2.begin(),
                    static_cast< int (*)(int) > (tolower));
            if (temp.compare(commandentered2) == 0) {
                //they match {{{
                //If it is an immortal command
                bool returnit = true;
                if (testCom.functor->attribs.immcmd == true) {
                    //if we are immortal all's good, go on
                    if (immortal) {
                    } else {
                        //if we arn't immortal move on to the next command
                        returnit = false;
                    }
                }
                //if it's an immortal command and we are an immortal simply don't return it.
                if (returnit) {
                    if (fuzzymatch == NULL) {
                        fuzzymatch = &testCom;
                    }
                }
                //}}}
            }
        }
        //}}}
    }
    if (fuzzymatch != NULL) {
        return fuzzymatch;
    }
//}}}
    iter = findme->rc.begin();
    for (; iter < findme->rc.end(); iter++) {
        if ((*(iter)).Name.find("dummy") == 0) {
            return &(*(iter));
        }
    }
    //shouldn't get here.
    return NULL;
}

/// }}}
//strips up command, extracts the first word and runs
//findCommand on it,
//then tries to execute the member function.
//If one is not found, it will call commandI::dummy() .
//{{{ Main execute entrace, all input comes in here, this sends it to the menusystem, then in the return at the very last line executes the fexecute function which actually parses and finds commands, if the menusystem allows. This way the menusystem can manipulate user input, ie insert command names into the input to make it go to any function.
bool commandI::execute(string *incommand, bool isDown, int sock_in) {
    int socket = sock_in;
    //use the menusystem ONLY if the sock_in is the same as socket{{{
    {
        if (menumode && sock_in == socket) {
            string l;
            string y;
            size_t x = incommand->find(" ");
            if (x < string::npos) {
                l.append(incommand->substr(0, x));
            } else {
                l.append(incommand->c_str());
            }
            string t;
            t.append((*(incommand)));
            if (x < string::npos) {
                y.append(incommand->substr(x, incommand->size() - 1));
            } else {
                y.append(incommand->c_str());
            }
            if (l.compare("\r\n") == 0) {
            } else {
                size_t lv = l.find("\r");
                while (lv < string::npos) {
                    l.replace(lv, 1, "");
                    lv = l.find("\r");
                }
                lv = l.find("\n");
                while (lv < string::npos) {
                    l.replace(lv, 1, "");
                    lv = l.find("\n");
                }
                lv = y.find("\r");
                while (lv < string::npos) {
                    y.replace(lv, 1, "");
                    lv = y.find("\r");
                }
                lv = y.find("\n");
                while (lv < string::npos) {
                    y.replace(lv, 1, "");
                    lv = y.find("\n");
                }
            }
            char *name_out = NULL;
            if (l.size() > 0) {
                name_out = (char *) l.c_str();
            }
            if (callMenu(name_out, (char *) y.c_str(), t)) {
                return false;
            }
            *incommand = string();
            incommand->append(t);             //t may have changed if we got this far
        }
    }
    //}}}
    return fexecute(incommand, isDown, sock_in);
}

//}}}
//broken up into two execute functions
//the one below is the real execute, the one above uses the menusystem
//it's broken up so the menusystem can call fexecute themself at the right
//time
//Main Execute Function {{{

bool commandI::fexecute(string *incommand, bool isDown, int sock_in) {
    size_t ls, y;
    bool breaker = false;
//************
    while (breaker == false) {
        ls = incommand->find(" ");
        if (ls != 0) {
            breaker = true;
        } else {
            incommand->replace(ls, 1, "");
        }
    }
    for (y = incommand->find("\r\n"); y != string::npos; y = incommand->find("\r\n", y + 1)) {
        incommand->replace(y, 2, "");
    }
    for (y = incommand->find("  "); y != string::npos; y = incommand->find("  ", y + 1)) {
        incommand->replace(y, 1, "");
    }
    breaker = false;     //reset our exit bool
    //************ try to replace erase leading space if there is one
    //eg, someone types: " do_something" instead of "do_something"
    while (breaker == false) {
        ls = incommand->find(" ");
        if (ls != 0) {
            breaker = true;
        } else {
            incommand->erase(ls, 1);
        }
    }
//Print back what the user typed.. {{{
//.. Sometimes people believe they typed python print "hello world\n"
//(and saw what they typed when they typed it)
//but may have actually typed oython print "hello world\n"
//and don't want to admit it, so they blame the system.
//So the system must sometimes politely tell the user what they typed
    {
        bool printit = false;
        if (menumode) {
//if(menu_in->selected) {
//if(menu_in->iselected->inputbit || menu_in->iselected->inputbit2) printit = true;
//}
        } else if (console) {
            printit = true;
        }
        if (printit) {
            string webout;
            webout.append(incommand->c_str());
            webout.append("\n\r");
            conoutf(webout);
        }
    }
//}}}
    //replace \r\n with a space {{{
    for (y = incommand->find("\r\n"); y != string::npos; y = incommand->find("\r\n", y + 1)) {
        incommand->replace(y, 2, " ");
    }
    //}}}
    //remove multiple spaces {{{
    for (y = incommand->find("  "); y != string::npos; y = incommand->find("  ", y + 1)) {
        incommand->replace(y, 1, "");
    }
    //}}}
    //{{{ ! to the last command typed
    {
        size_t x = incommand->find("!");
        if (x == 0) {
            incommand->replace(0, 1, lastcommand);
        }
        //}}}
        //{{{ : to python
        x = incommand->find(":");
        if (x == 0) {
            incommand->replace(0, 1, "python ");
        }
    }
    //}}}

    breaker = false;     //reset our exit bool

    //done with formatting
    //now make what our vector<string> {{{
    vector<string> strvec;     //to replace newincommand
    //to reduce data replication by one;
    {
        string::const_iterator scroller = incommand->begin();
        size_t last = 0, next = 0;
        bool quote = false;
        bool escape = false;
        next = incommand->find(" ");
        for (next = incommand->find("\"\"", 0);
                (next = incommand->find("\"\"", last), (last != string::npos));
                last = (next != string::npos) ? next + 1 : string::npos) {
            if (next < string::npos) {
                incommand->replace(next, 2, "\" \"");
            }
        }
        //replace "" with " "
        string starter("");
        strvec.push_back(starter);
        for (scroller = incommand->begin(); scroller < incommand->end(); scroller++) {
            if (*scroller == '\\') {
                escape = true;
                continue;
            }
            if (escape) {
                if (*scroller == '\"') {
                    strvec[strvec.size() - 1] += *scroller;
                }
                continue;
            }
            if (*scroller == '\"') {
                if (quote) {
                    quote = false;
                } else {
                    quote = true;
                }
                continue;
            }
            if (*scroller == ' ' && !quote) {
                strvec.push_back(starter);
                continue;
            }
            strvec[strvec.size() - 1] += *scroller;
        }
    }
    //}}}
    {
        //if the last argument is a space, erase it. {{{
        vector<string>::iterator iter = strvec.end();
        iter--;
        if ((*(iter)).compare(" ") == 0) {
            strvec.erase(iter);
        }
        //}}}
    }
    try {
        coms &theCommand = *findCommand((char *) strvec[0].c_str(), sock_in);
//Now, we try to replace what was typed with the name returned by findCommand {{{
//to autocomplete words (EX: translate gos into gossip so the gossip
//command only has to find it's access name and not all possible
//methods of accessing it.)
        if (theCommand.Name.compare("dummy") != 0) {
            size_t x = incommand->find_first_of(strvec[0]);
            if (x != string::npos) {
                strvec[0].erase();
                strvec[0].append(theCommand.Name);
            }
//}}}
            lastcommand.erase();
            lastcommand.append(*incommand);                                //set the
            //last command entered - use ! to trigger
        }
        //Try to execute now {{{
        try {
            //maybe if/else if would be more efficient, if this ever
            //gets really large.
            theCommand.functor->Call(strvec, sock_in, &isDown);
            //try to catch any errors that occured while executing
        }
        catch (const char *in) {
            string l;
            l.append(in);
            conoutf(l);             //print the error to the console
        }
        catch (const exception &e) {
            string l;
            l.append("Command processor: Exception occured: ");
            l.append(e.what());
            l.append("\n\r");
            cout << l;
            conoutf(l);
        }
        catch (...) {
            string y;
            y.append(
                    "Command processor: exception occurered: Unknown, most likely cause: Wrong Arg_type arguement sent with addCommand.\n\r");
            cout << y;
            conoutf(y);
        }

        //}}}
    }
    catch (const char *in) {
        //catch findCommand error
        cout << in;
    }
    return true;
}

//}}}

string commandI::display(string &in) {
    //If the menusystem has a value to display, eg:
    //Editing User
    //1) Change Username - Current Name: XXX
    //and XXX is replaced with a value here
    //basically, call: string.replace(xxx,3, display("uname") )
    //then display does:
    //if(in.compare(uname) == 0) return current_mob_editing.Name;
    //The value to pass to display is set when creating a menuitem
    string f;
    f.append("FAKE");
    return f;
}

//{{{ menusystem
/* ***************************************
*   An example of how the menusystem is used:
*   (the very first menu when a player logs onto the ANT-Engine http://daggerfall.dynu.com:5555/player1/index.html OR telnet://daggerfall.dynu.com:5555 )
*
*    {
*    menu *m = new menu("newuser", "Welcome to the <GREEN>ANT<NORM> engine", "\r\n");
*    m->autoselect = true; //automatically select a menuitem, MUST BE SET
*    m->noescape = true; //no escaping this menu except by forcing it
*    addMenu(m); //add the menu to the command processor
*    mItem *mi = new mItem; //make a new menuitem
*    mi->Name.append(" "); //argument to access menu  //must have a name
*    mi->action.append("UNAME "); //adds this to the function 2 call as the argument
*    mi->action.append(seccode); //add the security code.
*    mi->display.append(" "); // menu's display name
*    mi->func2call.append("loginfunc"); //function 2 call
*    mi->inputbit = true; // set single-line input mode
*    mi->selectstring.append("Enter a username"); //string to display when this menuitem is selected
*    addMenuItem(mi);// add the menuitem to the command processor, by default
*                                        // added to the last menu added, can be overredden by passing
*                                        // a menu * pointer as the second argument, eg:
*                                        // addMenuItem(mi, m);
*    m->aselect = mi; //this is the menu item to automatically select
*    }
*
*************************************** */
//add a menu {{{
bool commandI::addMenu(menu *menu_in) {
    menus.push_back(menu_in);
    lastmenuadded = menu_in;
    return true;
}

//}}}
//{{{ display menu function
string commandI::displaymenu() {
    if (menumode) {
        ostringstream ps;
        ps << menu_in->Display << "\n";
        for (vector<mItem *>::iterator iter = menu_in->items.begin();
                iter < menu_in->items.end(); iter++) {
            ps << (*(iter))->Name << " " << (*(iter))->display;
            if ((*(iter))->predisplay.size() > 0) {
                ps << " " << display((*(iter))->predisplay);
            }
            ps << "\n";
        }
        string buf;
        buf.append(ps.str());
        if (menu_in->autoselect == true) {
            if (menu_in->selected == true) {
                buf.append(menu_in->iselected->selectstring);
                buf.append(": ");
            }
        } else {
            if (!menu_in->noescape) {
                buf.append("Use: ");
                if (menu_in->escape.compare("\r\n") == 0) {
                    buf.append("enter");
                } else {
                    buf.append(menu_in->escape);
                }
                buf.append(" to quit: \n");
            } else {
                buf.append("Enter your selection: \n");
            }
        }
        return buf;
//conoutf(buf);
    }
    string buf;
    buf.append("Error, not in menumode!");
    return buf;
}

//}}}
//menuitem to be appended to the last menu appended, or an existing menu if {{{
//the menu2use is specified
bool commandI::addMenuItem(mItem *mi, menu *menuin) {
    menu *menu2use;
    if (menuin == NULL) {
        menu2use = lastmenuadded;
    } else {
        menu2use = menu_in;
    }
    //if the command isn't found it will return dummy or prompt.
    for (vector<menu *>::iterator iter = menus.begin(); iter < menus.end(); iter++) {
        if (menu2use == (*(iter))) {
            menu2use->items.push_back(mi);             //doh! :)
            return true;
        }
    }
    return false;
}

//}}}
//call a menu with arguements {{{
bool commandI::callMenu(char *name_in, char *args_in, string &d) {
    //if there is a menu operation return true;
    string name;
    if (name_in != NULL) {
        name.append(name_in);
    }
//bool freturnfalse = false; //force return false
    //{{{ if the name_in is the menu_in's escape charactor
    //change the menu_in to the last menu on menustack if there is
    //one, and pop the stack. If there is no menustack, set menumode
    //off.
    if (menumode) {
        if (!menu_in->selected) {
            if (!menu_in->noescape) {
                if (name.compare(menu_in->escape) == 0) {
                    if (menustack.size() > 0) {
                        vector<menu *>::iterator iter = menustack.end();
                        iter--;
                        menu_in = (*(iter));
                        menustack.pop_back();
                        //return true;
                    } else {
                        menu_in = NULL;
                        menumode = false;
                        return true;
                    }
                }
            }
        }
    }
    //}}}
    if (menumode) {
        if (menu_in->selected) {
            //Input mode 1  {{{
            if (menu_in->iselected->inputbit == true && menu_in->iselected->inputbit2 == false) {
                menu_in->selected = false;
                string arg;
                arg.append(menu_in->iselected->action);
                string funcn;
                funcn.append(menu_in->iselected->func2call);
                string dreplace;
                dreplace.append(d);
                d.erase();
                d.append(funcn);
                d.append(" ");
                d.append(arg);
                d.append(" ");
                d.append(dreplace);
                //setMenus {{{
                if (funcn.compare("setMenu") == 0) {
                    string l;
                    l.append(setMenu(arg));
                    conoutf(l);
                    return true;
                }
                //}}}
                size_t ylast = 0, xasd = 0;
                //login function {{{
                if (funcn.compare("loginfunc") == 0) {
                    vector<string *> d_out;
                    d.append(" ");
                    for (size_t x = d.find("\r\n"); x < string::npos; x = d.find("\r\n", x + 3)) {
                        d.replace(x, 1, " \r\n");
                    }
                    for (size_t iter = 0; iter < d.size(); iter++) {
                        if (d[iter] == 32) {
                            string *xs = new string();
                            xs->append(d.substr(ylast, xasd - ylast));
                            ylast = xasd;
                            d_out.push_back(xs);
                        }
                        xasd++;
                    }
//loginfunc(&d_out); //login function
                    vector<string *>::iterator itera = d_out.begin();
                    while (d_out.size() > 0) {
                        string *s = (*(itera));
                        delete s;
                        d_out.erase(itera);
                        itera = d_out.begin();
                    }
                    return true;
                }
                //}}}
                //autoreprint {{{
                if (menu_in->iselected->autoreprint == true) {
                    fexecute(&d, true, 0);
                    string x;
                    x.append(displaymenu());
                    conoutf(x);
                    return true;
                }
                //}}}
                return false;
            }
            //}}}
            //input mode 2 {{{
            if (menu_in->iselected->inputbit == false && menu_in->iselected->inputbit2 == true) {
                //wait until we find an escape seqence alone {{{
                if (name.compare(menu_in->escape) == 0) {
                    menu_in->selected = false;
                    string arg;
                    arg.append(menu_in->iselected->action);
                    string funcn;
                    funcn.append(menu_in->iselected->func2call);
                    d.erase();
                    d.append(funcn);
                    d.append(" ");
                    d.append(arg);
                    d.append(" ");
                    {
                        size_t l = 0;
                        bool y = false;
                        for (size_t x = menu_in->iselected->menubuf.find("\r\n");
                                x < string::npos;
                                x = menu_in->iselected->menubuf.find("\r\n", x + 1)) {
                            menu_in->iselected->menubuf.replace(x, 2, "<BR>");
                            l = x;
                            y = true;
                        }
                        if (y) {
                            menu_in->iselected
                                    ->menubuf
                                    .replace(l, 4, "");
                        }                              //replace the last <BR>
                    }
                    d.append(menu_in->iselected->menubuf);
                    d.append(" ");
                    menu_in->iselected->menubuf.erase();
                    if (funcn.compare("setMenu") == 0) {
                        string buf;
                        buf.append(setMenu(arg));
                        conoutf(buf);
                        return true;
                    }
                    if (funcn.compare("loginfunc") == 0) {
                        vector<string *> d_out;
                        d.append(" ");
                        for (size_t x = d.find("\r\n"); x < string::npos; x = d.find("\r\n", x + 1)) {
                            d.replace(x, 2, "<BR>");
                        }
                        size_t ylast = 0, xasd = 0;
                        for (size_t iter = 0; iter < d.size(); iter++) {
                            if (d[iter] == 32) {
                                string *xs = new string();
                                xs->append(d.substr(ylast, xasd - ylast));
                                ylast = xasd;
                                d_out.push_back(xs);
                            }
                            xasd++;
                        }
//loginfunc(&d_out); //login function
                        vector<string *>::iterator itera = d_out.begin();
                        while (d_out.size() > 0) {
                            string *s = (*(itera));
                            delete s;
                            d_out.erase(itera);
                            itera = d_out.begin();
                        }
                        return true;
                    }
                    if (menu_in->iselected->autoreprint == true) {
                        fexecute(&d, true, 0);
                        string x;
                        x.append(displaymenu());
                        conoutf(x);
                        return true;
                    }
                    return false;
                    //}}}
                    //or we append the input to the buffer  {{{
                } else {
                    menu_in->iselected->menubuf.append(d);
                    //}}}
                }
                return true;
            }
            //}}}
        }
        //if we don't have anything selected, select one.. {{{
        if (!menu_in->selected) {
            for (vector<mItem *>::iterator iter = menu_in->items.begin();
                    iter < menu_in->items.end(); iter++) {
                if ((*(iter))->Name.compare(name) == 0) {
                    menu_in->selected = true;
                    menu_in->iselected = (*(iter));
//if(menu_in->iselected->predisplay.size() > 0) {
//display(menu_in->iselected->predisplay);
//}
                    if (menu_in->iselected->inputbit2) {
                        string buf;
                        buf.append(menu_in->iselected->selectstring);
                        buf.append("\n\r");
                        buf.append("Use: ");
                        if (menu_in->escape.compare("\r\n") == 0) {
                            buf.append("enter");
                        } else {
                            buf.append(menu_in->escape);
                        }
                        buf.append(" to confirm: ");

                        conoutf(buf);
                    } else if (menu_in->iselected->inputbit) {
                        string buf;
                        buf.append(menu_in->iselected->selectstring);
                        buf.append(": ");
                        conoutf(buf);
                    }
                }
            }
            if (menu_in->selected) {
                if (!menu_in->iselected->inputbit && !menu_in->iselected->inputbit2) {
                    menu_in->selected = false;
                    string arg;
                    arg.append(menu_in->iselected->action);
                    string funcn;
                    funcn.append(menu_in->iselected->func2call);
                    string dreplace;
                    dreplace.append(d);
                    d = string();
                    d.append(funcn);
                    d.append(" ");
                    d.append(arg);
                    d.append(" ");
                    d.append(dreplace);
                    if (funcn.compare("setMenu") == 0) {
                        string l;
                        l.append(setMenu(arg));
                        conoutf(l);
                        return true;
                    }
                    return false;
                }
                return true;
            } else if (menu_in->defaultInput) {
                menu_in->selected = true;
                menu_in->iselected = menu_in->idefaultInput;
                execute(&d, true, 0);
                return true;
            }
        }
        //}}}
    }
    if (menumode && !menu_in->selected) {
        //we're in a menu but don't have anything selected {{{
        string y;
        y.append(displaymenu());
        conoutf(y);
        return true;
    }
    //}}}
    return false;
}

//}}}

//set a menu {{{

string commandI::setMenu(string name_in) {
    string name;
    name.append(name_in);
    if (name[0] == 32) {
        name.replace(0, 1, "");
    }
    for (vector<menu *>::iterator iter = menus.begin();
            iter < menus.end(); iter++) {
        if ((*(iter))->Name.compare(name) == 0) {
            if (!menumode) {
                menumode = true;
            } else {
                menustack.push_back(menu_in);
            }
            menu_in = (*(iter));
            menu_in->selected = false;
            if (menu_in->autoselect == true) {
                menu_in->selected = true;
                menu_in->iselected = menu_in->aselect;
            }
            iter = menus.end();
        }
    }
    return displaymenu();
}

//}}}

void commandI::breakmenu() {
    while (menustack.size() > 0) {
        menustack.pop_back();
    }
    menu_in = NULL;
    menumode = false;
}

//}}}

commandI *CommandInterpretor = NULL;

//{{{ Python object

RegisterPythonWithCommandInterpreter::RegisterPythonWithCommandInterpreter(commandI *addTo) {
    Functor<RegisterPythonWithCommandInterpreter> *l = new Functor<RegisterPythonWithCommandInterpreter>
            (this, &RegisterPythonWithCommandInterpreter::runPy);
    addTo->addCommand(l, "python");
}

//run a python string
void RegisterPythonWithCommandInterpreter::runPy(string &argsin) {
    string pyRunString;
    pyRunString.append(argsin);     //append the arguments in to the string to run
    size_t x = pyRunString.find("python ");     //strip out the name of the command
    //and the first space
    if (x == 0) {
        pyRunString.replace(x, 7, "");
    }          //replace here
//this method was copied from somewhere else in the vegastrike source
    //now replace <BR> with \r\n
    {
        size_t x = pyRunString.find("<BR>");
        while (x != string::npos) {
            pyRunString.replace(x, 4, "\r\n");
            x = pyRunString.find("<BR>");
        }
    }

    VegaPyRunString(pyRunString);
}

//}}};

/*---------------------------------------------------------------------------*/
/*!
 *  New input wrapper for new Command Processor SDL version
 *  \author  Rogue
 *  \date    Created:  2005-8-16
 */

//if(!keypress(event.key.keysym.sym, event.key.state==SDL_PRESSED, event.key.keysym.unicode))
void commandI::keypress(int code, int modifiers, bool isDown, int x, int y) {
    if (CommandInterpretor && CommandInterpretor->console) {
        if (code == WSK_ESCAPE) {
            CommandInterpretor->console = false;
            restore_main_loop();
//SDL_EnableUNICODE(false);
            return;
        }
        if (code == WSK_RETURN && isDown) {
            string commandBuf = CommandInterpretor->getcurcommand();
            commandBuf.append("\r\n");
            CommandInterpretor->execute(&commandBuf, isDown, 0);             //execute console on enter
            //don't return so the return get's processed by
            //CommandInterpretor->ConsoleKeyboardI, so it can clear the
            //command buffer
        }
        CommandInterpretor->ConsoleKeyboardI(code, isDown);
        return;
    } else {
        restore_main_loop();
        return;
    }
}

/* Proposed (Would need a couple commands inserted into the command processor
 *       // one to read a keymap file and one to re-map a single key
 *       // (and the keymap file would have to be read at startup)
 *       // struct keym { int code; char * name; char * action; }; or so
 *       vector<KeyMapObject>::iterator iter = keyMapVector.begin();
 *       while(iter < keyMapVector.end()) {
 *           keym *tester = &(*(iter));
 *           if(tester->code == code){
 *           // lookup in keymap and execute
 *           if(tester->action)
 *                   execCommand(tester->action, isdown);
 *               return true;
 *           }
 *           iter++;
 *       }
 *   }
 *
 *
 *
 */

/* ***************************************************************
*   Possible Optimizations:
*
*        Optimizations discussed here arn't the tiny little save 2 or 3 cpu ops by reforming a for loop.
*        These optimizations may make an impact on very very slow machines, or
*        when ram is limited for copying objects, or when certain copies or types
*        arn't ever needed.
*
*        Possible optimization for findCommand (small optimization, less game-time copying overhead (after boot, while playing))
*                copy a coms object when adding a command to the real command vector, (as it is now in addCommand)
*
*                return a reference to the coms object from findCommand, to avoid
*                copying every time a key is pressed or a command is entered.
*                (change      coms findCommand   to    coms *findCommand)
*
*
*
*
*         Possible optimization for the main execute function ( medium optimization, less unneeded allocated variables in the execute function when not needed)
*                Move findCommand higher up, before the string vector and 1str array are
*                built, and build those depending on the argument type, to avoid
*                excessive string copying when it's not needed, such as when not in
*                console mode but in game mode, when 1 bool is enough to tell
*                the function being called wether the key is pressed or not.
*        - This might make it a little more difficult to read the execute function
*
*
*************************************************************** */

namespace ConsoleKeys {
void BringConsole(const KBData &, KBSTATE newState) {
    //this way, keyboard state stays synchronized
    if (newState == RELEASE) {
        if (CommandInterpretor) {
            winsys_set_keyboard_func((winsys_keyboard_func_t) &commandI::keypress);
            CommandInterpretor->console = true;
        }
    }
}
}

//footer, leave at bottom
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */

