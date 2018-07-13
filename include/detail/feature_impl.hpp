/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef FEATURE_IMPL_H_GUARD
#define FEATURE_IMPL_H_GUARD
#include <vector>
#include <functional>
#include <unordered_map>
#include <utility>
//#include <boost/signals2.hpp>
#include <type_traits>
#include "detail/heap.hpp"
#include "detail/impl/group_index.hpp"

namespace cross {

namespace impl {
// template <typename Key,  typename Reduce, typename Dimension,
//           bool isGroupAll>
// struct GroupImpl;


template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll = false>
struct feature_impl {
  static constexpr int REMOVED_INDEX = -1;
  static constexpr bool isFlatIndex = !Dimension::get_is_iterable();

  typename Dimension::base_type_t *dimension;

  using reduce_type_t = Reduce;
  using key_type_t = Key;
  using value_type_t = typename Dimension::value_type_t;
  using record_type_t = typename Dimension::record_type_t;
  using group_type_t = std::pair<key_type_t, reduce_type_t>;
  using value_vec_t = std::vector<value_type_t>;
  using group_vec_t = std::vector<group_type_t>;

  template<typename F> using signal_type_t = typename Dimension::template signal_type_t<F>;
  using connection_type_t = typename Dimension::connection_type_t;

  std::vector<group_type_t> groups;

  // using group_index_t =  typename std::conditional<Dimension::getIsIterable(),
  //                                         std::vector<std::size_t>,
  //                                         std::size_t>::type;
  // std::vector<group_index_t> groupIndex;
  GroupIndex<isFlatIndex> group_index;

  bool reset_needed = true;

  std::function<Key(const value_type_t &)> key;
  std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> add_func;
  std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> remove_func;
  std::function<reduce_type_t()> initial_func;

  using order_func_t = std::function<reduce_type_t(const reduce_type_t&)>;

  std::function<group_vec_t(const group_vec_t &, std::size_t, std::size_t, std::size_t)>  select_func;
  std::function<group_vec_t &(group_vec_t &, std::size_t, std::size_t)>      sort_func;

  std::size_t data_size = 0;

  connection_type_t connection_filter;
  connection_type_t connection_add;
  connection_type_t connection_remove;
  connection_type_t connection_dimension;

 public:
  feature_impl(typename Dimension::base_type_t *dim, std::function<Key(const value_type_t &)> key_,
            std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> add_func_,
            std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> remove_func_,
            std::function<reduce_type_t()> initial_func_)
      : dimension(dim), key(key_), add_func(add_func_),
        remove_func(remove_func_), initial_func(initial_func_) {
    init_slots();
    order([](auto r) { return r;});
  }

  explicit feature_impl(typename Dimension::base_type_t *dim)
      : dimension(dim) {
    key = [](value_type_t && r) { return r;};
    order([](auto r) { return r;});
    init_slots();
  }

  feature_impl(feature_impl && g)
      :dimension(std::move(g.dimension)), groups(std::move(g.groups)),
       group_index(std::move(g.group_index)),
       reset_needed(g.reset_needed), key(std::move(g.key)),
       add_func(std::move(g.add_func)), remove_func(std::move(g.remove_func)),
       initial_func(std::move(g.initial_func)),
       select_func(std::move(g.select_func)), sort_func(std::move(g.sort_func)),
       data_size(g.data_size) {
    g.dispose();
    init_slots();
  }

  ~feature_impl() {
    dispose();
  }

  void init_slots();

  void dispose() {
    connection_remove.disconnect();
    connection_add.disconnect();
    connection_filter.disconnect();
    connection_dimension.disconnect();
  }

  void remove(const std::vector<int64_t> &removed);

  void update_one(std::size_t filter_offset, int filter_bit_num,
                 const std::vector<std::size_t> &added,
                 const std::vector<std::size_t> &removed, bool not_filter);

  void update(std::size_t filter_offset, int filter_bit_index,
              const std::vector<std::size_t> &added,
              const std::vector<std::size_t> &removed, bool not_filter);

  // template <bool Enable = true>
  // typename std::enable_if<isGroupAll && Enable>::type
  // add(const value_vec_t &newData,
  //     const std::vector<std::size_t> &, std::size_t oldDataSize, std::size_t newDataSize);
  

  //  template <bool Enable = true>
  //  typename std::enable_if<!isGroupAll && Enable>::type
  void add(const value_vec_t &new_data,
      const std::vector<std::size_t> &new_indexes, std::size_t old_data_size, std::size_t new_data_size);

  std::vector<group_type_t> top(std::size_t k);

  void reset_one();

  void reset();

  std::vector<group_type_t> &all();

  reduce_type_t value();

  feature_impl<Key, Reduce, Dimension, isGroupAll> &reduce(
      std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> add_,
      std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> remove_,
      Reduce initial_);

  feature_impl<Key, Reduce, Dimension, isGroupAll> &reduce_count();

  feature_impl<Key, Reduce, Dimension, isGroupAll> &
  reduce_sum(std::function<reduce_type_t(const record_type_t &)> value);

  template<typename OrderFunc>
  feature_impl<Key, Reduce, Dimension, isGroupAll> &
  order(OrderFunc value);

  feature_impl<Key, Reduce, Dimension, isGroupAll> &order_natural();

  std::size_t size() const { return groups.size(); }
  std::vector<group_type_t> & all2() {
    return groups;
  }
};
#include "detail/impl/feature_impl.ipp"
} //namespace impl
} //namespace cross




#endif
