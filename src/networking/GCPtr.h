#ifndef OS_GC_PTR_H
#define OS_GC_PTR_H

#include <stdio.h>
#include <assert.h>

/** A generic smart pointer that allows several references without
 *  copies that handles automatic deletion.
 *  It is a kind of smart pointer that allows an unlimited number
 *  of pointer to the same object with auto-deletion when all
 *  pointers are forgotten.
 *  The deletion takes place immediately when no pointer refers to
 *  the object any more. It does not run in independent cycles like
 *  a garbage collector.
 *  If the user wants an actual copy, an explicit copy of the object
 *  must be made through the copy constructor or a specialized clone
 *  function.
 *  This template has the following differences compared to the specialized
 *  SH::RPtr template:
 *  - The implicit deletion uses the delete operator instead of a
 *    specific deletion function of the object.
 *  - This template does not work with upcasts and downcasts but only
 *    for the single data type for which it is instantiated.
 */
template <class T>
class GCPtr
{
public:
    GCPtr( )
        : _pointee( NULL )
        , _copies( NULL )
    {
    }

    /** This constructor swallows an externally provided pointer
     *  and consumes it. Since the number of copies maintained by
     *  this smart pointer is set to 1, deletion of this smart
     *  pointer will trigger a call to the release function of
     *  the pointee. This means that the pointer that is provided
     *  for constructing this object would become invalid at the
     *  latest when the last GCPtr is deleted. To make this
     *  slightly more deterministic, the pointee that is used for
     *  the construction of the smart pointer is deleted immediately.
     */
    explicit GCPtr( T* pointee )
        : _pointee( pointee )
        , _copies( NULL )
    {
        if( pointee != NULL )
        {
            _copies = new size_t;
            *_copies = 1;
        }
    }

    GCPtr( const GCPtr& other )
        : _pointee( NULL )
        , _copies( NULL )
    {
        copy( other );
    }

    GCPtr& operator=( const GCPtr& other )
    {
        reduce( );
        copy( other );
        return *this;
    }

    GCPtr& operator=( T* pointee )
    {
        reduce( );
	_pointee = pointee;
        if( pointee != NULL )
        {
            _copies = new size_t;
            *_copies = 1;
        }
        return *this;
    }

    ~GCPtr( )
    {
        reduce( );
    }

    T& operator*( ) const
    {
        return *_pointee;
    }

    T* operator->( ) const
    {
        return _pointee;
    }

    inline bool isNull( ) const {
        assert( isConsistent() );
	return ( _pointee == NULL );
    }

private:
    bool operator==( T* pointee )
    {
        return ( _pointee == pointee );
    }

    bool operator!=( T* pointee )
    {
        return ( _pointee != pointee );
    }

    bool operator==( const GCPtr& other )
    {
        return ( _pointee == other._pointee );
    }

    bool operator!=( const GCPtr& other )
    {
        return ( _pointee != other._pointee );
    }

    bool isConsistent( ) const
    {
        if( _pointee == NULL && _copies == NULL ) return true;
        if( _pointee != NULL && _copies != NULL ) return true;
	return false;
    }

private:
    T*              _pointee;
    mutable size_t* _copies;

private:
    /** This function allows swallowing of report pointers that
     *  are through upcast if they are of the correct type.
     *  This should be simple, but we don't have rtti, typeid
     *  or dynamic_cast, so, in fact, it isn't.
     */
    void copy( const GCPtr& other )
    {
       _pointee = other._pointee;
       _copies  = other._copies;
       if( _copies )
       {
           *_copies += 1;
       }
    }

    /** If this pointer points to something, its _copies member
     *  variable is non-NULL and holds a value larger than 0.
     *  In that case, reduce _copies. If the value becomes 0,
     *  delete _copies, release the actual datum, reset both
     *  pointers to NULL.
     */
    void reduce( )
    {
        assert( isConsistent() );
        if( _copies )
        {
	    assert( *_copies > 0 );
            *_copies -= 1;
            if( *_copies == 0 )
            {
	        delete _pointee;
	        delete _copies;
		_copies = NULL;
		_pointee = NULL;
            }
        }
	else
	{
	    _pointee = NULL;
	}
    }
};

#endif /* OS_GC_PTR_H */

