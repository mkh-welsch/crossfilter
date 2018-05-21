/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include "crossfilter.hpp"

template<typename T>
template<typename C>
inline
typename std::enable_if<!std::is_same<C, T>::value, CrossFilter<T>&>::type
CrossFilter<T>::add(const C &newData) {
  CrossFilterImpl<T>::add(newData);
  return *this;
}

template<typename T>
inline
CrossFilter<T> & CrossFilter<T>::add(const T &newData) {
  CrossFilterImpl<T>::add(newData);
  return *this;
}

template<typename T>
inline
std::size_t CrossFilter<T>::size() const { return CrossFilterImpl<T>::size(); }



// removes all records matching the predicate (ignoring filters).
template<typename T>
inline
void CrossFilter<T>::remove(std::function<bool(const T&, int)> predicate) {
  CrossFilterImpl<T>::remove(predicate);
}

// Removes all records that match the current filters
template<typename T>
inline
void CrossFilter<T>::remove() {
  CrossFilterImpl<T>::remove();
}

template<typename T>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto
CrossFilter<T>::groupAll(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_) -> Group<std::size_t,
                                        decltype(initial_func_()), this_type_t, true> {
    return CrossFilterImpl<T>::groupAll(add_func_, remove_func_, initial_func_);
}

template <typename T>
Group<std::size_t, std::size_t, typename CrossFilter<T>::this_type_t, true>
CrossFilter<T>::groupAllReduceCount() {
    return groupAll(
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
auto CrossFilter<T>::groupAllReduceSum(G value)
    -> Group<std::size_t, decltype(value(record_type_t())), this_type_t, true> {
  using R = decltype(value(record_type_t()));

  return groupAll(
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

template<typename T>
inline
Group<std::size_t, std::size_t, typename CrossFilter<T>::this_type_t, true>
CrossFilter<T>::groupAll() {
  return groupAllReduceCount();
}
