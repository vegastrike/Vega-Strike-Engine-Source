//	This draws the mouse cursor
//	**********************************
void NavigationSystem::DrawCursor (float x, float y, float wid, float hei, const GFXColor &col) {
  float sizex, sizey;
  static bool modern_nav_cursor=XMLSupport::parse_bool(vs_config->getVariable("graphics","nav","modern_mouse_cursor","true"));
  if (modern_nav_cursor) {
    static string mouse_cursor_sprite=vs_config->getVariable("graphics","nav","mouse_cursor_sprite","mouse.spr");
    static VSSprite MouseVSSprite(mouse_cursor_sprite.c_str(),BILINEAR,GFXTRUE);
    GFXBlendMode(SRCALPHA,INVSRCALPHA);
    GFXColorf(GUI_OPAQUE_WHITE());

    // Draw the cursor sprite.
    GFXEnable(TEXTURE0);
    GFXDisable(DEPTHTEST);
    GFXDisable(TEXTURE1);
    MouseVSSprite.GetSize(sizex,sizey);
    MouseVSSprite.SetPosition(x+sizex/2,y+sizey/2);
    MouseVSSprite.Draw();
  }else {
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);
	GFXBegin(GFXLINE);
		GFXVertex3f(x,y,0);
		GFXVertex3f(x,(y-hei),0);

		GFXVertex3f(x,y,0);
		GFXVertex3f((x+wid),float(y-(0.75*hei)),0);

		GFXVertex3f(x,(y-hei),0);
		GFXVertex3f( (x+(0.35*wid)) , (y-(0.6*hei)),0);

		GFXVertex3f( (x+(0.35*wid)) , (y-(0.6*hei)),0);
		GFXVertex3f((x+wid),float(y-(0.75*hei)),0);
	GFXEnd();
	GFXEnable(TEXTURE0);
  }
}
//	**********************************








//	This draws the grid over the nav screen area
//	**********************************
void NavigationSystem::DrawGrid(float &x1, float &x2, float &y1, float &y2, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);


	float deltax = x2-x1;
	deltax = deltax / 10;
	float deltay = y2-y1;
	deltay = deltay / 10;


	GFXBegin(GFXLINE);
	int i;
	for(i=1; i < 10;i++)
	{
		GFXVertex3f(x1+(i*deltax),y1,0);
		GFXVertex3f(x1+(i*deltax),y2,0);
	}
//	GFXEnd();
//	GFXBegin(GFXLINE);

	for(i=1; i < 10;i++)
	{
		GFXVertex3f(x1,y1+(i*deltay),0);
		GFXVertex3f(x2,y1+(i*deltay),0);
	}
	GFXEnd();
	GFXEnable(TEXTURE0);
}
//	**********************************








//	This will draw a circle over the screen
//	**********************************
void NavigationSystem::DrawCircle(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	float i = 0.0;
	GFXBegin(GFXLINE);

	for(i = 0; i < 6.28; i+=(6.28/20))
	{
		GFXVertex3f( (x+(0.5*size*cos(i))) ,(y+(0.5*size*sin(i))),0);
		GFXVertex3f( (x+(0.5*size*cos(i+(6.28/20)))) ,(y+(0.5*size*sin(i+(6.28/20)))),0);
	}

	GFXEnd();
	GFXEnable(TEXTURE0);
}
//	**********************************








//	This will draw a half circle, centered at the top 1/4 center
//	**********************************
void NavigationSystem::DrawHalfCircleTop(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	float i = 0.0;
	GFXBegin(GFXLINE);

	for(i = 0; i < (3.14-(6.28/20)); i+=(6.28/20))
	{
		GFXVertex3f( (x+(0.5*size*cos(i))) ,(y-(0.25*size)+(0.5*size*sin(i))),0);
		GFXVertex3f( (x+(0.5*size*cos(i+(6.28/20)))) ,(y-(0.25*size)+(0.5*size*sin(i+(6.28/20)))),0);
	}


	GFXEnd();
	GFXEnable(TEXTURE0);
}
//	**********************************







//	This will draw a half circle, centered at the bottom 1/4 center
//	**********************************
void NavigationSystem::DrawHalfCircleBottom(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	float i = 0.0;
	GFXBegin(GFXLINE);
	for(i = 3.14; i < (6.28-(6.28/20)); i+=(6.28/20))
	{
		GFXVertex3f( (x+(0.5*size*cos(i))) ,((0.25*size)+y+(0.5*size*sin(i))),0);
		GFXVertex3f( (x+(0.5*size*cos(i+(6.28/20)))) ,((0.25*size)+y+(0.5*size*sin(i+(6.28/20)))),0);
	}
	GFXEnd();
	GFXEnable(TEXTURE0);
}
//	**********************************








//	This will draw a planet icon. circle + lightning thingy
//	**********************************
void NavigationSystem::DrawPlanet(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	float i = 0.0;
	GFXBegin(GFXLINE);

	for(i = 0; i < 6.28; i+=(6.28/20))
	{
		GFXVertex3f( (x+(0.5*size*cos(i))) ,(y+(0.5*size*sin(i))),0);
		GFXVertex3f( (x+(0.5*size*cos(i+(6.28/20)))) ,(y+(0.5*size*sin(i+(6.28/20)))),0);
	}

	GFXVertex3f(float(x-(0.5*size)),float(y),				0);
	GFXVertex3f(float(x),			float(y+(0.2*size)),	0);

	GFXVertex3f(float(x),			float(y+(0.2*size)),	0);
	GFXVertex3f(float(x),			float(y-(0.2*size)),	0);

	GFXVertex3f(float(x),			float(y-(0.2*size)),	0);
	GFXVertex3f(float(x+(0.5*size)),float(y),				0);

	GFXEnd();
	GFXEnable(TEXTURE0);
}
//	**********************************








//	This will draw a station icon. 3x3 grid
//	**********************************
void NavigationSystem::DrawStation(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	float segment = size/3;
	GFXBegin(GFXLINE);
	int i;
	for(i = 0; i < 4; i++)
	{
		GFXVertex3f( (x-(0.5*size)), ( (y-(0.5*size))+(i*segment) ), 0);
		GFXVertex3f( (x+(0.5*size)), ( (y-(0.5*size))+(i*segment) ), 0);
	}

	for(i = 0; i < 4; i++)
	{
		GFXVertex3f( ( (x-(0.5*size))+(i*segment) ), (y-(0.5*size)), 0);
		GFXVertex3f( ( (x-(0.5*size))+(i*segment) ), (y+(0.5*size)), 0);
	}
	GFXEnd();
	GFXEnable(TEXTURE0);
}
//	**********************************






//	This will draw a jump node icon
//	**********************************
void NavigationSystem::DrawJump(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	float i = 0.0;
	GFXBegin(GFXLINE);

	for(i = 0; i < 6.28; i+=(6.28/20))
	{
		GFXVertex3f( (x+(0.5*size*cos(i))) ,(y+(0.5*size*sin(i))),0);
		GFXVertex3f( (x+(0.5*size*cos(i+(6.28/20)))) ,(y+(0.5*size*sin(i+(6.28/20)))),0);
	}


	GFXVertex3f(x,float(y+(0.5 * size)),0);
	GFXVertex3f((x+(0.125*size)),(y+(0.125*size)),0);

	GFXVertex3f(x,float(y+(0.5 * size)),0);
	GFXVertex3f((x-(0.125*size)),(y+(0.125*size)),0);

	GFXVertex3f(x,float(y-(0.5 * size)),0);
	GFXVertex3f((x+(0.125*size)),(y-(0.125*size)),0);

	GFXVertex3f(x,float(y-(0.5 * size)),0);
	GFXVertex3f((x-(0.125*size)),(y-(0.125*size)),0);

	GFXVertex3f(float(x-(0.5*size)),y,0);
	GFXVertex3f((x-(0.125*size)),(y+(0.125*size)),0);

	GFXVertex3f(float(x-(0.5*size)),y,0);
	GFXVertex3f((x-(0.125*size)),(y-(0.125*size)),0);

	GFXVertex3f(float(x+(0.5*size)),y,0);
	GFXVertex3f((x+(0.125*size)),(y+(0.125*size)),0);

	GFXVertex3f(float(x+(0.5*size)),y,0);
	GFXVertex3f((x+(0.125*size)),(y-(0.125*size)),0);

	GFXEnd();
	GFXEnable(TEXTURE0);
}


//	**********************************

















//	This will draw a missile icon
//	**********************************
void NavigationSystem::DrawMissile(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	GFXBegin(GFXLINE);
		GFXVertex3f( (x-(0.5*size)), (y-(0.125*size)), 0);
		GFXVertex3f( x, float(y+(0.375*size)), 0);

		GFXVertex3f( (x+(0.5*size)), (y-(0.125*size)), 0);
		GFXVertex3f( x, float(y+(0.375*size)), 0);

		GFXVertex3f( (x-(0.25*size)), (y-(0.125*size)), 0);
		GFXVertex3f( (x-(0.25*size)), (y+(0.125*size)), 0);

		GFXVertex3f( (x+(0.25*size)), (y-(0.125*size)), 0);
		GFXVertex3f( (x+(0.25*size)), (y+(0.125*size)), 0);

		GFXVertex3f( (x-(0.25*size)), (y+(0.125*size)), 0);
		GFXVertex3f( x, float(y-(0.125*size)), 0);

		GFXVertex3f( (x+(0.25*size)), (y+(0.125*size)), 0);
		GFXVertex3f( x, float(y-(0.125*size)), 0);
	GFXEnd();

	GFXEnable(TEXTURE0);
}
//	**********************************








//	This will draw a square set of corners
//	**********************************
void NavigationSystem::DrawTargetCorners(float x, float y, float size, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);

	GFXBegin(GFXLINE);

		GFXVertex3f( (x-(0.5*size)), (y+(0.5*size)), 0);
		GFXVertex3f( (x-(0.3*size)), (y+(0.5*size)), 0);

		GFXVertex3f( (x-(0.5*size)), (y+(0.5*size)), 0);
		GFXVertex3f( (x-(0.5*size)), (y+(0.3*size)), 0);

		GFXVertex3f( (x+(0.5*size)), (y+(0.5*size)), 0);
		GFXVertex3f( (x+(0.3*size)), (y+(0.5*size)), 0);

		GFXVertex3f( (x+(0.5*size)), (y+(0.5*size)), 0);
		GFXVertex3f( (x+(0.5*size)), (y+(0.3*size)), 0);

		GFXVertex3f( (x-(0.5*size)), (y-(0.5*size)), 0);
		GFXVertex3f( (x-(0.3*size)), (y-(0.5*size)), 0);

		GFXVertex3f( (x-(0.5*size)), (y-(0.5*size)), 0);
		GFXVertex3f( (x-(0.5*size)), (y-(0.3*size)), 0);

		GFXVertex3f( (x+(0.5*size)), (y-(0.5*size)), 0);
		GFXVertex3f( (x+(0.3*size)), (y-(0.5*size)), 0);

		GFXVertex3f( (x+(0.5*size)), (y-(0.5*size)), 0);
		GFXVertex3f( (x+(0.5*size)), (y-(0.3*size)), 0);

	GFXEnd();
	GFXEnable(TEXTURE0);
}
//	**********************************




//	This will draw an oriented circle
//	**********************************
void NavigationSystem::DrawNavCircle(float x, float y, float size, float rot_x, float rot_y, const GFXColor &col)
{
	GFXColorf(col);
	GFXDisable(TEXTURE0);
	GFXDisable(LIGHTING);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);


	float alpha = 0.15;
	int circles = 4;

	float i = 0.0;
	GFXBegin(GFXLINE);
	float segments = 20.0;

	for(i = 0; i < 6.28; i+=(6.28/segments))
	{
		QVector pos1((0.6*size*cos(i)) ,(0.6*size*sin(i)),0);
		QVector pos2((0.6*size*cos(i+(6.28/segments))) ,(0.6*size*sin(i+(6.28/segments))),0);
		pos1 = dxyz(pos1, 0, 0, rot_y);
		pos1 = dxyz(pos1, rot_x, 0, 0);

		pos2 = dxyz(pos2, 0, 0, rot_y);
		pos2 = dxyz(pos2, rot_x, 0, 0);

		float standard_unit = 0.25 * 1.2 * size;
		float zdistance1 = ((1.2*size)-pos1.k);
		float zdistance2 = ((1.2*size)-pos2.k);
		float zscale1 = standard_unit/zdistance1;
		float zscale2 = standard_unit/zdistance2;
		pos1 *= (zscale1*5.0);
		pos2 *= (zscale2*5.0);
		//pos1 *= (-0.6*size)/((pos1.k) + (0.6*size));
		//pos2 *= (-0.6*size)/((pos2.k) + (0.6*size));

		
		GFXColorf(GFXColor( col.r, col.g, ( col.b*fabs(sin(i/2.0)) ), col.a));

		for(int j=circles; j> 0; j--)
		{
			pos1 *= (float(j)/float(circles));
			pos2 *= (float(j)/float(circles));

			QVector pos1t( (x + pos1.i), (y + (pos1.j)), 0);
			QVector pos2t( (x + pos2.i), (y + (pos2.j)), 0);

			GFXVertex3f( pos1t.i, pos1t.j, 0.0);
			GFXVertex3f( pos2t.i, pos2t.j, 0.0);
		}
	}


	for(i = 0; i < 6.28; i+=(6.28/12.0))
	{
		QVector pos1( (0.6*size*cos(i)/float(circles*2)), (0.6*size*sin(i)/float(circles*2)), 0);
		QVector pos2( (0.6*size*cos(i)), (0.6*size*sin(i)), 0);

		if( (fabs(i-1.57) < 0.01) || (fabs(i-3.14) < 0.01) || (fabs(i-4.71) < 0.01) || (i < 0.01) )
			pos2 *= 1.1;

		pos1 = dxyz(pos1, 0, 0, rot_y);
		pos1 = dxyz(pos1, rot_x, 0, 0);

		pos2 = dxyz(pos2, 0, 0, rot_y);
		pos2 = dxyz(pos2, rot_x, 0, 0);

		float standard_unit = 0.25 * 1.2 * size;
		float zdistance1 = ((1.2*size)-pos1.k);
		float zdistance2 = ((1.2*size)-pos2.k);
		float zscale1 = standard_unit/zdistance1;
		float zscale2 = standard_unit/zdistance2;
		pos1 *= (zscale1*5.0);
		pos2 *= (zscale2*5.0);
//		pos1 *= (pos1.k + size)/size;
//		pos2 *= (pos2.k + size)/size;

//		pos1.j *= 1.33333;
//		pos2.j *= 1.33333;

		pos1.i += x;
		pos1.j += y;
		pos2.i += x;
		pos2.j += y;

		GFXColorf(GFXColor( col.r, col.g, ( col.b*fabs(sin(i/2.0)) ), col.a));

		GFXVertex3f( pos1.i, pos1.j, 0.0);
		GFXVertex3f( pos2.i, pos2.j, 0.0);
	}

	GFXEnd();
	GFXEnable(TEXTURE0);


}
//	**********************************

