//	This draws the mouse cursor
//	**********************************
void NavigationSystem::DrawCursor (float x, float y, float wid, float hei, const GFXColor &col) {

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
	GFXEnd();

	GFXBegin(GFXLINE);
	for(i=1; i < 10;i++)
	{
		GFXVertex3f(x1,y1+(i*deltax),0);
		GFXVertex3f(x2,y1+(i*deltax),0);
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
	float j = 0.0;
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
	float j = 0.0;
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
	float j = 0.0;
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
	float j = 0.0;
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
	float j = 0.0;
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
