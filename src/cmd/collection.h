
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
				virtual void remove();
				void moveBefore(UnitCollection&);
				virtual void preinsert(class Unit*);
				virtual void postinsert(class Unit *unit);
				virtual void advance();
				Unit* next();
				
//				virtual inline class Unit* current() { return(this->operator*());}
				UnitIterator& operator=( const UnitIterator&);
				inline const UnitIterator operator ++(int)
				{	
					UnitCollection::UnitIterator tmp(*this);
					advance();
					return(tmp);
				}
				inline const UnitIterator& operator ++() 
				{
					advance();
					return(*this);
				}
				virtual inline class Unit * operator * ()
				{
					if(col && it != col->u.end())
						return(*it);
					return(NULL);
				}
				
				
			protected:
				friend class UnitCollection;
				UnitCollection *col;
				std::list<class Unit*>::iterator it;

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
				inline const class Unit * operator * () const { if(it!=col->u.end()&&!col->empty())return(*it);return(NULL);}
			protected:
				friend class UnitCollection;
				const UnitCollection *col;
				std::list<class Unit*>::const_iterator it;
		};
		typedef ConstIterator ConstFastIterator;
		typedef UnitIterator FastIterator;

		UnitCollection();
		UnitCollection (const UnitCollection&);
		~UnitCollection();
		inline UnitIterator createIterator() { return(UnitIterator(this)); }
		inline FastIterator fastIterator() { return(FastIterator(this)); }
		inline ConstIterator constIterator() const { return(ConstIterator(this));}
		inline ConstFastIterator constFastIterator() const { return(ConstFastIterator(this));}

		void insert_unique(Unit*);
		inline bool empty() const { if(u.size() - removedIters.size() > 0) return false;return true; }
		void prepend(Unit*);
		void prepend(UnitIterator*);
		void append(class Unit*);
		void append(UnitIterator*);
		std::list<Unit*>::iterator insert(std::list<Unit*>::iterator, Unit*);
		void clear();
		bool contains(const class Unit*) const;
		std::list<class Unit*>::iterator erase(std::list<class Unit*>::iterator);
		bool remove(const class Unit*);
		inline const int size() const { return(u.size());}
		inline Unit* back() 
		{ 
			for(std::list<Unit*>::reverse_iterator it = u.rbegin();it != u.rend();++it){
				if(*it) 
					return(*it);
			} 
			return(NULL);
		}
		inline Unit* front()
		{
			for(std::list<Unit*>::iterator it = u.begin();it!= u.end();++it){
				if(*it)
					return(*it);
			}
			return(NULL);
		}
					
		
		
	private:
		friend class UnitIterator;
		friend class ConstIterator;
		void destr();
		const UnitCollection& operator= (const UnitCollection&);
		void reg(UnitCollection::UnitIterator*);
		void unreg(UnitCollection::UnitIterator*);

		std::vector<class UnitCollection::UnitIterator*> activeIters;
		std::vector<std::list<class Unit*>::iterator> removedIters;
		std::list<class Unit*> u;

};

typedef UnitCollection::UnitIterator un_iter;
typedef UnitCollection::ConstIterator un_kiter;
typedef UnitCollection::UnitIterator un_fiter;
typedef UnitCollection::ConstIterator un_fkiter;
#endif
