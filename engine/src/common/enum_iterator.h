/*
 * enum_iterator.h
 *
 * Copied from the first half of https://github.com/BatmanAoD/PublicPaste/blob/master/GenericCppCode/EnumIterator.cpp
 * Accessed 2026-05-04
 */

#ifndef VEGA_STRIKE_COMMON_ENUM_ITERATOR_H
#define VEGA_STRIKE_COMMON_ENUM_ITERATOR_H

#include <cassert>
#include <type_traits>

#define RAW_VAL(enum_val) \
  static_cast<RawType>(enum_val)

#define ENUM_VAL(raw_val) \
  static_cast<ENUM_TYPE>(raw_val)

template <typename ENUM_TYPE, ENUM_TYPE beginVal, ENUM_TYPE endVal>
class EnumIterator
{
  using RawType = std::underlying_type_t<ENUM_TYPE>;

  static_assert(
      RAW_VAL(beginVal) <= RAW_VAL(endVal),
      "Cannot create iterator where 'beginVal' comes after 'endVal'!");

  // XXX TODO there MAY be undefined behavior if 'endVal' is the maximum
  // positive value of the underlying type, because '::end()' will cause a
  // wrap-around, which is not defined for signed types.
  // However, it seems unlikely that this would cause issues.
  // Nevertheless, it would be best to add a compile-time assertion or something
  // to ensure valid behavior.

  public:
    EnumIterator(void) noexcept
      : iter_(RAW_VAL(beginVal))
    {}

    explicit EnumIterator(const ENUM_TYPE& start) noexcept
      : iter_{RAW_VAL(start)}
    {
      const bool valid {
        iter_ >= RAW_VAL(beginVal)
          // '*end()' is the last valid enum value for iteration
        && iter_ <= RAW_VAL(*end())
      };
      assert(valid);
      // In release mode:
      if (!valid) *this = end();
    }

  private:    // Used by 'begin()' and 'end()'
    explicit EnumIterator(
        const RawType& raw_start) noexcept
      : iter_{raw_start}
    {}

  public:

    EnumIterator operator++()
    {
      if (*this != end())
      {
        // Skip invalid enum values for iter_.
        do ++iter_; while (!IsEnumValid(**this) && *this != end());
      }

      return *this;
    }

    ENUM_TYPE operator*()
    {
      return ENUM_VAL(iter_);
    }

    EnumIterator begin()
    {
      static const EnumIterator beginIter{RAW_VAL(beginVal)};
      return beginIter;
    }

    EnumIterator end() {
      static const EnumIterator endIter{RAW_VAL(endVal) +1};
      return endIter;
    }

    bool operator!=(
        const EnumIterator& rhs)
    {
      return iter_ != rhs.iter_;
    }

  private:
    RawType iter_;
};

#undef RAW_VAL
#undef ENUM_VAL

#endif //VEGA_STRIKE_COMMON_ENUM_ITERATOR_H
