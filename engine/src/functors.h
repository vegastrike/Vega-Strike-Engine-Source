/*
 * functors.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

// NO HEADER GUARD

#include <cstdlib>

#ifdef FUNCTORS_INC
#else //FUNCTORS_INC
#define FUNCTORS_INC 1
class Attributes {
public:
    Attributes() {
        hidden = false;
        webbcmd = false;
        immcmd = false;
    }

    bool hidden;     //hidden
    bool webbcmd;     //web command
    bool immcmd;     //immortal command
    int type;
//nothing returns yet anyway, strings may be the most useful?
    class returnType {
    public:
        returnType() {
        }

        returnType(const returnType &in) {
            if (in.s.size() > 0) {
                s.append(in.s);
            }
        }

        std::string s;
    };
    returnType m_return;
};

class TFunctor {
public:
    Attributes attribs;

    virtual ~TFunctor() {
    }

    virtual void *Call(std::vector<std::string> &d, int &sock_in, bool *isDown) = 0;
};
template<class TClass>
class Functor : public TFunctor {
//To add a new callback method, add a new fpt type here,
//set it to NULL in nullify, then add it to the list
//of if/else in the main Call method.
private:
    void (TClass::*fpt1)();
    void (TClass::*fpt2)(std::string &);
    void (TClass::*fpt3)(const char *);
    void (TClass::*fpt4)(const char *array[]);
    void (TClass::*fpt5)(const char *, const char *);
    void (TClass::*fpt6)(bool *);
    void (TClass::*fpt7)(int);
    void (TClass::*fpt8)(char);
    void (TClass::*fpt9)(std::vector<std::string *> *d);
    void (TClass::*fpt10)(std::vector<std::string *> *d, int &sock_in);
    void (TClass::*fpt11)(std::string &, int &);
    void (TClass::*fpt12)(std::vector<std::string *> *, int &, bool);
    TClass *pt2Object;             //pointer to object
public:
//New singularlized call method {{{:
    virtual void *Call(std::vector<std::string> &d, int &sock_in, bool *isDown) {
        //Comments {{{
        //ok, d[0] == command typed
        //d[1] == arg1
        //d[2] == arg2, etc.
        //sometimes socket can be ignored
        //}}}
        if (fpt1 != NULL) {
            //fpt1() no args {{{
            (*pt2Object.*fpt1)();
        }
            //}}}
        else if (fpt2 != NULL) {
            //fpt2(std::string &)  {{{
            std::string a;
            unsigned int x;
            for (x = 0; x < d.size(); x++) {
                a.append(d[x]);
                a.append(" ");
            }
            (*pt2Object.*fpt2)(a);
            //}}}
        } else if (fpt3 != NULL) {
            //fpt3(const char *); {{{
            if (d.size() >= 2) {
                (*pt2Object.*fpt3)(d[1].c_str());
            } else {
                (*pt2Object.*fpt3)((const char *) NULL);
            }
            //}}}
        } else if (fpt4 != NULL) {
            //(const char *array[]); {{{
            std::vector<const char *> buf;
            for (unsigned int c = 0; c < d.size();) {
                buf.push_back(d[c].c_str());
                c++;
                if (!(c < d.size())) {
                    buf.push_back(" ");
                }
            }
            (*pt2Object.*fpt4)(&buf[0]);
            //}}}
        } else if (fpt5 != NULL) {
            //(const char *, const char *); {{{
            if (d.size() < 2) {
                (*pt2Object.*fpt5)((const char *) NULL, (const char *) NULL);
            } else if (d.size() < 3) {
                (*pt2Object.*fpt5)(d[1].c_str(), (const char *) NULL);
            } else {
                (*pt2Object.*fpt5)(d[1].c_str(), d[2].c_str());
            }
            //}}}
        } else if (fpt6 != NULL) {
            //(bool *); {{{
            (*pt2Object.*fpt6)(isDown);
        }
            //}}}
        else if (fpt7 != NULL) {
            //(int) {{{
            if (d.size() < 2) {
                (*pt2Object.*fpt7)(0);
            } else {
                (*pt2Object.*fpt7)(atoi(d[1].c_str()));
            }
            //}}}
        } else if (fpt8 != NULL) {
            //(char) {{{
            if (d.size() < 2) {
                char err = 0;
                (*pt2Object.*fpt8)(err);
            } else {
                (*pt2Object.*fpt8)(d[1][0]);
                //}}}
            }
        } else if (fpt9 != NULL) {
            //(std::vector<std::string *> *d) {{{
            std::vector<std::string *> dup;
            std::vector<std::string>::iterator ptr = d.begin();
            while (ptr < d.end()) {
                dup.push_back(&(*(ptr)));
                ptr++;
            }
            (*pt2Object.*fpt9)(&dup);
            //}}}
        } else if (fpt10 != NULL) {
            //(std::vector<std::string *> *d, int) {{{
            std::vector<std::string *> dup;
            std::vector<std::string>::iterator ptr = d.begin();
            while (ptr < d.end()) {
                dup.push_back(&(*(ptr)));
                ptr++;
            }
            (*pt2Object.*fpt10)(&dup, sock_in);
            //}}}
        } else if (fpt11 != NULL) {
            //(std::string &, int&); {{{
            std::string a;
            unsigned int x;
            for (x = 0; x < d.size(); x++) {
                a.append(d[x]);
                a.append(" ");
            }
            (*pt2Object.*fpt11)(a, sock_in);
            //}}}
        } else if (fpt12 != NULL) {
            //(std::vector<std::string *> *, int &, bool); // {{{
            std::vector<std::string *> dup;
            std::vector<std::string>::iterator ptr = d.begin();
            while (ptr < d.end()) {
                dup.push_back(&(*(ptr)));
                ptr++;
            }
            (*pt2Object.*fpt12)(&dup, sock_in, false);
        }                 //}}}
        return &(attribs.m_return);

        return NULL;
    }             //}}}
    void nullify() {
        //Set all the fpt's to null {{{
        fpt1 = NULL;
        fpt2 = NULL;
        fpt3 = NULL;
        fpt4 = NULL;
        fpt5 = NULL;
        fpt6 = NULL;
        fpt7 = NULL;
        fpt8 = NULL;
        fpt9 = NULL;
        fpt10 = NULL;
        fpt11 = NULL;
        fpt12 = NULL;
    }              //Nullify }}}
    //Constructors, call nullify, set pt2object and function pointer {{{
    Functor(TClass *_pt2Object, void(TClass::*_fpt)()) {
        nullify();
        pt2Object = _pt2Object;
        fpt1 = _fpt;
    }

//1 std::string
    Functor(TClass *_pt2Object, void(TClass::*_fpt)(std::string &)) {
        nullify();
        pt2Object = _pt2Object;
        fpt2 = _fpt;
    }

//1 c string
    Functor(TClass *_pt2Object, void(TClass::*_fpt)(const char *)) {
        nullify();
        pt2Object = _pt2Object;
        fpt3 = _fpt;
    }

    Functor(TClass *_pt2Object, void(TClass::*_fpt)(const char *array[])) {
        nullify();
        pt2Object = _pt2Object;
        fpt4 = _fpt;
    }

//2 c strings
    Functor(TClass *_Obj, void(TClass::*_fpt)(const char *, const char *)) {
        nullify();
        pt2Object = _Obj;
        fpt5 = _fpt;
    }

//1 bool
    Functor(TClass *_Obj, void(TClass::*_fpt)(bool *)) {
        nullify();
        pt2Object = _Obj;
        fpt6 = _fpt;
    }

    Functor(TClass *_Obj, void(TClass::*_fpt)(int)) {
        nullify();
        pt2Object = _Obj;
        fpt7 = _fpt;
    }

    Functor(TClass *_Obj, void(TClass::*_fpt)(char)) {
        nullify();
        pt2Object = _Obj;
        fpt8 = _fpt;
    }

    Functor(TClass *_Obj, void(TClass::*_fpt)(std::vector<std::string *> *d)) {
        nullify();
        pt2Object = _Obj, fpt9 = _fpt;
    }

    Functor(TClass *_Obj, void(TClass::*_fpt)(std::vector<std::string *> *d, int &)) {
        nullify();
        pt2Object = _Obj, fpt10 = _fpt;
    }

    Functor(TClass *_pt2Object, void(TClass::*_fpt)(std::string &, int &)) {
        nullify();
        pt2Object = _pt2Object;
        fpt11 = _fpt;
    }

    Functor(TClass *_Obj, void(TClass::*_fpt)(std::vector<std::string *> *d, int &, bool)) {
        nullify();
        pt2Object = _Obj, fpt12 = _fpt;
    }
//}}}

    virtual ~Functor() {
    }
};

#endif //FUNCTORS_INC

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
