/*

sky/container/find.h

Copyright (c) 2016-2017 Boris Rasin (boris@pointx.org)

*/

#ifndef SKY_CONTAINER_FIND_H
#define SKY_CONTAINER_FIND_H

#include <type_traits>

namespace sky {
namespace detail {

template <typename...>
using void_t = void;

template <typename, typename = void_t<>>
struct has_mapped_type : std::false_type {};

template <typename T>
struct has_mapped_type<T, void_t<typename T::mapped_type>> : std::true_type {};

template <typename, typename = void_t<>>
struct has_key_from_value : std::false_type {};

template <typename T>
struct has_key_from_value<T, void_t<typename T::key_from_value>> : std::true_type {};

template <typename, typename = void_t<>>
struct has_lower_bound : std::false_type {};

template <typename T>
struct has_lower_bound<T, void_t<decltype(std::declval<T>().lower_bound(std::declval<typename T::key_type>()))>> : std::true_type {};

// 3 key/value extractors:
//   map (mapped_type)
//   multi_index (key_from_value)
//   set (none of the above)

template <typename C, typename = void>
struct extract;

template <typename C>
struct extract<C, std::enable_if_t<has_mapped_type<C>::value>>
{
	template <typename Iterator>
	static decltype(auto) key(C& c, Iterator& i)
	{
		return i->first;
	}

	template <typename Iterator>
	static auto& value(Iterator& i)
	{
		return i->second;
	}
};

template <typename C>
struct extract<C, std::enable_if_t<has_key_from_value<C>::value>>
{
	template <typename Iterator>
	static decltype(auto) key(C& c, Iterator& i)
	{
		return c.key_extractor()(*i);
	}

	template <typename Iterator>
	static auto& value(Iterator& i)
	{
		return *i;
	}
};

template <typename C>
struct extract<C, std::enable_if_t<!has_mapped_type<C>::value && !has_key_from_value<C>::value>>
{
	template <typename Iterator>
	static decltype(auto) key(C& c, Iterator& i)
	{
		return *i;
	}

	template <typename Iterator>
	static auto& value(Iterator& i)
	{
		return *i;
	}
};

template <typename C, typename Iterator>
class conditional_iterator
{
public:

	conditional_iterator(Iterator i, bool condition) : i(i), condition(condition) {}

	explicit operator bool () const { return condition; }
	operator typename C::const_iterator () const { return i; }

	auto operator -> () const { return &extract<C>::value(i); }
	auto operator * () const { return extract<C>::value(i); }

	conditional_iterator operator ! () const { return { i, !condition }; }

private:

	Iterator i;
	bool condition;

};

template <typename C, typename = void>
struct find;

template <typename C>
struct find<C, std::enable_if_t<has_lower_bound<C>::value>>
{
	static auto call(C& c, const typename C::key_type& key) -> conditional_iterator<C, decltype(c.lower_bound(key))>
	{
		auto it = c.lower_bound(key);
		return { it, it != c.end() && !(c.key_comp()(key, extract<C>::key(c, it))) };
	}
};

template <typename C>
struct find<C, std::enable_if_t<!has_lower_bound<C>::value>>
{
	static auto call(C& c, const typename C::key_type& key) -> conditional_iterator<C, decltype(c.find(key))>
	{
		auto it = c.find(key);
		return { it, it != c.end() };
	}
};

} // namespace detail

// If element is found, return value evaluates to true and contains iterator to found element.
// If element is not found, return value evaluates to false and contains hint iterator for insert/emplace.

template <typename C>
inline auto find(C& c, const typename C::key_type& key)
{
	return detail::find<C>::call(c, key);
}

} // namespace sky

#endif // SKY_CONTAINER_FIND_H
