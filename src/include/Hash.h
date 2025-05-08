#pragma once

#include <functional>

// This is an implementation of N3876 found at:
//   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf

namespace TrainTracks
{

inline
void
hash_combine (std::size_t&)
{
}

template <typename T>
inline
void
hash_combine (std::size_t& seed, const T& val)
{
    seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template <typename T, typename... Types>
inline
void
hash_combine (std::size_t& seed, const T& val, const Types&... args)
{
    hash_combine(seed, val);
    hash_combine(seed, args...);
}

template <typename... Types>
inline
std::size_t
hash_val (const Types&... args)
{
    std::size_t seed = 0;
    hash_combine(seed, args...);
    return seed;
}

}
