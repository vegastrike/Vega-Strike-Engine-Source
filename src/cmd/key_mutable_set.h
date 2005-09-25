#include <set>
#include <assert.h>
template <class T> class MutableShell {
	mutable T t;
public:
	MutableShell (const T &t) : t(t) {}
	T &get ()const {return t;}
	T &operator* ()const {return t;}
	T *operator-> ()const {return &t;}
	operator T& () const {return t;}
	bool operator< (const MutableShell <T>& other) const {return t<other.t;}
};

///This set inherits from the STL multiset, with a slight variation: The value is nonconst--that means you are allowed to change things but must not alter the key.
///This set inherits from the STL multiset, with a slight variation: You are allowed to update the key of a particular iterator that you have obtained.
/** Note: T is the type that each element is pointing to. */
template <class T, class _Compare = std::less <MutableShell<T> > >
class KeyMutableSet : public std::multiset <MutableShell<T>,_Compare> {
	typedef std::multiset <MutableShell<T>,_Compare> SUPER;
public:
	/// This just checks the order of the set for testing purposes..
	void checkSet () {
		_Compare comparator;
		if (this->begin()!=this->end()) {
			for (typename SUPER::iterator newiter=this->begin(), iter=newiter++;newiter!=this->end();iter=newiter++) {
                          assert(!comparator(*newiter,*iter));
			}
		}
	}
	///Given an iterator you can alter that iterator's key to be the one passed in.
	/** The type must have a function called changeKey(const Key &newKey) that
		changes its key to the specified new key.
	 */
	void changeKey (typename SUPER::iterator &iter, const T & newKey, typename SUPER::iterator &templess, typename SUPER::iterator &rettempmore) {
		MutableShell<T> newKeyShell(newKey);
		templess=tempmore=iter;
		++rettempmore;
                typename SUPER::iterator tempmore=rettempmore;
                if (tempmore==this->end())
                  --tempmore;
		if (templess!=this->begin())
                  --templess;
		_Compare comparator;
		if (comparator(newKeyShell,*templess)||comparator(*tempmore,newKeyShell)) {
                  this->erase(iter);
                  rettempmore=templess=iter=this->insert (newKeyShell);
                  ++rettempmore;
                  if (templess!=this->begin())
                    --templess;                  
		}else {
                  (*iter).get()=newKey;
		}
		return iter;
	}

	typename SUPER::iterator changeKey (typename SUPER::iterator iter, const T & newKey) {
		typename SUPER::iterator templess=iter,tempmore=iter;
		changeKey(iter,newKey,templess,tempmore);
                return iter;
	}
};
