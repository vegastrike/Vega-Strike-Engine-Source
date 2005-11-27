// Boost Lambda Library -  function_adaptors.hpp ----------------------------
 
// Copyright (C) 1999, 2000 Jaakko Järvi (jaakko.jarvi@cs.utu.fi)
//
// Permission to copy, use, sell and distribute this software is granted
// provided this copyright notice appears in all copies. 
// Permission to modify the code and to distribute modified code is granted
// provided this copyright notice appears in all copies, and a notice 
// that the code was modified is included with the copyright notice.
//
// This software is provided "as is" without express or implied warranty, 
// and with no claim as to its suitability for any purpose.
//
// For more information, see www.boost.org


#ifndef BOOST_LAMBDA_FUNCTION_ADAPTORS_HPP
#define BOOST_LAMBDA_FUNCTION_ADAPTORS_HPP

#include "boost/type_traits/same_traits.hpp"

namespace boost { 
namespace lambda {

template <class Func> struct function_adaptor {

  typedef detail::unspecified type;
  template <class T> struct sig { typedef detail::unspecified type; };

  template<class RET, class A1>
  static RET apply(A1& a1) {
    return a1();
  }
  template<class RET, class A1, class A2>
  static RET apply(A1& a1, A2& a2) {
    return a1(a2);
  }
  template<class RET, class A1, class A2, class A3>
  static RET apply(A1& a1, A2& a2, A3& a3) {
    return a1(a2, a3);
  }
  template<class RET, class A1, class A2, class A3, class A4>
  static RET apply(A1& a1, A2& a2, A3& a3, A4& a4) {
    return a1(a2, a3, a4);
  }
  template<class RET, class A1, class A2, class A3, class A4, class A5>
  static RET apply(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return a1(a2, a3, a4, a5);
  }
  template<class RET, class A1, class A2, class A3, class A4, class A5, class A6>
  static RET apply(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6) {
    return a1(a2, a3, a4, a5, a6);
  }
  template<class RET, class A1, class A2, class A3, class A4, class A5, class A6, 
           class A7>
  static RET apply(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, 
                           A7& a7) {
    return a1(a2, a3, a4, a5, a6, a7);
  }
  template<class RET, class A1, class A2, class A3, class A4, class A5, class A6, 
           class A7, class A8>
  static RET apply(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, 
                           A7& a7, A8& a8) {
    return a1(a2, a3, a4, a5, a6, a7, a8);
  }
  template<class RET, class A1, class A2, class A3, class A4, class A5, class A6, 
           class A7, class A8, class A9>
  static RET apply(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, 
                           A7& a7, A8& a8, A9& a9) {
    return a1(a2, a3, a4, a5, a6, a7, a8, a9);
  }
  template<class RET, class A1, class A2, class A3, class A4, class A5, class A6, 
           class A7, class A8, class A9, class A10>
  static RET apply(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, 
                           A7& a7, A8& a8, A9& a9, A10& a10) {
    return a1(a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }
};

template <class Func> struct function_adaptor<const Func>; // error 

// -- function adaptors with 1 argument apply
   
template <class Result>
struct function_adaptor<Result (void)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET>
  static Result apply(Result (*func)()) {
    return func();
  }
};

template <class Result>
struct function_adaptor<Result (*)(void)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET>
  static Result apply(Result (*func)()) {
    return func();
  }
};


// -- function adaptors with 2 argument apply
template <class Object, class Result>
struct function_adaptor<Result (Object::*)() const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET>
  static Result apply( Result (Object::*func)() const, const Object* o) {
    return (o->*func)();
  }
  template <class RET>
  static Result apply( Result (Object::*func)() const, const Object& o) {
    return (o.*func)();
  }
};

template <class Object, class Result>
struct function_adaptor<Result (Object::*)()> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET>
  static Result apply( Result (Object::*func)(), Object* o) {
    return (o->*func)();
  }
  template <class RET>
  static Result apply( Result (Object::*func)(), Object& o) {
    return (o.*func)();
  }
};

template <class Arg1, class Result>
struct function_adaptor<Result (Arg1)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1>
  static Result apply(Result (*func)(Arg1), A1& a1) {
    return func(a1);
  }
};

template <class Arg1, class Result>
struct function_adaptor<Result (*)(Arg1)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1>
  static Result apply(Result (*func)(Arg1), A1& a1) {
    return func(a1);
  }
};


// -- function adaptors with 3 argument apply
template <class Object, class Arg1, class Result>
struct function_adaptor<Result (Object::*)(Arg1) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1>
  static Result apply( Result (Object::*func)(Arg1) const, const Object* o, 
    A1& a1) {
    return (o->*func)(a1);
  }
  template <class RET, class A1>
  static Result apply( Result (Object::*func)(Arg1) const, const Object& o, 
    A1& a1) {
    return (o.*func)(a1);
  }
};

template <class Object, class Arg1, class Result>
struct function_adaptor<Result (Object::*)(Arg1)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1>
  static Result apply( Result (Object::*func)(Arg1), Object* o, A1& a1) {
    return (o->*func)(a1);
  }
  template <class RET, class A1>
  static Result apply( Result (Object::*func)(Arg1), Object& o, A1& a1) {
    return (o.*func)(a1);
  }
};

template <class Arg1, class Arg2, class Result>
struct function_adaptor<Result (Arg1, Arg2)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2>
  static Result apply(Result (*func)(Arg1, Arg2), A1& a1, A2& a2) {
    return func(a1, a2);
  }
};

template <class Arg1, class Arg2, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2>
  static Result apply(Result (*func)(Arg1, Arg2), A1& a1, A2& a2) {
    return func(a1, a2);
  }
};


// -- function adaptors with 4 argument apply
template <class Object, class Arg1, class Arg2, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2>
  static Result apply( Result (Object::*func)(Arg1, Arg2) const, const Object* o, A1& a1, A2& a2) {
    return (o->*func)(a1, a2);
  }
  template <class RET, class A1, class A2>
  static Result apply( Result (Object::*func)(Arg1, Arg2) const, const Object& o, A1& a1, A2& a2) {
    return (o.*func)(a1, a2);
  }
};

template <class Object, class Arg1, class Arg2, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2>
  static Result apply( Result (Object::*func)(Arg1, Arg2), Object* o, A1& a1, A2& a2) {
    return (o->*func)(a1, a2);
  }
  template <class RET, class A1, class A2>
  static Result apply( Result (Object::*func)(Arg1, Arg2), Object& o, A1& a1, A2& a2) {
    return (o.*func)(a1, a2);
  }
};

template <class Arg1, class Arg2, class Arg3, class Result>
struct function_adaptor<Result (Arg1, Arg2, Arg3)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3), A1& a1, A2& a2, A3& a3) {
    return func(a1, a2, a3);
  }
};

template <class Arg1, class Arg2, class Arg3, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2, Arg3)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3), A1& a1, A2& a2, A3& a3) {
    return func(a1, a2, a3);
  }
};


// -- function adaptors with 5 argument apply
template <class Object, class Arg1, class Arg2, class Arg3, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3) const, const Object* o, A1& a1, A2& a2, A3& a3) {
    return (o->*func)(a1, a2, a3);
  }
  template <class RET, class A1, class A2, class A3>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3) const, const Object& o, A1& a1, A2& a2, A3& a3) {
    return (o.*func)(a1, a2, a3);
  }
};

template <class Object, class Arg1, class Arg2, class Arg3, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3)> {
  typedef Result type;
  template <class RET, class A1, class A2, class A3>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3), Object* o, A1& a1, A2& a2, A3& a3) {
    return (o->*func)(a1, a2, a3);
  }
  template <class RET, class A1, class A2, class A3>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3), Object& o, A1& a1, A2& a2, A3& a3) {
    return (o.*func)(a1, a2, a3);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Result>
struct function_adaptor<Result (Arg1, Arg2, Arg3, Arg4)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4), A1& a1, A2& a2, A3& a3, A4& a4) {
    return func(a1, a2, a3, a4);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2, Arg3, Arg4)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4), A1& a1, A2& a2, A3& a3, A4& a4) {
    return func(a1, a2, a3, a4);
  }
};


// -- function adaptors with 6 argument apply
template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4) const, const Object* o, A1& a1, A2& a2, A3& a3, A4& a4) {
    return (o->*func)(a1, a2, a3, a4);
  }
  template <class RET, class A1, class A2, class A3, class A4>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4) const, const Object& o, A1& a1, A2& a2, A3& a3, A4& a4) {
    return (o.*func)(a1, a2, a3, a4);
  }
};

template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4), Object* o, A1& a1, A2& a2, A3& a3, A4& a4) {
    return (o->*func)(a1, a2, a3, a4);
  }
  template <class RET, class A1, class A2, class A3, class A4>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4), Object& o, A1& a1, A2& a2, A3& a3, A4& a4) {
    return (o.*func)(a1, a2, a3, a4);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Result>
struct function_adaptor<Result (Arg1, Arg2, Arg3, Arg4, Arg5)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return func(a1, a2, a3, a4, a5);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2, Arg3, Arg4, Arg5)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return func(a1, a2, a3, a4, a5);
  }
};


// -- function adaptors with 7 argument apply
template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5) const, const Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return (o->*func)(a1, a2, a3, a4, a5);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5) const, const Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return (o.*func)(a1, a2, a3, a4, a5);
  }
};

template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5), Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return (o->*func)(a1, a2, a3, a4, a5);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5), Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5) {
    return (o.*func)(a1, a2, a3, a4, a5);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Result>
struct function_adaptor<Result (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6) {
    return func(a1, a2, a3, a4, a5, a6);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6) {
    return func(a1, a2, a3, a4, a5, a6);
  }
};


// -- function adaptors with 8 argument apply
template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const, const Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6) {
    return (o->*func)(a1, a2, a3, a4, a5, a6);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const, const Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6) {
    return (o.*func)(a1, a2, a3, a4, a5, a6);
  }
};

template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6), Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6) {
    return (o->*func)(a1, a2, a3, a4, a5, a6);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6), Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6) {
    return (o.*func)(a1, a2, a3, a4, a5, a6);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Result>
struct function_adaptor<Result (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7) {
    return func(a1, a2, a3, a4, a5, a6, a7);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7) {
    return func(a1, a2, a3, a4, a5, a6, a7);
  }
};


// -- function adaptors with 9 argument apply
template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const, const Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7) {
    return (o->*func)(a1, a2, a3, a4, a5, a6, a7);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const, const Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7) {
    return (o.*func)(a1, a2, a3, a4, a5, a6, a7);
  }
};

template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7), Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7) {
    return (o->*func)(a1, a2, a3, a4, a5, a6, a7);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7), Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7) {
    return (o.*func)(a1, a2, a3, a4, a5, a6, a7);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Result>
struct function_adaptor<Result (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8) {
    return func(a1, a2, a3, a4, a5, a6, a7, a8);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8) {
    return func(a1, a2, a3, a4, a5, a6, a7, a8);
  }
};


// -- function adaptors with 10 argument apply
template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const, const Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8) {
    return (o->*func)(a1, a2, a3, a4, a5, a6, a7, a8);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const, const Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8) {
    return (o.*func)(a1, a2, a3, a4, a5, a6, a7, a8);
  }
};

template <class Object, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Result>
struct function_adaptor<Result (Object::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8), Object* o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8) {
    return (o->*func)(a1, a2, a3, a4, a5, a6, a7, a8);
  }
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
  static Result apply( Result (Object::*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8), Object& o, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8) {
    return (o.*func)(a1, a2, a3, a4, a5, a6, a7, a8);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9, class Result>
struct function_adaptor<Result (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9)> {
  typedef Result type;
  template<class T> struct sig { typedef Result type; };
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9) {
    return func(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }
};

template <class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9, class Result>
struct function_adaptor<Result (*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9)> {
  typedef Result type;
  template <class RET, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
  static Result apply(Result (*func)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9), A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9) {
    return func(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }
};

namespace detail {

template <class Args> class get_sig_result_type {
  typedef typename Args::head_type Func; 
  typedef typename detail::remove_reference_and_cv<Func>::type plainF;
public:
  // To sig we pass a cons list, where the head is the function object type
  // itself (potentially cv-qualified)
  // and the tail contains the types of the actual arguments to be passed
  // to the function object. The arguments can be cv qualified
  // as well.
  typedef typename plainF::template sig<Args>::type type;
};

} // end detail


template <class Args> 
class function_adaptor_with_actuals 
{
  typedef typename Args::head_type Func;
  typedef typename detail::remove_reference_and_cv<Func>::type plain_Func;

  // get the return type from the function adaptor class
  // this succeeds for func.pointers, member functions etc.
  typedef typename function_adaptor<plain_Func>::type type1;

public: 

  // if we get unspecified, Func is a function object class
  // thus it is ok to try the sig template
  typedef typename 
    detail::IF_type<
      boost::is_same<type1, detail::unspecified>::value,
      detail::get_sig_result_type<Args>, 
      function_adaptor<plain_Func> 
    >::type type;
  
};


} // namespace lambda
} // namespace boost

#endif













