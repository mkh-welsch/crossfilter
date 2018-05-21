/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef DIMENSION_H_GUARD
#define DIMENSION_H_GUARD
#include <vector>
#include <functional>
#include <utility>
#include <tuple>
//#include <boost/signals2.hpp>
#include "detail/dimension_impl.hpp"

template <typename K, typename R, typename P, bool> struct Group;

template <typename V, typename T, bool isIterable>
struct  Dimension : private DimensionImpl<V, T, isIterable> {
  using value_type_t = typename DimensionImpl<V, T, isIterable>::value_type_t;
  using field_type_t = typename DimensionImpl<V, T, isIterable>::field_type_t;
  using record_type_t = typename DimensionImpl<V, T, isIterable>::record_type_t;
  using this_type_t = Dimension<V, T, isIterable>;
  using base_type_t = DimensionImpl<V,T,isIterable>;
  using connection_type_t = typename DimensionImpl<V,T,isIterable>::connection_type_t;
  template<typename F> using signal_type_t = typename DimensionImpl<V,T,isIterable>::template signal_type_t<F>;
  static constexpr  bool getIsIterable() {
    return isIterable;
  }


  //  static const std::function<value_type_t(const value_type_t &)> identity_function =

  Dimension() {}

  Dimension(CrossFilterImpl<T> *cf, std::tuple<std::size_t, int> filterPos_,
            std::function<field_type_t(const record_type_t &)> getter_);

  Dimension(Dimension<V, T, isIterable> && dim)
      :DimensionImpl<V, T, isIterable>(std::move(dim)) {  }

  Dimension & operator = (Dimension && dim) {
    if (this == &dim)
      return *this;
    DimensionImpl<V, T, isIterable>::operator=(std::move(dim));
    return *this;
  }


  void dispose();

  std::size_t getOffset() const {
    return DimensionImpl<V, T, isIterable>::dimensionOffset;
  }

  std::size_t getBitIndex() const {
    return DimensionImpl<V, T, isIterable>::dimensionBitIndex;
  }
  /**
     Filters records such that this dimension's value is greater than or equal to left, and less than right.
   */
  void filterRange(const value_type_t &left, const value_type_t &right);
  /**
     Filters records such that this dimension's value equals value
  */
  void filterExact(const value_type_t &value);

  /**
     Clears any filters on this dimension
  */
  void filterAll();

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filterWithPredicate(std::function<bool(const value_type_t &)> filterFunction);

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filterFunction(std::function<bool(const value_type_t &)> predicate);

  /**
     Clears any filters on this dimension
   */
  void filter() { filterAll();}

  /**
     Filters records such that this dimension's value is greater than or equal to left, and less than right.
  */
  void filter(const value_type_t  & left, const value_type_t & right) { filterRange(left, right); }

  /**
     Filters records such that this dimension's value equals value
   */
  void filter(const value_type_t & value) { filterExact(value); }

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filter(std::function<bool(const value_type_t &)> filterFunction) { filterWithPredicate(filterFunction); }

  /**
     Returns a new array containing the bottom k records, according to the natural order of this dimension.
     The returned array is sorted by ascending natural order. This method intersects the crossfilter's current filters,
     returning only records that satisfy every active filter (including this dimension's filter).
     Optionally, retrieve k records offset by offset
   */
  std::vector<record_type_t> bottom(int64_t k,
                                    int64_t  bottom_offset = 0);

  /**
     Returns a new array containing the top k records, according to the natural order of this dimension.
     The returned array is sorted by descending natural order.
     This method intersects the crossfilter's current filters, returning only records that satisfy every
     active filter (including this dimension's filter). Optionally, retrieve k records offset by offset (records offset - offset + k - 1)
   */
  std::vector<record_type_t> top(int64_t k, int64_t top_offset = 0);

  /**
     Constructs a new grouping for the given dimension, according to the specified reduce functions.
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto
  group(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_ )
      -> Group<value_type_t,
               decltype(initial_func_()), this_type_t, false>  {
    auto key =  [](auto v) { return v;};
    return group(add_func_, remove_func_, initial_func_, key);
  }

  /**
     Constructs a new grouping for the given dimension, according to the specified reduce and key functions.
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc, typename KeyFunc>
  auto
  group(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_,
      KeyFunc key) -> Group<decltype(key(std::declval<value_type_t>())),
                            decltype(initial_func_()), this_type_t, false>;

  Group<value_type_t, std::size_t, this_type_t, false>
  group() {
    return groupReduceCount([](auto v) { return v;});
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by count 
   */
  template<typename G>
  auto  group(G key_) -> Group<decltype(key_(std::declval<value_type_t>())),
                                std::size_t, this_type_t, false> {
    return groupReduceCount(key_);
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by count 
   */
  template<typename K>
  auto groupReduceCount(K key) ->   Group<decltype(key(std::declval<value_type_t>())),
                                          std::size_t, this_type_t, false>;
  /**
     Constructs a new grouping for the given dimension to reduce elements by sum
   */
  template<typename ValueFunc>
  auto groupReduceSum(ValueFunc value)
      -> Group<value_type_t, decltype(value(record_type_t())), this_type_t, false> {
    auto key =  [](auto v) { return v;};
    return groupReduceSum(value, key);
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by sum
   */
  template<typename ValueFunc, typename KeyFunc>
  auto groupReduceSum(ValueFunc value,
                       KeyFunc key) -> Group<decltype(key(std::declval<value_type_t>())),
                                             decltype(value(std::declval<record_type_t>())), this_type_t, false>;
  /**
     A convenience function for grouping all records into a single group.  
   */
  template <typename R>
  Group<std::size_t, R, this_type_t, true>
  groupAll(
      std::function<R(R &, const record_type_t &, bool)> add_func_,
      std::function<R(R &, const record_type_t &, bool)> remove_func_,
      std::function<R()> initial_func_);

  /**
     A convenience function for grouping all records into a single group to reduce by count
   */
  Group<std::size_t, std::size_t, this_type_t, true> groupAllReduceCount();

  /**
     A convenience function for grouping all records into a single group to reduce by sum
   */
  template<typename G>
  auto groupAllReduceSum(G value) -> Group<std::size_t, decltype(value(record_type_t())), this_type_t, true>;

  /**
     Equivalent to groupAllReduceCount()
   */
  template <typename R>
  Group<std::size_t, R, this_type_t, true>
  groupAll() {
    return groupAllReduceCount();
  }
};

#include "detail/impl/dimension.ipp"


#endif
