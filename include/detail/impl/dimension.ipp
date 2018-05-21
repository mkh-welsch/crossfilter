/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include <numeric>
#include "detail/dimension.hpp"


template<typename V, typename T, bool isIterable>
inline
Dimension<V, T, isIterable>::Dimension(CrossFilterImpl<T> *cf, std::tuple<std::size_t, int> filterPos_,
                                   std::function<field_type_t(const record_type_t &)> getter_)
    : DimensionImpl<V, T, isIterable>(cf, filterPos_, getter_) {
}


template<typename V, typename T, bool isIterable>
inline
void  Dimension<V, T, isIterable>::dispose() {
  DimensionImpl<V, T, isIterable>::dispose();
}


template<typename V, typename T, bool  isIterable>
inline
void  Dimension<V, T, isIterable>::filterRange(const value_type_t & left, const value_type_t & right) {
  DimensionImpl<V, T, isIterable>::filterRange(left, right);
}

template<typename V, typename T, bool  isIterable>
inline
void  Dimension<V, T, isIterable>::filterExact(const value_type_t & value) {
  DimensionImpl<V, T, isIterable>::filterExact(value);
}

template<typename V, typename T, bool  isIterable>
inline
void  Dimension<V, T, isIterable>::filterAll() {
  DimensionImpl<V, T, isIterable>::filterAll();
}

template<typename V, typename T, bool  isIterable>
inline
void
Dimension<V, T, isIterable>::filterWithPredicate(std::function<bool(const value_type_t&)> filterFunction) {
  DimensionImpl<V, T, isIterable>::filterWithPredicate(filterFunction);
}
template<typename V, typename T, bool  isIterable>
inline
void  Dimension<V, T, isIterable>::filterFunction(std::function<bool(const value_type_t&)> predicate) {
  DimensionImpl<V, T, isIterable>::filterWithPredicate(predicate);
}

template<typename V, typename T, bool  isIterable>
inline
auto
Dimension<V, T, isIterable>::bottom(int64_t k, int64_t bottom_offset) -> std::vector<record_type_t> {
  return DimensionImpl<V, T, isIterable>::bottom(k, bottom_offset);
}

template<typename V, typename T, bool  isIterable>
inline
std::vector<typename Dimension<V, T, isIterable>::record_type_t>
Dimension<V, T, isIterable>::top(int64_t k, int64_t top_offset) {
  return DimensionImpl<V, T, isIterable>::top(k, top_offset);
}

template<typename V, typename T, bool  isIterable>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc, typename KeyFunc>
inline
auto Dimension<V, T, isIterable>::group(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_,
    KeyFunc key) -> Group<decltype(key(std::declval<value_type_t>())),
                          decltype(initial_func_()), this_type_t, false> {
  using K = decltype(key(std::declval<value_type_t>()));
  using R = decltype(initial_func_());
  return DimensionImpl<V, T, isIterable>::template group<K, R>(key, add_func_, remove_func_, initial_func_);
}

template<typename V, typename T, bool  isIterable>
template<typename F>
inline
auto
Dimension<V, T, isIterable>::groupReduceCount(F key) ->
    Group<decltype(key(std::declval<value_type_t>())),
          std::size_t, this_type_t, false> {
  return group(
      [](std::size_t & r, const record_type_t &, bool ) {
        r++;
        return r;
      },
      [](std::size_t & r, const record_type_t &, bool ) {
        r--;
        return r;
      },
      []() {
        return std::size_t(0);
      },
      key);
}

template<typename V, typename T, bool  isIterable>
template<typename ValueFunc, typename KeyFunc>
inline
auto Dimension<V, T, isIterable>::groupReduceSum(ValueFunc value,
                                                          KeyFunc key)
    -> Group<decltype(key(std::declval<value_type_t>())),
             decltype(value(std::declval<record_type_t>())), this_type_t, false> {
  return group(
      [value](auto & r, const auto & rec, bool) {
        r += value(rec);
        return r;// + value(rec);
      },
      [value](auto & r, const auto & rec, bool) {
        r -=value(rec);
        return r;// - value(rec);
      },
      []() {
        return decltype(value(std::declval<record_type_t>()))();
      },
      key);
}

template<typename V, typename T, bool isIterable>
template <typename R>
inline
auto
Dimension<V, T, isIterable>::groupAll(
    std::function<R(R &, const record_type_t &, bool)> add_func_,
    std::function<R(R &, const record_type_t &, bool)> remove_func_,
    std::function<R()> initial_func_) -> Group<std::size_t, R, this_type_t, true> {
  return DimensionImpl<V, T, isIterable>::template groupAll<R>(add_func_, remove_func_, initial_func_);
}


template<typename V, typename T, bool  isIterable>
inline
auto
Dimension<V, T, isIterable>::groupAllReduceCount() -> Group<std::size_t, std::size_t, this_type_t, true> {
    return groupAll<std::size_t>(
        [](std::size_t & r, const record_type_t &, bool ) {
          return r + 1;
        },
        [](std::size_t & r, const record_type_t &, bool ) {
          return r - 1;
        },
        []() {
          return std::size_t(0);
        });
  }

template<typename V, typename T, bool isIterable>
template<typename G>
inline
auto Dimension<V, T, isIterable>::groupAllReduceSum(G value)
    -> Group<std::size_t, decltype(value(record_type_t())), this_type_t, true> {
  using reduce_type = decltype(value(record_type_t()));
  return groupAll<reduce_type>(
      [value](reduce_type & r, const record_type_t & rec, bool) {
        return r + value(rec);
      },
      [value](reduce_type & r, const record_type_t & rec, bool) {
        return r - value(rec);
      },
      []() {
        return reduce_type();;
      });
}

