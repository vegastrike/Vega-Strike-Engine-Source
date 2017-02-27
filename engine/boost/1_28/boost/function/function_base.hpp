// Boost.Function library

// Copyright (C) 2001 Doug Gregor (gregod@cs.rpi.edu)
//
// Permission to copy, use, sell and distribute this software is granted
// provided this copyright notice appears in all copies.
// Permission to modify the code and to distribute modified code is granted
// provided this copyright notice appears in all copies, and a notice
// that the code was modified is included with the copyright notice.
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.
 
// For more information, see http://www.boost.org

#ifndef BOOST_FUNCTION_BASE_HEADER
#define BOOST_FUNCTION_BASE_HEADER

#include <string>
#include <stdexcept>
#include <memory>
#include <new>
#include <typeinfo>
#include <boost/config.hpp>
#include <boost/type_traits.hpp>
#include <boost/ref.hpp>

#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300 || defined(__ICL) && __ICL <= 600 || defined(__MWERKS__) && __MWERKS__ < 0x2406
#  define BOOST_FUNCTION_TARGET_FIX(x) x
#else
#  define BOOST_FUNCTION_TARGET_FIX(x)
#endif // not MSVC

namespace boost {
  namespace detail {
    namespace function {
      template<bool> struct truth {};

      /*
       * The ct_if implementation is temporary code. When a Boost metaprogramming
       * library is introduced, Boost.Function will use it instead. 
       */
      namespace intimate {
        struct SelectThen 
        {       
          template<typename Then, typename Else>
          struct Result
          {       
            typedef Then type;
          };
        };
 
        struct SelectElse
        {
          template<typename Then, typename Else>
          struct Result
          { 
            typedef Else type;
          };
        };
 
        template<bool Condition>
        struct Selector
        {
          typedef SelectThen type;
        };
 
        template<>
        struct Selector<false>
        {
          typedef SelectElse type;
        };
      } // end namespace intimate 
 
      template<bool Condition, typename Then, typename Else>
      struct ct_if
      {
        typedef typename intimate::Selector<Condition>::type select;
        typedef typename select::template Result<Then,Else>::type type;
      };

      /**
       * A union of a function pointer and a void pointer. This is necessary
       * because 5.2.10/6 allows reinterpret_cast<> to safely cast between 
       * function pointer types and 5.2.9/10 allows static_cast<> to safely
       * cast between a void pointer and an object pointer. But it is not legal
       * to cast between a function pointer and a void* (in either direction),
       * so function requires a union of the two. */
      union any_pointer 
      {
        void* obj_ptr;
        const void* const_obj_ptr;
        void (*func_ptr)();

        explicit any_pointer(void* p) : obj_ptr(p) {}
        explicit any_pointer(const void* p) : const_obj_ptr(p) {}
        explicit any_pointer(void (*p)()) : func_ptr(p) {}
      };

      /**
       * The unusable class is a placeholder for unused function arguments
       * It is also completely unusable except that it constructable from
       * anything. This helps compilers without partial specialization to
       * handle Boost.Function objects returning void.
       */
      struct unusable 
      {
        unusable() {}
        template<typename T> unusable(const T&) {}
      };

      /* Determine the return type. This supports compilers that do not support
       * void returns or partial specialization by silently changing the return
       * type to "unusable". 
       */
      template<typename T> struct function_return_type { typedef T type; };

      template<> 
      struct function_return_type<void> 
      {
        typedef unusable type;
      };

      // The operation type to perform on the given functor/function pointer
      enum functor_manager_operation_type { 
        clone_functor_tag, 
        destroy_functor_tag
      };

      // Tags used to decide between different types of functions
      struct function_ptr_tag {};
      struct function_obj_tag {};
      struct member_ptr_tag {};
      struct function_obj_ref_tag {};
      struct stateless_function_obj_tag {};

      template<typename F>
      class get_function_tag
      {
        typedef typename ct_if<(is_pointer<F>::value),
                            function_ptr_tag,
                            function_obj_tag>::type ptr_or_obj_tag;

        typedef typename ct_if<(is_member_pointer<F>::value),
                            member_ptr_tag,
                            ptr_or_obj_tag>::type ptr_or_obj_or_mem_tag;

        typedef typename ct_if<(is_reference_wrapper<F>::value),
                             function_obj_ref_tag,
                             ptr_or_obj_or_mem_tag>::type or_ref_tag;

      public:
        typedef typename ct_if<(is_stateless<F>::value),
                            stateless_function_obj_tag,
                            or_ref_tag>::type type;
      };

      // The trivial manager does nothing but return the same pointer (if we
      // are cloning) or return the null pointer (if we are deleting).
      inline any_pointer trivial_manager(any_pointer f, 
					 functor_manager_operation_type op)
      {
        if (op == clone_functor_tag)
          return f;
        else
          return any_pointer(reinterpret_cast<void*>(0));
      }

      /**
       * The functor_manager class contains a static function "manage" which
       * can clone or destroy the given function/function object pointer. 
       */
      template<typename Functor, typename Allocator>
      struct functor_manager
      {
      private:
        typedef Functor functor_type;

        // For function pointers, the manager is trivial
        static inline any_pointer
        manager(any_pointer function_ptr, 
		functor_manager_operation_type op,
                function_ptr_tag)
        {
          if (op == clone_functor_tag)
            return function_ptr;
          else
            return any_pointer(static_cast<void (*)()>(0));
        }

        // For function object pointers, we clone the pointer to each 
        // function has its own version.
        static inline any_pointer
        manager(any_pointer function_obj_ptr, 
                functor_manager_operation_type op,
                function_obj_tag)
        {
#ifndef BOOST_NO_STD_ALLOCATOR
        typedef typename Allocator::template rebind<functor_type>::other 
          allocator_type;
        typedef typename allocator_type::pointer pointer_type;
#else
        typedef functor_type* pointer_type;
#endif // BOOST_NO_STD_ALLOCATOR

#  ifndef BOOST_NO_STD_ALLOCATOR
          allocator_type allocator;
#  endif // BOOST_NO_STD_ALLOCATOR

          if (op == clone_functor_tag) {
            functor_type* f = 
              static_cast<functor_type*>(function_obj_ptr.obj_ptr);

            // Clone the functor
#  ifndef BOOST_NO_STD_ALLOCATOR
            pointer_type copy = allocator.allocate(1);
            allocator.construct(copy, *f);

            // Get back to the original pointer type
            functor_type* new_f = static_cast<functor_type*>(copy);
#  else
            functor_type* new_f = new functor_type(*f);
#  endif // BOOST_NO_STD_ALLOCATOR
            return any_pointer(static_cast<void*>(new_f));
          }
          else {
            /* Cast from the void pointer to the functor pointer type */
            functor_type* f = 
              reinterpret_cast<functor_type*>(function_obj_ptr.obj_ptr);

#  ifndef BOOST_NO_STD_ALLOCATOR
            /* Cast from the functor pointer type to the allocator's pointer
               type */
            pointer_type victim = static_cast<pointer_type>(f);

            // Destroy and deallocate the functor
            allocator.destroy(victim);
            allocator.deallocate(victim, 1);
#  else
            delete f;
#  endif // BOOST_NO_STD_ALLOCATOR

            return any_pointer(static_cast<void*>(0));
          }
        }
      public:
        /* Dispatch to an appropriate manager based on whether we have a
           function pointer or a function object pointer. */
        static any_pointer
        manage(any_pointer functor_ptr, functor_manager_operation_type op)
        {
          typedef typename get_function_tag<functor_type>::type tag_type;
          return manager(functor_ptr, op, tag_type());
        }
      };

      // value=1 if the given type is not "unusable"
      template<typename T>
      struct count_if_used
      {
        BOOST_STATIC_CONSTANT(int, value = 1);
      };
    
      // value=0 for unusable types
      template<>
      struct count_if_used<unusable>
      {
        BOOST_STATIC_CONSTANT(int, value = 0);
      };
    
      // Count the number of arguments (from the given set) which are not 
      // "unusable" (therefore, count those arguments that are used).
      template<typename T1, typename T2, typename T3, typename T4, 
               typename T5, typename T6, typename T7, typename T8, 
               typename T9, typename T10>
      struct count_used_args
      {
        BOOST_STATIC_CONSTANT(int, value = 
                              (count_if_used<T1>::value + 
                               count_if_used<T2>::value +
                               count_if_used<T3>::value + 
                               count_if_used<T4>::value +
                               count_if_used<T5>::value + 
                               count_if_used<T6>::value +
                               count_if_used<T7>::value + 
                               count_if_used<T8>::value +
                               count_if_used<T9>::value +
                               count_if_used<T10>::value));
      };
    } // end namespace function
  } // end namespace detail

  /**
   * The function_base class contains the basic elements needed for the
   * function1, function2, function3, etc. classes. It is common to all
   * functions (and as such can be used to tell if we have one of the
   * functionN objects). 
   */
  class function_base 
  {
  public:
    function_base() : manager(0), functor(static_cast<void*>(0)) {}
    
    // Is this function empty?
    bool empty() const { return !manager; }
    
  public: // should be protected, but GCC 2.95.3 will fail to allow access
    detail::function::any_pointer (*manager)(
                           detail::function::any_pointer, 
                           detail::function::functor_manager_operation_type);
    detail::function::any_pointer functor;

#if (defined __SUNPRO_CC) && (__SUNPRO_CC <= 0x530) && !(defined BOOST_NO_COMPILER_CONFIG)
    // Sun C++ 5.3 can't handle the safe_bool idiom, so don't use it
    operator bool () const { return !this->empty(); }
#else
  private:
    struct dummy {
      void nonnull() {};
    };

    typedef void (dummy::*safe_bool)();

  public:
    operator safe_bool () const 
      { return (this->empty())? 0 : &dummy::nonnull; }

    safe_bool operator!() const
      { return (this->empty())? &dummy::nonnull : 0; }
#endif
  };

  /* Poison comparison between Boost.Function objects (because it is 
   * meaningless). The comparisons would otherwise be allowed because of the
   * conversion required to allow syntax such as:
   *   boost::function<int, int> f;
   *   if (f) { f(5); }
   */
  void operator==(const function_base&, const function_base&);
  void operator!=(const function_base&, const function_base&);

  namespace detail {
    namespace function {
      inline bool has_empty_target(const function_base* f)
      {
        return f->empty();
      }

      inline bool has_empty_target(...)
      {
        return false;
      }
    } // end namespace function
  } // end namespace detail

  // The default function policy is to do nothing before and after the call.
  struct empty_function_policy
  {
    inline void precall(const function_base*) {}
    inline void postcall(const function_base*) {}
  };

  // The default function mixin does nothing. The assignment and
  // copy-construction operators are all defined because MSVC defines broken
  // versions.
  struct empty_function_mixin 
  {
    empty_function_mixin() {}
    empty_function_mixin(const empty_function_mixin&) {}

    empty_function_mixin& operator=(const empty_function_mixin&) 
    {
      return *this; 
    }
  };
}

#endif // BOOST_FUNCTION_BASE_HEADER
