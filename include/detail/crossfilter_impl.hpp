/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef CROSSFILTER_IMPL_H_GUARD
#define CROSSFILTER_IMPL_H_GUARD
#include <vector>
#include <functional>
#include <iostream>
#include <limits>
#include <tuple>
#include <unordered_map>
//#include <boost/signals2.hpp>
#include "detail/signal_base.hpp"
//#include "nod/nod.hpp"
#include "detail/bitarray.hpp"
#include "detail/filter_base.hpp"

namespace cross {
template <typename, typename, bool> struct dimension;
template <typename, typename, typename, typename, bool, bool> struct feature;
template <typename T, typename H> struct filter;
namespace impl {

template<typename T, typename Hash> struct filter_impl : public filter_dim_base<T>, filter_feature_base<T,T> {

 public:
  template <typename, typename, bool> friend struct dimension;
  template <typename , typename, typename, typename,  bool, bool> friend struct feature_impl;


  using record_type_t = T;
  using this_type_t = filter_impl<T,Hash>;
  using value_type_t = T;

  using index_vec_t = std::vector<std::size_t>;
  using index_set_t = std::vector<std::size_t>;
  using data_iterator = typename std::vector<T>::const_iterator;



  // template<typename F> using signal_type_t = typename boost::signals2::signal<F>;
  // using connection_type_t = boost::signals2::connection;
  template<typename F> using signal_type_t = MovableSignal<typename signals::signal<F>, signals::connection>;
  using connection_type_t = signals::connection;

  signal_type_t<void(std::size_t, int,
                     const index_set_t &, const index_set_t &, bool)>  filter_signal;

  signal_type_t<void(const std::vector<int64_t> &)> remove_signal;

  signal_type_t<void(const std::vector<value_type_t> &, const std::vector<std::size_t>&, std::size_t, std::size_t)> add_group_signal;
  signal_type_t<void()> dispose_signal;

  signal_type_t<void(std::size_t)>                  post_add_signal;
  signal_type_t<void(data_iterator, data_iterator)> add_signal;
  signal_type_t<void(const std::string&)>           on_change_signal;

  BitArray filters;
  std::vector<record_type_t> data;
  std::size_t data_size = 0;

  std::unordered_map<uint64_t,uint64_t> hash_table;
  Hash hash;

  ~filter_impl() {}
  filter_impl() {}
  
  template<typename Iterator>
  filter_impl(Iterator begin, Iterator end):filter_dim_base<T>(), filter_feature_base<T,T>() {
    data.assign(begin,end);
    data_size = data.size();
    filters.resize(data_size);
  }
  std::vector<T> bottom(std::size_t low, std::size_t high, int64_t k,
                        int64_t offset, const index_vec_t &indexes, const index_vec_t &empty);

  std::vector<T> top(std::size_t low, std::size_t high, int64_t k,
                     int64_t offset, const index_vec_t &indexes, const index_vec_t &empty);

  connection_type_t
  connect_filter_slot(std::function<void(std::size_t, int,
                                         const index_set_t &, const index_set_t &, bool)> slot) override;

  connection_type_t
  connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot) override;

  connection_type_t
  connect_add_slot(std::function<void(const std::vector<value_type_t> &, const std::vector<std::size_t> &,
                                      std::size_t, std::size_t)>) override;
  connection_type_t
  connect_dispose_slot(std::function<void()> slot) override;

  connection_type_t
  connect_post_add_slot(std::function<void(std::size_t)> slot) override;


  connection_type_t
  connect_add_slot(std::function<void(data_iterator, data_iterator)> slot) override;

  connection_type_t
  on_change(std::function<void(const std::string &)> callback) override;

  void
  trigger_on_change(const std::string & event_name) override {
    on_change_signal(event_name);
  }

  void
  emit_filter_signal(std::size_t filter_offset, int filter_bit_num,
                     const index_set_t &added, const index_set_t &removed) override {
    filter_signal(filter_offset, filter_bit_num, added, removed, false);
  }



  std::size_t size() const  override {
    return data.size();
  }

  std::vector<T> all() const override {
    return data;
  } 

  const T & get_raw(std::size_t index) const override {
    return data[index];
  }


  template<typename C>
  filter_impl<T,Hash> & add(const C & new_data, bool allow_duplicates);

  filter_impl<T,Hash> & add(const T & new_data,  bool allow_duplicates);


  std::vector<uint8_t> & mask_for_dimension(std::vector<uint8_t> & mask) const;

  template<typename D>
  std::vector<uint8_t> & mask_for_dimension(std::vector<uint8_t> & mask,
                                            const D & dim) const;

  template<typename D, typename ...Ts>
  std::vector<uint8_t> & mask_for_dimension(std::vector<uint8_t> & mask,
                                            const D & dim,
                                            Ts&... dimensions) const;

  template<typename ...Ts>
  std::vector<T> all_filtered(Ts&... dimension) const;

  template<typename ...Ts>
  bool is_element_filtered(std::size_t index, Ts&... dimension) const;


  template<typename G>
  auto dimension(G getter) -> dimension<decltype(getter(std::declval<record_type_t>())), T, false>;

  template<typename V, typename G>
  auto iterable_dimension(G getter) -> cross::dimension<V, T, true>;


  void flip_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                              int dimension_bit_index);

  void
  set_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index) override;
  void
  reset_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                          int dimension_bit_index) override;

  bool
  get_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index) override;

  std::tuple<data_iterator, data_iterator>
  get_data_iterators_for_indexes(std::size_t low, std::size_t high);

  void remove_data(std::function<bool(int)> should_remove);

  // removes all records matching the predicate (ignoring filters).
  void remove(std::function<bool(const T&, int)> predicate);

  // Removes all records that match the current filters
  void remove();

  bool
  zero_except(std::size_t index) override {
    return filters.zero_except(index,
                               this->dimension_offset,
                               this->dimension_bit_index);
  }

  bool
  zero_except(std::size_t index, std::size_t offset, int bit_index) override {
    return filters.zero_except(index, offset, bit_index);
  }

  bool
  only_except(std::size_t index, std::size_t offset, int bit_index) override {
    return filters.only(index, offset, bit_index);
  }

  bool
  only_except(std::size_t index, std::size_t offset1, int bit_index1,
              std::size_t offset2, int bit_index2) override {
    return filters.only_except(index, offset1, bit_index1, offset2, bit_index2);
  }


  bool zero(std::size_t index) override {
    return filters.zero(index);
  }

  bool only(std::size_t index, std::size_t offset, std::size_t bit_index) {
    return filters.only(index, offset, bit_index);
  }

  record_type_t & get_raw(std::size_t index) { return data[index]; }

  std::size_t translate_index(std::size_t index) const {
    return index;
  }


  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto feature(
      AddFunc  add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) -> cross::feature<std::size_t,
                                                   decltype(initial_func_()), T, value_type_t, true, false>;


  

  // static constexpr  bool get_is_iterable() {
  //   return false;
  // }

  // void dump() {
  //   std::cout << "connectionNum, " << add_signal.num_slots() << std::endl;
  // }
};
} //namespace impl
} //namespace cross
#include "impl/crossfilter_impl.ipp"

#endif
