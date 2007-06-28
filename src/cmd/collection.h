
/* unitCollection.h
 *
 *****/

#ifndef _UNITCOLLECTION_H_
#define _UNITCOLLECTION_H_
#include <list>
#include <vector>

class Unit;

class UnitCollection
{
	public:

		class UnitIterator
		{
			public:
				UnitIterator() : col(NULL) {;}
				UnitIterator( const UnitIterator&);
				UnitIterator(UnitCollection*);
				virtual ~UnitIterator();
				bool isDone();
				bool notDone();
				//removes current unit.
				virtual void remove();
				//moves current unit to front of list passed
				void moveBefore(UnitCollection&);
				//inserts in front of current
				virtual void preinsert(class Unit*);
				// inserts after current
				virtual void postinsert(class Unit *unit);
				//advances the counter
				virtual void advance();
				//Returns next unit in list
				Unit* next();

				UnitIterator& operator=( const UnitIterator&);
				const UnitIterator operator ++(int);
				const UnitIterator& operator ++();
				class Unit * operator * ();
				std::list<class Unit*>::iterator it;
			protected:
				// Pointer to list
				UnitCollection *col;
				// Current position in list

		};

		class ConstIterator
		{
			public:
				ConstIterator() : col(NULL) {;}
				ConstIterator( const ConstIterator&);
				ConstIterator( const UnitCollection*);
				virtual ~ConstIterator( );
				ConstIterator& operator=( const ConstIterator& orig );
				const Unit * next();
				bool isDone();
				bool notDone();
				virtual void advance();
				const ConstIterator& operator ++();
				const ConstIterator operator ++(int);
				const class Unit * operator * () const { if(it!=col->u.end()&&!col->empty())return(*it);return(NULL);}
			protected:
				const UnitCollection *col;
				std::list<class Unit*>::const_iterator it;
		};
		// For backwards compatibility: no difference between fast and regular
		typedef ConstIterator ConstFastIterator;
		typedef UnitIterator FastIterator;

		UnitCollection();
		UnitCollection (const UnitCollection&);
		~UnitCollection();
		// Returns iterators of list in "this" object
		UnitIterator createIterator();
		FastIterator fastIterator();
		ConstIterator constIterator() const { return(ConstIterator(this));}
		ConstFastIterator constFastIterator() const { return(ConstFastIterator(this));}

		// prepend unit to beginning of list if not in list
		void insert_unique(Unit*);

		// Check if list is empty
		bool empty() const { return(u.empty()); }
		// adds unit to beginning of list
		void prepend(Unit*);
		// adds remainder of list pointed to by UnitIterator to start of this list
		void prepend(UnitIterator*);
		// adds unit to end of list
		void append(class Unit*);
		// adds remainder of list pointed to by UnitIterator to end of this list
		void append(UnitIterator*);
		// Inserts elements into list anywhere.
		std::list<Unit*>::iterator insert(std::list<Unit*>::iterator, Unit*);
		// removes elements from list, beginning state
		void clear();
		// checks if a unit is in the list
		bool contains(const class Unit*) const;
		// unreferences units before removing them
		std::list<class Unit*>::iterator erase(std::list<class Unit*>::iterator);
		// removes each instance of unit in list, unref'ing each
		bool remove(const class Unit*);
		// Copy all units in other list, killed and otherwise
		const UnitCollection& operator= (const UnitCollection&);
		// traverses list and removes invalid units
		void cleanup();

		const int size() const { return(u.size());}
		std::list<class Unit*> u;
		void reg(UnitCollection::UnitIterator*);
		void unreg(UnitCollection::UnitIterator*);
		
	private:
		// Removes elements from list, beginning state
		void destr();
		std::vector<class UnitCollection::UnitIterator*> activeIters;
};

typedef UnitCollection::UnitIterator un_iter;
typedef UnitCollection::ConstIterator un_kiter;
typedef UnitCollection::UnitIterator un_fiter;
typedef UnitCollection::ConstIterator un_fkiter;
#endif
