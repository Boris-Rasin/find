/*

sky/container/find.h

Copyright (c) 2016-2017 Boris Rasin (boris@pointx.org)

*/

#ifndef SKY_CONTAINER_FIND_H
#define SKY_CONTAINER_FIND_H

#include <algorithm>
#include <experimental/type_traits>

namespace sky {
namespace detail {

template<typename T>
using mapped_type_t = typename T::mapped_type;

template<typename T>
using key_from_value_t = typename T::key_from_value;

// 3 key/value extractors:
//   map (mapped_type)
//   multi_index (key_from_value)
//   set/vector/array, etc. (none of the above)

template <typename C, typename Iterator>
inline decltype(auto) extract_key(C& c, Iterator& i)
{
    if constexpr (std::experimental::is_detected_v<mapped_type_t, C>)
    {
        return i->first;
    }
    else if constexpr (std::experimental::is_detected_v<key_from_value_t, C>)
    {
        return c.key_extractor()(*i);
    }
    else
    {
        return *i;
    }
}

template <typename C, typename Iterator>
inline auto& extract_value(Iterator& i)
{
    if constexpr (std::experimental::is_detected_v<mapped_type_t, C>)
    {
        return i->second;
    }
    else
    {
        return *i;
    }
}

template <typename C, typename Iterator>
class conditional_iterator
{
public:
    conditional_iterator(Iterator i, bool condition)
        : i(i), condition(condition) {}

    explicit operator bool() const { return condition; }
    operator typename C::const_iterator() const { return i; }

    auto operator-> () const { return &extract_value<C>(i); }
    auto& operator*() const { return extract_value<C>(i); }

    conditional_iterator operator!() const { return {i, !condition}; }

private:
    Iterator i;
    bool condition;
};

template<typename T>
using lower_bound_t = decltype(std::declval<T>().lower_bound(std::declval<typename T::key_type>()));

template<typename T>
using find_t = decltype(std::declval<T>().find(std::declval<typename T::key_type>()));

} // namespace detail

// If element is found, return value evaluates to true and contains iterator to found element.
// If element is not found, return value evaluates to false and contains hint iterator for insert/emplace.

template <typename C, typename Key>
inline auto find(C& c, const Key& key)
{
    if constexpr (std::experimental::is_detected_v<detail::lower_bound_t, C>)
    {
        auto it = c.lower_bound(key);
        return detail::conditional_iterator<C, decltype(c.lower_bound(key))> {it, it != c.end() && !(c.key_comp()(key, detail::extract_key(c, it)))};
    }
    else if constexpr (std::experimental::is_detected_v<detail::find_t, C>)
    {
        auto it = c.find(key);
        return detail::conditional_iterator<C, decltype(c.find(key))> {it, it != c.end()};
    }
    else
    {
        auto it = std::find(std::begin(c), std::end(c), key);
        return detail::conditional_iterator<C, decltype(std::find(std::begin(c), std::end(c), key))> {it, it != c.end()};
    }
}

} // namespace sky

#endif // SKY_CONTAINER_FIND_H
