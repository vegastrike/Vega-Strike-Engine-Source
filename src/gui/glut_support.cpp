/***************************************************************************
 *                           glut_support.cpp  -  description
 *                           --------------------------
 *                           begin                : December 28, 2001
 *                           copyright            : (C) 2001 by David Ranger
 *                           email                : ussreliant@users.sourceforge.net
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
using namespace VSFileSystem;

#define isspace(chr) ((chr=='\t')||(chr=='\n')||(chr=='\v')||(chr=='\f')||(chr=='\r')||(chr==' '))

float colors[] = {1, 1, 1, 1};

GUITexture ReadTex(char *texfile) {
	VSFile file;
	VSError err;
	int PNG_HAS_ALPHA=4;
        GUITexture tex;
        if (texfile)
        	if (texfile[0])
					err=file.OpenReadOnly(texfile,TextureFile);
		GLuint name=0;
		unsigned int width=0,height=0;
		if (err<=Ok) {
				VSImage img;
				int bpp,colortype;
				unsigned char * palette;
				unsigned char * image=img.ReadImage( &file);
						//	readImage(file,bpp,colortype,width,height,palette);
				bpp = img.Depth();
				colortype = img.Format();
				glGenTextures (1,&name);
				glBindTexture (GL_TEXTURE_2D,name);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glTexImage2D(GL_TEXTURE_2D,0,(colortype&PNG_HAS_ALPHA)?GL_RGBA8:GL_RGB8,img.sizeX,img.sizeY,0,(colortype&PNG_HAS_ALPHA)?GL_RGBA:GL_RGB,GL_UNSIGNED_BYTE,image);
				file.Close();
		tex.wid = img.sizeX;
		tex.hei = img.sizeY;
		tex.name = name;

		free(image);
	}
	else {
		printf("Error opening file: %s\n",texfile);
		tex.wid = 0;
		tex.hei = 0;
		tex.name = 0;
	}
        return tex;
}

char * readString (FILE * fp,char endchar) {
        std::vector <char> chr_vec;
        char tmpchr;
        for (int i=0;;i++) {
                VSFileSystem::vs_read(&tmpchr,sizeof(char),1,fp);
                if ((tmpchr==endchar)||(tmpchr=='\0')||feof(fp)) break;
                chr_vec.push_back(tmpchr);
        }
        char * chr_str = new char [chr_vec.size()+1];
        memcpy (chr_str, &(chr_vec[0]),chr_vec.size()*sizeof (char));
        chr_str[chr_vec.size()]='\0';
        return chr_str;
}

void png_cexcept_error(png_structp png_ptr, png_const_charp msg) {
//      if(png_ptr)
//              ;
#ifndef PNG_NO_CONSOLE_IO
        fprintf(stderr, "libpng error: %s\n", msg);
#endif
}

unsigned char * readImage (FILE *fp, int & bpp, int &color_type, unsigned int &width, unsigned int &height, unsigned char * &palette) {
        bool strip_16 = 0;
        palette = NULL;
        unsigned char sig[8];
        png_structp png_ptr;
        png_bytepp row_pointers;
        png_infop info_ptr;
        int     interlace_type;
        VSFileSystem::vs_read(sig, 1, 8, fp);
        if (!png_check_sig(sig, 8))
                return NULL;    /* bad signature */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,

                (png_error_ptr)png_cexcept_error,

                (png_error_ptr)NULL);
        if (png_ptr == NULL)
        {
                return NULL;
        }
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL)
        {
                png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
                return NULL;
        }
        if (setjmp(png_jmpbuf(png_ptr))) {
                /* Free all of the memory associated with the png_ptr and info_ptr */
                png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
                /* If we get here, we had a problem reading the file */
                return NULL;
        }
        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

//      fprintf (stderr,"Loading Done. Decompressing\n");
        png_read_info(png_ptr, info_ptr);       /* read all PNG info up to image data */
        png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&width, (png_uint_32 *)&height, &bpp, &color_type, &interlace_type, NULL, NULL);
# if __BYTE_ORDER != __BIG_ENDIAN
        if (bpp==16)
         png_set_swap (png_ptr);
#endif
        if (bpp==16&&strip_16)
                png_set_strip_16(png_ptr);
        if (strip_16&&color_type == PNG_COLOR_TYPE_PALETTE)
                png_set_palette_to_rgb(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY &&
                        bpp < 8)
                png_set_gray_1_2_4_to_8(png_ptr);
        png_set_expand (png_ptr);
        png_read_update_info (png_ptr,info_ptr);
        png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&width, (png_uint_32 *)&height, &bpp, &color_type, &interlace_type, NULL, NULL);
        row_pointers = (unsigned char **)malloc (sizeof (unsigned char *) *height);
        int numchan=1;
        if (color_type&PNG_COLOR_MASK_COLOR)
                numchan =3;
        if (color_type &PNG_COLOR_MASK_PALETTE)
                numchan =1;
        if (color_type&PNG_COLOR_MASK_ALPHA)
                numchan++;
        unsigned long stride = numchan*sizeof (unsigned char)*bpp/8;
        unsigned char * image = (unsigned char *) malloc (stride*width*height);
        for (unsigned int i=0;i<height;i++) {
                row_pointers[i] = &image[i*stride*width];
        }
        png_read_image (png_ptr,row_pointers);
//      png_read_image(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND , NULL);
//      row_pointers = png_get_rows(png_ptr, info_ptr);
        free (row_pointers);
//      png_infop end_info;
        png_read_end(png_ptr, info_ptr);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
//      fprintf (stderr,"Decompressing Done.\n");
        /* close the file */
        return image;
}

// x,y must be -1 to 1, with -1,-1 being the bottom left corner. x,y must be the top left corner to draw
void ShowColor(float x, float y, float wid, float hei, float red, float green, float blue, float alpha) {
        float cols[4] = {red, green, blue, alpha};

        if (wid < 0 || hei < 0) {printf("Cannot draw color with negative height or width\n"); return; }

        // Make sure we don't exceed the program
        if (x+wid > 1) { wid = 1 - x; }
        if (y-hei < -1) { hei = -1 + y; }

#ifdef DEBUG
        cout << "Displaying color at " << x << "," << y << "\n";
        cout << "with the dimensions of " << wid << "," << hei << "\n";
        cout << "With the color " << red << "," << green << "," << blue << "," << alpha << "\n";
        cout << "-----------------------------\n";
#endif
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
                glColor4fv(cols);
                glVertex2f(x,y-hei);
                glColor4fv(cols);
                glVertex2f(x,y);
                glColor4fv(cols);
                glVertex2f(x+wid,y);
                glColor4fv(cols);
                glVertex2f(x+wid,y-hei);
        glEnd();
        glEnable(GL_TEXTURE_2D);
}

void ShowImage(float x, float y, float wid, float hei, GUITexture image, int tile_h, int tile_w) {
	#ifdef DEBUG
        cout << "Displaying image at " << x << "," << y << "\n";
        cout << "In area with dimensions of " << wid << "," << hei << "\n";
        cout << "Image dimensions: " << image.wid << "," << image.hei << "\n";
	#endif
        //float cols[] = {0,0,0.9,1};
        float cols[] = {colors[0], colors[1], colors[2], colors[3]};

        if (wid < 0 || hei < 0) {printf("Cannot draw image with negative height or width\n"); return; }

        if (x+wid > 1) { wid = 1 - x; }
        if (y-hei < -1) { hei = -1 + y; }
	#ifdef DEBUG
        if (tile_w == 0 && tile_h == 0) { cout << "Image will be stretched to fit\n"; }
        else { cout << "Image will be tiled\n"; }
        cout << "----------------------\n";
	#endif
	glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,image.name);
        glBegin(GL_QUADS);
                glColor4fv(cols);
                glTexCoord2f(0,1);
                glVertex2f(x,y-hei);

                glColor4fv(cols);
                glTexCoord2f(0,0);
                glVertex2f(x,y);

                glColor4fv(cols);
                glTexCoord2f(1,0);
                glVertex2f(x+wid,y);

                glColor4fv(cols);
                glTexCoord2f(1,1);
                glVertex2f(x+wid,y-hei);
        glEnd();
	glDisable(GL_TEXTURE_2D);
}
float word_length(const char * str) {
  float length=0;
  while (*str&&*str!='\\'&&(!isspace(*str))) {
    char mychar = *str++;
    length+=glutStrokeWidth (GLUT_STROKE_ROMAN,mychar);
  }
  return length/2500;
}
void ShowText(float x, float y, float wid, int size, const char *str, int no_end) {
  static float rescale_font = XMLSupport::parse_float (vs_config->getVariable("graphics","gui_font_scale",".75"));
  float font_size_float=rescale_font*5./100;

        int cur;
        float font_size = size*rescale_font;
	float width = 0;
	float cur_width = 0;
	float end = 0;
        font_size /= 10000;

	glDisable(GL_TEXTURE_2D);
        glLoadIdentity();
        glEnable(GL_LINE_SMOOTH);
	glLineWidth(wid);
	float page_wid=wid/rescale_font;
	glTranslatef(x,y,0);
	glScalef(font_size,font_size,1);
	end = no_end?0:glutStrokeWidth(GLUT_STROKE_ROMAN, 'A');
	end /= 2500;
	//	if (no_end == 1) { end = 0; }
	int h=0;
        for (cur = 0; str[cur] != '\0'; cur++) {
		cur_width = glutStrokeWidth(GLUT_STROKE_ROMAN, str[cur]);
		cur_width /= 2500;
		
		if ((width+end+word_length(str+cur) > page_wid|| str[cur]=='\\' )&& str[cur+1] != '\0' ) {
		  if (no_end==0) {
		    width += cur_width;
		    for (int i = 1; i <= 3; i++) { glutStrokeCharacter(GLUT_STROKE_ROMAN, '.'); }
		    break;
		  }else {
		    width=0;
		    glLoadIdentity();
		    glEnable(GL_LINE_SMOOTH);
		    glLineWidth(wid);
		    glTranslatef(x,y-(++h)*font_size_float,0);
		    glScalef(font_size,font_size,1);
		  }
		}else {
		  width += cur_width;
		}
		if (str[cur]!='\\') 
		  glutStrokeCharacter(GLUT_STROKE_ROMAN, str[cur]);
        }
	glLoadIdentity();
	glDisable(GL_LINE_SMOOTH);
}

float WidthOfChar(char chr) {
	float width = glutStrokeWidth(GLUT_STROKE_ROMAN, chr);
	width /= 2500;
	return width;
}



static int mmx=0;
static int mmy=0;
void SetSoftwareMousePosition (int x, int y) {
  mmx = x;
  mmy = y;
}
/** Starts a Frame of OpenGL with proper parameters and mouse
 */
void StartGUIFrame(GFXBOOL clr) {
  //  glutSetCursor(GLUT_CURSOR_INHERIT);
  //  GFXViewPort (0,0,g_game.x_resolution,g_game.y_resolution);
  GFXHudMode (true);
  GFXColor4f (1,1,1,1);

  GFXDisable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);
  GFXDisable (LIGHTING);
  GFXDisable (CULLFACE);
  GFXClear (clr);
  GFXDisable(DEPTHWRITE);
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (TEXTURE1);
  GFXEnable (TEXTURE0);
}
void DrawGlutMouse(int mousex, int mousey, Sprite * spr) {
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXColor4f (1,1,1,1);
  GFXDisable(TEXTURE1);
  GFXEnable(TEXTURE0);
  GFXDisable (DEPTHTEST);
  GFXDisable(LIGHTING);
  float sizex=0,sizey=0;
  spr->GetSize(sizex,sizey);
  spr->SetPosition(-1+.5*sizex+float(mousex)/(.5*g_game.x_resolution),1+.5*sizey-float(mousey)/(.5*g_game.y_resolution));
  spr->Draw();
  GFXDisable(TEXTURE0);
  GFXEnable(TEXTURE0);
}
extern void	ConditionalCursorDraw(bool);
void EndGUIFrame(bool drawmouseover) {
  static Sprite MouseOverSprite ("mouseover.spr",BILINEAR,GFXTRUE);
  static Sprite MouseSprite ("mouse.spr",BILINEAR,GFXTRUE);
  static Texture dummy ("white.bmp",0,NEAREST,TEXTURE2D,TEXTURE_2D,GFXTRUE);
  dummy.MakeActive();
  GFXDisable(CULLFACE);
  DrawGlutMouse(mmx,mmy,drawmouseover?&MouseOverSprite:&MouseSprite);
  //  GFXEndScene();bad things...only call this once
  GFXHudMode(false);
  GFXEnable (CULLFACE);
  ConditionalCursorDraw(true);
	

}
