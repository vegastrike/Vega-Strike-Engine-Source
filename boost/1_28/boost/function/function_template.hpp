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

// Note: this header is a header template and must NOT have multiple-inclusion
// protection. 

#ifndef BOOST_FUNCTION_FUNCTION_TEMPLATE_HPP
#define BOOST_FUNCTION_FUNCTION_TEMPLATE_HPP
#  include <cassert>
#  include <algorithm>
#  include <boost/config.hpp>
#  include <boost/function/function_base.hpp>
#  include <boost/mem_fn.hpp>
#endif // BOOST_FUNCTION_FUNCTION_TEMPLATE_HPP

// Type of the default allocator
#ifndef BOOST_NO_STD_ALLOCATOR
#  define BOOST_FUNCTION_DEFAULT_ALLOCATOR std::allocator<function_base>
#else
#  define BOOST_FUNCTION_DEFAULT_ALLOCATOR int
#endif // BOOST_NO_STD_ALLOCATOR

// Comma if nonzero number of arguments
#if BOOST_FUNCTION_NUM_ARGS == 0
#  define BOOST_FUNCTION_COMMA 
#else
#  define BOOST_FUNCTION_COMMA ,
#endif // BOOST_FUNCTION_NUM_ARGS > 0

// Class names used in this version of the code
#define BOOST_FUNCTION_FUNCTION BOOST_JOIN(function,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_FUNCTION_INVOKER \
  BOOST_JOIN(function_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_VOID_FUNCTION_INVOKER \
  BOOST_JOIN(void_function_invoker,BOOST_FUNCTION_NUM_ARGS) 
#define BOOST_FUNCTION_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(void_function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_STATELESS_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(stateless_function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_STATELESS_VOID_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(stateless_void_function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_GET_FUNCTION_INVOKER \
  BOOST_JOIN(get_function_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_GET_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(get_function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_GET_STATELESS_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(get_stateless_function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)

namespace boost {
  namespace detail {
    namespace function {
      template<
        typename FunctionPtr,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
        >
      struct BOOST_FUNCTION_FUNCTION_INVOKER
      {
        static R invoke(any_pointer function_ptr BOOST_FUNCTION_COMMA
                        BOOST_FUNCTION_PARMS)
        {
          FunctionPtr f = reinterpret_cast<FunctionPtr>(function_ptr.func_ptr);
          return f(BOOST_FUNCTION_ARGS);
        }
      };

      template<
        typename FunctionPtr,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
        >
      struct BOOST_FUNCTION_VOID_FUNCTION_INVOKER
      {
        static unusable invoke(any_pointer function_ptr BOOST_FUNCTION_COMMA
                               BOOST_FUNCTION_PARMS)

        {
          FunctionPtr f = reinterpret_cast<FunctionPtr>(function_ptr.func_ptr);
          f(BOOST_FUNCTION_ARGS);
          return unusable();
        }
      };

      template<
        typename FunctionObj,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
      >
      struct BOOST_FUNCTION_FUNCTION_OBJ_INVOKER
      {
        static R invoke(any_pointer function_obj_ptr BOOST_FUNCTION_COMMA
                        BOOST_FUNCTION_PARMS)

        {
          FunctionObj* f = (FunctionObj*)(function_obj_ptr.obj_ptr);
          return (*f)(BOOST_FUNCTION_ARGS);
        }
      };

      template<
        typename FunctionObj,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
      >
      struct BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER
      {
        static unusable invoke(any_pointer function_obj_ptr 
                               BOOST_FUNCTION_COMMA
                               BOOST_FUNCTION_PARMS)

        {
          FunctionObj* f = (FunctionObj*)(function_obj_ptr.obj_ptr);
          (*f)(BOOST_FUNCTION_ARGS);
          return unusable();
        }
      };

      template<
        typename FunctionObj,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
      >
      struct BOOST_FUNCTION_STATELESS_FUNCTION_OBJ_INVOKER
      {
        static R invoke(any_pointer BOOST_FUNCTION_COMMA BOOST_FUNCTION_PARMS)
        {
          FunctionObj f = FunctionObj();
          return f(BOOST_FUNCTION_ARGS);
        }
      };

      template<
        typename FunctionObj,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
      >
      struct BOOST_FUNCTION_STATELESS_VOID_FUNCTION_OBJ_INVOKER
      {
        static unusable invoke(any_pointer BOOST_FUNCTION_COMMA 
                               BOOST_FUNCTION_PARMS)

        {
          FunctionObj f = FunctionObj();
          f(BOOST_FUNCTION_ARGS);
          return unusable();
        }
      };

      template<
        typename FunctionPtr,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
      >
      struct BOOST_FUNCTION_GET_FUNCTION_INVOKER
      {
        typedef typename ct_if<(is_void<R>::value),
                            BOOST_FUNCTION_VOID_FUNCTION_INVOKER<
                            FunctionPtr,
                            R BOOST_FUNCTION_COMMA
                            BOOST_FUNCTION_TEMPLATE_ARGS
                          >,
                          BOOST_FUNCTION_FUNCTION_INVOKER<
                            FunctionPtr,
                            R BOOST_FUNCTION_COMMA
                            BOOST_FUNCTION_TEMPLATE_ARGS
                          >
                       >::type type;
      };

      template<
        typename FunctionObj,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
       >
      struct BOOST_FUNCTION_GET_FUNCTION_OBJ_INVOKER
      {
        typedef typename ct_if<(is_void<R>::value),
                            BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER<
                            FunctionObj,
                            R BOOST_FUNCTION_COMMA
                            BOOST_FUNCTION_TEMPLATE_ARGS
                          >,
                          BOOST_FUNCTION_FUNCTION_OBJ_INVOKER<
                            FunctionObj,
                            R BOOST_FUNCTION_COMMA
                            BOOST_FUNCTION_TEMPLATE_ARGS
                          >
                       >::type type;
      };

      template<
        typename FunctionObj,
        typename R BOOST_FUNCTION_COMMA
        BOOST_FUNCTION_TEMPLATE_PARMS
       >
      struct BOOST_FUNCTION_GET_STATELESS_FUNCTION_OBJ_INVOKER
      {
        typedef typename ct_if<(is_void<R>::value),
                            BOOST_FUNCTION_STATELESS_VOID_FUNCTION_OBJ_INVOKER<
                            FunctionObj,
                            R BOOST_FUNCTION_COMMA
                            BOOST_FUNCTION_TEMPLATE_ARGS
                          >,
                          BOOST_FUNCTION_STATELESS_FUNCTION_OBJ_INVOKER<
                            FunctionObj,
                            R BOOST_FUNCTION_COMMA
                            BOOST_FUNCTION_TEMPLATE_ARGS
                          >
                       >::type type;
      };

    } // end namespace function
  } // end namespace detail

  template<
    typename R BOOST_FUNCTION_COMMA
    BOOST_FUNCTION_TEMPLATE_PARMS,
    typename Policy    = empty_function_policy,
    typename Mixin     = empty_function_mixin,
    typename Allocator = BOOST_FUNCTION_DEFAULT_ALLOCATOR
  >
  class BOOST_FUNCTION_FUNCTION : public function_base, public Mixin
  {
    typedef typename detail::function::function_return_type<R>::type 
      internal_result_type;

  public:
    BOOST_STATIC_CONSTANT(int, args = BOOST_FUNCTION_NUM_ARGS);
    
#if BOOST_FUNCTION_NUM_ARGS == 1
    typedef T0 argument_type;
#elif BOOST_FUNCTION_NUM_ARGS == 2
    typedef T0 first_argument_type;
    typedef T1 second_argument_type;
#endif

#ifndef BOOST_NO_VOID_RETURNS
    typedef R         result_type;
#else
    typedef internal_result_type result_type;
#endif // BOOST_NO_VOID_RETURNS
    typedef Policy    policy_type;
    typedef Mixin     mixin_type;
    typedef Allocator allocator_type;
    typedef BOOST_FUNCTION_FUNCTION self_type;

    BOOST_FUNCTION_FUNCTION() : function_base(), Mixin(), invoker(0) {}

    explicit BOOST_FUNCTION_FUNCTION(const Mixin& m) : 
      function_base(), Mixin(m), invoker(0) 
    {
    }

    // MSVC chokes if the following two constructors are collapsed into
    // one with a default parameter.
    template<typename Functor>
    BOOST_FUNCTION_FUNCTION(Functor BOOST_FUNCTION_TARGET_FIX(const &) f) :
      function_base(), Mixin(), invoker(0)
    {
      this->assign_to(f);
    }

    template<typename Functor>
    BOOST_FUNCTION_FUNCTION(Functor f, const Mixin& m) :
      function_base(), Mixin(m), invoker(0)
    {
      this->assign_to(f);
    }

    BOOST_FUNCTION_FUNCTION(const BOOST_FUNCTION_FUNCTION& f) :
      function_base(), Mixin(static_cast<const Mixin&>(f)), invoker(0)
    {
      this->assign_to_own(f);
    }

    ~BOOST_FUNCTION_FUNCTION() { clear(); }

    result_type operator()(BOOST_FUNCTION_PARMS) const
    {
      assert(!this->empty());

      policy_type policy;
      policy.precall(this);

      internal_result_type result = invoker(function_base::functor 
                                            BOOST_FUNCTION_COMMA
                                            BOOST_FUNCTION_ARGS);

      policy.postcall(this);
#ifndef BOOST_NO_VOID_RETURNS
      return static_cast<result_type>(result);
#else
      return result;
#endif // BOOST_NO_VOID_RETURNS
    }

    // The distinction between when to use BOOST_FUNCTION_FUNCTION and
    // when to use self_type is obnoxious. MSVC cannot handle self_type as
    // the return type of these assignment operators, but Borland C++ cannot
    // handle BOOST_FUNCTION_FUNCTION as the type of the temporary to 
    // construct.
    template<typename Functor>
    BOOST_FUNCTION_FUNCTION& 
    operator=(Functor BOOST_FUNCTION_TARGET_FIX(const &) f)
    {
      self_type(f, static_cast<const Mixin&>(*this)).swap(*this);
      return *this;
    }

    template<typename Functor>
    void set(Functor BOOST_FUNCTION_TARGET_FIX(const &) f)
    {
      self_type(f, static_cast<const Mixin&>(*this)).swap(*this);
    }

    // Assignment from another BOOST_FUNCTION_FUNCTION
    BOOST_FUNCTION_FUNCTION& operator=(const BOOST_FUNCTION_FUNCTION& f)
    {
      if (&f == this)
        return *this;

      self_type(f).swap(*this);
      return *this;
    }

    // Assignment from another BOOST_FUNCTION_FUNCTION
    void set(const BOOST_FUNCTION_FUNCTION& f)
    {
      if (&f == this)
        return;

      self_type(f).swap(*this);
    }

    void swap(BOOST_FUNCTION_FUNCTION& other)
    {
      if (&other == this)
        return;

      std::swap(function_base::manager, other.manager);
      std::swap(function_base::functor, other.functor);
      std::swap(invoker, other.invoker);
      std::swap(static_cast<Mixin&>(*this), static_cast<Mixin&>(other));
    }

    // Clear out a target, if there is one
    void clear()
    {
      if (function_base::manager) {
        function_base::functor = 
	  function_base::manager(function_base::functor, 
				 detail::function::destroy_functor_tag);
      }

      function_base::manager = 0;
      invoker = 0;
    }

  private:
    void assign_to_own(const BOOST_FUNCTION_FUNCTION& f)
    {
      if (!f.empty()) {
        invoker = f.invoker;
        function_base::manager = f.manager;
        function_base::functor = 
	  f.manager(f.functor, detail::function::clone_functor_tag);
      }          
    }

    template<typename Functor>
    void assign_to(Functor f)
    {
      typedef typename detail::function::get_function_tag<Functor>::type tag;
      this->assign_to(f, tag());
    }

    template<typename FunctionPtr>
    void assign_to(FunctionPtr f, detail::function::function_ptr_tag)
    {
      clear();
        
      if (f) {
        typedef typename detail::function::BOOST_FUNCTION_GET_FUNCTION_INVOKER<
                           FunctionPtr,
                           R BOOST_FUNCTION_COMMA
                           BOOST_FUNCTION_TEMPLATE_ARGS
                         >::type
          invoker_type;
    
        invoker = &invoker_type::invoke;
        function_base::manager = 
	  &detail::function::functor_manager<FunctionPtr, Allocator>::manage;
        function_base::functor = 
	  function_base::manager(detail::function::any_pointer(
                            // should be a reinterpret cast, but some compilers
                            // insist on giving cv-qualifiers to free functions
                            (void (*)())(f)
                          ),
                          detail::function::clone_functor_tag);
      }
    }  

#if BOOST_FUNCTION_NUM_ARGS > 0
    template<typename MemberPtr>
    void assign_to(MemberPtr f, detail::function::member_ptr_tag)
    {
      this->assign_to(mem_fn(f));
    }
#endif // BOOST_FUNCTION_NUM_ARGS > 0
        
    template<typename FunctionObj>
    void assign_to(FunctionObj f, detail::function::function_obj_tag)
    {
      if (!detail::function::has_empty_target(addressof(f))) {
        typedef 
          typename detail::function::BOOST_FUNCTION_GET_FUNCTION_OBJ_INVOKER<
                                       FunctionObj,
                                       R BOOST_FUNCTION_COMMA
                                       BOOST_FUNCTION_TEMPLATE_ARGS
                                     >::type
          invoker_type;
    
        invoker = &invoker_type::invoke;
        function_base::manager = &detail::function::functor_manager<
	                            FunctionObj, Allocator>::manage;
#ifndef BOOST_NO_STD_ALLOCATOR
        typedef typename Allocator::template rebind<FunctionObj>::other 
          allocator_type;
        typedef typename allocator_type::pointer pointer_type;
	allocator_type allocator;
	pointer_type copy = allocator.allocate(1);
	allocator.construct(copy, f);

	// Get back to the original pointer type
	FunctionObj* new_f = static_cast<FunctionObj*>(copy);
#else
	FunctionObj* new_f = new FunctionObj(f);
#endif // BOOST_NO_STD_ALLOCATOR
        function_base::functor = 
	  detail::function::any_pointer(static_cast<void*>(new_f));
      }
    }
    
    template<typename FunctionObj>
    void assign_to(const reference_wrapper<FunctionObj>& f, 
                   detail::function::function_obj_ref_tag)
    {
      if (!detail::function::has_empty_target(f.get_pointer())) {
        typedef 
          typename detail::function::BOOST_FUNCTION_GET_FUNCTION_OBJ_INVOKER<
                                       FunctionObj,
                                       R BOOST_FUNCTION_COMMA
                                       BOOST_FUNCTION_TEMPLATE_ARGS
                                     >::type
          invoker_type;
    
        invoker = &invoker_type::invoke;
        function_base::manager = &detail::function::trivial_manager;
        function_base::functor = 
          function_base::manager(
            detail::function::any_pointer(
              const_cast<FunctionObj*>(f.get_pointer())),
	    detail::function::clone_functor_tag);
      }
    }
    
    template<typename FunctionObj>
    void assign_to(FunctionObj, detail::function::stateless_function_obj_tag)
    {
      typedef 
          typename detail::function::
                     BOOST_FUNCTION_GET_STATELESS_FUNCTION_OBJ_INVOKER<
                       FunctionObj,
                       R BOOST_FUNCTION_COMMA
                       BOOST_FUNCTION_TEMPLATE_ARGS
                     >::type
          invoker_type;
      invoker = &invoker_type::invoke;
      function_base::manager = &detail::function::trivial_manager;
      function_base::functor = detail::function::any_pointer(this);
    }

    typedef internal_result_type (*invoker_type)(detail::function::any_pointer
                                                 BOOST_FUNCTION_COMMA
                                                 BOOST_FUNCTION_TEMPLATE_ARGS);
    
    invoker_type invoker;
  };

  template<typename R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_PARMS ,
           typename Policy, typename Mixin, typename Allocator>
  inline void swap(BOOST_FUNCTION_FUNCTION<
                     R BOOST_FUNCTION_COMMA
                     BOOST_FUNCTION_TEMPLATE_ARGS ,
                     Policy,
                     Mixin,
                     Allocator
                   >& f1,
                   BOOST_FUNCTION_FUNCTION<
                     R BOOST_FUNCTION_COMMA 
                     BOOST_FUNCTION_TEMPLATE_ARGS,
                     Policy,
                     Mixin,
                     Allocator
                   >& f2)
  {
    f1.swap(f2);
  }
}

// Cleanup after ourselves...
#undef BOOST_FUNCTION_DEFAULT_ALLOCATOR
#undef BOOST_FUNCTION_COMMA
#undef BOOST_FUNCTION_FUNCTION
#undef BOOST_FUNCTION_FUNCTION_INVOKER
#undef BOOST_FUNCTION_VOID_FUNCTION_INVOKER
#undef BOOST_FUNCTION_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_STATELESS_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_STATELESS_VOID_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_GET_FUNCTION_INVOKER
#undef BOOST_FUNCTION_GET_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_GET_STATELESS_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_GET_MEM_FUNCTION_INVOKER
