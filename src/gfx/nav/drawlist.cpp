#include "gui/glut_support.h"
#include "gfx/hud.h"
#include "cmd/unit_generic.h" 
#include "gui/glut_support.h"
#include "universe_util.h"


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
	n_contents = 0;
	screenoccupation = screenoccupation_;
	localcolours = 0;
	factioncolours = factioncolours_;
	unselectedalpha = 0.8;
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




void navdrawlist::drawdescription(Unit* source, float x_, float y_, float size_x, float size_y, bool ignore_occupied_areas, const GFXColor &col)	//	take the head and stick it in the back
{
	if(source == NULL)
		return;

	
	if(source->name.size() == 0)
		return;

	TextPlane displayname;	//	will be used to display shits names


	displayname.col = col;


	int length = source->name.size();
	float offset = (float(length)*0.005);


	if(ignore_occupied_areas)
	{
		displayname.SetPos((x_-offset), y_);
		displayname.SetText(source->name);
		displayname.SetCharSize(size_x, size_y);
		displayname.Draw();
	}

	else
	{
		float new_y = screenoccupation->findfreesector(x_, y_);
		displayname.SetPos((x_-offset), new_y);
		displayname.SetText(source->name);
		displayname.SetCharSize(size_x, size_y);
		displayname.Draw();
	}
}





void navdrawlist::drawdescription(string text, float x_, float y_, float size_x, float size_y, bool ignore_occupied_areas, const GFXColor &col)	//	take the head and stick it in the back
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
		displayname.Draw();
	}

	else
	{
		float new_y = screenoccupation->findfreesector(x_, y_);
		displayname.SetPos((x_-offset), new_y);
		displayname.SetText(text);
		displayname.SetCharSize(size_x, size_y);
		displayname.Draw();
	}
}











Unit* navdrawlist::gettailunit()
{
	return tail->source;
}









void navdrawlist::draw()	//	Draw the items in the list
{
	if(head == NULL)
		return;


	else
	{
		navdrawnode* current = head;
		float relation = 0.0;


			while(current != NULL)
			{

//				if(current->source != NULL)
//					relation = 	FactionUtil::GetIntRelation( ( UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()) )->faction ,current->source->faction);
//				else
//					relation = 0;


				//	the realtime relationship
				if(current->source != NULL)
					relation = current->source->getRelation( UniverseUtil::getPlayerX(UniverseUtil::getCurrentPlayer()) ) ;

				else
					relation = 0;



				relation = relation * 0.5;
				relation = relation + 0.5;





	//			GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),1)


				if(current->type == navsun)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,1,.7,1));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
					}

					else
					{
						if(current == tail)
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}



				else if (current->type == navplanet)
				{

					if(!inmouserange)
					{
						NavigationSystem::DrawPlanet(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
					}

					else
					{
						if(current == tail)
							NavigationSystem::DrawPlanet(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawPlanet(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}



				else if (current->type == navcurrentplayer)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(.3,.3,1,.8));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.3, .3, 1, unselectedalpha));
					}

					else
					{
						if(current == tail)
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}



				else if (current->type == navplayer)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(.3,.3,1,.8));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.3, .3, .8, unselectedalpha));
					}

					else
					{
						if(current == tail)
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}


				else if (current->type == navstation)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawStation(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
					}

					else
					{
						if(current == tail)
							NavigationSystem::DrawStation(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawStation(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}



				else if (current->type == navfighter)
				{
					if(!inmouserange)
					{
						if(factioncolours == NULL)
						{
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7));
							drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
						}
						else
						{
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7));
							GFXColor thecolor = factioncolours[current->source->faction];
							thecolor.a = unselectedalpha;
							drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, thecolor);
						}
					}
					else
					{
						if(current == tail)
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawHalfCircleTop(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}


				else if (current->type == navcapship)
				{
					if(!inmouserange)
					{
						if(factioncolours == NULL)
						{
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7));
							drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
						}
						else
						{
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7));
							GFXColor thecolor = factioncolours[current->source->faction];
							thecolor.a = unselectedalpha;
							drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, thecolor);
						}
					}
					else
					{
						if(current == tail)
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}


				else if (current->type == navmissile)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawMissile(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.7));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
					}
					else
					{
						if(current == tail)
							NavigationSystem::DrawMissile(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawMissile(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}


				else if (current->type == navasteroid)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,.8,.8,.6));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
					}
					else
					{
						if(current == tail)
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}


				else if (current->type == navnebula)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,.5,1,.6));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
					}
					else
					{
						if(current == tail)
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}


				else if (current->type == navjump)
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawJump(current->x, current->y, current->size, GFXColor(.5, .9, .9, .6));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.3, 1, .8, unselectedalpha));
					}
					else
					{
						if(current == tail)
							NavigationSystem::DrawJump(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawJump(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}


				else
				{
					if(!inmouserange)
					{
						NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor((1.0-relation),relation,(1.0-(2.0*Delta(relation, 0.5))),.6));
						drawdescription(current->source, current->x, current->y, 1.0, 1.0, 0, GFXColor(.2, 1, .5, unselectedalpha));
					}
					else
					{
						if(current == tail)
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,.3,.3,.8));
						else
							NavigationSystem::DrawCircle(current->x, current->y, current->size, GFXColor(1,1,1,.8));
					}
				}



				//	SHOW THE NAME ALL BIG AND SHIT
				if((current == tail)&&(inmouserange == 1))
				{
					//	DISPLAY THE NAME
					drawdescription(current->source, current->x, current->y, 2.0, 2.0, 0, GFXColor(1, 1, .7, 1));
				}
				current = current->nextitem;
			}
	}
}

