/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include <numeric>
#include "detail/dimension.hpp"
namespace cross {

template<typename V, typename T, bool isIterable>
inline
dimension<V, T, isIterable>::dimension(impl::filter_dim_base<T> *cf, std::tuple<std::size_t, int> filter_pos_,
                                   std::function<field_type_t(const record_type_t &)> getter_)
    : impl::dimension_impl<V, T, isIterable>(cf, filter_pos_, getter_) {
}


template<typename V, typename T, bool isIterable>
inline
void  dimension<V, T, isIterable>::dispose() {
  impl::dimension_impl<V, T, isIterable>::dispose();
}


template<typename V, typename T, bool  isIterable>
inline
void  dimension<V, T, isIterable>::filter_range(const value_type_t & left, const value_type_t & right) {
  impl::dimension_impl<V, T, isIterable>::filter_range(left, right);
}

template<typename V, typename T, bool  isIterable>
inline
void  dimension<V, T, isIterable>::filter_exact(const value_type_t & value) {
  impl::dimension_impl<V, T, isIterable>::filter_exact(value);
}

template<typename V, typename T, bool  isIterable>
inline
void  dimension<V, T, isIterable>::filter_all() {
  impl::dimension_impl<V, T, isIterable>::filter_all();
}

template<typename V, typename T, bool  isIterable>
inline
void
dimension<V, T, isIterable>::filter_with_predicate(std::function<bool(const value_type_t&)> filter_function) {
  impl::dimension_impl<V, T, isIterable>::filter_with_predicate(filter_function);
}
template<typename V, typename T, bool  isIterable>
inline
void  dimension<V, T, isIterable>::filter_function(std::function<bool(const value_type_t&)> predicate) {
  impl::dimension_impl<V, T, isIterable>::filter_with_predicate(predicate);
}

template<typename V, typename T, bool  isIterable>
inline
auto
dimension<V, T, isIterable>::bottom(int64_t k, int64_t bottom_offset) -> std::vector<record_type_t> {
  return impl::dimension_impl<V, T, isIterable>::bottom(k, bottom_offset);
}

template<typename V, typename T, bool  isIterable>
inline
std::vector<typename dimension<V, T, isIterable>::record_type_t>
dimension<V, T, isIterable>::top(int64_t k, int64_t top_offset) {
  return impl::dimension_impl<V, T, isIterable>::top(k, top_offset);
}

template<typename V, typename T, bool  isIterable>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc, typename KeyFunc>
inline
auto dimension<V, T, isIterable>::feature(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_,
    KeyFunc key) -> cross::feature<decltype(key(std::declval<value_type_t>())),
                          decltype(initial_func_()), T, value_type_t, false, isIterable> {
  using K = decltype(key(std::declval<value_type_t>()));
  using R = decltype(initial_func_());
  return impl::dimension_impl<V, T, isIterable>::template feature<K, R>(key, add_func_, remove_func_, initial_func_);
}

template<typename V, typename T, bool  isIterable>
template<typename F>
inline
auto
dimension<V, T, isIterable>::feature_count(F key) ->
    cross::feature<decltype(key(std::declval<value_type_t>())),
          std::size_t, T, value_type_t, false, isIterable> {
  return feature(
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
auto dimension<V, T, isIterable>::feature_sum(ValueFunc value,
                                                          KeyFunc key)
    -> cross::feature<decltype(key(std::declval<value_type_t>())),
                      decltype(value(std::declval<record_type_t>())), T, value_type_t, false, isIterable> {
  return feature(
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
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto
dimension<V, T, isIterable>::feature_all(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_) -> cross::feature<std::size_t, decltype(initial_func_()), T, value_type_t, true, isIterable> {
  return impl::dimension_impl<V, T, isIterable>::template feature_all(add_func_, remove_func_, initial_func_);
}


template<typename V, typename T, bool  isIterable>
inline
auto
dimension<V, T, isIterable>::feature_all_count() -> cross::feature<std::size_t, std::size_t, T, value_type_t, true, isIterable> {
    return feature_all(
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
auto dimension<V, T, isIterable>::feature_all_sum(G value)
    -> cross::feature<std::size_t, decltype(value(record_type_t())), T, value_type_t, true, isIterable> {
  using reduce_type = decltype(value(record_type_t()));
  return feature_all(
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

} //namespace cross
