/*
 * base_maker.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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


/* Base Maker: A program to create Vega Strike bases using opengl.
 * ...
 */
#define BASE_MAKER
#include <cstring>
#include <vector>
#include <queue>
#include <iostream>
#include "sprite.h"
#include <stdio.h>
#include "base_maker_texture.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "gfx/hud.h"
#include "vs_globals.h"

#include <time.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h> //for mkdir
#include <sys/types.h>

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#include <glut.h>
#include <glext.h>

/*
 * Globals
 */
game_data_t g_game;
ForceFeedback *forcefeedback = NULL;
VegaConfig *vs_config = NULL;
bool cleanexit = false;
bool run_only_player_starsystem = true;
NetClient *Network = NULL;
NetServer *VSServer = NULL;
FILE *fpread = NULL;
float simulation_atom_var = (float) (1.0 / 10.0);
Mission *mission = NULL;
double benchmark = -1.0;
bool STATIC_VARS_DESTROYED = false;
const char *mission_key = "unit_to_dock_with";

/* for speed test */
int loop_count = 0;
double avg_loop = 0;
int nb_checks = 1;
double last_check = 1;
double cur_check = 1;

/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
#ifndef SPRITE_H
#define SPRITE_H

#include "gfx/quaternion.h"

class Texture;

class VSSprite
{
    float    xcenter;
    float    ycenter;
    float    widtho2;
    float    heighto2;
    float    maxs, maxt;
    float    rotation;
    Texture *surface;
public:
	VSSprite( const char *file, enum FILTER texturefilter = BILINEAR, GFXBOOL force = GFXFALSE );
    ~VSSprite();
    bool LoadSuccess()
    {
        return surface != NULL;
    }
    void Draw();
    void DrawHere( Vector &ll, Vector &lr, Vector &ur, Vector &ul );
    void Rotate( const float &rad )
    {
        rotation += rad;
    }
    void SetST( const float s, const float t );
    void SetPosition( const float &x1, const float &y1 );
    void GetPosition( float &x1, float &y1 );
    void SetSize( float s1, float s2 );
    void GetSize( float &x1, float &y1 );
    void SetRotation( const float &rot );
    void GetRotation( float &rot );
    void ReadTexture( FILE *f );
//float &Rotation(){return rotation;};
    Texture * getTexture()
    {
        return surface;
    }
};

#endif

/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <math.h>

#ifndef M_PI_2
# define M_PI_2 (1.57079632679489661923)
#endif

static float *mview = NULL;
static int    bogus_int; //added by chuck_starchaser to squash ignored returns warnings.

VSSprite::VSSprite( const char *file, enum FILTER texturefilter, GFXBOOL force )
{
    xcenter   = 0;
    ycenter   = 0;
    widtho2   = 0;
    heighto2  = 0;
    rotation  = 0;
    surface   = NULL;
    maxs      = maxt = 0;
    bogus_int = chdir( "sprites" );
    FILE *f;
    if (file[0] != '\0')
        f = fopen( file, "rt" );
    if (f) {
        char texture[64]  = {0};
        char texturea[64] = {0};
        bogus_int = fscanf( f, "%63s %63s", texture, texturea );
        bogus_int = fscanf( f, "%f %f", &widtho2, &heighto2 );
        bogus_int = fscanf( f, "%f %f", &xcenter, &ycenter );

        widtho2  /= 2;
        heighto2 /= -2;
        surface   = NULL;
        if (texturea[0] == '0')
            surface = new Texture( texture );
        else
            surface = new Texture( texture );
        if (!surface) {
            delete surface;
            surface = NULL;
        }
        //Finally close file
        fclose( f );
    } else {
        widtho2 = heighto2 = 0;
        xcenter = ycenter = 0;
    }
    bogus_int = chdir( ".." );
}

using std::cout;
using std::cerr;
using std::endl;

void VSSprite::ReadTexture( FILE *f )
{
    if (!f) {
        widtho2 = heighto2 = 0;
        xcenter = ycenter = 0;
        cerr<<"VSSprite::ReadTexture error : VSFile not valid"<<endl;
        return;
    }
    surface = new Texture( f );
}

VSSprite::~VSSprite()
{
    if (surface != nullptr) {
        delete surface;
        surface = nullptr;
    }
}

void VSSprite::SetST( const float s, const float t )
{
    maxs = s;
    maxt = t;
}
void VSSprite::DrawHere( Vector &ll, Vector &lr, Vector &ur, Vector &ul )
{
    if (rotation) {
        const float cw   = widtho2*cos( rotation );
        const float sw   = widtho2*sin( rotation );
        const float ch   = heighto2*cos( M_PI_2+rotation );
        const float sh   = heighto2*sin( M_PI_2+rotation );
        const float wnew = cw+ch;
        const float hnew = sw+sh;
        ll = Vector( xcenter-wnew, ycenter+hnew, 0.00f );
        lr = Vector( xcenter+wnew, ycenter+hnew, 0.00f );
        ur = Vector( xcenter+wnew, ycenter-hnew, 0.00f );
        ul = Vector( xcenter-wnew, ycenter-hnew, 0.00f );
    } else {
        ll = Vector( xcenter-widtho2, ycenter+heighto2, 0.00f );
        lr = Vector( xcenter+widtho2, ycenter+heighto2, 0.00f );
        ur = Vector( xcenter+widtho2, ycenter-heighto2, 0.00f );
        ul = Vector( xcenter-widtho2, ycenter-heighto2, 0.00f );
    }
}
void VSSprite::Draw()
{
    if (surface) {
        //don't do anything if no surface
        glDisable( GL_CULL_FACE );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_TEXTURE_2D );
        surface->bind();
        glBegin( GL_QUADS );
        Vector ll, lr, ur, ul;
        DrawHere( ll, lr, ur, ul );
        glTexCoord2f( maxs, 1 );
        glVertex3f( ll.i, ll.j, ll.k );
        glTexCoord2f( 1, 1 );
        glVertex3f( lr.i, lr.j, lr.k );
        glTexCoord2f( 1, maxt );
        glVertex3f( ur.i, ur.j, ur.k );
        glTexCoord2f( maxs, maxt );
        glVertex3f( ul.i, ul.j, ul.k );
        glEnd();
        glDisable( GL_TEXTURE_2D );
        glEnable( GL_CULL_FACE );
    }
}

void VSSprite::SetPosition( const float &x1, const float &y1 )
{
    xcenter = x1;
    ycenter = y1;
}

void VSSprite::GetPosition( float &x1, float &y1 )
{
    x1 = xcenter;
    y1 = ycenter;
}
void VSSprite::SetSize( float x1, float y1 )
{
    widtho2  = x1/2;
    heighto2 = y1/2;
}
void VSSprite::GetSize( float &x1, float &y1 )
{
    x1 = widtho2*2;
    y1 = heighto2*2;
}

void VSSprite::SetRotation( const float &rot )
{
    rotation = rot;
}

void VSSprite::GetRotation( float &rot )
{
    rot = rotation;
}

*/

void VSExit(int code) {
    STATIC_VARS_DESTROYED = true;
    exit(code);
}

#include <ctype.h>
#include "gfx/hud.h"

TextPlane::TextPlane(const GFXColor &c, const GFXColor &bgcol) {
    col = c;
    this->bgcol = bgcol;
    myDims.i = 2;
    myDims.j = -2;
    myFontMetrics.Set(.06, .08, 0);
    SetPos(0, 0);
}

TextPlane::~TextPlane() {
}

int TextPlane::Draw(int offset) {
    return Draw(myText, offset, true);
}

static char *CreateLists() {
    static char lists[256] = {0};
    void *fnt = g_game.x_resolution >= 800 ? GLUT_BITMAP_HELVETICA_12 : GLUT_BITMAP_HELVETICA_10;
    static bool use_bit = false;
    bool use_display_lists = false;
    /*
     *  if (use_display_lists) {
     *  for (unsigned char i=32;i<128;i++){
     *   lists[i]= GFXCreateList();
     *   if (use_bit)
     *     glutBitmapCharacter (fnt,i);
     *   else
     *     glutStrokeCharacter (GLUT_STROKE_ROMAN,i);
     *   if (!GFXEndList ()) {
     *     lists[i]=0;
     *   }
     *  }
     *  }
     */
    return lists;
}

static unsigned char HexToChar(char a) {
    if (a >= '0' && a <= '9') {
        return a - '0';
    } else if (a >= 'a' && a <= 'f') {
        return 10 + a - 'a';
    } else if (a >= 'A' && a <= 'F') {
        return 10 + a - 'A';
    }
    return 0;
}

static unsigned char TwoCharToByte(char a, char b) {
    return 16 * HexToChar(a) + HexToChar(b);
}

static float TwoCharToFloat(char a, char b) {
    return TwoCharToByte(a, b) / 255.;
}

void DrawSquare(float left, float right, float top, float bot) {
    glBegin(GFXQUAD);
    glVertex3f(left, top, 0);
    glVertex3f(left, bot, 0);
    glVertex3f(right, bot, 0);
    glVertex3f(right, top, 0);
    glVertex3f(right, top, 0);
    glVertex3f(right, bot, 0);
    glVertex3f(left, bot, 0);
    glVertex3f(left, top, 0);

    glEnd();
}

int TextPlane::Draw(const string &newText, int offset, bool startlower, bool force_highquality, bool automatte) {
    int retval = 1;
    bool drawbg = (bgcol.a != 0);
    static char *display_lists = CreateLists();
    //some stuff to draw the text stuff
    string::const_iterator text_it = newText.begin();
    static bool use_bit = force_highquality || false;
    static float font_point = 16;
    static bool font_antialias = true;
    void *fnt = g_game.x_resolution >= 800 ? GLUT_BITMAP_HELVETICA_12 : GLUT_BITMAP_HELVETICA_10;
    static float std_wid = glutStrokeWidth(GLUT_STROKE_ROMAN, 'W');
    myFontMetrics.i = font_point * std_wid / (119.05 + 33.33);
    myFontMetrics.j = font_point;
    myFontMetrics.i /= .5 * g_game.x_resolution;
    myFontMetrics.j /= .5 * g_game.y_resolution;
    float tmp, row, col;
    GetPos(row, col);
    float rowheight =
            (use_bit) ? ((fnt == GLUT_BITMAP_HELVETICA_12) ? (26. / g_game.y_resolution) : (22.
                    / g_game.y_resolution)) : (
                    myFontMetrics
                            .j);
    if (startlower) {
        row -= rowheight;
    }
    glLineWidth(1);
    if (!use_bit && font_antialias) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
    } else {
        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_LINE_SMOOTH);
    }
    glColor4f(this->col.r, this->col.g, this->col.b, this->col.a);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glDisable(GL_LIGHTING);

    glDisable(GL_TEXTURE0);
    glDisable(GL_TEXTURE1);

    glPushMatrix();
    glLoadIdentity();
    if (drawbg) {
        glColor4f(this->bgcol.r, this->bgcol.g, this->bgcol.b, this->bgcol.a);
        DrawSquare(col, this->myDims.i, row - rowheight * .25, row + rowheight);
    }
    glColor4f(this->col.r, this->col.g, this->col.b, this->col.a);

    glRasterPos2f(0, 0);
    int entercount = 0;
    for (; entercount < offset && text_it != newText.end(); text_it++) {
        if (*text_it == '\n') {
            entercount++;
        }
    }
    glTranslatef(col, row, 0);
    //glRasterPos2f (g_game.x_resolution*(1-(col+1)/2),g_game.y_resolution*(row+1)/2);
    glRasterPos2f(0, 0);
    float scalex = 1;
    float scaley = 1;
    int potentialincrease = 0;
    if (!use_bit) {
        scalex = myFontMetrics.i / std_wid;
        scaley = myFontMetrics.j / (119.05 + 33.33);
    }
    glScalef(scalex, scaley, 1);
    while (text_it != newText.end() && row > myDims.j) {
        if (*text_it == '#') {
            if (newText.end() > text_it + 6) {
                float r, g, b;
                r = TwoCharToFloat(*(text_it + 1), *(text_it + 2));
                g = TwoCharToFloat(*(text_it + 3), *(text_it + 4));
                b = TwoCharToFloat(*(text_it + 5), *(text_it + 6));
                if (r == 0 && g == 0 && b == 0) {
                    glColor4f(this->col.r, this->col.g, this->col.b, this->col.a);
                } else {
                    glColor4f(r,
                            g,
                            b,
                            this->col.a);
                }
                text_it = text_it + 6;
            }
            text_it++;
            continue;
        } else if (*text_it >= 32) {
            //always true
            unsigned char myc = *text_it;
//if (myc=='_') {
//myc = ' ';
//}
            //glutStrokeCharacter (GLUT_STROKE_ROMAN,*text_it);
            retval += potentialincrease;
            potentialincrease = 0;
            int lists = 0;             //display_lists[myc];
            if (lists) {
//GFXCallList(lists);
            } else {
                if (use_bit) {
                    glutBitmapCharacter(fnt, myc);
                } else {
                    glutStrokeCharacter(GLUT_STROKE_ROMAN, myc);
                }
            }
        }
        if (*text_it == '\t') {
            col += glutBitmapWidth(fnt, ' ') * 5. / (2 * g_game.x_resolution);
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
        } else {
            if (use_bit) {
                col += glutBitmapWidth(fnt, *text_it) / (float) (2 * g_game.x_resolution);
            } else {
                col += myFontMetrics.i * glutStrokeWidth(GLUT_STROKE_ROMAN, *text_it) / std_wid;
            }
        }
        if (col
                + ((text_it + 1
                        != newText.end()) ? (use_bit ? (glutBitmapWidth(fnt,
                        *text_it)
                        / (float) (2 * g_game.x_resolution)) : myFontMetrics.i) : 0) >= myDims.i
                || *text_it == '\n') {
            GetPos(tmp, col);
            row -= rowheight;
            glPopMatrix();
            glPushMatrix();
            glLoadIdentity();
            if (drawbg) {
                glColor4f(this->bgcol.r, this->bgcol.g, this->bgcol.b, this->bgcol.a);
                DrawSquare(col, this->myDims.i, row - rowheight * .25, row + rowheight);
                glColor4f(this->col.r, this->col.g, this->col.b, this->col.a);
            }
            if (*text_it == '\n') {
                glColor4f(this->col.r, this->col.g, this->col.b, this->col.a);
            }
            glTranslatef(col, row, 0);
            glScalef(scalex, scaley, 1);
            glRasterPos2f(0, 0);
            potentialincrease++;
        }
        text_it++;
    }
    glDisable(GL_LINE_SMOOTH);
    glPopMatrix();

    glColor4f(this->col.r, this->col.g, this->col.b, this->col.a);
    return retval;
}

enum DisplayMode {
    CARGO = 0,                //Buy and sell cargo.
    UPGRADE,                //Buy and sell ship upgrades.
    SHIP_DEALER,            //Replace current ship.
    MISSIONS,               //Show available missions.
    NEWS,                   //Show news items.
    INFO,                               //Show basic info.
    DISPLAY_MODE_COUNT,     //Number of display modes.
    NULL_DISPLAY = DISPLAY_MODE_COUNT,         //No display.
};

#define BASE_EXTENSION ".py"

class Base {
public:
    class Room {
    public:
        class Link {
        public:
            std::string pythonfile;
            float x, y, wid, hei;
            std::string text;
            const std::string index;
            unsigned int parentindex;
            virtual void Click(::Base *base, float x, float y, int button, int state);

            explicit Link(unsigned int parind, std::string ind, std::string pfile)
                    : parentindex(parind), pythonfile(pfile), index(ind) {
            }

            virtual ~Link() {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Goto : public Link {
        public:
            int index;
            virtual void Click(::Base *base, float x, float y, int button, int state);

            virtual ~Goto() {
            }

            explicit Goto(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind,
                    ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Comp : public Link {
        public:
            std::vector<DisplayMode> modes;
            virtual void Click(::Base *base, float x, float y, int button, int state);

            virtual ~Comp() {
            }

            explicit Comp(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind,
                    ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Launch : public Link {
        public:
            virtual void Click(::Base *base, float x, float y, int button, int state);

            virtual ~Launch() {
            }

            explicit Launch(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind,
                    ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Bar : public Link {
        public:
            virtual ~Bar() {
            }

            explicit Bar(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind, ind, pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Commodity : public Link {
        public:
            virtual ~Commodity() {
            }

            explicit Commodity(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind, ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Merchant : public Link {
        public:
            virtual ~Merchant() {
            }

            explicit Merchant(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind, ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Mercenary : public Link {
        public:
            virtual ~Mercenary() {
            }

            explicit Mercenary(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind, ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Upgrades : public Link {
        public:
            virtual ~Upgrades() {
            }

            explicit Upgrades(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind, ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };

        class Weapon : public Link {
        public:
            virtual ~Weapon() {
            }

            explicit Weapon(unsigned int parind, std::string ind, std::string pythonfile) : Link(parind,
                    ind,
                    pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Talk : public Link {
        public:
//At the moment, the Base::Room::Talk class is unused... but I may find a use for it later...
            std::vector<std::string> say;
            std::vector<std::string> soundfiles;
            int index;
            int curroom;
            virtual void Click(::Base *base, float x, float y, int button, int state);
            explicit Talk(unsigned int parind, std::string ind, std::string pythonfile);

            virtual ~Talk() {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class Python : public Link {
        public:
            std::string file;
            virtual void Click(::Base *base, float x, float y, int button, int state);
            Python(unsigned int parind, std::string ind, std::string pythonfile);

            virtual ~Python() {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif
        };
        class BaseObj {
        public:
            const std::string index;
            virtual void Draw(::Base *base);
#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif

            virtual ~BaseObj() {
            }

            explicit BaseObj(std::string ind) : index(ind) {
            }
        };
        class BaseShip : public BaseObj {
        public:
            virtual void Draw(::Base *base);
            Matrix mat;

            virtual ~BaseShip() {
            }

#ifdef BASE_MAKER
            virtual void EndXML(FILE *fp);
#endif

            explicit BaseShip(std::string ind) : BaseObj(ind) {
            }

            BaseShip(float r0,
                    float r1,
                    float r2,
                    float r3,
                    float r4,
                    float r5,
                    float r6,
                    float r7,
                    float r8,
                    QVector pos,
                    std::string ind) :
                    BaseObj(ind), mat(r0, r1, r2, r3, r4, r5, r6, r7, r8, QVector(pos.i / 2, pos.j / 2, pos.k)) {
            }
        };
        class BaseVSSprite : public BaseObj {
        public:
            virtual void Draw(::Base *base);
            VSSprite spr;
#ifdef BASE_MAKER
            std::string texfile;
            std::string extension;
            virtual void EndXML(FILE *fp);
#endif

            virtual ~BaseVSSprite() {
            }

            BaseVSSprite(const char *spritefile, std::string ind) :
                    BaseObj(ind), spr(spritefile, BILINEAR, GFXTRUE) {
            }
        };
        class BaseTalk : public BaseObj {
        public:
            static bool hastalked;
            virtual void Draw(::Base *base);
//Talk * caller;
//int sayindex;
            int curchar;
            float curtime;

            virtual ~BaseTalk() {
            }

            std::string message;
//BaseTalk (Talk *caller) : caller (caller),  sayindex (0),curchar(0) {}
            BaseTalk(std::string msg, std::string ind, bool only_one_talk);
#ifdef BASE_MAKER

            virtual void EndXML(FILE *fp) {
            }

#endif
        };
        std::string soundfile;
        std::string deftext;
        std::vector<Link *> links;
        std::vector<BaseObj *> objs;
        unsigned int index;
#ifdef BASE_MAKER
        void EndXML(FILE *fp);
        void PrintLinks(FILE *fp);
#endif
        void Draw(::Base *base);
        void Click(::Base *base, float x, float y, int button, int state);
        int MouseOver(::Base *base, float x, float y);
        Room(unsigned int index);
        ~Room();
    };
    friend class Room;
    friend class Room::BaseTalk;
    int curroom;
    std::vector<Room *> rooms;
    std::string basefile;
    bool time_of_day;
    TextPlane othtext;
    TextPlane curtext;
    static Base *CurrentBase;
    bool CallComp;
    int curlinkindex;
#ifdef BASE_MAKER
    void EndXML();
#endif
    void Terminate();
    void GotoLink(int linknum);
    void InitCallbacks();
    void CallCommonLinks(std::string name, std::string value);
//static void Base::beginElement(void *userData, const XML_Char *names, const XML_Char **atts);
//void Base::beginElement(const string &name, const AttributeList attributes);
//static void Base::endElement(void *userData, const XML_Char *name);
    void Load(const char *filename, const char *faction);
    static void ClickWin(int x, int y, int button, int state);
    void Click(int x, int y, int button, int state);
    static void PassiveMouseOverWin(int x, int y);
    static void ActiveMouseOverWin(int x, int y);
    void MouseOver(int x, int y);
    Base(const char *basefile);
    ~Base();
    static void DrawWin();
    void Draw();
};

#ifdef BASE_MAKER
    #include <stdio.h>
    #ifdef _WIN32
        #include <windows.h>
    #endif
static char makingstate = 0;
#endif
extern const char *mission_key; //defined in main.cpp
bool Base::Room::BaseTalk::hastalked = false;
//using namespace VSFileSystem;

static int mmx = 0;
static int mmy = 0;
#include <stdio.h>

static int indentlevel = 0;
static int bogus_int; //added by chuck_starchaser to squash ignored returns warnings.

static void Indent(FILE *fp) {
    for (int i = 0; i < indentlevel; ++i) {
        fwrite("\t", sizeof(char), 1, fp);
    }
}

void Base::Room::Link::EndXML(FILE *fp) {
    fprintf(fp, "room%d, '%s', %g, %g, %g, %g, '%s'", parentindex, index.c_str(), x, y, wid, hei, text.c_str());
}

void Base::Room::Goto::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "Base.Link (");
    Link::EndXML(fp);
    fprintf(fp, ", room%d)\n", Goto::index);
}

void Base::Room::Python::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "Base.Python (");
    Link::EndXML(fp);
    fprintf(fp, ", '%s')\n", file.c_str());
}

void Base::Room::Talk::EndXML(FILE *fp) {
    char randstr[100];
    sprintf(randstr, "NEW_SCRIPT_%d.py", (int) (rand()));
    Indent(fp);
    fprintf(fp, "Base.Python (");
    Link::EndXML(fp);
    fprintf(fp, ", '%s')\n", randstr);
    FILE *py = fopen(randstr, "wt");
    fprintf(py, "import Base\nimport VS\nimport random\n\nrandnum=random.randrange(0,%d)\n", int(say.size()));
    for (int i = 0; i < say.size(); i++) {
        fprintf(py, "if (randnum==%d):\n", i);
        for (int j = 0; j < say[i].size(); j++) {
            if (say[i][j] == '\n') {
                say[i][j] = '\\';
                static const char *ins = "n";
                say[i].insert(j, ins);
            }
        }
        fprintf(py, "  Base.Message ('%s')\n", say[i].c_str());
        if (!(soundfiles[i].empty())) {
            fprintf(py, "  VS.playSound ('%s', (0,0,0), (0,0,0))\n", soundfiles[i].c_str());
        }
    }
    //obolete... creates a file that uses the Python function instead.
}

void Base::Room::Bar::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "import bar_lib\n");
    Indent(fp);
    fprintf(fp, "bar = bar_lib.MakeBar (room%d,time_of_day)\n", parentindex);
    Indent(fp);
    fprintf(fp, "Base.Link (");
    Link::EndXML(fp);
    fprintf(fp, ", bar)\n");
}

void Base::Room::Commodity::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "import commodity_lib\n");
    Indent(fp);
    fprintf(fp, "commodity = commodity_lib.MakeCommodity (room%d,time_of_day)\n", parentindex);
    Indent(fp);
    fprintf(fp, "Base.Link (");
    Link::EndXML(fp);
    fprintf(fp, ", commodity)\n");
}

void Base::Room::Merchant::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "import merchant_guild\n");
    Indent(fp);
    fprintf(fp, "merchant = merchant_guild.MakeMerchantGuild (room%d,time_of_day)\n", parentindex);
    Indent(fp);
    fprintf(fp, "Base.Link (");
    Link::EndXML(fp);
    fprintf(fp, ", merchant)\n");
}

void Base::Room::Mercenary::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "import mercenary_guild\n");
    Indent(fp);
    fprintf(fp, "merchant = mercenary_guild.MakeMercenaryGuild (room%d,time_of_day)\n", parentindex);
    Indent(fp);
    fprintf(fp, "Base.Link (");
    Link::EndXML(fp);
    fprintf(fp, ", merchant)\n");
}

void Base::Room::Weapon::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "import weapons_lib\n");
    Indent(fp);
    fprintf(fp, "weap = weapons_lib.MakeWeapon (room%d,time_of_day)\n", parentindex);
    Indent(fp);

    fprintf(fp, "Base.Link (");
    Link::EndXML(fp);
    fprintf(fp, ", weap)\n");
}

void Base::Room::Launch::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "Base.LaunchPython (");
    fprintf(fp, "room%d, '%s', 'bases/launch_music.py', %g, %g, %g, %g, '%s'", parentindex,
            index.c_str(), x, y, wid, hei, text.c_str());
    fprintf(fp, ")\n");
}

void Base::Room::Comp::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "Base.Comp (");
    Link::EndXML(fp);
    fwrite(", '", 3, 1, fp);
    for (int i = 0; i < modes.size(); i++) {
        char const *mode = NULL;
        switch (modes[i]) {
            case CARGO:
                mode = "Cargo";
                break;
            case UPGRADE:
                mode = "Upgrade";
                break;
            case SHIP_DEALER:
                mode = "ShipDealer";
                break;
            case MISSIONS:
                mode = "Missions";
                break;
            case NEWS:
                mode = "News";
                break;
            case INFO:
                mode = "Info";
                break;
        }
        if (mode) {
            fprintf(fp, "%s ", mode);
        }
        if ((i + 1) == (modes.size())) {
            fprintf(fp, "'");
        }
    }
    fprintf(fp, ")\n");
}

void Base::Room::BaseObj::EndXML(FILE *fp) {
//Do nothing
}

void Base::Room::BaseShip::EndXML(FILE *fp) {
    Indent(fp);
    fprintf(fp, "Base.Ship (room, '%s', (%lg,%lg,%lg), (%g, %g, %g), (%g, %g, %g))\n", index.c_str(),
            mat.p.i, mat.p.j, mat.p.k,
            mat.getR().i, mat.getR().j, mat.getR().k,
            mat.getQ().i, mat.getQ().j, mat.getQ().k);
}

string ridExt(string inp) {
    string::size_type where = inp.rfind(".");
    if (where == string::npos) {
        return inp;
    }
    return inp.substr(0, where);
}

void Base::Room::BaseVSSprite::EndXML(FILE *fp) {
    float x, y;
    spr.GetPosition(x, y);
    Indent(fp);
    string noextname = ridExt(texfile);
    fprintf(fp, "Base.Texture (room, '%s', 'bases/%s/%s'+time_of_day+'.spr', %g, %g)\n",
            index.c_str(), Base::CurrentBase->basefile.c_str(), noextname.c_str(), x, y);
}

bool room1 = false;

void Base::Room::EndXML(FILE *fp) {
    int i;
    Indent(fp);
    fprintf(fp, "room = Base.Room ('%s')\n", deftext.c_str());
    Indent(fp);
    fprintf(fp, "room%d = room\n", index);
    for (i = 0; i < objs.size(); i++) {
        if (objs[i]) {
            objs[i]->EndXML(fp);
        }
    }
    /*
     *  for (i=0;i<links.size();i++) {
     *       if (links[i])
     *               links[i]->EndXML(fp);
     *  }
     */                                                                                                                               //lata
    fprintf(fp, "\n");
    fflush(fp);
}

void Base::Room::PrintLinks(FILE *fp) {
    for (unsigned int i = 0; i < links.size(); i++) {
        if (links[i]) {
            links[i]->EndXML(fp);
        }
    }
}

void Base::EndXML() {
    bogus_int = chdir("bases");
    FILE *fp = NULL;
    if (time_of_day) {
        const char *times_of_day[3] = {"_day", "_night", "_sunset"};
        for (int i = 0; i < 3; ++i) {
            fp = fopen((basefile + times_of_day[i] + ".py").c_str(), "wt");
            fprintf(
                    fp,
                    "import Base\nimport sys\nimport %s_lib\ntime_of_day='%s'\n\n(landing_platform,bar,weap) = %s_lib.Make%s (time_of_day)\n",
                    basefile.c_str(),
                    times_of_day[i],
                    basefile.c_str(),
                    basefile.c_str());
            fclose(fp);
        }
        fp = fopen((basefile + "_lib.py").c_str(), "wt");
    } else {
        fp = fopen((basefile + ".py").c_str(), "wt");
    }
    fprintf(fp, "import Base\nimport dynamic_mission\nimport VS\n\n");
    if (time_of_day) {
        fprintf(fp, "def Make%s (time_of_day='_day'):\n", basefile.c_str());
        indentlevel++;
    } else {
        fprintf(fp, "time_of_day=''\n");
    }
    Indent(fp);
    fprintf(fp, "bar=-1\n");
    Indent(fp);
    fprintf(fp, "weap=-1\n");
    Indent(fp);
    fprintf(fp, "room0=-1\n");
    Indent(fp);
    fprintf(fp, "plist=VS.musicAddList('%s.m3u')\n", basefile.c_str());
    Indent(fp);
    fprintf(fp, "VS.musicPlayList(plist)\n");
    Indent(fp);
    fprintf(fp, "dynamic_mission.CreateMissions()\n");
    for (int i = 0; i < rooms.size(); i++) {
        room1 = (i == 0);
        rooms[i]->EndXML(fp);
    }
    for (int i = 0; i < rooms.size(); i++) {
        rooms[i]->PrintLinks(fp);
    }
    if (time_of_day) {
        Indent(fp);
        fprintf(fp, "return (room0,bar,weap)");
    }
    fclose(fp);
    bogus_int = chdir("..");
}

static void CalculateRealXAndY(int xbeforecalc, int ybeforecalc, float *x, float *y) {
    (*x) = (((float) (xbeforecalc * 2)) / g_game.x_resolution) - 1;
    (*y) = -(((float) (ybeforecalc * 2)) / g_game.y_resolution) + 1;
}

/** Starts a Frame of OpenGL with proper parameters and mouse
 */
void StartGUIFrame(GFXBOOL clr) {
    //glutSetCursor(GLUT_CURSOR_INHERIT);
    //GFXViewPort (0,0,g_game.x_resolution,g_game.y_resolution);
//GFXHudMode (true);
    glColor4f(1, 1, 1, 1);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_WRITEMASK);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glClear(clr);
    glDisable(GL_DEPTH_WRITEMASK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE1);
    glEnable(GL_TEXTURE0);
}

void DrawGlutMouse(int mousex, int mousey) {
    //, VSSprite * spr) {
    /*
     *  glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
     *  glColor4f (1,1,1,1);
     *  glDisable(GL_TEXTURE1);
     *  glEnable(GL_TEXTURE0);
     *  glDisable (GL_DEPTH_TEST);
     *  glDisable(GL_LIGHTING);
     *  float sizex=0,sizey=0;
     *  spr->GetSize(sizex,sizey);
     *  spr->SetPosition(-1+.5*sizex+float(mousex)/(.5*g_game.x_resolution),1+.5*sizey-float(mousey)/(.5*g_game.y_resolution));
     *  spr->Draw();
     *  glDisable(GL_TEXTURE0);
     *  glEnable(GL_TEXTURE0);
     */
    float myx;
    float myy;
    CalculateRealXAndY(mousex, mousey, &myx, &myy);
    glBegin(GL_LINE_LOOP);
    glVertex3f(myx, myy, 0);
    glVertex3f(myx + .12, myy - .04, 0);
    glVertex3f(myx + .04, myy - .12, 0);
    glEnd();
}

void ConditionalCursorDraw(bool iscursor) {
    glutSetCursor(iscursor ? GLUT_CURSOR_LEFT_ARROW : GLUT_CURSOR_NONE);
}

void EndGUIFrame(bool drawmouseover) {
//static VSSprite MouseVSSprite ("mouseover.spr",BILINEAR,GFXTRUE);
    glDisable(GL_CULL_FACE);
    DrawGlutMouse(mmx, mmy);     //,&MouseVSSprite);
    //glEndScene();bad things...only call this once
//GFXHudMode(false);
//glEnable (GL_CULL_FACE);
}

void SetSoftwareMousePosition(int x, int y) {
    mmx = x;
    mmy = y;
    float nx, ny;
    CalculateRealXAndY(x, y, &nx, &ny);
    printf("\r(%f,%f)", nx, ny);
    fflush(stdout);
}

Base::Room::~Room() {
    size_t i;
    for (i = 0; i < links.size(); ++i) {
        if (links[i] != nullptr) {
            delete links[i];
            links[i] = nullptr;
        }
    }
    for (i = 0; i < objs.size(); ++i) {
        if (objs[i] != nullptr) {
            delete objs[i];
            objs[i] = nullptr;
        }
    }
}

typedef bool
(*InputFunc)(std::string input, unsigned int inputroomindex, void *dat1, void *dat2, const void *dat3, float x,
        float y);

unsigned int input_base_index_dat;
void *input_dat1;
void *input_dat2;
const void *input_dat3;
float input_x;
float input_y;
InputFunc input_func;
const char *input_prompt;
std::string input_string;
bool is_input = false;
bool input_cancel = true;
std::string input_buffer;
int NUMLINES;

void Output(const char *message) {
    input_buffer += message + string("\n");
    unsigned int newlines = 0;
    std::string::size_type pos = 0;
    while ((pos = input_buffer.find('\n', pos)) != std::string::npos) {
        newlines++;
        pos++;
    }
    while (newlines > NUMLINES) {
        pos = input_buffer.find('\n');
        input_buffer = input_buffer.substr(pos + 1);
        newlines--;
    }
}

void Output(const std::string &message) {
    Output(message.c_str());
}

void Error(const std::string &message) {
    Output(string("#ff3333") + message);
}

void InputDraw() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//static int draw=0;
//draw++;
    TextPlane drawificator(GFXColor(.8, .8, .8, 1), GFXColor(0, 0, 0, 1));
    float x, y;
    std::string add;
//if (draw>10) {
    add = "#ff6666|#000000";
//if (draw>20)
//draw=0;
//}
    drawificator.GetCharSize(x, y);
    drawificator.SetCharSize(x / 2, y / 2);
    NUMLINES = int(3 / (y)) - 2;
    drawificator.SetText(input_buffer + "#ffff00" + input_prompt + "\n#ffffff" + input_string + add);
    drawificator.SetSize(1 - .01, -2);
    drawificator.SetPos(-1, 1);
    drawificator.Draw();
    glFinish();
    glutSwapBuffers();
}

void InputKeyDown(unsigned char key, int x, int y) {
    if (is_input) {
        if (key >= 32 && key < 127) {
            input_string += key;
        }
        if (key == 27 && input_cancel) {
            is_input = false;
        }
        if (key == 8 && (!input_string.empty())) {
            input_string = input_string.substr(0, input_string.size() - 1);
        }
    }
    glutPostRedisplay();
}

void InputKeyUp(unsigned char key, int x, int y) {
    if (is_input) {
        if (key == '\r' || key == '\n') {
            Output(input_prompt);
            Output(input_string);
            is_input =
                    !((*input_func)(input_string,
                            input_base_index_dat,
                            input_dat1,
                            input_dat2,
                            input_dat3,
                            input_x,
                            input_y));
        }
    }
    glutPostRedisplay();
}

void Input(const char *prompt,
        InputFunc myfunction,
        bool cancancel,
        unsigned int inputroomindex,
        void *dat1,
        void *dat2,
        const void *dat3,
        float x,
        float y) {
    input_dat1 = dat1;
    input_dat2 = dat2;
    input_dat3 = dat3;
    input_base_index_dat = inputroomindex;
    input_x = x;
    input_y = y;
    input_func = myfunction;
    input_prompt = prompt;
    input_string = string();
    input_cancel = cancancel;
    is_input = true;
}

struct MyTmpXY {
    float x;
    float y;
}
        tmp_xy;

bool AddRoomSprite(std::string input,
        unsigned int room_index,
        void *dat1,
        void *dat2,
        const void *dat3,
        float x,
        float y) {
    std::vector<Base::Room::BaseObj *> *objs = (std::vector<Base::Room::BaseObj *> *) dat1;
    if (input.empty()) {
        //add a ship.
        objs->push_back(new Base::Room::BaseShip(0, .34, .93, -1, 0, 0, 0, .93, -.34,
                QVector((x + tmp_xy.x) / 2, (tmp_xy.y + y) / 2, 4), "my_ship"));
    }
        //FIXME: orientation cannot be changed from editor.
    else {
        bool time = false;
        string startdir;
        if (Base::CurrentBase->time_of_day) {
            FILE *fp = fopen(("sprites/bases/" + Base::CurrentBase->basefile + "/" + input + "_day.spr").c_str(), "rt");
            if (fp) {
                fclose(fp);
                startdir = "sprites/bases/" + Base::CurrentBase->basefile + "/";
                time = true;
            }
        }
        if (!time) {
            FILE *fp = fopen(("sprites/bases/" + Base::CurrentBase->basefile + "/" + input + ".spr").c_str(), "rt");
            if (fp) {
                fclose(fp);
                startdir = "sprites/bases/" + Base::CurrentBase->basefile + "/";
            }
        }
//bool time=false;
        string extension = "png";
        if (Base::CurrentBase->time_of_day) {
            FILE *fp = fopen((startdir + input + "_day.png").c_str(), "rb");
            if (fp) {
                fclose(fp);
                time = true;
            } else {
                extension = "jpg";
                fp = fopen((startdir + input + "_day.jpg").c_str(), "rb");
                if (fp) {
                    fclose(fp);
                    time = true;
                }
            }
        }
        if (!time) {
            extension = "png";
            FILE *fp = fopen((startdir + input + ".png").c_str(), "rb");
            if (fp) {
                fclose(fp);
            } else {
                extension = "jpg";
                fp = fopen((startdir + input + ".jpg").c_str(), "rb");
                if (fp) {
                    fclose(fp);
                } else {
                    Error("File not found!");
                    return false;
                }
            }
        }
        const char *thetimes[4] = {"_day", "_sunset", "_night", ""};
        if (startdir.empty()) {
//if (time==false) {
//return;
//}
            float wid = tmp_xy.x - x;
            float hei = tmp_xy.y - y;
            if (wid < 0) {
                wid = -wid;
            }
            if (hei < 0) {
                hei = -hei;
            }
            for (int i = time ? 0 : 3; i < 4; ++i) {
                FILE *fp = fopen(("sprites/bases/" + Base::CurrentBase->basefile + "/" + input + std::string(
                        thetimes[i]) + ".spr").c_str(), "wt");
                std::string curfilename = input + thetimes[i] + "." + extension;
                std::string filename = std::string("bases/") + Base::CurrentBase->basefile + "/" + curfilename;
                fprintf(fp, "%s true\n%g %g\n0,0", filename.c_str(), wid, hei);
                fclose(fp);
                rename(curfilename.c_str(), ("sprites/" + filename).c_str());
            }
            AddRoomSprite(input, room_index, dat1, dat2, dat3, x, y);
        } else {
            Base::Room::BaseVSSprite *tmp =
                    new Base::Room::BaseVSSprite(
                            ("bases/" + Base::CurrentBase->basefile + "/" + input + thetimes[time ? 0 : 3]
                                    + ".spr").c_str(),
                            string((char *) dat3));
            if (tmp->spr.LoadSuccess() && tmp->spr.getTexture()->LoadSuccess()) {
                objs->push_back(tmp);
                if (time) {
                    ((Base::Room::BaseVSSprite *) objs->back())->texfile = input;
                    ((Base::Room::BaseVSSprite *) objs->back())->extension = extension;
                } else {
                    ((Base::Room::BaseVSSprite *) objs->back())->texfile = input + "." + extension;
                    ((Base::Room::BaseVSSprite *) objs->back())->extension = "";
                }
                ((Base::Room::BaseVSSprite *) objs->back())->spr.SetPosition((tmp_xy.x + x) / 2, (tmp_xy.y + y) / 2);
            } else {
                Error("Invalid texture!");
                return false;
            }
        }
    }
    return true;
}

Base::Room::Room(unsigned int index) : index(index) {
    tmp_xy.x = -0.97;
    tmp_xy.y = 0.97;
}

void Base::Room::BaseObj::Draw(Base *base) {
//Do nothing...
}

void Base::Room::BaseVSSprite::Draw(Base *base) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE0);
    spr.Draw();
}

void Base::Room::BaseShip::Draw(Base *base) {
    /*
     *  Unit *un=base->caller.GetUnit();
     *  if (un) {
     *       GFXHudMode (GFXFALSE);
     *       GFXEnable (DEPTHTEST);
     *       GFXEnable (DEPTHWRITE);
     *       Vector p,q,r;
     *       _Universe->AccessCamera()->GetOrientation (p,q,r);
     *       int co=_Universe->AccessCamera()->getCockpitOffset();
     *       _Universe->AccessCamera()->setCockpitOffset(0);
     *       _Universe->AccessCamera()->UpdateGFX();
     *       QVector pos =  _Universe->AccessCamera ()->GetPosition();
     *       Matrix cam (p.i,p.j,p.k,q.i,q.j,q.k,r.i,r.j,r.k,pos);
     *       Matrix final;
     *       Matrix newmat = mat;
     *       newmat.p.k*=un->rSize();
     *       newmat.p+=QVector(0,0,g_game.znear);
     *       newmat.p.i*=newmat.p.k;
     *       newmat.p.j*=newmat.p.k;
     *       MultMatrix (final,cam,newmat);
     *       (un)->DrawNow(final);
     *       GFXDisable (DEPTHTEST);
     *       GFXDisable (DEPTHWRITE);
     *       _Universe->AccessCamera()->setCockpitOffset(co);
     *       _Universe->AccessCamera()->UpdateGFX();
     *  //		_Universe->AccessCockpit()->SetView (CP_PAN);
     *       GFXHudMode (GFXTRUE);
     *  }
     */
    glDisable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
    glBegin(GL_TRIANGLES);
    glVertex3f(mat.p.i - .2, mat.p.j, mat.p.k);
    glVertex3f(mat.p.i + .3, mat.p.j + .2, mat.p.k);
    glVertex3f(mat.p.i + .3, mat.p.j - .2, mat.p.k);
    glVertex3f(mat.p.i + .2, mat.p.j, mat.p.k);
    glVertex3f(mat.p.i + .25, mat.p.j + .4, mat.p.k);
    glVertex3f(mat.p.i + .3, mat.p.j, mat.p.k);
    glVertex3f(mat.p.i + .2, mat.p.j, mat.p.k);
    glVertex3f(mat.p.i + .25, mat.p.j - .4, mat.p.k);
    glVertex3f(mat.p.i + .3, mat.p.j, mat.p.k);
    glEnd();
}

void Base::Room::Draw(Base *base) {
    int i;
    for (i = 0; i < objs.size(); i++) {
        if (objs[i]) {
            objs[i]->Draw(base);
        }
    }
}

static Base::Room::BaseTalk *only_one_talk = NULL;

Base::Room::BaseTalk::BaseTalk(std::string msg, std::string ind, bool only_one)
        : BaseObj(ind), curchar(0), curtime(0), message(msg) {
    if (only_one) {
        only_one_talk = this;
    }
}

void Base::Room::BaseTalk::Draw(Base *base) {
    if (hastalked) {
        return;
    }
    base->othtext.SetText(message);
    hastalked = true;
}

int Base::Room::MouseOver(Base *base, float x, float y) {
    for (int i = 0; i < links.size(); i++) {
        if (links[i]) {
            if (x >= links[i]->x
                    && x <= (links[i]->x + links[i]->wid)
                    && y >= links[i]->y
                    && y <= (links[i]->y + links[i]->hei)) {
                return i;
            }
        }
    }
    return -1;
}

Base *Base::CurrentBase = 0;

bool RefreshGUI(void) {
    Base::CurrentBase->Draw();
    return true;
}

bool LinkStage3Goto(std::string input,
        unsigned int unusedindexdat,
        void *dat1,
        void *dat2,
        const void *dat3,
        float x,
        float y) {
    return sscanf(input.c_str(), "%d", reinterpret_cast< int * > (dat1)) >= 1;
}

bool LinkStage3Comp(std::string input,
        unsigned int unused,
        void *dat1,
        void *dat2,
        const void *dat3,
        float x,
        float y) {
    int modearg = DISPLAY_MODE_COUNT;
    input = strtolower(input);
    if (input == "cargo") {
        modearg = CARGO;
    } else if (input == "upgrade") {
        modearg = UPGRADE;
    } else if (input == "shipdealer") {
        modearg = SHIP_DEALER;
    } else if (input == "missions") {
        modearg = MISSIONS;
    } else if (input == "news") {
        modearg = NEWS;
    } else if (input == "info") {
        modearg = INFO;
    }
    if (modearg < DISPLAY_MODE_COUNT) {
        ((std::vector<DisplayMode> *) dat1)->push_back((DisplayMode) (modearg));
    }
    input_string = string();
    return false;
}

bool LinkStage2(std::string input,
        unsigned int inputroomindex,
        void *dat1,
        void *dat2,
        const void *dat3,
        float x,
        float y) {
    std::vector<Base::Room::Link *> *links = (std::vector<Base::Room::Link *> *) dat1;
    bool ret = true;
    input = strtolower(input);
    if (input == "link") {
        links->push_back(new Base::Room::Goto(inputroomindex, "my_link_id", "link"));
        /*
         *  bogus_int = fscanf(fp,"%d",&((Goto*)links.back())->index);
         */
        Input("Which room should this go to?", &LinkStage3Goto, false, inputroomindex,
                &(((Base::Room::Goto *) (links->back()))->index), NULL, NULL, 0, 0);
        ret = false;
    } else if (input == "launch") {
        links->push_back(new Base::Room::Launch(inputroomindex, "my_launch_id", "launch"));
        ret = true;
    } else if (input == "bar") {
        links->push_back(new Base::Room::Bar(inputroomindex, "bar", "bar"));
        ret = true;
    } else if (input == "commodity") {
        links->push_back(new Base::Room::Commodity(inputroomindex, "commodity", "commodity"));
        ret = true;
    } else if (input == "merchant") {
        links->push_back(new Base::Room::Merchant(inputroomindex, "merchant", "merchant"));
        ret = true;
    } else if (input == "mercenary") {
        links->push_back(new Base::Room::Mercenary(inputroomindex, "mercenary", "mercenary"));
        ret = true;
    } else if (input == "weapon") {
        links->push_back(new Base::Room::Weapon(inputroomindex, "weapon_room", "weapon"));
        ret = true;
    } else if (input == "comp") {
        links->push_back(new Base::Room::Comp(inputroomindex, "my_comp_id", "comp"));
        /*
         *  fscanf(fp,"%d",&index);
         *  for (i=0;i<index&&(!feof(fp));i++) {
         *       bogus_int = fscanf(fp,"%d",&ret);
         *       ((Comp*)(links->back()))->modes.push_back((DisplayMode)ret);
         *  }
         */
        Input("What modes does it have [Cargo,Upgrade,ShipDealer,Missions,News,Info] (ESC=stop)?",
                &LinkStage3Comp,
                true,
                inputroomindex,
                &(((Base::Room::Comp *) (links->back()))->modes),
                NULL,
                NULL,
                0,
                0);
        ret = false;
    } else {
        input_string = string();
        //Too bad... try again!
        return false;
    }
    links->back()->text = string((char *) dat2);
    free(dat2);
    links->back()->x = tmp_xy.x;
    links->back()->y = tmp_xy.y;
    links->back()->wid = x - tmp_xy.x;
    if (links->back()->wid < 0) {
        links->back()->wid = -links->back()->wid;
    }
    links->back()->hei = y - tmp_xy.y;
    if (links->back()->hei < 0) {
        links->back()->hei = -links->back()->hei;
    }
    return ret;
}

bool LinkStage1(std::string input,
        unsigned int inputroomindex,
        void *dat1,
        void *dat2,
        const void *dat3,
        float x,
        float y) {
    Input("What link type [comp,link,launch,bar,weapon,mercenary,merchant,commodity]? ",
            &LinkStage2,
            false,
            inputroomindex,
            dat1,
            strdup(input.c_str()),
            NULL,
            x,
            y);
    return false;
}

void Base::Room::Click(Base *base, float x, float y, int button, int state) {
    if (makingstate == 2) {
        Input("Add a sprite (ESC=cancel, blank=Ship) (No extension. file must be png or jpg format.) ",
                &AddRoomSprite,
                true,
                this->index,
                &objs,
                NULL,
                "texture",
                x,
                y);
        makingstate = 0;
        return;
    }
    if (makingstate == 1) {
        Input("Input the title of the link (ESC=cancel)? ", &LinkStage1, true, this->index, &links, NULL, NULL, x, y);
        makingstate = 0;
        return;
    }
    if (button == GLUT_LEFT_BUTTON) {
        int linknum = MouseOver(base, x, y);
        if (linknum >= 0) {
            Link *link = links[linknum];
            if (link) {
                link->Click(base, x, y, button, state);
            }
        }
    } else {
#ifdef BASE_MAKER
        if (state == GLUT_UP) {
            char input[201];
            char *str;
            if (button != GLUT_MIDDLE_BUTTON && button != GLUT_RIGHT_BUTTON) {
                return;
            }
            /*
             *  if (button==GLUT_MIDDLE_BUTTON&&makingstate==0) {
             *       bogus_int = fscanf(fp,"%d",&rmtyp);
             *       switch(rmtyp) {
             *       case 1:
             *               links.push_back(new Goto("linkind","link"));
             *               bogus_int = fscanf(fp,"%d",&((Goto*)links.back())->index);
             *               break;
             *       case 2:
             *               links.push_back(new Launch("launchind","launch"));
             *               break;
             *       case 3:
             *               links.push_back(new Comp("compind","comp"));
             *               bogus_int = fscanf(fp,"%d",&index);
             *               for (i=0;i<index&&(!feof(fp));i++) {
             *                       bogus_int = fscanf(fp,"%d",&ret);
             *                       ((Comp*)links.back())->modes.push_back((DisplayMode)ret);
             *               }
             *               break;
             *       default:
             *               return;
             *       }
             *       bogus_int = fscanf(fp,"%200s",input);
             *       input[200]=input[199]='\0';
             *       links.back()->text=string(input);
             * #ifdef _WIN32
             *       fclose(fp);
             * #endif
             *  }
             */
            if (button == GLUT_RIGHT_BUTTON && makingstate == 0) {
                tmp_xy.x = x;
                tmp_xy.y = y;
                makingstate = 2;
            } else if (button == GLUT_MIDDLE_BUTTON && makingstate == 0) {
                tmp_xy.x = x;
                tmp_xy.y = y;
                makingstate = 1;
            }
        }
#else
        if ( state == GLUT_UP && links.size() ) {
            int count = 0;
            while ( count++ < links.size() ) {
                Link *curlink = links[base->curlinkindex++%links.size()];
                if (curlink) {
                    int x = ( ( ( curlink->x+(curlink->wid/2) )+1 )/2 )*g_game.x_resolution;
                    int y = -( ( ( curlink->y+(curlink->hei/2) )-1 )/2 )*g_game.y_resolution;
                    winsys_warp_pointer( x, y );
                    PassiveMouseOverWin( x, y );
                    break;
                }
            }
        }
#endif
    }
}

void Base::MouseOver(int xbeforecalc, int ybeforecalc) {
    float x, y;
    CalculateRealXAndY(xbeforecalc, ybeforecalc, &x, &y);
    int i = rooms[curroom]->MouseOver(this, x, y);
    Room::Link *link = 0;
    if (i < 0) {
        link = 0;
    } else {
        link = rooms[curroom]->links[i];
    }
    if (link) {
        curtext.SetText(link->text);
        curtext.col = GFXColor(1, .666667, 0, 1);
    } else {
        curtext.SetText(rooms[curroom]->deftext);
        curtext.col = GFXColor(0, 1, 0, 1);
    }
}

void Base::Click(int xint, int yint, int button, int state) {
    float x, y;
    CalculateRealXAndY(xint, yint, &x, &y);
    rooms[curroom]->Click(this, x, y, button, state);
}

void Base::ClickWin(int button, int state, int x, int y) {
    if (is_input) {
        return;
    }
    if (CurrentBase) {
        CurrentBase->Click(x, y, button, state);
    }
}

void Base::PassiveMouseOverWin(int x, int y) {
    SetSoftwareMousePosition(x, y);
    if (is_input) {
        return;
    }
    if (CurrentBase) {
        CurrentBase->MouseOver(x, y);
    }
}

void Base::ActiveMouseOverWin(int x, int y) {
    SetSoftwareMousePosition(x, y);
    if (is_input) {
        return;
    }
    if (CurrentBase) {
        CurrentBase->MouseOver(x, y);
    }
}

bool SetRoomString(std::string input,
        unsigned int room_index,
        void *dat1,
        void *dat2,
        const void *dat3,
        float x,
        float y) {
    if (input.empty()) {
        return false;
    }
    std::string *deftext = (std::string *) dat1;
    *deftext = input;
//if (Base::CurrentBase&&Base::CurrentBase->rooms.size()==0) {
//AddRoomSprite(Base::CurrentBase->basefile, &objs, NULL, "background", 0.97, -0.97);
//} else {
    Input("What is the texture for this room (No extension; file must be png or jpeg format.)?",
            &AddRoomSprite,
            false,
            room_index,
            dat2,
            NULL,
            "background",
            0.97,
            -0.97);
//}
    return false;
}

void Base::GotoLink(int linknum) {
    othtext.SetText("");
    if (rooms.size() > linknum && linknum >= 0) {
        curlinkindex = 0;
        curroom = linknum;
        curtext.SetText(rooms[curroom]->deftext);
    } else {
#ifndef BASE_MAKER
        fprintf( stderr, "\nWARNING: base room #%d tried to go to an invalid index: #%d", curroom, linknum );
        assert( 0 );
#else
        while (rooms.size() <= linknum) {
            rooms.push_back(new Room(rooms.size()));
            Input("Input Name of Room (Shown at bottom of screen): ", &SetRoomString, false, linknum,
                    &(rooms.back()->deftext), &(rooms.back()->objs), NULL, 0, 0);
        }
        GotoLink(linknum);
#endif
    }
}

Base::~Base() {
#ifdef BASE_MAKER
    EndXML();
#endif
    Base::CurrentBase = NULL;
    for (int i = 0; i < rooms.size(); i++) {
        delete rooms[i];
    }
}

void Base::InitCallbacks() {
    glutMouseFunc(ClickWin);
    glutMotionFunc(ActiveMouseOverWin);
    glutPassiveMotionFunc(PassiveMouseOverWin);
    CurrentBase = this;
//UpgradeCompInterface(caller,baseun);
    CallComp = false;
}

Base::Room::Talk::Talk(unsigned int parind, std::string ind, std::string pythonfile) :
        Base::Room::Link(parind, ind, pythonfile) {
    index = -1;
#ifndef BASE_MAKER
    gameMessage last;
    int    i = 0;
    vector< std::string >who;
    string newmsg;
    string newsound;
    who.push_back( "bar" );
    while ( ( mission->msgcenter->last( i++, last, who ) ) ) {
        newmsg   = last.message;
        newsound = "";
        string::size_type first = newmsg.find_first_of( "[" );
        {
            string::size_type last = newmsg.find_first_of( "]" );
            if ( first != string::npos && (first+1) < newmsg.size() ) {
                newsound = newmsg.substr( first+1, last-first-1 );
                newmsg   = newmsg.substr( 0, first );
            }
        }
        this->say.push_back( newmsg );
        this->soundfiles.push_back( newsound );
    }
#endif
}

Base::Room::Python::Python(unsigned int parind, std::string ind, std::string pythonfile) :
        Base::Room::Link(parind, ind, pythonfile) {
}

Base::Base(const char *basefile) :
        curtext(GFXColor(0, 1, 0, 1), GFXColor(0, 0, 0, 1)),
        othtext(GFXColor(1, 1, .5, 1), GFXColor(0, 0, 0, 1)),
        time_of_day(false) {
    CurrentBase = this;
    curlinkindex = 0;
    float x, y;
    curtext.GetCharSize(x, y);
    curtext.SetCharSize(x * 2, y * 2);
    //curtext.SetSize(2-(x*4 ),-2);
    curtext.SetSize(1 - .01, -2);
    othtext.GetCharSize(x, y);
    othtext.SetCharSize(x * 2, y * 2);
    //othtext.SetSize(2-(x*4),-.75);
    othtext.SetSize(1 - .01, -.75);
    this->basefile = basefile;
    mkdir("sprites"
#ifndef _WIN32
            , 0755
#endif
    );
    mkdir("sprites/bases"
#ifndef _WIN32
            , 0755
#endif
    );
    mkdir("bases"
#ifndef _WIN32
            , 0755
#endif
    );
    mkdir((std::string("sprites/bases/") + basefile).c_str()
#ifndef _WIN32
            , 0755
#endif
    );
    GotoLink(0);
}

void Base::Room::Python::Click(Base *base, float x, float y, int button, int state) {
    if (state == GLUT_UP) {
        Link::Click(base, x, y, button, state);
    }
//Do nothing...
}

//Need this for NEW_GUI.  Can't ifdef it out because it needs to link.
void InitCallbacks(void) {
    if (Base::CurrentBase) {
        Base::CurrentBase->InitCallbacks();
    }
}

void TerminateCurrentBase(void) {
    Base::CurrentBase->Terminate();
}

//end NEW_GUI.

void Base::Room::Comp::Click(Base *base, float x, float y, int button, int state) {
    if (state == GLUT_UP) {
        Link::Click(base, x, y, button, state);
    }
}

void Base::Terminate() {
    delete this;
}

extern void abletodock(int dock);

void Base::Room::Launch::Click(Base *base, float x, float y, int button, int state) {
    if (state == GLUT_UP) {
        base->Terminate();
    }
}

void Base::Room::Goto::Click(Base *base, float x, float y, int button, int state) {
    if (state == GLUT_UP) {
        Link::Click(base, x, y, button, state);
        base->GotoLink(index);
    }
}

void Base::Room::Talk::Click(Base *base, float x, float y, int button, int state) {
    if (state == GLUT_UP) {
        Link::Click(base, x, y, button, state);
        if (index >= 0) {
            delete base->rooms[curroom]->objs[index];
            base->rooms[curroom]->objs[index] = NULL;
            index = -1;
            base->othtext.SetText("");
        } else if (say.size()) {
            curroom = base->curroom;
//index=base->rooms[curroom]->objs.size();
            int sayindex = rand() % say.size();
            base->rooms[curroom]->objs.push_back(new Room::BaseTalk(say[sayindex], "currentmsg", true));
//((Room::BaseTalk*)(base->rooms[curroom]->objs.back()))->sayindex=(sayindex);
//((Room::BaseTalk*)(base->rooms[curroom]->objs.back()))->curtime=0;
        } else {
            fprintf(stderr, "\nThere are no things to say...\n");
        }
    }
}

void Base::Room::Link::Click(Base *base, float x, float y, int button, int state) {
    if (state == GLUT_UP) {
        /*
         *  const char * filnam=this->pythonfile.c_str();
         *  if (filnam[0]) {
         *       if (filnam[0]=='#') {
         *       }else {
         *       FILE *fp=fopen(filnam,"r");
         *       if (fp) {
         *               int length=strlen(filnam);
         *               char *newfile=new char[length+1];
         *               strncpy(newfile,filnam,length);
         *               newfile[length]='\0';
         *               ::Python::reseterrors();
         *               PyRun_SimpleFile(fp,newfile);
         *               ::Python::reseterrors();
         *               fclose(fp);
         *       } else {
         *               fprintf(stderr,"Warning:python link file '%s' not found\n",filnam);
         *       }
         *  }
         *  }
         */
    }
}

void Base::Draw() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//GFXColor(0,0,0,0);
    StartGUIFrame(GFXTRUE);
    Room::BaseTalk::hastalked = false;
    rooms[curroom]->Draw(this);
    float x, y;
    curtext.GetCharSize(x, y);
    curtext.SetPos(-.99, -1 + (y * 1.5));
//if (!drawlinkcursor)
//GFXColor4f(0,1,0,1);
//else
//GFXColor4f(1,.333333,0,1);
    curtext.Draw();
    othtext.SetPos(-.99, 1);
//GFXColor4f(0,.5,1,1);
    othtext.SetText(
            "To add a texture/ship, right click. To add a link, middle click. Save and exit by clicking a launch link.");
    othtext.Draw();
    EndGUIFrame(false);
    glFinish();
    glutSwapBuffers();
}

std::queue<std::string> bases;

void Base::DrawWin() {
    if (is_input) {
        InputDraw();
        return;
    }
    if (Base::CurrentBase) {
        Base::CurrentBase->Draw();
    } else if (!bases.empty()) {
        bool istimeofday = false;
        if ((!bases.front().empty()) && bases.front()[0] == '-') {
            istimeofday = true;
            bases.pop();
        }
        Base *newBase = new Base(bases.front().c_str());
        Base::CurrentBase = newBase;
        newBase->Draw();
        bases.pop();
        if (istimeofday) {
            newBase->time_of_day = true;
            istimeofday = false;
        }
    } else {
        exit(0);
    }
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    printf("Loading...\n");
    Base::CurrentBase = NULL;
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    g_game.x_resolution = 800;
    g_game.y_resolution = 600;
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("Vega Strike Base Maker");
    glutMouseFunc(Base::ClickWin);
    glutMotionFunc(Base::ActiveMouseOverWin);
    glutPassiveMotionFunc(Base::PassiveMouseOverWin);
    glutDisplayFunc(Base::DrawWin);
    glutKeyboardFunc(InputKeyDown);
    glutKeyboardUpFunc(InputKeyUp);
    ConditionalCursorDraw(false);
//glMatrixMode(GL_PROJECTION);
//glLoadIdentity();
//glOrtho(-1,1,-1,1,0,100);
//glMatrixMode(GL_MODELVIEW);

//glOrtho(-1,1,1,-1,0,100);
//glPushMatrix();
//glLoadIdentity();
//glMatrixMode (GL_PROJECTION);
//glPushMatrix();
//glLoadIdentity();
    /*
     *       glMatrixMode (GL_PROJECTION);
     *       glPopMatrix();
     *       glMatrixMode (GL_MODELVIEW);
     *       glPopMatrix();
     */
    bool istimeofday = false;
    for (unsigned int i = 1; i < argc; ++i) {
        printf(".");
        bases.push(argv[i]);
    }
    printf("!");
    glutShowWindow();
    printf("\nLoaded!\n");
    glutMainLoop();

    return 0;
}

