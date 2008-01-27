#include "gui/glut_support.h"

#include "gfx/hud.h"

#include "cmd/unit_generic.h" 

#include "gui/glut_support.h"

#include "universe_util.h"

#include "config_xml.h"


#include "drawlist.h"
#include "gfx/masks.h"




navdrawnode::navdrawnode()	//	new undefined node, check for these values if wondering if assignment didnt happen.
{
	type = -1;
	size = 0.0;
	x = 0;
	y = 0;
	nextitem = NULL;
	source = NULL;
}



navdrawnode::navdrawnode(int type_, float size_, float x_, float y_, navdrawnode* nextitem_)	//	new node into list
{
	type = type_;
	size = size_;
	x = x_;
	y = y_;
	nextitem = nextitem_;
	source = NULL;

}








navdrawnode::navdrawnode(int type_, float size_, float x_, float y_, Unit* source_, navdrawnode* nextitem_)	//	new node into list
{
	type = type_;
	size = size_;
	x = x_;
	y = y_;
	nextitem = nextitem_;
	source = source_;
}




navdrawlist::navdrawlist(bool mouse, navscreenoccupied* screenoccupation_, GFXColor* factioncolours_)	//	start list with a 'mouselist' flag
{
	inmouserange = mouse;
	head = NULL;
	tail = NULL;
	unselectedalpha=0.8;
	n_contents = 0;
	screenoccupation = screenoccupation_;
	localcolours = 0;
	factioncolours = factioncolours_;
}



navdrawlist::~navdrawlist()	//	destroy list
{
	inmouserange = 0;
	wipe();
	head = NULL;
	tail = NULL;
	n_contents = 0;
}





























int navdrawlist::get_n_contents()	//	return the amount of items in the list
{
	return n_contents;
}








void navdrawlist::insert(int type, float size, float x, float y)	//	insert iteam at head of list
{


	if(head == NULL)
	{
		head = new navdrawnode(type, size, x, y, NULL);
		tail = head;
	}


	else
	{
		head = new navdrawnode(type, size, x, y, head);
	}



	n_contents += 1;
}





void navdrawlist::insert(int type, float size, float x, float y, Unit* source)	//	insert iteam at head of list
{


	if(head == NULL)
	{
		head = new navdrawnode(type, size, x, y, source, NULL);
		tail = head;
	}


	else
	{
		head = new navdrawnode(type, size, x, y, source, head);
	}



	n_contents += 1;
}








void navdrawlist::wipe()	//	whipe the list clean
{
	navdrawnode* temp = head;
	navdrawnode* tempdelete = NULL;

	while(temp != NULL)
	{
		tempdelete = temp;
		temp = temp->nextitem;
		delete tempdelete;
	}



	head = NULL;
	n_contents = 0;
}








void navdrawlist::rotate()	//	take the head and stick it in the back
{
	if(head != NULL)				//	|
	{								//	|
		if(head->nextitem != NULL)	//	there is something there, and its not alone
		{
			tail->nextitem = head;
			tail = head;
			head = head->nextitem;
			tail->nextitem = NULL;
		}
	}
}







string getUnitNameAndFgNoBase(Unit * target);
void drawdescription(Unit* source, float x_, float y_, float size_x, float size_y, bool ignore_occupied_areas, navscreenoccupied* screenoccupation, const GFXColor &col)	//	take the head and stick it in the back
{
	if(source == NULL)
		return;

	drawdescription(getUnitNameAndFgNoBase(source),x_,y_,size_x,size_y,ignore_occupied_areas,screenoccupation,col);
	
}








void drawdescription (string text, float x_, float y_, float size_x, float size_y, bool ignore_occupied_areas, navscreenoccupied* screenoccupation, const GFXColor &col)	//	take the head and stick it in the back
{
	if(text.size() == 0)
		return;


	TextPlane displayname;	//	will be used to display shits names


	displayname.col = col;



	int length = text.size();
	float offset = (float(length)*0.005);




	if(ignore_occupied_areas)
	{
		displayname.SetPos((x_-offset), y_);
		displayname.SetText(text);
		displayname.SetCharSize(size_x, size_y);
	}


	else
	{
		float new_y = screenoccupation->findfreesector(x_, y_);
		displayname.SetPos((x_-offset), new_y);
		displayname.SetText(text);
		displayname.SetCharSize(size_x, size_y);
	}
	static float background_alpha=XMLSupport::parse_float(vs_config->getVariable("graphics","hud","text_background_alpha","0.0625"));
	  GFXColor tpbg=displayname.bgcol;
	  displayname.bgcol=GFXColor(0,0,0,background_alpha);
	  displayname.Draw();
	  displayname.bgcol=tpbg;
}

















Unit* navdrawlist::gettailunit()
{
	return tail->source;
}




#define INIT_COL_ARRAY(col,r,g,b,a) col[0]=r;col[1]=g;col[2]=b;col[3]=a




static GFXColor getUnitTypeColor (std::string name, bool text, float col[4], float unselectedalpha) {
	vs_config->getColor("nav", (std::string("unhighlighted_")+name)+(text?"_text":""), col, true);
	if (col[3]==0) {
		if (name!="unit"&&col[0]==0&&col[1]==0&&col[2]==0) {
			if (!text) {
				INIT_COL_ARRAY(col,1,1,.7,1);
				return getUnitTypeColor("unit", text, col,unselectedalpha);
			} else {
				INIT_COL_ARRAY(col,.2, 1, .5, 0);
				GFXColor temp=getUnitTypeColor("unit", text, col,unselectedalpha);
				temp.a=unselectedalpha;
				return temp;
			}
		}
		col[3]=unselectedalpha;
	}
	return GFXColor(col[0],col[1],col[2],col[3]);
}



void drawlistitem(int type, float size, float x, float y, Unit* source, navscreenoccupied* screenoccupation, bool inmouserange, bool currentistail, float unselectedalpha, GFXColor* factioncolours) {
	float relation = 0.0;
	static GFXColor unhighlighted_sun_col;
	static GFXColor unhighlighted_sun_text;
	static GFXColor unhighlighted_planet_text;
	static GFXColor unhighlighted_c_player_col;
	static GFXColor unhighlighted_c_player_text;
	static GFXColor unhighlighted_player_col;
	static GFXColor unhighlighted_player_text;
	static GFXColor unhighlighted_asteroid_col;
	static GFXColor unhighlighted_asteroid_text;
	static GFXColor unhighlighted_nebula_col;
	static GFXColor unhighlighted_nebula_text;
	static GFXColor unhighlighted_jump_col;
	static GFXColor unhighlighted_jump_text;
	static GFXColor unhighlighted_station_text;
	static GFXColor unhighlighted_fighter_text;
	static GFXColor unhighlighted_unit_text;
	static GFXColor highlighted_tail_col;
	static GFXColor highlighted_tail_text;
	static GFXColor highlighted_untail_col;
	static GFXColor unhighlighted_capship_text;
	static bool init = false;
	if (!init) {
		// Get a color from the config
		float col[4];

		INIT_COL_ARRAY(col,1,.3,.3,.8);
		vs_config->getColor("nav", "highlighted_unit_on_tail", col, true);
		highlighted_tail_col=GFXColor(col[0],col[1],col[2],col[3]);

		INIT_COL_ARRAY(col,1,1,.7,1);
		vs_config->getColor("nav", "highlighted_text_on_tail", col, true);
		highlighted_tail_text=GFXColor(col[0],col[1],col[2],col[3]);

		INIT_COL_ARRAY(col,1,1,1,.8);
		vs_config->getColor("nav", "highlighted_unit_off_tail", col, true);
		highlighted_untail_col=GFXColor(col[0],col[1],col[2],col[3]);

		INIT_COL_ARRAY(col,0,0,0,0); // If not found, use defaults
		unhighlighted_sun_col=getUnitTypeColor("sun", false, col,unselectedalpha);
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_sun_text=getUnitTypeColor("sun", true, col,unselectedalpha);
				
		// Planet color is the relation color, so is not defined here.
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_planet_text=getUnitTypeColor("planet", true, col,unselectedalpha);

		INIT_COL_ARRAY(col,.3,.3,1,.8); // If not found, use defaults
		unhighlighted_c_player_col=getUnitTypeColor("curplayer", false, col,.8);
		INIT_COL_ARRAY(col,.3,.3,1,0);
		unhighlighted_c_player_text=getUnitTypeColor("curplayer", true, col,unselectedalpha);

		INIT_COL_ARRAY(col,.3,.3,1,.8); // If not found, use defaults
		unhighlighted_player_col=getUnitTypeColor("player", false, col,.8);
		INIT_COL_ARRAY(col,.3,.3,1,0);
		unhighlighted_player_text=getUnitTypeColor("player", true, col,unselectedalpha);

		INIT_COL_ARRAY(col,.3,.3,1,.8); // If not found, use defaults
		unhighlighted_player_col=getUnitTypeColor("player", false, col,.8);
		INIT_COL_ARRAY(col,.3,.3,1,0);
		unhighlighted_player_text=getUnitTypeColor("player", true, col,unselectedalpha);

		INIT_COL_ARRAY(col,1,.8,.8,.6); // If not found, use defaults
		unhighlighted_asteroid_col=getUnitTypeColor("asteroid", false, col,.6);
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_asteroid_text=getUnitTypeColor("asteroid", true, col,unselectedalpha);

		INIT_COL_ARRAY(col,1,.5,1,.6); // If not found, use defaults
		unhighlighted_nebula_col=getUnitTypeColor("nebula", false, col,.6);
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_nebula_text=getUnitTypeColor("nebula", true, col,unselectedalpha);

		INIT_COL_ARRAY(col,.5, .9, .9, .6); // If not found, use defaults
		unhighlighted_jump_col=getUnitTypeColor("jump", false, col,.6);
		INIT_COL_ARRAY(col,.3, 1, .8,0);
		unhighlighted_jump_text=getUnitTypeColor("jump", true, col,unselectedalpha);

				
		// Basic unit types:
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_station_text=getUnitTypeColor("station", true, col,unselectedalpha);
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_fighter_text=getUnitTypeColor("fighter", true, col,unselectedalpha);
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_capship_text=getUnitTypeColor("capship", true, col,unselectedalpha);
		INIT_COL_ARRAY(col,0,0,0,0);
		unhighlighted_unit_text=getUnitTypeColor("unit", true, col,unselectedalpha);

		init=true;
	}

//	if(source != NULL)
//		relation = 	FactionUtil::GetIntRelation( ( UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()) )->faction ,source->faction);
//	else
//		relation = 0;




	//	the realtime relationship
	if(source != NULL)
		relation = source->getRelation( UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()) ) ;
	else
		relation = 0;



	relation = relation * 0.5;
	relation = relation + 0.5;



// to avoid duplicate code
	GFXColor relColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7);




//	GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),1)




	if(type == navsun)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawCircle(x, y, size, unhighlighted_sun_col);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_sun_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
		}
	}





	else if (type == navplanet)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawPlanet(x, y, size, relColor);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_planet_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawPlanet(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawPlanet(x, y, size, highlighted_untail_col);
		}
	}




	else if (type == navcurrentplayer)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawHalfCircleTop(x, y, size, unhighlighted_c_player_col);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_c_player_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_tail_col);
			else
				
				NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_untail_col);
		}
	}
	

	

	else if (type == navplayer)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawHalfCircleTop(x, y, size, unhighlighted_player_col);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_player_col);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_untail_col);
		}
	}



	
	else if (type == navstation)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawStation(x, y, size, relColor);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_station_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawStation(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawStation(x, y, size, highlighted_untail_col);
		}
	}




	else if (type == navfighter)
	{
		if(!inmouserange)
		{
			if(factioncolours == NULL)
			{
				NavigationSystem::DrawHalfCircleTop(x, y, size, relColor);
				drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_fighter_text);
			}
			else
			{
				NavigationSystem::DrawHalfCircleTop(x, y, size, relColor);
				GFXColor thecolor = factioncolours[source->faction];
				thecolor.a = unselectedalpha;
				drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, thecolor);
			}
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawHalfCircleTop(x, y, size, highlighted_untail_col);
		}
	}
	


	else if (type == navcapship)
	{
		if(!inmouserange)
		{
			if(factioncolours == NULL)
			{
				NavigationSystem::DrawCircle(x, y, size, relColor);
				drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_capship_text);
			}
			else
			{
				NavigationSystem::DrawCircle(x, y, size, relColor);
				GFXColor thecolor = factioncolours[source->faction];
				
				thecolor.a = unselectedalpha;
				
				drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, thecolor);
			}
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
		}
	}



	else if (type == navmissile)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawMissile(x, y, size, relColor);
//			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, GFXColor(.2, 1, .5, unselectedalpha));
//			NOT DRAWING NAME OF MISSILE TO MAKE ROOM FOR IMPORTANT TEXT ON SCREEN
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawMissile(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawMissile(x, y, size, highlighted_untail_col);
		}
	}




	else if (type == navasteroid)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawCircle(x, y, size, unhighlighted_asteroid_col);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_asteroid_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
		}
	}
	


	else if (type == navnebula)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawCircle(x, y, size, unhighlighted_nebula_col);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_nebula_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
		}
	}




	else if (type == navjump)
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawJump(x, y, size, unhighlighted_jump_col);
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_jump_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawJump(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawJump(x, y, size, highlighted_untail_col);
		}
	}



	else
	{
		if(!inmouserange)
		{
			NavigationSystem::DrawCircle(x, y, size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.6));
			drawdescription(source, x, y, 1.0, 1.0, false, screenoccupation, unhighlighted_unit_text);
		}
		else
		{
			if(currentistail)
				NavigationSystem::DrawCircle(x, y, size, highlighted_tail_col);
			else
				NavigationSystem::DrawCircle(x, y, size, highlighted_untail_col);
		}
	}





	//	SHOW THE NAME ALL BIG AND SHIT
	if((currentistail)&&(inmouserange == 1))
	{
		//	DISPLAY THE NAME
		drawdescription(source, x, y, 2.0, 2.0, false, screenoccupation, GFXColor(1, 1, .7, 1));
	}
}



void navdrawlist::draw()	//	Draw the items in the list
{
	if(head == NULL)
		return;




	else
	{
			navdrawnode* current = head;


			while(current != NULL)
			{

				drawlistitem(current->type, current->size, current->x, current->y, current->source, screenoccupation, inmouserange, current==tail, unselectedalpha, factioncolours);
				
				current = current->nextitem;
			}
	}
}


