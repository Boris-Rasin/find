# find
Advanced find() for associative containers

template <typename C>
auto find(C& c, const typename C::key_type& key);

If element is found, return value evaluates to true and contains iterator to found element. If element is not found, return value evaluates to false and contains hint iterator for insert/emplace.

Example 1:

if (auto i = find(container, key))
{
    i->value_member();
    container.erase(i);
}

Example 2:

if (auto i = !find(container, key))
{
    container.insert(i, expensive_value_construction());
}

Tested with the following containers:

std::map
std::multimap
std::unordered_map
std::set
std::multiset
std::unordered_set
boost::container::map
boost::container::set
boost::container::flat_map
boost::container::flat_set
boost::multi_index::ordered_unique
boost::multi_index::ordered_non_unique
boost::multi_index::hashed_unique
boost::multi_index::hashed_non_unique
boost::multi_index::ranked_unique
boost::multi_index::ranked_non_unique
