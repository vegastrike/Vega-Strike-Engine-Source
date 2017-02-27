//
// Boost.Pointer Container
//
//  Copyright Thorsten Ottosen 2003-2005. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/ptr_container/
//


#ifndef BOOST_PTR_CONTAINER_DETAIL_ASSOCIATIVE_PTR_CONTAINER_HPP
#define BOOST_PTR_CONTAINER_DETAIL_ASSOCIATIVE_PTR_CONTAINER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <boost/ptr_container/detail/reversible_ptr_container.hpp>

namespace boost
{

namespace ptr_container_detail
{
    template
    <
        class Config,
        class CloneAllocator
    >
    class associative_ptr_container :
        public reversible_ptr_container<Config,CloneAllocator>
    {
        typedef reversible_ptr_container<Config,CloneAllocator>
                                base_type;

        typedef BOOST_DEDUCED_TYPENAME base_type::scoped_deleter
                                scoped_deleter;

    public: // typedefs
        typedef BOOST_DEDUCED_TYPENAME Config::key_type
                                key_type;
        typedef BOOST_DEDUCED_TYPENAME Config::key_compare
                                key_compare;
        typedef BOOST_DEDUCED_TYPENAME Config::value_compare
                                value_compare;
        typedef BOOST_DEDUCED_TYPENAME Config::iterator
                                iterator;
        typedef BOOST_DEDUCED_TYPENAME Config::const_iterator
                                const_iterator;
        typedef BOOST_DEDUCED_TYPENAME base_type::size_type
                                size_type;

    public: // foundation

       template< class Compare, class Allocator >
       associative_ptr_container( const Compare& comp,
                                  const Allocator& a )
         : base_type( comp, a )
       { }
                                                     
       template< class InputIterator, class Compare, class Allocator >
       associative_ptr_container( InputIterator first, InputIterator last,
                                  const Compare& comp,
                                  const Allocator& a )
         : base_type( first, last, comp, a )
       { }

       template< class PtrContainer >
       explicit associative_ptr_container( std::auto_ptr<PtrContainer> r )
         : base_type( r, key_compare() )
       { }

       explicit associative_ptr_container( const associative_ptr_container& r )
         : base_type( r.begin(), r.end(), key_compare(), 
                      BOOST_DEDUCED_TYPENAME Config::allocator_type() )
       { }

       template< class C, class V >
       explicit associative_ptr_container( const associative_ptr_container<C,V>& r )
         : base_type( r.begin(), r.end(), key_compare(), 
                      BOOST_DEDUCED_TYPENAME Config::allocator_type() )
       { }

       template< class PtrContainer >
       associative_ptr_container& operator=( std::auto_ptr<PtrContainer> r ) // nothrow
       {
           base_type::operator=( r );
           return *this;
       }

       template< class C, class V >
       associative_ptr_container& operator=( const associative_ptr_container<C,V>& r ) // strong 
       {
           associative_ptr_container clone( r );
           this->swap( clone );
           return *this;   
       }
        
       associative_ptr_container& operator=( const associative_ptr_container& r ) // strong
       {
           associative_ptr_container clone( r );
           this->swap( clone );
           return *this;   
       }

    public: // associative container interface
        key_compare key_comp() const
        {
            return this->base().key_comp();
        }

        value_compare value_comp() const
        {
            return this->base().value_comp();
        }

        iterator erase( iterator before ) // nothrow
        {
            BOOST_ASSERT( !this->empty() );
            BOOST_ASSERT( before != this->end() );

            this->remove( before );                      // nothrow
            iterator res( before );                      // nothrow
            ++res;                                       // nothrow
            this->base().erase( before.base() );         // nothrow
            return res;                                  // nothrow
        }

        size_type erase( const key_type& x ) // nothrow
        {
            iterator i( this->base().find( x ) );       
                                                        // nothrow
            if( i == this->end() )                      // nothrow
                return 0u;                              // nothrow
            this->remove( i );                          // nothrow
            return this->base().erase( x );             // nothrow 
        }

        iterator erase( iterator first,
                        iterator last ) // nothrow
        {
            iterator res( last );                                // nothrow
            if( res != this->end() )
                ++res;                                           // nothrow

            this->remove( first, last );                         // nothrow
            this->base().erase( first.base(), last.base() );     // nothrow
            return res;                                          // nothrow
        }

#if defined(BOOST_NO_SFINAE) || defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)
#else    
        template< class Range >
        BOOST_DEDUCED_TYPENAME boost::disable_if< boost::is_convertible<Range&,key_type&>, 
                                                  iterator >::type
        erase( const Range& r )
        {
            return erase( boost::begin(r), boost::end(r) );
        }

#endif

    protected:

        template< class AssociatePtrCont >
        void multi_transfer( BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator object,
                             AssociatePtrCont& from ) // strong
        {
            BOOST_ASSERT( (void*)&from != (void*)this );
            BOOST_ASSERT( !from.empty() && "Cannot transfer from empty container" );

            this->base().insert( *object.base() );     // strong
            from.base().erase( object.base() );        // nothrow
        }

        template< class AssociatePtrCont >
        size_type multi_transfer( BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator first,
                                  BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator last,
                                  AssociatePtrCont& from ) // basic
        {
            BOOST_ASSERT( (void*)&from != (void*)this );
 
            size_type res = 0;
            for( ; first != last; )
            {
                BOOST_ASSERT( first != from.end() );
                this->base().insert( *first.base() );     // strong
                BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator 
                    to_delete( first );
                ++first;
                from.base().erase( to_delete.base() );    // nothrow
                ++res;
            }

            return res;
        }

        template< class AssociatePtrCont >
        bool single_transfer( BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator object,
                              AssociatePtrCont& from ) // strong
        {
            BOOST_ASSERT( (void*)&from != (void*)this );
            BOOST_ASSERT( !from.empty() && "Cannot transfer from empty container" );

            std::pair<BOOST_DEDUCED_TYPENAME base_type::ptr_iterator,bool> p =
                this->base().insert( *object.base() );     // strong
            if( p.second )
                from.base().erase( object.base() );        // nothrow

            return p.second;
        }

        template< class AssociatePtrCont >
        size_type single_transfer( BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator first,
                                   BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator last,
                                   AssociatePtrCont& from ) // basic
        {
            BOOST_ASSERT( (void*)&from != (void*)this );

            size_type res = 0;
            for( ; first != last; )
            {
                BOOST_ASSERT( first != from.end() );
                std::pair<BOOST_DEDUCED_TYPENAME base_type::ptr_iterator,bool> p =
                    this->base().insert( *first.base() );     // strong
                BOOST_DEDUCED_TYPENAME AssociatePtrCont::iterator 
                    to_delete( first );
                ++first;
                if( p.second )
                {
                    from.base().erase( to_delete.base() );   // nothrow
                    ++res;
                }
            }
            return res;
        }

     }; // class 'associative_ptr_container'
    
} // namespace 'ptr_container_detail'
    
} // namespace 'boost'


#endif
