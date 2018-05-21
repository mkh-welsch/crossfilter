/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef GROUP_H_GUARD
#define GROUP_H_GUARD
#include <vector>
#include <functional>
#include <unordered_map>
#include <utility>
//#include <boost/signals2.hpp>
#include "detail/group_impl.hpp"

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll = false>
struct Group: private GroupImpl<Key, Reduce, Dimension, isGroupAll> {
  using GroupImplT = GroupImpl<Key, Reduce, Dimension, isGroupAll>;

 public:
  using group_type_t = typename GroupImplT::group_type_t;
  using reduce_type_t = typename GroupImplT::reduce_type_t;
  using record_type_t = typename GroupImplT::record_type_t;
  using value_type_t = typename GroupImplT::value_type_t;

  using this_type_t = Group<Key, Reduce, Dimension, isGroupAll>;

 private:
  friend  typename Dimension::base_type_t;
  
  template<typename KeyFunc, typename AddFunc, typename RemoveFunc, typename InitialFunc>
  Group(typename Dimension::base_type_t *dim, KeyFunc key_,
        AddFunc add_func_,
        RemoveFunc remove_func_,
        InitialFunc initial_func_)
      : GroupImplT(dim, key_, add_func_, remove_func_, initial_func_) {}


 public:

  void dispose() { GroupImpl<Key, Reduce, Dimension, isGroupAll>::dispose();}

  Group(Group<Key, Reduce, Dimension, isGroupAll> && g):
      GroupImpl<Key, Reduce, Dimension, isGroupAll>(std::move(g)) {}

  /**
     Returns a new array containing the top k groups,
     according to the group order of the associated reduce value. The returned array is in descending order by reduce value.
   */
  std::vector<group_type_t> top(std::size_t k) {
    return GroupImplT::top(k);
  }

  /**
     Returns a new array containing the top k groups,
     according to the order defined by orderFunc of the associated reduce value.
   */
  template<typename OrderFunc>
  std::vector<group_type_t> top(std::size_t k, OrderFunc orderFunc) {
    return GroupImplT::top(k, orderFunc);
  }

  /**
     Returns the array of all groups, in ascending natural order by key. 
   */
  std::vector<group_type_t> &all() {
    return GroupImplT::all();
  }

  /**
     Equivalent to all()[0].second.
   */
  reduce_type_t value() {
    return GroupImplT::value();
  }

  /**
     Specifies the order value for computing the top-K groups.
     The default order is the identity function,
     which assumes that the reduction values are naturally-ordered (such as simple counts or sums).
   */
  template<typename OrderFunc>
  this_type_t &   order(OrderFunc value) {
    GroupImplT::order(value);
    return *this;
  }

  /**
     A convenience method for using natural order for reduce values. Returns this grouping.
   */
  this_type_t & orderNatural() {
    GroupImplT::orderNatural();
    return *this;
  }
  /**
     Returns the number of distinct values in the group, independent of any filters; the cardinality.
   */
  std::size_t size() const { return GroupImplT::size(); }

  // std::vector<group_type_t> & all2() {
  //   return GroupImplT::all();
  // }
  
};

#endif
