/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef FEATURE_H_GUARD
#define FEATURE_H_GUARD
#include <vector>
#include <functional>
#include <unordered_map>
#include <utility>
//#include <boost/signals2.hpp>
#include "detail/feature_impl.hpp"
#include "detail/filter_base.hpp"
#include "detail/crossfilter_impl.hpp"
#include "detail/dimension_impl.hpp"

namespace cross {
template <typename Key, typename Reduce, typename Record, typename Dimension,
          bool isGroupAll = false, bool is_iterable = false>
struct feature: private impl::feature_impl<Key, Reduce, Record, Dimension, isGroupAll, is_iterable> {
  using feature_impl_t = impl::feature_impl<Key, Reduce, Record, Dimension, isGroupAll, is_iterable>;

 public:
  using group_type_t = typename feature_impl_t::group_type_t;
  using reduce_type_t = typename feature_impl_t::reduce_type_t;
  using record_type_t = typename feature_impl_t::record_type_t;
  using value_type_t = typename feature_impl_t::value_type_t;

  using this_type_t = feature<Key, Reduce, Record, Dimension, isGroupAll, is_iterable>;

 private:

  template <typename T,typename H> friend struct impl::filter_impl;
  template <typename V,typename T,bool> friend struct impl::dimension_impl;
  template<typename KeyFunc, typename AddFunc, typename RemoveFunc, typename InitialFunc, typename Base>
  feature(Base *dim, KeyFunc key_,
          AddFunc add_func_,
          RemoveFunc remove_func_,
          InitialFunc initial_func_)
      : feature_impl_t(dim, key_, add_func_, remove_func_, initial_func_) {}


 public:

  void dispose() { impl::feature_impl<Key, Reduce, Record, Dimension,isGroupAll, is_iterable>::dispose();}

  feature(feature<Key, Reduce, Record, Dimension, isGroupAll, is_iterable> && g):
      feature_impl_t(std::move(g)) {}
      //      feature_impl_t<Key, Reduce, Record, isGroupAll>(std::move(g)) {}

  /**
     Returns a new array containing the top k groups,
     according to the group order of the associated reduce value. The returned array is in descending order by reduce value.
   */
  std::vector<group_type_t> top(std::size_t k) {
    return feature_impl_t::top(k);
  }

  /**
     Returns a new array containing the top k groups,
     according to the order defined by orderFunc of the associated reduce value.
   */
  template<typename OrderFunc>
  std::vector<group_type_t> top(std::size_t k, OrderFunc orderFunc) {
    return feature_impl_t::top(k, orderFunc);
  }

  /**
     Returns the array of all groups, in ascending natural order by key. 
   */
  std::vector<group_type_t> &all() {
    return feature_impl_t::all();
  }

  /**
     Equivalent to all()[0].second.
   */
  reduce_type_t value() {
    return feature_impl_t::value();
  }

  /**
     Specifies the order value for computing the top-K groups.
     The default order is the identity function,
     which assumes that the reduction values are naturally-ordered (such as simple counts or sums).
   */
  template<typename OrderFunc>
  this_type_t &   order(OrderFunc value) {
    feature_impl_t::order(value);
    return *this;
  }

  /**
     A convenience method for using natural order for reduce values. Returns this grouping.
   */
  this_type_t & order_natural() {
    feature_impl_t::order_natural();
    return *this;
  }
  /**
     Returns the number of distinct values in the group, independent of any filters; the cardinality.
   */
  std::size_t size() const { return feature_impl_t::size(); }

  // std::vector<group_type_t> & all2() {
  //   return feature_impl_t::all();
  // }
  
};
} //namespace cross
#endif
