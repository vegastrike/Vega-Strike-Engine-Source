/***************************************************************************
*                           glut_support.cpp  -  description
*                           --------------------------
*                           begin                : December 28, 2001
*                           copyright            : (C) 2001 by David Ranger
*                           email                : ussreliant@users.sourceforge.net
*                           copyright            : (C) 2020-2021 by Stephen G. Tuggy
*                           email                : sgt@stephengtuggy.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   any later version.                                                    *
*                                                                         *
***************************************************************************/

#include <png.h>
#include "glut_support.h"
#include "gfx/sprite.h"
#include "vs_globals.h"
#include "gfx/aux_texture.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "gfx/vsimage.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "gldrv/gl_globals.h"
using namespace VSFileSystem;

#define isspAce( chr )                                                                  \
    ( (chr == '\t') || (chr == '\n') || (chr == '\v') || (chr == '\f') || (chr == '\r') \
     || (chr == ' ') )

float colors[] = {1, 1, 1, 1};

//x,y must be -1 to 1, with -1,-1 being the bottom left corner. x,y must be the top left corner to draw
void ShowColor( float x, float y, float wid, float hei, float red, float green, float blue, float alpha )
{
    float cols[4] = {red, green, blue, alpha};
    if (wid < 0 || hei < 0) {
        VS_LOG(error, "Cannot draw color with negative height or width");
        return;
    }
    //Make sure we don't exceed the program
    if (x+wid > 1) wid = 1-x;
    if (y-hei < -1) hei = -1+y;
#ifdef DEBUG
    VS_LOG(debug, (boost::format("Displaying color at %1%,%2%") % x % y));
    VS_LOG(debug, (boost::format("with the dimensions of %1%,%2%") % wid % hei));
    VS_LOG(debug, (boost::format("With the color %1%,%2%,%3%,%4%") % red % green % blue % alpha));
    VS_LOG(debug, "-----------------------------");
#endif
    glDisable( GL_TEXTURE_2D );
    glBegin( GL_QUADS );
    glColor4fv( cols );
    glVertex2f( x, y-hei );
    glColor4fv( cols );
    glVertex2f( x, y );
    glColor4fv( cols );
    glVertex2f( x+wid, y );
    glColor4fv( cols );
    glVertex2f( x+wid, y-hei );
    glEnd();
    glEnable( GL_TEXTURE_2D );
}

float word_length( const char *str )
{
    float length = 0;
    while ( *str && *str != '\\' && ( !isspAce( *str ) ) ) {
        char mychar = *str++;
        length += glutStrokeWidth( GLUT_STROKE_ROMAN, mychar );
    }
    return length/2500;
}
void ShowText( float x, float y, float wid, int size, const char *str, int no_end )
{
    static float rescale_font = XMLSupport::parse_float( vs_config->getVariable( "graphics", "gui_font_scale", ".75" ) );
    float font_size_float     = rescale_font*5./100;

    int   cur;
    float font_size = size*rescale_font;
    float width     = 0;
    float cur_width = 0;
    float end = 0;
    font_size /= 10000;

    glDisable( GL_TEXTURE_2D );
    glLoadIdentity();
    if (gl_options.smooth_lines)
        glEnable( GL_LINE_SMOOTH );
    glLineWidth( wid );
    float page_wid = wid/rescale_font;
    glTranslatef( x, y, 0 );
    glScalef( font_size, font_size, 1 );
    end  = no_end ? 0 : glutStrokeWidth( GLUT_STROKE_ROMAN, 'A' );
    end /= 2500;
    //if (no_end == 1) { end = 0; }
    int h = 0;
    for (cur = 0; str[cur] != '\0'; cur++) {
        cur_width  = glutStrokeWidth( GLUT_STROKE_ROMAN, str[cur] );
        cur_width /= 2500;
        if ( (width+end+word_length( str+cur ) > page_wid || str[cur] == '\\') && str[cur+1] != '\0' ) {
            if (no_end == 0) {
                width += cur_width;
                for (int i = 1; i <= 3; i++)
                    glutStrokeCharacter( GLUT_STROKE_ROMAN, '.' );
                break;
            } else {
                width = 0;
                glLoadIdentity();
                if (gl_options.smooth_lines)
                    glEnable( GL_LINE_SMOOTH );
                glLineWidth( wid );
                glTranslatef( x, y-(++h)*font_size_float, 0 );
                glScalef( font_size, font_size, 1 );
            }
        } else {
            width += cur_width;
        }
        if (str[cur] != '\\')
            glutStrokeCharacter( GLUT_STROKE_ROMAN, str[cur] );
    }
    glLoadIdentity();
    if (gl_options.smooth_lines)
        glDisable( GL_LINE_SMOOTH );
}

float WidthOfChar( char chr )
{
    float width = glutStrokeWidth( GLUT_STROKE_ROMAN, chr );
    width /= 2500;
    return width;
}

static int mmx = 0;
static int mmy = 0;
void SetSoftwareMousePosition( int x, int y )
{
    mmx = x;
    mmy = y;
}
/** Starts a Frame of OpenGL with proper parameters and mouse
 */
void StartGUIFrame( GFXBOOL clr )
{
    //glutSetCursor(GLUT_CURSOR_INHERIT);
    //GFXViewPort (0,0,g_game.x_resolution,g_game.y_resolution);
    GFXHudMode( true );
    GFXColor4f( 1, 1, 1, 1 );

    GFXDisable( DEPTHTEST );
    GFXEnable( DEPTHWRITE );
    GFXDisable( LIGHTING );
    GFXDisable( CULLFACE );
    GFXClear( clr );
    GFXDisable( DEPTHWRITE );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXDisable( TEXTURE1 );
    GFXEnable( TEXTURE0 );
}
void DrawGlutMouse( int mousex, int mousey, VSSprite *spr )
{
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXColor4f( 1, 1, 1, 1 );
    GFXDisable( TEXTURE1 );
    GFXEnable( TEXTURE0 );
    GFXDisable( DEPTHTEST );
    GFXDisable( LIGHTING );
    float sizex = 0, sizey = 0;
    spr->GetSize( sizex, sizey );
    float tempx = 0, tempy = 0;
    spr->GetPosition( tempx, tempy );
    spr->SetPosition( tempx+ -1+.5*sizex+float(mousex)
                     /(.5*g_game.x_resolution), tempy+1+.5*sizey-float(mousey)/(.5*g_game.y_resolution) );
    spr->Draw();
    GFXDisable( TEXTURE0 );
    GFXEnable( TEXTURE0 );
    spr->SetPosition( tempx, tempy );
}
extern void ConditionalCursorDraw( bool );
void EndGUIFrame( MousePointerStyle pointerStyle )
{
    static VSSprite MouseOverVSSprite( "mouseover.spr", BILINEAR, GFXTRUE );
    static VSSprite MouseVSSprite( "mouse.spr", BILINEAR, GFXTRUE );
    static Texture  dummy( "white.bmp", 0, NEAREST, TEXTURE2D, TEXTURE_2D, GFXTRUE );

    if (pointerStyle != MOUSE_POINTER_NONE) {
        dummy.MakeActive();
        GFXDisable( CULLFACE );

        VSSprite *whichSprite = &MouseVSSprite;
        switch(pointerStyle) {
        case MOUSE_POINTER_NORMAL:  whichSprite = &MouseVSSprite; break;
        case MOUSE_POINTER_HOVER:   whichSprite = &MouseOverVSSprite; break;
        case MOUSE_POINTER_NONE: return;
        }

        DrawGlutMouse( mmx, mmy, whichSprite );

        //GFXEndScene();bad things...only call this once
        GFXHudMode( false );
        GFXEnable( CULLFACE );
        ConditionalCursorDraw( true );
    }
}

