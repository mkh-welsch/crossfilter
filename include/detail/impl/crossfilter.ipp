/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include "crossfilter.hpp"
namespace cross {

template<typename T>
template<typename C>
inline
typename std::enable_if<!std::is_same<C, T>::value, filter<T>&>::type
filter<T>::add(const C &newData) {
  impl::filter_impl<T>::add(newData);
  return *this;
}

template<typename T>
inline
filter<T> & filter<T>::add(const T &newData) {
  impl::filter_impl<T>::add(newData);
  return *this;
}

template<typename T>
inline
std::size_t filter<T>::size() const { return impl::filter_impl<T>::size(); }



// removes all records matching the predicate (ignoring filters).
template<typename T>
inline
void filter<T>::remove(std::function<bool(const T&, int)> predicate) {
  impl::filter_impl<T>::remove(predicate);
}

// Removes all records that match the current filters
template<typename T>
inline
void filter<T>::remove() {
  impl::filter_impl<T>::remove();
}

template<typename T>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto
filter<T>::feature(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_) -> cross::feature<std::size_t,
                                        decltype(initial_func_()), this_type_t, true> {
  return impl::filter_impl<T>::feature(add_func_, remove_func_, initial_func_);
}

template <typename T>
inline
auto
filter<T>::feature_count() -> cross::feature<std::size_t, std::size_t, this_type_t, true> {
  return feature(
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

template<typename T>
template<typename G>
inline
auto filter<T>::feature_sum(G value)
    -> cross::feature<std::size_t, decltype(value(record_type_t())), this_type_t, true> {
  using R = decltype(value(record_type_t()));

  return feature(
        [value](R & r, const record_type_t & rec, bool) {
          return r + value(rec);
        },
        [value](R & r, const record_type_t & rec, bool) {
          return r - value(rec);
        },
        []() {
          return R();
        });
  }

// template<typename T>
// inline
// feature<std::size_t, std::size_t, typename filter<T>::this_type_t, true>
// filter<T>::feature() {
//   return feature_count();
// }

} // namespace cross
