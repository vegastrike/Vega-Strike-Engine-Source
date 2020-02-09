#ifndef __VECTORIZE_H
#define __VECTORIZE_H

#include <functional>

#include "aligned.h"
#include "vectorizable.h"
#include <assert.h>
#include <boost/concept_check.hpp>
#include <boost/static_assert.hpp>

/**
 * Terminal operator node for input data
 */
template<typename T> class vector_input {
    
public:
    typedef typename vectorizable_traits<T>::component_type component_type;
    typedef T vector_type;
    typedef component_type result_type;
    
    static const size_t components_per_element = vectorizable_traits<T>::components_per_element;
    
private:
    const component_type *begin;
    const component_type *end;

public:
    vector_input(const T &v)
    {
        begin = vectorizable_traits<T>::begin(v);
        end = vectorizable_traits<T>::end(v);
    }
    
    size_t size() const {
        return end - begin;
    }
    
    const result_type& get(size_t ix) const {
        return begin[ix];
    }
};

/**
 * Terminal operator node for broadcasted scalar input data
 */
template<typename T, int NELEM> class vector_broadcast {
    
public:
    typedef T component_type;
    typedef T vector_type;
    typedef component_type result_type;
    
    static const size_t components_per_element = NELEM;
    
private:
    const T& value;
    const size_t _size;

public:
    explicit vector_broadcast(const T &v, size_t size) : value(v), _size(size)
    {
    }
    
    size_t size() const {
        return _size;
    }
    
    const result_type& get(size_t ix) const {
        return value;
    }
};

/**
 * Terminal operator node for broadcasted vector element data
 */
template<typename T> class vector_broadcast_element {
    
public:
    typedef typename vectorizable_traits<T>::component_type component_type;
    typedef T vector_type;
    typedef component_type result_type;
    
    static const size_t components_per_element = vectorizable_traits<T>::components_per_element;
    
private:
    const vector_input<T> value;
    const size_t _size;

public:
    explicit vector_broadcast_element(const T &v, size_t size) : value(v), _size(size)
    {
    }
    
    size_t size() const {
        return _size;
    }
    
    const result_type& get(size_t ix) const {
        return value.get(ix % components_per_element);
    }
};

/**
 * Base node class for binary temp-based ops
 */
template<typename L, typename OP> class vector_unop {
    
public:
    typedef L l_vector_type;
    typedef typename OP::result_type result_type;
    typedef typename vectorizable_traits<L>::component_type l_component_type;
    
    static const size_t components_per_element = vectorizable_traits<L>::components_per_element;
    
private:
    const L &lvector;
    OP op;
    
public:
    explicit vector_unop(const L &l, OP op_ = OP()) : lvector(l), op(op_)
    {
    }
    
    vector_unop(const vector_unop<L,OP> &other) : lvector(other.lvector), op(other.op)
    {
    }
    
    size_t size() const {
        return lvector.size();
    }
    
    result_type get(size_t ix) const {
        return op(lvector.get(ix));
    }
};

/**
 * Base node class for binary temp-based ops
 */
template<typename L, typename R, typename OP> class vector_binop {
    
public:
    typedef L l_vector_type;
    typedef R r_vector_type;
    typedef typename OP::result_type result_type;
    typedef typename vectorizable_traits<L>::component_type l_component_type;
    typedef typename vectorizable_traits<R>::component_type r_component_type;
    
    static const size_t components_per_element = vectorizable_traits<L>::components_per_element;
    
private:
    const L &lvector;
    const R &rvector;
    OP op;
    
    BOOST_STATIC_ASSERT(L::components_per_element == R::components_per_element);
    
public:
    vector_binop(const L &l, const R &r, OP op_ = OP()) : lvector(l), rvector(r), op(op_)
    {
        assert(lvector.size() == rvector.size());
    }
    
    vector_binop(const vector_binop<L,R,OP> &other) : lvector(other.lvector), rvector(other.rvector), op(other.op)
    {
    }
    
    size_t size() const {
        return lvector.size();
    }
    
    result_type get(size_t ix) const {
        return op(lvector.get(ix), rvector.get(ix));
    }
};

template<typename L, typename R, typename RV>
vector_binop<L,R,std::plus<RV> > operator+(const L &l, const R &r)
{
    return vector_binop<L,R,std::plus<RV> >(l, r);
}

template<typename L, typename R, typename RV>
vector_binop<L,R,std::minus<RV> > operator-(const L &l, const R &r)
{
    return vector_binop<L,R,std::minus<RV> >(l, r);
}

template<typename L, typename R, typename RV>
vector_binop<L,R,std::multiplies<RV> > operator*(const L &l, const R &r)
{
    return vector_binop<L,R,std::multiplies<RV> >(l, r);
}

template<typename L, typename R, typename RV>
vector_binop<L,R,std::divides<RV> > operator/(const L &l, const R &r)
{
    return vector_binop<L,R,std::divides<RV> >(l, r);
}

template<typename L, typename RV>
vector_unop<L,std::negate<RV> > operator-(const L &l)
{
    return vector_unop<L,std::negate<RV> >(l);
}

template<typename L>
vector_unop<L,std::binder2nd<std::plus<typename L::component_type> > > operator+(const L &l, typename L::component_type r)
{
    return vector_unop<L,std::binder2nd<std::plus<typename L::component_type> > >(l, std::bind2nd(std::plus<typename L::component_type>(), r));
}

template<typename L>
vector_unop<L,std::binder2nd<std::minus<typename L::component_type> > > operator-(const L &l, typename L::component_type r)
{
    return vector_unop<L,std::binder2nd<std::minus<typename L::component_type> > >(l, std::bind2nd(std::minus<typename L::component_type>(), r));
}

template<typename L>
vector_unop<L,std::binder2nd<std::multiplies<typename L::component_type> > > operator*(const L &l, typename L::component_type r)
{
    return vector_unop<L,std::binder2nd<std::multiplies<typename L::component_type> > >(l, std::bind2nd(std::multiplies<typename L::component_type>(), r));
}

template<typename L>
vector_unop<L,std::binder2nd<std::multiplies<typename L::component_type> > > operator/(const L &l, typename L::component_type r)
{
    return vector_unop<L,std::binder2nd<std::multiplies<typename L::component_type> > >(l, std::bind2nd(std::multiplies<typename L::component_type>(), typename L::component_type(1) / r));
}

template<typename R>
vector_unop<std::binder1st<std::plus<typename R::component_type> >, R> operator+(typename R::component_type l, const R &r)
{
    return vector_unop<std::binder1st<std::plus<typename R::component_type> >, R>(r, std::bind1st(std::plus<typename R::component_type>(), l));
}

template<typename R>
vector_unop<std::binder1st<std::minus<typename R::component_type> >, R> operator-(typename R::component_type l, const R &r)
{
    return vector_unop<std::binder1st<std::minus<typename R::component_type> >, R>(r, std::bind1st(std::minus<typename R::component_type>(), l));
}

template<typename R>
vector_unop<std::binder1st<std::multiplies<typename R::component_type> >, R> operator*(typename R::component_type l, const R &r)
{
    return vector_unop<std::binder1st<std::multiplies<typename R::component_type> >, R>(r, std::bind1st(std::multiplies<typename R::component_type>(), l));
}

template<typename R>
vector_unop<std::binder1st<std::divides<typename R::component_type> >, R> operator/(typename R::component_type l, const R &r)
{
    return vector_unop<std::binder1st<std::divides<typename R::component_type> >, R>(r, std::bind1st(std::divides<typename R::component_type>(), l));
}

namespace vectorized {

    template<typename L, typename OP>
    vector_unop<L, OP> map(const L &l, OP op = OP())
    {
        return vector_unop<L, OP>(l, op);
    }

}

/**
 * Terminal operator node for input data
 */
template<typename T> class vector_output {
    
public:
    typedef typename vectorizable_traits<T>::component_type component_type;
    typedef T vector_type;
    typedef component_type result_type;
    
    static const size_t components_per_element = vectorizable_traits<T>::components_per_element;
    
private:
    component_type *begin;
    component_type *end;

public:
    vector_output(T &v)
    {
        begin = vectorizable_traits<T>::begin(v);
        end = vectorizable_traits<T>::end(v);
    }
    
    size_t size() const {
        return end - begin;
    }
    
    result_type& get(size_t ix) {
        return begin[ix];
    }
    
    const result_type& get(size_t ix) const {
        return begin[ix];
    }
    
    template<typename R>
    vector_output<T>& operator=(const R& r)
    {
        assert(size() == r.size());
        for (size_t i=0, n=size(); i<n; ++i) {
            get(i) = r.get(i);
        }
        return *this;
    }

    template<typename R>
    vector_output<T>& operator+=(const R& r)
    {
        assert(size() == r.size());
        for (size_t i=0, n=size(); i<n; ++i) {
            get(i) += r.get(i);
        }
        return *this;
    }

    template<typename R>
    vector_output<T>& operator-=(const R& r)
    {
        assert(size() == r.size());
        for (size_t i=0, n=size(); i<n; ++i) {
            get(i) -= r.get(i);
        }
        return *this;
    }

    template<typename R>
    vector_output<T>& operator*=(const R& r)
    {
        assert(size() == r.size());
        for (size_t i=0, n=size(); i<n; ++i) {
            get(i) *= r.get(i);
        }
        return *this;
    }

    template<typename R>
    vector_output<T>& operator/=(const R& r)
    {
        assert(size() == r.size());
        for (size_t i=0, n=size(); i<n; ++i) {
            get(i) /= r.get(i);
        }
        return *this;
    }
    
};

namespace vectorized {

    template<typename T>
    vector_input<T> vectorize(const T& v)
    {
        return vector_input<T>(v);
    }

    template<typename T>
    vector_output<T> vectorize(T& v)
    {
        return vector_output<T>(v);
    }
    
}

#define VECTORIZE_MEMBER_FUNC(name,T,R,F)                   \
    class name {                                            \
    public:                                                 \
        typedef T argument_type;                            \
        typedef R result_type;                              \
                                                            \
        R operator()(const T &x) const {                    \
            return x.F();                                   \
        }                                                   \
    }


#endif

