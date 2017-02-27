
#ifndef BOOST_ITERATOR_MIXIN_HPP
#define BOOST_ITERATOR_MIXIN_HPP

#include <boost/operators.hpp>

namespace boost {

// must be in boost namespace, otherwise the inline friend trick fails
template<class Generator, class ResultType>
class generator_iterator_mixin_adapter
  : incrementable<Generator>, equality_comparable<Generator>
{
public:
  typedef std::input_iterator_tag iterator_category;
  typedef ResultType value_type;
  typedef std::ptrdiff_t difference_type;
  typedef const value_type * pointer;
  typedef const value_type & reference;
  Generator& operator++() { v = cast()(); return cast(); }
  const value_type& operator*() const { return v; }

protected:
  // instantiate from derived classes only
  generator_iterator_mixin_adapter() { }
  void iterator_init() { operator++(); }
private:
  Generator & cast() { return static_cast<Generator&>(*this); }
  value_type v;
};

} // namespace boost

#endif // BOOST_ITERATOR_MIXIN_HPP
