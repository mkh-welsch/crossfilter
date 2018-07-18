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
namespace cross {
template <typename, typename, typename, typename,  bool, bool> struct feature;

template <typename V, typename T, bool isIterable>
struct  dimension : private impl::dimension_impl<V, T, isIterable> {
  using value_type_t = typename impl::dimension_impl<V, T, isIterable>::value_type_t;
  using field_type_t = typename impl::dimension_impl<V, T, isIterable>::field_type_t;
  using record_type_t = typename impl::dimension_impl<V, T, isIterable>::record_type_t;
  using this_type_t = dimension<V, T, isIterable>;
  using base_type_t = impl::dimension_impl<V,T,isIterable>;
  using data_iterator = typename impl::dimension_impl<V,T,isIterable>::data_iterator;
  using connection_type_t = typename impl::dimension_impl<V,T,isIterable>::connection_type_t;
  template<typename F> using signal_type_t = typename impl::dimension_impl<V,T,isIterable>::template signal_type_t<F>;

  template <typename, typename> friend struct filter_impl;

  static constexpr  bool get_is_iterable() {
    return isIterable;
  }

  void add(data_iterator begin, data_iterator end) {
    impl::dimension_impl<V,T,isIterable>::add(begin,end);
  }
 public:

  //  static const std::function<value_type_t(const value_type_t &)> identity_function =

  dimension() {}

  dimension(impl::filter_dim_base<T> *cf, std::tuple<std::size_t, int> filterPos_,
            std::function<field_type_t(const record_type_t &)> getter_);

  dimension(dimension<V, T, isIterable> && dim)
      :impl::dimension_impl<V, T, isIterable>(std::move(dim)) {  }

  dimension & operator = (dimension && dim) {
    if (this == &dim)
      return *this;
    impl::dimension_impl<V, T, isIterable>::operator=(std::move(dim));
    return *this;
  }


  void dispose();

  std::size_t get_offset() const {
    return impl::dimension_impl<V, T, isIterable>::dimension_offset;
  }

  std::size_t get_bit_index() const {
    return impl::dimension_impl<V, T, isIterable>::dimension_bit_index;
  }
  /**
     Filters records such that this dimension's value is greater than or equal to left, and less than right.
   */
  void filter_range(const value_type_t &left, const value_type_t &right);
  /**
     Filters records such that this dimension's value equals value
  */
  void filter_exact(const value_type_t &value);

  /**
     Clears any filters on this dimension
  */
  void filter_all();

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filter_with_predicate(std::function<bool(const value_type_t &)> filterFunction);

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filter_function(std::function<bool(const value_type_t &)> predicate);

  /**
     Clears any filters on this dimension
   */
  void filter() { filter_all();}

  /**
     Filters records such that this dimension's value is greater than or equal to left, and less than right.
  */
  void filter(const value_type_t  & left, const value_type_t & right) { filter_range(left, right); }

  /**
     Filters records such that this dimension's value equals value
   */
  void filter(const value_type_t & value) { filter_exact(value); }

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filter(std::function<bool(const value_type_t &)> filterFunction) { filter_with_predicate(filterFunction); }

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
  feature(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_ )
      -> cross::feature<value_type_t,
                        decltype(initial_func_()), T, value_type_t, false, isIterable>  {
    return feature(add_func_, remove_func_, initial_func_, [](auto v) { return v;});
  }

  /**
     Constructs a new grouping for the given dimension, according to the specified reduce and key functions.
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc, typename KeyFunc>
  auto
  feature(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_,
      KeyFunc key) -> cross::feature<decltype(key(std::declval<value_type_t>())),
                                     decltype(initial_func_()), T, value_type_t, false, isIterable>;


  auto  feature_count() -> cross::feature<value_type_t, std::size_t, T, value_type_t, false, isIterable>{
    return feature_count([](auto v) { return v;});
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by count 
   */
  template<typename G>
  auto  feature(G key_) -> cross::feature<decltype(key_(std::declval<value_type_t>())),
                                          std::size_t, T, value_type_t, false, isIterable> {
    return feature_count(key_);
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by count 
   */
  template<typename K>
  auto feature_count(K key) -> cross::feature<decltype(key(std::declval<value_type_t>())),
                                              std::size_t, T, value_type_t, false, isIterable>;
  /**
     Constructs a new grouping for the given dimension to reduce elements by sum
   */
  template<typename ValueFunc>
  auto feature_sum(ValueFunc value)
      -> cross::feature<value_type_t, decltype(value(record_type_t())), T, value_type_t, false, isIterable> {
    return feature_sum(value, [](auto v) { return v;});
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by sum
   */
  template<typename ValueFunc, typename KeyFunc>
  auto feature_sum(ValueFunc value,
                   KeyFunc key) -> cross::feature<decltype(key(std::declval<value_type_t>())),
                                                  decltype(value(std::declval<record_type_t>())), T, value_type_t, false, isIterable>;
  /**
     A convenience function for grouping all records into a single group.  
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto
  feature_all(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) -> cross::feature<std::size_t, decltype(initial_func_()), T, value_type_t, true, isIterable>;

  /**
     A convenience function for grouping all records into a single group to reduce by count
   */
  auto feature_all_count() -> cross::feature<std::size_t, std::size_t, T, value_type_t, true, isIterable>;

  /**
     A convenience function for grouping all records into a single group to reduce by sum
   */
  template<typename G>
  auto feature_all_sum(G value) -> cross::feature<std::size_t, decltype(value(record_type_t())), T, value_type_t, true, isIterable>;

  // /**
  //    Equivalent to groupAllReduceCount()
  //  */
  // template <typename R>
  // feature<std::size_t, R, this_type_t, true>
  // groupAll() {
  //   return groupAllReduceCount();
  // }
};
} //namespace cross
#include "detail/impl/dimension.ipp"


#endif
