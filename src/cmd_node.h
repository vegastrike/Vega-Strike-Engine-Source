#include "gfx.h"
#include "cmd_unit.h"

const int numneighbors = 26;

struct UnitList{
	Vector position;
	Unit *ships;
};

class Node{
	Unit *structure;
	UnitList *ships;
	Node *neighbors[26];
	Unit *closestships[26];
	Vector position;

	//const int numneighbors = 26;

public:
	Node(const Vector &newpos, Node *neighbors[numneighbors])
	{
		position = newpos;
		structure = ships = NULL;
		for(int index_a = 0; index_a<numneighbors; index_a++)
			neighbors[index_a] = NULL;
	};

	Node *MoveUnit(Unit *unit, Vector position)
	{
		Node *retval;
		/* 
		 *check if new position is in this node
		 */
		/*If so, retval = this;
		 *else
		 *Remove the unit
		 */
		//ships->RemoveUnit(unit);
		/*
		 *figure out which neighbor to go to and recurse
		 */
		return retval;
	}
	Unit *ClosestAlly(Unit *unit)
	{
		/*Find the closest unit in this node
		 *If one can't be found then go to a neighboring node
		 *How to do this efficiently? maybe store the closest Unit in the neighboring stuffs for this sort of thing.
		 */
	}
	Unit *ClosestAlly(Unit *unit, const Vector &direction)
	{
		/*Find the closest unit in this node going in a certain direction
		 *If one can't be found then go to a neighboring node
		 */
	}

	Unit *ClosestEnemy(Unit *unit)
	{
		/*Find the closest unit in this node going in a certain direction
		 *If one can't be found then go to a neighboring node
		 */
	}

	void DeadUnit(Unit *unit)
	{
		ships->RemoveUnit(unit);
		structure = NULL;
	}
	void DeadStructure()
	{
		structure = NULL;
	}
};