/*This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Dmitry Vinokurov */

#ifndef FILTER_BASE_H_GUARD
#define FILTER_BASE_H_GUARD
#include "detail/signal_base.hpp"
#include "detail/bitarray.hpp"

#include <vector>
#include <utility>
namespace cross {
namespace impl {
template<typename T> struct filter_base {
  template<typename F> using signal_type_t = MovableSignal<typename signals::signal<F>, signals::connection>;
  using connection_type_t = signals::connection;
  using index_vec_t = std::vector<std::size_t>;
  using record_type_t = T;
  using value_type_t = T;
  using index_set_t = std::vector<std::size_t>;
  using data_iterator = typename std::vector<T>::const_iterator;
  bool is_moved = false;

  // signal_type_t<void(std::size_t, int,
  //                    const index_set_t &, const index_set_t &, bool)>  filter_signal;

  // signal_type_t<void(const std::vector<int64_t> &)> remove_signal;

  virtual ~filter_base() {}
  filter_base() = default;
  
  // filter_base<T>(filter_base<T>&& v):filter_signal(std::move(v.filter_signal)),
  //                                    remove_signal(std::move(v.remove_signal)) {}

  // filter_base<T> & operator=(filter_base<T> && v) {
  //   filter_signal = std::move(v.filter_signal);
  //   remove_signal = std::move(v.remove_signal);
  //   v.is_moved = true;
  //   return *this;
  // }
  virtual
  connection_type_t
  connect_filter_slot(std::function<void(std::size_t, int,
                                         const index_set_t &, const index_set_t &, bool)> slot) = 0;
  virtual
  connection_type_t
  connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot) =0;

  virtual bool
  zero_except(std::size_t index) = 0;

  virtual bool
  zero_except(std::size_t index, std::size_t offset, int bit_index) = 0;

  virtual bool
  only_except(std::size_t index, std::size_t offset, int bit_index) = 0;


  virtual const T &
  get_raw(std::size_t index) const = 0;
};

template<typename T> struct filter_dim_base : public virtual filter_base<T> {
    using filter_base<T>::only_except;

  using connection_type_t = typename filter_base<T>::connection_type_t;

  template <typename F>
  using signal_type_t = typename filter_base<T>::template signal_type_t<F>;

  using data_iterator = typename filter_base<T>::data_iterator;

  using index_set_t = typename filter_base<T>::index_set_t;

  // signal_type_t<void(std::size_t)>                  post_add_signal;
  // signal_type_t<void(data_iterator, data_iterator)> add_signal;
  // signal_type_t<void(const std::string&)>           on_change_signal;

  virtual ~filter_dim_base() {}
  filter_dim_base<T>() = default;

  // filter_dim_base<T>(filter_dim_base<T> && v):filter_base<T>(v),post_add_signal(std::move(v.post_add_signal)),
  //                                             add_signal(std::move(v.add_signal)),
  //                                             on_change_signal(std::move(v.on_change_signal)) {}

  // filter_dim_base<T> & operator = (filter_dim_base<T> && v) {
  //   if(!v.is_moved) {
  //     static_cast<filter_base<T>&>(*this) = static_cast<filter_base<T>&>(v);
  //   }
  //   post_add_signal = std::move(v.post_add_signal);
  //   add_signal = std::move(v.add_signal);
  //   on_change_signal = std::move(v.on_change_signal);
  //   return *this;
  // }
  virtual std::size_t size() const  = 0;

  virtual std::vector<T> all() const = 0;

  virtual bool
  zero(std::size_t index) = 0;

  virtual
  connection_type_t
  connect_post_add_slot(std::function<void(std::size_t)> slot) = 0;

  virtual
  connection_type_t
  connect_add_slot(std::function<void(data_iterator, data_iterator)> slot) = 0;

  virtual
  void
  emit_filter_signal(std::size_t filter_offset, int filter_bit_num,
                     const index_set_t &added, const index_set_t &removed) = 0;
  //   filter_base<T>::filter_signal(filter_offset, filter_bit_num, added, removed, false);
  // }

  virtual
  connection_type_t
  on_change(std::function<void(const std::string &)> callback) =0;

  virtual 
  void
  trigger_on_change(const std::string & event_name) = 0;
  //   on_change_signal(event_name);
  // }

  virtual void
  set_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index) = 0;
  virtual void
  reset_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                          int dimension_bit_index) = 0;

  virtual bool
  get_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index) = 0;

  virtual bool
  only_except(std::size_t index, std::size_t offset, int bit_index,
              std::size_t offset2, int bit_index2) = 0;

};

template<typename T, typename V> struct filter_feature_base : public virtual filter_base<T> {
  using connection_type_t = typename filter_base<T>::connection_type_t;

  template <typename F>
  using signal_type_t = typename filter_base<T>::template signal_type_t<F>;
  using value_type_t = V;

  std::size_t dimension_offset = std::numeric_limits<std::size_t>::max();
  int dimension_bit_index = -1;

  // signal_type_t<void(const std::vector<value_type_t> &, const std::vector<std::size_t>&, std::size_t, std::size_t)> add_group_signal;
  // signal_type_t<void()> dispose_signal;

  filter_feature_base() = default;
  filter_feature_base(std::size_t offset, int bit_index):dimension_offset(offset), dimension_bit_index(bit_index) {}

  filter_feature_base(filter_feature_base && v):filter_base<T>(std::move(v)),
                                                dimension_offset(v.dimension_offset),
                                                dimension_bit_index(v.dimension_bit_index) {}
  virtual ~filter_feature_base() {}

  // filter_feature_base(filter_feature_base && v):filter_base<T>(std::move(v)),
  //                                               add_group_signal(std::move(v.add_group_signal)),
  //                                               dispose_signal(std::move(v.dispose_signal)) {}

  // filter_feature_base<T,V> & operator = (filter_feature_base<T,V> && v) {
  //   if(!v.is_moved) {
  //     static_cast<filter_base<T>&>(*this) = static_cast<filter_base<T>&>(v);
  //   }
  //   add_group_signal = std::move(add_group_signal);
  //   dispose_signal = std::move(dispose_signal);
  //   return *this;
  // }
  virtual
  connection_type_t
  connect_add_slot(std::function<void(const std::vector<value_type_t> &, const std::vector<std::size_t> &,
                                      std::size_t, std::size_t)>) = 0;
  virtual
  connection_type_t
  connect_dispose_slot(std::function<void()> slot) = 0;
  //   return  dispose_signal.connect(slot);
  // }
  //  virtual const T & get_raw(std::size_t index) const = 0;
};
//#include "detail/impl/filter_base.ipp"
} // namespace impl
} // namespace cross
#endif
